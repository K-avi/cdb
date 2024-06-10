#include "key.h"
#include "err_handler.h"
#include "timestamp.h"
#include <string.h>

#define default_max_key_size 256
uint64_t max_key_size = default_max_key_size;

errflag_t key_init(char* key, timestamp_t ts, s_key *key_struct){
    def_err_handler(!key, "key_init key", ERR_NULL);
    def_err_handler(!key_struct, "key_init key_struct", ERR_NULL);
    
    key_struct->key = strndup(key, default_max_key_size);
    key_struct->ts = ts;
    
    return ERR_OK;
}//not tested

errflag_t key_create(char *key, s_timegen *generator, s_key *key_struct){
    def_err_handler(!key, "key_create key", ERR_NULL);
    def_err_handler(!generator, "key_create generator", ERR_NULL);
    def_err_handler(!key_struct, "key_create key_struct", ERR_NULL);
    
    key_struct->key = strndup(key, default_max_key_size);
     
    errflag_t failure = get_timestamp(generator, &(key_struct->ts));
    def_err_handler(failure,"key_create get_timestamp", failure);

    return ERR_OK;
}//not tested

errflag_t key_destroy(s_key *key_struct){
    def_err_handler(!key_struct, "key_destroy key_struct", ERR_NULL);
    
    free(key_struct->key);
    key_struct->key = NULL;
    key_struct->ts = 0;
    
    return ERR_OK;
}//not tested
