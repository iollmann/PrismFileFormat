//
//  FileNode.cpp
//  ParsePrism
//
//  Created by Ian Ollmann on 11/7/22.
//

#include "FileNode.h"
#include <math.h>

template <typename T>  T min( T a, T b){ return a < b ? a : b;}
template <typename T>  T max( T a, T b){ return a > b ? a : b;}

static inline FileNode * __nullable ParseObject( const char * & where, size_t & size);

static inline FileNodeSet * __nullable ParseSet( const char * & where, size_t & size)
{
    FileNodeSet * set = new FileNodeSet();
    if( NULL == set)
        return set;

    if( 0 == size  || '}' == where[0])
        return set;

    FileNode * object;
    while((object = ParseObject(where, size)))
    {
        set->AppendNode(object);
        if( 0 == size || where[0] != ',')
            break;

        where++;
        size--;
    }
    
    return set;
}

static inline FileNodeArray * __nullable ParseArray( const char * & where, size_t & size)
{
    if( 0 == size  )
        return NULL;
    
    if( ']' == where[0])
        return new FileNodeArray(NULL);
    
    FileNodeSet * set = ParseSet(where, size);
    if( NULL == set )
        return NULL;
    
    FileNodeArray * result = new FileNodeArray(set);
    delete set;
    return result;
}

static inline FileNodeString * __nullable ParseString( const char * & where, size_t & size)
{
    if( size < 2 || where[0] != '"')
        return NULL;
    
    unsigned long len = 0;
    const char * p = &where[1];
    char last = '\0';
    for( len = 0; len < size - 2; len++)
    {
        if( p[len] == '"' && last != '\\' )
            break;
        last = p[len];
    }
    
    if( '"'  != p[len])
        return NULL;
    
    FileNodeString * result = FileNodeString::Create(&where[1], len);
    if( NULL == result)
        return NULL;
    

    where += len + 2;
    size -= len + 2;

    return result;
}

static inline bool IsSame( double a, double b)
{
    if( isnan(a) && isnan(b))
        return true;
    
    return a == b;
}


static inline FileNodeKeyValuePair * __nullable ParseKeyValuePair( const char * & where, size_t & size,  const FileNodeString * key)
{
    if( NULL == key )
        return NULL;
    
    FileNode * node =  NULL;
    if( size >= 2 )
        node = ParseObject(where, size);
    
    return new FileNodeKeyValuePair( key, node);
}


static inline FileNode * __nullable ParseObject( const char * & where, size_t & size)
{
    if( 0 == size )
        return NULL;
    
    FileNode * __nullable result = NULL;
    char closeChar = '\0';
    char next = where[0];
    switch(next)
    {
        case '{':   // set
            where++; size--;
            result = ParseSet( where, size);
            closeChar = '}';
            break;
        case '[':   // array
            where++; size--;
            result = ParseArray( where, size);
            closeChar = ']';
            break;
        case '"':
            result = ParseString(where, size);
            if( NULL == result || size == 0 || where[0] != ':')
                break;
            
            // key value pair
            {
                FileNodeString * key = (FileNodeString *) result;
                
                // skip ':'
                where++;
                size--;
                result = ParseKeyValuePair( where, size, key);
                if( NULL == result)
                    delete key;
            }
            break;
        default:
            // A constant of some kind
            if( next == '.' || next == '-' || (next >= '0' && next <= '9') )
            {
                char * end = const_cast<char*>(where);
                double value = strtod(where, &end);     // all int32_ts are exactly representable as doubles
                if( where != end)
                {
                    if( IsSame(value, trunc(value)) && value >= double(INT32_MIN) && value <= double(INT32_MAX))
                        result = new FileNodeInt( int32_t(value));
                    else
                        result = new FileNodeDouble(value);
                }
                size_t len = min(size_t(end - where), size);
                size -= len;
                where += len;
            }
            else if( 0 == strncasecmp( where, "false", min( size, 5UL)))
            {
                result = new FileNodeBoolean(false);
                size_t len = min(size, 5UL);
                where += len;
                size -= len;
            }
            else if( 0 == strncasecmp( where, "true", min( size, 4UL)))
            {
                result = new FileNodeBoolean(true);
                size_t len = min(size, 4UL);
                where += len;
                size -= len;
            }
            else
                abort();
            break;
    }
    
    if( '\0' != closeChar)
    {
        if( size == 0 || *where != closeChar)
        {
            delete result;
            return NULL;
        }

        where++;
        size--;
    }
    
    return result;
}


FileNode * __nullable FileNode::ParseFile( const char * __nonnull where, size_t size )
{
    return ParseObject(where, size);
}


