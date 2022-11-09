//
//  FileNode.h
//  ParsePrism
//
//  Created by Ian Ollmann on 11/7/22.
//
//
// MIT license:
//
// Copyright 2022, Ian Ollmann
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
// IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// This software does not contain code by Samuel Harmon or PrismScroll, nor is it endorsed or maintained by him in any way. This work
// contains no copyrighted material belonging to Wizards of the Coast(TM) or Hasbro(TM).
//


#ifndef FileNode_h
#define FileNode_h

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/*! @abstract Basic RTTI typing codes for different node types for recognition later */
typedef enum NodeType : int8_t
{
    NodeTypeInvalid = -1,
    NodeTypeKeyValuePair = 0,
    NodeTypeSet,
    NodeTypeArray,
    NodeTypeBoolean,
    NodeTypeInteger,
    NodeTypeDouble,
    NodeTypeString
}NodeType;

/*! @abstract Base class for tree of data objects from deserialized file */
class FileNode
{
private:
    FileNode * __nullable next;
    
public:
    FileNode(){ next = NULL; }
    virtual ~FileNode(){ delete next; }
    
    // Each node can be used in a single linked list
    /*! @abstract Get the next node in the list*/
    FileNode * __nullable GetNext() const { return next;}
    
    /*! @abstract Set the next node in the list. Delete whatever was there before. */
    void SetNext( FileNode * __nullable newNext )
    {
        delete next;
        next = newNext;
    }
    
    /*! @abstract Some basic RTTI */
    virtual NodeType    GetType() const = 0;
    
    /*! @abstract  Print human readable version */
    virtual void        Print(int indentDepth) const = 0;
    
    /*! @abstract Write out tree to disk */
    virtual void        write( FILE * __nonnull ) const = 0;
    
    /*! @abstract  Read in a file from disk and create a tree of nodes */
    static FileNode * __nullable ParseFile( const char * __nonnull where, size_t size );
};

static inline void Indent( int depth)
{
    for( int i = 0; i < depth; i++)
        putc( '\t', stdout);
}

/*! @abstract Implements a node which is a set of other nodes */
class FileNodeSet : public FileNode
{
private:
    FileNode * __nullable list;
    FileNode * __nullable end;

protected:
    friend class FileNodeArray;
    inline FileNode * __nullable StealList(void)
    {
        FileNode * result = list;
        list = end = NULL;
        return result;
    }
    
public:
    FileNodeSet() : FileNode(){ list = end = NULL; }
    virtual ~FileNodeSet()
    {
        delete list;
        list = end = NULL;
    }
    
    virtual NodeType    GetType() const { return NodeTypeSet; };

    void AppendNode( FileNode * __nullable node )
    {
        if( NULL == node)
            return;
        
        if( NULL == list)
        {
            assert(end == NULL);
            list = end = node;
            return;
        }
        
        assert(end);
        end->SetNext(node);
        end = node;
    }
    
    inline const FileNode * __nullable GetSet() const { return list; }
    
    virtual void Print(int indentDepth) const
    {
        if( NULL == list)
        {
            printf( "{}");
            return;
        }
        
        printf("{\n");
        for( FileNode * node = list; node; node = node->GetNext() )
        {
            if( node != list )
                printf( ",\n");
            
            Indent(indentDepth + 1);
            node->Print( indentDepth + 1);
        }
        printf( "\n");
        Indent(indentDepth);
        printf( "}");
    }
    
    virtual void write( FILE * __nonnull file ) const
    {
        fprintf( file, "{" );
        for( FileNode * node = list; node; node = node->GetNext() )
        {
            if( node != list )
                fprintf(file, ",");

            node->write(file);
        }
        fprintf( file, "}");
    }

};

/*! @abstract Implements a node which is a array of other nodes */
class FileNodeArray : public FileNode
{
    const FileNode * __nullable * __nonnull nodes;
    unsigned long                           count;
    
public:
    FileNodeArray() : FileNode(), nodes(NULL), count(0){}
    FileNodeArray( FileNodeSet * __nullable the_set) : FileNodeArray()
    {
        if( NULL == the_set)
            return;

        // count the elements
        const FileNode * list = the_set->GetSet();
        for( const FileNode * __nullable p = list; p; p = p->GetNext())
            count++;
        
        nodes = (const FileNode**) calloc( count, sizeof(FileNode*));
        if( NULL == nodes)
            return;
        
        unsigned long index = 0;
        list = the_set->StealList();
        for( const FileNode * __nullable p = list; p; p = p->GetNext())
            nodes[index++] = p;
    }
    virtual ~FileNodeArray()
    {
        if( count && nodes)
            delete nodes[0];
        free(nodes);
    }
    
    virtual NodeType    GetType() const { return NodeTypeArray; };
    
    inline const FileNode * __nullable operator[] (int index) const { assert(index < count);  return nodes[index]; }
    inline unsigned long GetCount() const { return count;}
    
    virtual void Print(int indentDepth) const
    {
        if( 0 == count)
        {
            printf( "[]");
            return;
        }
        
        printf("[\n");
        for( unsigned long i = 0; i < count; i++ )
        {
            if( i != 0)
                printf( ",\n");
 
            Indent(indentDepth+1);
            const FileNode * node = nodes[i];
            if( node)
                node->Print( indentDepth + 1);
            else
                printf( "NULL");
        }
        printf( "\n");
        Indent(indentDepth);
        printf( "]");
    }

