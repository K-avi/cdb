#include "map.h"
#include <stdint.h>


/*
bucket will be stored as a doubly linked circular list 
because it's easier for concurrent access 
I generally prefer dynamic arrays but it's easier to use 
a linked list for concurrecnt access (I'm afraid that the realloc would be annoying to implement)
*/
typedef struct s_map_bucket{

}s_map_bucket; 


struct s_map{
    uint32_t nb_buckets; 
    
    s_map_bucket *buckets;
}; 
