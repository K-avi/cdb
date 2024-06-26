#ifndef cdb_bloom_h
#define cdb_bloom_h

#include "common.h"
#include "err_handler.h"
#include "key.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>


typedef uint64_t* (*bloom_hash_function) (uint32_t num_hashes, const char *str);
typedef struct s_bloom_filter{
    uint64_t nb_bits;

    uint32_t nb_elements; 
    uint32_t nb_hashes;

    uint32_t bloom_size;
    byte_t *filter;

    bloom_hash_function hash_function;

}s_bloom_filter;

errflag_t bloom_init(uint32_t size, uint32_t estimate_elements, s_bloom_filter *filter);
/*
@param: nonzero size - the size of the bloom filter in bytes
@param: nonzero num_hashes - the number of hash functions to use
@param: filter - the bloom filter to initialize
@brief: initializes a bloom filter with the given size and number of hash functions
*/
errflag_t bloom_insert(s_bloom_filter *filter, s_key *key);
/*
@param: filter non null & initialized ;  the bloom filter to insert the key into
@param: key ; non null & initialized ; the key to insert
@brief: inserts the key into the bloom filter
*/
errflag_t bloom_lookup(s_bloom_filter *filter, s_key *key, bool *result);
/*
@param: filter non null & initialized ;  the bloom filter to perform the lookup on
@param: key ; non null & initialized ; the key to lookup
@param: result ; non null ; the result of the lookup will be stored in the boolean pointed to by result

@brief : performs a lookup on the bloom filter for the given key
*/
void bloom_free(s_bloom_filter *filter);
/*
@param : filter  the bloom filter to free
@brief : frees the memory allocated for the bloom filter; doesn't free the filter pointer
*/

#ifdef debug 
void bloom_print(s_bloom_filter *filter);
#endif

#endif 