    virtual void write( FILE * __nonnull file ) const
    {
        fprintf( file, "[" );
        for( unsigned long i = 0; i < count; i++ )
        {
            if( i != 0 )
                fprintf(file, ",");

            const FileNode * node = nodes[i];
            if(node)
                node->write(file);
        }
        fprintf( file, "]");
    }

};

/*! @abstract Implements a node which holds a Boolean true/false value */
class FileNodeBoolean : public FileNode
{
private:
    bool    value;
    
public:
    FileNodeBoolean(bool v) : FileNode(), value(v) {}
    virtual ~FileNodeBoolean(){}
    
    virtual NodeType    GetType() const { return NodeTypeBoolean; };
    
    inline bool GetValue() const { return value; }
    
    virtual void Print(int indentDepth) const
    {
        if( value )
            printf( "true");
        else
            printf( "false");
    }
    virtual void write( FILE * __nonnull file ) const
    {
        fprintf( file, "%s", value ? "true" : "false");
    }

};

/*! @abstract Implements a node which holds a integer
 *  @bug I couldn't tell from the file format what precision this integer was supposed to have */
class FileNodeInt : public FileNode
{
private:
    int32_t    value;
    
public:
    FileNodeInt(int32_t v) : FileNode(), value(v) {}
    virtual ~FileNodeInt(){}
    
    virtual NodeType    GetType() const { return NodeTypeInteger; };
    
    inline int32_t GetValue() const { return value; }
    virtual void Print(int indentDepth) const { printf( "%d", value); }
    virtual void write( FILE * __nonnull file ) const
    {
        fprintf( file, "%d", value );
    }
};

/*! @abstract Implements a node which holds a double */
class FileNodeDouble : public FileNode
{
private:
    double    value;
    
public:
    FileNodeDouble( double v) : FileNode(), value(v) {}
    virtual ~FileNodeDouble(){}
    
    virtual NodeType    GetType() const { return NodeTypeDouble; };
    
    inline double GetValue() const { return value; }
    virtual void Print(int indentDepth) const {  printf( "%f", value); }
    virtual void write( FILE * __nonnull file ) const
    {
        // workaround for bug in MacOS wherein 0.500000 is not trimmed to 0.5
        // for %g format, which I would therwise like to use here
        char string[30];
        int len = snprintf( string, 30, "%g", value);

        bool hasDecimal = false;
        for( unsigned long i = 0; i < len; i++)
            if( string[i] == '.')
            {
                hasDecimal = true;
                break;
            }
        
        if( hasDecimal && len > 0)
            for( unsigned long last = len - 1; string[last] != '.'; last--)
            {
                if( '0' != string[last] )
                    break;
                
                string[last] = '\0';
            }
        
        fprintf( file, "%s", string );
    }
};


/*! @abstract Implements a node which holds a string */
class FileNodeString : public FileNode
{
private:
    char * __nonnull string;

protected:
    FileNodeString(){ string = (char *)(this + 1);}
    FileNodeString(const char * __nonnull s, size_t count) : FileNodeString(){ strncpy( string, s, count + 1); string[count] = '\0';}
    void * __nonnull operator new(size_t size, void * __nonnull where){ return where; }
    
public:
    static inline FileNodeString * __nullable Create( const char * __nullable s, size_t size)
    {
        if( NULL == s )
            return NULL;
        
        void * allocation = calloc( 1, sizeof( FileNodeString) + size + 1);
        if( NULL == allocation)
            return NULL;
        
        return new(allocation) FileNodeString(s, size);
    }
    static inline FileNodeString * __nullable Create( const char * __nullable s)
    {
        if( NULL == s)
            return NULL;
        
        return FileNodeString::Create(s, strlen(s));
    }
    virtual ~FileNodeString(){ string[0]='\0';}

    
    virtual NodeType    GetType() const { return NodeTypeString; };
    const char * __nonnull GetString() const { return string;}
    virtual void Print(int indentDepth) const{  printf( "\"%s\"", string);}
    virtual void write( FILE * __nonnull file ) const
    {
        size_t len = strlen(string);
        fputc( '"', file);
        fwrite( string, len, 1, file);
        fputc( '"', file);
    }
};

/*! @abstract Implements a node which holds a key-value pair
 *  @discussion A key-value pair is a value with a name (key) attached to it, so it can be found by name  */
class FileNodeKeyValuePair : public FileNode
{
private:
    const FileNodeString * __nonnull key;
    FileNode * __nullable value;
    
public:
    FileNodeKeyValuePair( const FileNodeString * __nonnull the_key, FileNode * __nullable the_value)
    {
        key = the_key;
        value = the_value;
    }
    virtual ~FileNodeKeyValuePair(){ delete key; delete value; }
    
    virtual NodeType    GetType() const { return NodeTypeKeyValuePair; };

    inline const char * __nonnull GetKey() const { return key->GetString();}
    inline const FileNode * __nullable GetValue() const { return value; }
    virtual void Print(int indentDepth) const
    {
        printf( "\"%s\" = ", key->GetString());
        if( value )
            value->Print(indentDepth);
        else
            printf( "NULL");
    }
    virtual void write( FILE * __nonnull file ) const
    {
        key->write(file);
        fputc(':', file);
        if( NULL == value)
            fprintf( file, "{}");
        else
            value->write(file);
    }
};

#endif /* FileNode_h */
