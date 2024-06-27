#ifndef cdb_key_h
#define cdb_key_h

#include "common.h"
#include "err_handler.h"
#include "timestamp.h"
#include <sys/types.h>


extern uint64_t max_key_size;

typedef struct s_key{
    uint32_t key_size;
    timestamp_t ts ; 
    char* key ; 
}s_key; 
//a key is "theoretically" just the char* part 
//but the timestamp is used to access a key anyways 
//so I'll make the struct for a key contain both 



//I'm not sure if I should define the key manipulations functions here tbh 
errflag_t key_init(char* key, timestamp_t ts, s_key *key_struct);
/*
@param: key -> non null ; null terminated string the "value" of the key to store
@param: ts -> the timestamp that will be assigned to the key
@param: key_struct -> non null ; memleak on already initialized struct the key structure that will receive 
a copy of the key string 

@brief: initialises the fields of key_struct with a copy of key and ts
*/

errflag_t key_create(char* key, s_timegen *generator, s_key *key_struct); 
/*
@param: key -> non null ; null terminated string the "value" of the key to store
@param: generator -> non null; initialized; the timestamp generator to get a valid timestamp from 
@param: key_struct -> non null ; memleak on already initialized struct; the key_structure that will receive a timestamp/ 
a copy of the key string

@brief: initializes the fields of key_struct with a new timestamp from the generator 
and a copy of the key string
*/

errflag_t key_free(s_key *key_struct);
/*
@param: key_struct -> whatever 

@brief: frees the fields of key_struct and zeroes them
*/

errflag_t key_duplicate(s_key *src, s_key *dst);
/*
@param: src -> not null ; initialized
@param: dst -> not null ; initialized

@brief: duplicates the key in src into dst
by allocating a new string and copying the timestamp
*/

errflag_t key_hash(s_key *key, uint32_t* hashed_key);
/*
@param key: non null ; initialized the key to hash
@param hashed_key: non null ; initialized ; the hashed key will be returned in this variable 

@brief: hashes the key into hashed_key
*/

errflag_t key_to_byte_array(s_key* key, s_byte_array* byte_array);
/*
@param: key -> initialized & non null key to convert to byte array
@param: byte_array -> non null ; initialized error if byte_array->max < key->key_size + sizeof(timestamp_t) + sizeof(uint32_t)

@brief: converts the key into a byte array
*/
errflag_t key_from_byte_array(s_key* key, s_byte_array* byte_array);
/*
@param: key -> non null ; memleak on already initialized struct; the key that will receive the data from the byte array
@param: byte_array -> non null ; initialized; the byte array that contains the data to convert to a key

@brief: converts the byte array into a key
*/

#ifdef debug 
void key_print(s_key *key);
#endif

#endif
