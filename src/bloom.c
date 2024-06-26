#include "bloom.h"
#include "err_handler.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

//this is a placeholder for the bloom filter implementation

#define DEFAULT_BLOOM_SIZE 128 
uint32_t glob_bloom_size = DEFAULT_BLOOM_SIZE;
#define DEFAULT_NB_HASHES 1
uint32_t glob_nb_hashes = DEFAULT_NB_HASHES; 

#define LN2 0.69314718055994530942

static uint64_t* __default_hash(uint32_t num_hashes,  const char *str);

errflag_t bloom_init(uint32_t bloom_size, uint32_t estimate_elements,s_bloom_filter *filter){
    def_err_handler(!filter, "bloom_init filter", ERR_NULL);
    warning_handler(
        !bloom_size, 
        "bloom_init size", 
        ERR_VALS,
        errflag_t failure=bloom_init(glob_bloom_size, estimate_elements, filter); 
        return failure;
    );
    warning_handler(
        !bloom_size, 
        "bloom_init size", 
        ERR_VALS,
        errflag_t failure=bloom_init(glob_bloom_size, estimate_elements, filter); 
        return failure;
    );

    filter->filter = calloc(bloom_size, sizeof(byte_t));
    def_err_handler(!filter->filter, "bloom_init filter->filter", ERR_ALLOC);

    filter->hash_function = __default_hash;

    filter->nb_bits = bloom_size * 8;
    filter->nb_elements = 0;

    filter->nb_hashes = LN2 * filter->nb_bits / filter->nb_elements;
    if(!filter->nb_hashes) filter->nb_hashes = 1;

    filter->bloom_size = bloom_size;

    return ERR_OK;
}//not done 

//from : https://github.com/barrust/bloom/blob/master/src/bloom.c
static uint64_t __fnv_1a(const char *key, int seed) {
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    int i, len = strlen(key);
    uint64_t h = 14695981039346656037ULL + (31 * seed); // FNV_OFFSET 64 bit with magic number seed
    for (i = 0; i < len; ++i){
            h = h ^ (unsigned char) key[i];
            h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}

static uint64_t* __default_hash(uint32_t num_hashes, const char *str) {
    uint64_t *results = (uint64_t*)calloc(num_hashes, sizeof(uint64_t));
    int i;
    for (i = 0; i < num_hashes; ++i) {
        results[i] = __fnv_1a(str, i);
    }
    return results;
}



errflag_t bloom_insert(s_bloom_filter *filter, s_key *key){
    def_err_handler(!filter, "bloom_init filter", ERR_NULL);
    def_err_handler(!key, "bloom_init key", ERR_NULL);

    uint64_t *hashes = filter->hash_function(filter->nb_hashes, key->key);

    for(uint32_t i = 0; i < filter->nb_hashes; i++){
        uint64_t hash = hashes[i];
        uint64_t index = hash % filter->nb_bits;
        filter->filter[index / 8] |= 1 << (index % 8);
    }

    filter->nb_elements++;

    free(hashes);
    return ERR_OK;
}//note tested 

errflag_t bloom_lookup(s_bloom_filter *filter, s_key *key, bool *result){
    def_err_handler(!filter, "bloom_lookup filter", ERR_NULL);
    def_err_handler(!key, "bloom_lookup key", ERR_NULL);
    def_err_handler(!result, "bloom_lookup result", ERR_NULL);

    uint64_t *hashes = filter->hash_function(filter->nb_hashes, key->key);

    for(uint32_t i = 0; i < filter->nb_hashes; i++){
        uint64_t hash = hashes[i];
        uint64_t index = hash % filter->nb_bits;
        if(!(filter->filter[index / 8] & (1 << (index % 8)))){
            *result = false;
            free(hashes);
            return ERR_OK;
        }
    }

    *result = true;
    free(hashes);
    
    return ERR_OK;
}//not tested; prolly wrong 

void bloom_free(s_bloom_filter *filter){
    if(filter){
        if(filter->filter)
            free(filter->filter);
    
        filter->bloom_size = 0;
        filter->hash_function = NULL;
        filter->nb_bits = 0;
        filter->nb_elements = 0;
        filter->nb_hashes = 0;
        filter->filter = NULL;
    }
}//should be ok 

#ifdef debug 
void bloom_print(s_bloom_filter *filter){
    if(filter){
        printf("nb_bits: %lu\n", filter->nb_bits);
        printf("nb_elements: %u\n", filter->nb_elements);
        printf("nb_hashes: %u\n", filter->nb_hashes);
        printf("bloom_size: %u\n", filter->bloom_size);
        printf("filter: ");
        for(uint32_t i = 0; i < filter->bloom_size; i++){        
            printf("%2.2x", filter->filter[i]);
            printf(" ");
        }
        printf("\n");
    }
}
#endif
