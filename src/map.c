#include "map.h"
#include "err_handler.h"
#include "key.h"
#include "murmurhash.h"
#include "value.h"

#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <threads.h>
#include <stdbool.h>

#define TOMB_FLAG 0x1
#define is_tombstone(bucket) (!bucket->key && bucket->value)
#define is_empty(bucket) (!bucket->key && !bucket->value)

errflag_t map_init(uint32_t nb_buckets, s_map *map){
    def_err_handler(!map, "map_init map", ERR_NULL);
    def_err_handler(!nb_buckets, "map_init nb_buckets", ERR_INVALID);

    map->buckets = calloc(nb_buckets , sizeof(s_mapbucket));
    def_err_handler(!map->buckets, "map_init malloc", ERR_ALLOC);

    for(uint32_t i = 0; i < nb_buckets; i++){
        pthread_mutex_init(&map->buckets[i].lock, NULL);
        //pthread_cond_init(&map->buckets[i].cond, NULL);
    }//should do error handling here

    map->nb_buckets = nb_buckets;
    
    return ERR_OK;
}//not tested

errflag_t map_insert(s_map *map, s_key *key, s_value *value){
    def_err_handler(!map, "map_insert map", ERR_NULL);
    def_err_handler(!key, "map_insert key", ERR_NULL);
    def_err_handler(!value, "map_insert value", ERR_NULL);

    uint32_t hash = murmurhash(key->key, strnlen(key->key, max_key_size), current_seed);
    uint32_t index = hash % map->nb_buckets;

    uint32_t start_index = index;

    do{

        s_mapbucket * cur_bucket = &map->buckets[index];
        
        pthread_mutex_lock(&cur_bucket->lock);
        //pthread_cond_wait(&cur_bucket->cond, &cur_bucket->lock);
        
        if(is_empty(cur_bucket) || is_tombstone(cur_bucket)){
            cur_bucket->key = key;
            cur_bucket->value = value;
            pthread_mutex_unlock(&cur_bucket->lock);
            //pthread_cond_signal(&cur_bucket->cond);    
            break;
        }

        //pthread_cond_signal(&cur_bucket->cond);
        pthread_mutex_unlock(&cur_bucket->lock);

        index = (index + 1) % map->nb_buckets;
    
    }while(index != start_index);


   return ERR_MAPFULL;
}
//not tested ; O(map->nb_buckets) 
//check for map saturation ; seems ok enough 
//critical function; should be thoroughly tested
//idkidk

errflag_t map_remove(s_map *map, s_key *key){
    def_err_handler(!map, "map_remove map", ERR_NULL);
    def_err_handler(!key, "map_remove key", ERR_NULL);

    uint32_t hash = murmurhash(key->key, strnlen(key->key, max_key_size), current_seed);
    uint32_t index = hash % map->nb_buckets;
    uint32_t start_index = index;

    s_mapbucket * cur_bucket;
        
    do{
        cur_bucket = &map->buckets[index];    
        pthread_mutex_lock(&cur_bucket->lock);
        //pthread_cond_wait(&cur_bucket->cond, &cur_bucket->lock);
        
        if(!strncmp(key->key, cur_bucket->key->key, max_key_size)){
            
            key_free(cur_bucket->key); 
            value_free(cur_bucket->value);

            cur_bucket->key = NULL;
            cur_bucket->value = (s_value*) TOMB_FLAG;

            pthread_mutex_unlock(&cur_bucket->lock);
            //pthread_cond_signal(&cur_bucket->cond);    
            break;
        }
        //pthread_cond_signal(&cur_bucket->cond);
        pthread_mutex_unlock(&cur_bucket->lock);
        index = (index + 1) % map->nb_buckets;
    
    }while(index != start_index && ! is_empty(cur_bucket) );
    
    if(is_empty(cur_bucket) || index == start_index) return ERR_NOTFOUND;
    return ERR_OK;
}//not tested

errflag_t map_lookup(s_map *map, s_key *key, s_value **value_ret){
    def_err_handler(!map, "map_lookup map", ERR_NULL);
    def_err_handler(!key, "map_lookup key", ERR_NULL);
    def_err_handler(!value_ret, "map_lookup value_ret", ERR_NULL);

    uint32_t hash = murmurhash(key->key, strnlen(key->key, max_key_size), current_seed);
    uint32_t index = hash % map->nb_buckets;
    uint32_t start_index = index;

    s_mapbucket * cur_bucket;
        
    do{
        cur_bucket = &map->buckets[index];    
        pthread_mutex_lock(&cur_bucket->lock);
        //pthread_cond_wait(&cur_bucket->cond, &cur_bucket->lock);
        
        if(!strncmp(key->key, cur_bucket->key->key, max_key_size)){
            
            *value_ret = cur_bucket->value;

            pthread_mutex_unlock(&cur_bucket->lock);
            //pthread_cond_signal(&cur_bucket->cond);    
            break;
        }
        //pthread_cond_signal(&cur_bucket->cond);
        pthread_mutex_unlock(&cur_bucket->lock);
        index = (index + 1) % map->nb_buckets;
    
    }while(index != start_index && ! is_empty(cur_bucket) );
    
    if(is_empty(cur_bucket) || index == start_index) return ERR_NOTFOUND;
    return ERR_OK;
}//not tested
