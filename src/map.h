#ifndef cdb_map_h 
#define cdb_map_h 

#include "common.h"
#include "key.h"
#include "value.h"


typedef struct s_map_bucket{

    s_key *key; 
    s_value *value;

}s_mapbucket; 



typedef struct s_map{
    uint32_t nb_buckets; 
    _Atomic(s_mapbucket) *buckets; //array of atomic buckets 
}s_map; 
/*
Changed my mind ; will implement the map w linear probing resolution 
I'll deal w rehash later (I won't)
*/

errflag_t map_init(uint32_t nb_buckets, s_map *map);
/*
s_map -> not null ; memleak on already initialized map
nb_buckets -> non zero

initializes the map with nb_buckets buckets
*/

errflag_t map_insert(s_map *map, s_key *key, s_value *value);
/*
map -> non null & initialized
key -> non null & initialized 
value -> non null & initialized
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

errflag_t map_lookup(s_map *map, s_key *key, s_value *value_ret);
/*
map -> non null & initialized
key -> non null & initialized 
value_ret -> non null ; memleak on already initialized value_ret
thread safe

fetches key from map and puts it in value_ret
returns unknown value and initializes key to unknown value 
on key not found
*/

#ifdef  debug
void map_print(s_map *map);
#endif //debug


#endif 

