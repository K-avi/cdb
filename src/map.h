#ifndef cdb_map_h 
#define cdb_map_h 

#include "common.h"
#include "err_handler.h"
#include "key.h"
#include "value.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdint.h>


typedef struct s_map_bucket{

    s_key *key; 
    s_value *value;

    pthread_mutex_t lock; //friendship ended w atomic
    //pthread_cond_t cond;

    struct s_map_bucket *next;
}s_mapbucket; 
//this is silly bc I use linear probing
//I have no need to store the key in the bucket; I should 
//only store a copy of the key in the head smh 


typedef struct s_bucketthead{
    s_mapbucket *head; 
    pthread_mutex_t lock;
    uint8_t flags; //0x1 -> empty ; 0x2 -> tombstone 
//named struct bc I might add stuff to it
}s_buckethead;

typedef struct s_map{
    uint32_t nb_buckets; 
    s_buckethead *bucket_heads; 
}s_map; 
/*
*/

errflag_t map_init(uint32_t nb_buckets, s_map *map);
/*
@param: s_map -> not null ; memleak on already initialized map
@param: nb_buckets -> non zero

@returns: ERR_OK on success.
@brief: initializes the map with nb_buckets buckets; allocates memory
*/

errflag_t map_insert(s_map *map, s_key *key, s_value *value);
/*
@param: map -> non null & initialized
@param: key -> non null & initialized 
@param: value -> non null & initialized

@brief: thread safe insertion of a duplicate of the key in the map. 
*/

errflag_t map_remove(s_map *map, s_key *key);
/*
@param: map -> non null & initialized
@param: key -> non null & initialized 
@brief: thread safe removal of the key from the map if the key exists at the timestamp given; no op otherwise
*/

errflag_t map_lookup(s_map *map, s_key *key, s_value *value_ret);
/*
@param: map -> non null & initialized
@param: key -> non null & initialized 
@param: value_ret -> non null ; memleak on already initialized value_ret; returns a COPY of the value in the map;
thread safe

@brief: fetches the reference of the corresponding key to the value from the map and puts it in value_ret
returns unknown value and initializes key to unknown value on key not found
*/

errflag_t map_free(s_map *map);
/*
@param: map -> non null & initialized
@brief: frees the map and all the keys and values in it 
*/

errflag_t map_delete_key(s_map *map, s_key *key);
/*
@param: map -> non null & initialized
@param: key -> non null & initialized
@brief: deletes every instance of the key in the map (whatever the timestamp) if it exists 
no op otherwise
*/

#ifdef  debug
void map_print(s_map *map);
#endif //debug


#endif 

