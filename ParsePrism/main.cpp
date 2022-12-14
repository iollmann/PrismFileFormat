//
//  main.cpp
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

#include <iostream>
#include "FileNode.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

template <typename T> T min( T a, T b){ return a < b ? a : b;}
template <typename T> T max( T a, T b){ return a > b ? a : b;}

const char * __nullable ReadFile( const char * __nonnull fileName, size_t * __nonnull sizeOut)
{
    struct stat buf;
    int err = stat( fileName, &buf);
    if( err )
        return NULL;
    sizeOut[0] = buf.st_size;
    
    int fd = open(fileName, O_RDONLY);
    if( fd < 0)
    {
        printf( "Open of \"%s\" failed\n", fileName);
        return NULL;
    }
    
    void * result = mmap( NULL, sizeOut[0], PROT_READ, MAP_FILE | MAP_SHARED, fd, 0 );
    if( MAP_FAILED == result)
    {
        sizeOut[0] = 0;
        return NULL;
    }
    
    return (const char *) result;
}

int main(int argc, const char * argv[])
{
    if( argc < 2)
        return -1;
    
    size_t fileSize = 0;
    const char * fileData = ReadFile( argv[1], &fileSize);
    if( NULL == fileData)
        return -1;
    
    FileNode * node = FileNode::ParseFile( fileData, fileSize);
    
    if(node)
        node->Print(0);
    else
        printf( "NULL result\n");
    
//    char path2[PATH_MAX];
//    snprintf( path2, sizeof(path2), "%s_out", argv[1]);
//    FILE * outFile = fopen( path2, "rw");
//    if( outFile)
//    {
//        fpos_t position = 0;
//        fsetpos( outFile, &position);
//        node->write(outFile);
//        fclose(outFile);
//        
//        struct stat buf;
//        stat( path2, &buf);
//        size_t size = buf.st_size;
//        int fd = open( path2, O_RDONLY);
//        char * data2 = (char*) mmap( NULL, size, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0 );
//        if( MAP_FAILED != data2)
//        {
//            if( memcmp( fileData, data2, min(size, fileSize)))
//            {
//                for( unsigned long i = 0; i < min(size, fileSize); i++ )
//                {
//                    if( fileData[i] == data2[i] )
//                        continue;
//                    
//                    printf( "Fail @ %lu\n", i);
//                    
//                    unsigned long start = i - min(20UL, i);
//                    unsigned long end = min( start + 40, min(size, fileSize));
//                    const char * p0 = fileData + start;
//                    const char * p1 = data2 + start;
//                    
//                    char correct[48];   snprintf( correct, end - start, "%s", p0);
//                    char test[48];      snprintf( test, end - start, "%s", p1);
//                    
//                    printf( "*\t%s\n", correct);
//                    printf( " \t%s\n", test);
//                }
//            }
//            munmap(data2, size);
//        }
//        else
//            printf( "errno: %d\n", errno);
//        
//        close(fd);
//    }
 
    munmap( (void*) fileData, fileSize);
    delete node;
    
    return 0;
}
