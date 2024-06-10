#ifndef cdb_map_h 
#define cdb_map_h 

#include "common.h"
#include "key.h"
#include "value.h"




typedef struct s_map_bucket{

    s_key *key; 
    pthread_mutex_t lock; 

    struct s_map_bucket *next; 
    struct s_map_bucket *prev;
}s_mapbucket; 
/*
bucket will be stored as a doubly linked circular list 
because it's easier for concurrent access 
I generally prefer dynamic arrays but it's easier to use 
a linked list bc I'm afraid realloc will mess things up 
whith concurrency 
*/


typedef struct s_map{
    uint32_t nb_buckets; 
    
    s_mapbucket *buckets;
}s_map; 
/*
the simply stores the head of each bucket 
each time there is a new key inserted in a bucket, 
you insert it at the head of the list 

fetching is linear (bad) but inserting is O(1) (good)

If I can find the time I'll implement the buckets as actual 
trees like a madlad (I'm scared)
*/

errflag_t map_init(uint32_t nb_buckets, s_map *map);
/*
s_map -> not null ; memleak on already initialized map
nb_buckets -> non zero

initializes the map with nb_buckets buckets
*/

errflag_t map_insert(s_map *map, s_key *key);
/*
map -> non null & initialized
key -> non null & initialized 
thread safe 

inserts the key in the map 
*/

errflag_t map_remove(s_map *map, s_key *key);
/*
map -> non null & initialized
key -> non null & initialized 
thread safe 

removes the key from the map if it exists; 
no op otherwise
*/

errflag_t map_get(s_map *map, s_key *key, s_value *value_ret);
/*
map -> non null & initialized
key -> non null & initialized 
value_ret -> non null ; memleak on already initialized value_ret
thread safe

fetches key from map and puts it in value_ret
returns unknown value and initializes key to unknown value 
on key not found
*/

#endif 

