#include "key.h"
#include "common.h"
#include "err_handler.h"
#include "murmurhash.h"
#include "timestamp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define default_max_key_size 256
uint64_t max_key_size = default_max_key_size;

errflag_t key_init(char* key, timestamp_t ts, s_key *key_struct){
    def_err_handler(!key, "key_init key", ERR_NULL);
    def_err_handler(!key_struct, "key_init key_struct", ERR_NULL);
    
    key_struct->key = strndup(key, default_max_key_size);
    key_struct->key_size = strnlen(key, default_max_key_size);
    key_struct->ts = ts;
    
    return ERR_OK;
}//tested ; ok

errflag_t key_create(char *key, s_timegen *generator, s_key *key_struct){
    def_err_handler(!key, "key_create key", ERR_NULL);
    def_err_handler(!generator, "key_create generator", ERR_NULL);
    def_err_handler(!key_struct, "key_create key_struct", ERR_NULL);
    
    key_struct->key = strndup(key, default_max_key_size);
    key_struct->key_size = strnlen(key, default_max_key_size);
     
    errflag_t failure = get_timestamp(generator, &(key_struct->ts));
    def_err_handler(failure,"key_create get_timestamp", failure);

    return ERR_OK;
}//tested ; ok 

errflag_t key_free(s_key *key_struct){
    if(key_struct){
        if(key_struct->key) {
            free(key_struct->key);
            key_struct->key = NULL;
        }
        key_struct->ts = 0;
        key_struct->key_size = 0;
    }

    return ERR_OK;
}//tested; ok 

errflag_t key_duplicate(s_key *src, s_key *dst){
    def_err_handler(!src, "key_duplicate src", ERR_NULL);
    def_err_handler(!dst, "key_duplicate dst", ERR_NULL);
    
    dst->key = strndup(src->key, default_max_key_size);
    dst->ts = src->ts;
    dst->key_size = src->key_size;
    
    return ERR_OK;
}//tested; ok


errflag_t key_hash(s_key *key, uint32_t* hashed_key){
    def_err_handler(!key, "key_hash key", ERR_NULL);
    def_err_handler(!hashed_key, "key_hash hashed_key", ERR_NULL);
    
    *hashed_key = murmurhash(key->key, key->key_size, current_seed );
 
    return ERR_OK;
}//not tested; should be ok

errflag_t key_to_byte_array(s_key* key, s_byte_array* byte_array){
    def_err_handler(!key, "key_to_byte_array key", ERR_NULL);
    def_err_handler(!byte_array, "key_to_byte_array byte_array", ERR_NULL);
    
    uint32_t size =  key->key_size + sizeof(timestamp_t) + sizeof(uint32_t) ;
    def_err_handler(size + byte_array->cur > byte_array->max, "key_to_byte_array size", ERR_VALS);

    memcpy(byte_array->data + byte_array->cur, &key->key_size, sizeof(uint32_t));
    memcpy(byte_array->data + byte_array->cur + sizeof(uint32_t), &key->ts, sizeof(timestamp_t));
    memcpy(byte_array->data + byte_array->cur + sizeof(uint32_t) + sizeof(timestamp_t), key->key, key->key_size);
    
    byte_array->cur += size;

    return ERR_OK;
}// tested; seems ok
/*
WRONG WHAT IF THE BYTE ARRAY IS NOT EMPTY YOU DIMWIT 
*/

errflag_t key_from_byte_array(s_key* key, s_byte_array* byte_array, uint32_t offset){
    def_err_handler(!key, "key_from_byte_array key", ERR_NULL);
    def_err_handler(!byte_array, "key_from_byte_array byte_array", ERR_NULL);
    
    key->key_size = *(uint32_t*)(byte_array->data + offset);
    key->ts = *(timestamp_t*)(byte_array->data + offset + sizeof(uint32_t));
    key->key = strndup((char*)((byte_array->data+offset) + sizeof(uint32_t) + sizeof(timestamp_t)), key->key_size);
    
    return ERR_OK;
}//tested; seems ok


#ifdef debug
void key_print(s_key *key){
    
    printf("key: %s\n", key->key);
    printf("timestamp: %lu\n", key->ts);
    printf("key_size: %u\n", key->key_size);
}//not tested
#endif
