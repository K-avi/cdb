#include "map.h"
#include "err_handler.h"
#include "murmurhash.h"
#include "value.h"

#include <stdatomic.h>
#include <stdint.h>
#include <string.h>
#include <threads.h>
#include <stdbool.h>


#define is_tombstone(bucket) (!bucket.key && bucket.value)

errflag_t map_init(uint32_t nb_buckets, s_map *map){
    def_err_handler(!map, "map_init map", ERR_NULL);
    def_err_handler(!nb_buckets, "map_init nb_buckets", ERR_INVALID);

    map->buckets = calloc(nb_buckets , sizeof(_Atomic s_mapbucket));
    def_err_handler(!map->buckets, "map_init malloc", ERR_ALLOC);

    map->nb_buckets = nb_buckets;
    
    return ERR_OK;
}//not tested

errflag_t map_insert(s_map *map, s_key *key, s_value *value){
    def_err_handler(!map, "map_insert map", ERR_NULL);
    def_err_handler(!key, "map_insert key", ERR_NULL);
    def_err_handler(!value, "map_insert value", ERR_NULL);

    s_mapbucket new_bucket = {key, value};
    
    s_mapbucket tombstone_bucket = {NULL, (s_value*) true};
    s_mapbucket empty_bucket = {NULL, NULL};
    

    uint32_t hash = murmurhash(key->key, strlen(key->key), 0); //shite
    uint32_t index = hash % map->nb_buckets;
    uint32_t start_index = index ; 
   
    do{//linear probing 
        index = (index + 1) % map->nb_buckets;
           
        //not sure if this works really unsure both operations r needed
        bool stored = atomic_compare_exchange_weak(&map->buckets[index], &tombstone_bucket , new_bucket);
        if (stored) {return ERR_OK;}

        stored = atomic_compare_exchange_weak(&map->buckets[index], &empty_bucket , new_bucket);
        if (stored) { return ERR_OK;}
    }while(start_index != index);//you can overwrite tombstones
  
    //will be handled better once i'll add rehasinh / elements counters
    return ERR_MAPFULL;
}
//not tested
//check for map saturation ; seems ok enough 
//critical function; should be thoroughly tested
//idkidk

errflag_t map_remove(s_map *map, s_key *key){
    def_err_handler(!map, "map_remove map", ERR_NULL);
    def_err_handler(!key, "map_remove key", ERR_NULL);

    uint32_t hash = murmurhash(key->key, strlen(key->key), 0); //shite
    uint32_t index = hash % map->nb_buckets;
    uint32_t start_index = index ; 

    s_mapbucket bucket = atomic_load(&map->buckets[index]);
   
    if(!bucket.key){
        return ERR_OK;
    }

    do{//linear probing 
        if(!strcmp(bucket.key->key, key->key)){
            s_mapbucket tombstone_bucket = {NULL,(s_value*) true};
            atomic_store(&map->buckets[index], tombstone_bucket);
            return ERR_OK;
        }
        index = (index + 1) % map->nb_buckets;
        bucket = atomic_load(&map->buckets[index]);
    }while(bucket.key && start_index != index);//this is probably wrong 
    //bc the store and load are not atomic and te condition might be true avec a load and before the 
    //store for a thread and another thread will also load since the first hasn't stored yet 
    //which will fuck things up imo 

    return ERR_OK;
}//not tested ; very very very likely wrong 

errflag_t map_lookup(s_map *map, s_key *key, s_value *value_ret){
    def_err_handler(!map, "map_lookup map", ERR_NULL);
    def_err_handler(!key, "map_lookup key", ERR_NULL);
    def_err_handler(!value_ret, "map_lookup value_ret", ERR_NULL);

    uint32_t hash = murmurhash(key->key, strlen(key->key), 0); //shite
    uint32_t index = hash % map->nb_buckets;
    uint32_t start_index = index ; 

    //fuck

    do{

    }while(1); 

    return ERR_OK;
}//not done 

//why did I want to do lock free why am I like this smh 
