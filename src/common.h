#ifndef cdb_common_h
#define cdb_common_h 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //all my homies hate windows
#include <pthread.h>
#include <stdint.h>


typedef uint8_t byte_t; 

typedef struct s_byte_array{
    uint32_t max;
    uint32_t cur; 
    byte_t* data;
}s_byte_array;

#define debug 

#endif
