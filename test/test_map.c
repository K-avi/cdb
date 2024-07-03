#include "../src/map.h"
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>



typedef struct map_gen{
    s_map *map;
    s_timegen *tsgen;
}s_mapgen;

void * test_map_insert(void * mapgen){
    s_map *m = ((s_mapgen*) mapgen)->map;
    s_timegen *t = ((s_mapgen*) mapgen)->tsgen;

    s_key key;

    for(uint32_t i = 0 ; i < 8 ; i++){
        get_timestamp(t, &key.ts);

        char buff[256]; 
        snprintf(buff, 255, "test%lu", pthread_self());
        
        key_init(buff, key.ts, &key);

        s_value val ; 

        u_value uval; 
        uval.u64 = pthread_self();

        value_init(uval, U64 ,  &val);

        map_insert(m, &key, &val);

        key_free(&key);
        value_free(&val);
    }
    return NULL;
}


void * test_map_lookup(void * mapgen){
    s_map *m = ((s_mapgen*) mapgen)->map;
    s_timegen *t = ((s_mapgen*) mapgen)->tsgen;

    s_key key;

    key.ts = UINT32_MAX;

    char buff[256]; 
    snprintf(buff, 255, "test%lu", pthread_self());
    
    key_init(buff, key.ts, &key);

    s_value value_ret ;
    map_lookup(m, &key, &value_ret);

    printf("lookup: key.key %s key.ts %lu\n", key.key, key.ts);

    value_print(&value_ret);

    key_free(&key); 
    value_free(&value_ret);

    return NULL;
}

void * test_map_delete(void* mapgen){
    s_map *m = ((s_mapgen*) mapgen)->map;
    //s_timegen *t = ((s_mapgen*) mapgen)->tsgen;

    s_key key;
    key.ts = 0;

    char buff[256]; 
    snprintf(buff, 255, "test%lu", pthread_self());
    
    key_init(buff, key.ts, &key);

    map_delete_key(m, &key);

    key_free(&key);

    printf("deleted key %s\n", buff);
    return NULL;

}


void * multi_test(void*mapgen){
    for(uint32_t i = 0 ; i < 64 ; i++){
        test_map_insert(mapgen);
        test_map_lookup(mapgen);
        test_map_delete(mapgen);
    }
}

int main(void){

    s_map map ; 
    errflag_t failure =  map_init(16, &map);
    def_err_handler(failure, "what", failure);


    map_print(&map);

    s_timegen tsgen ; 
    init_timestamp_gen(&tsgen);

    s_mapgen mapgen ;
    mapgen.map = &map;
    mapgen.tsgen = &tsgen;

    s_key key;

    key_init("test", 1, &key);

    pthread_t threads[16];

    for(uint32_t i = 0 ; i < 16 ; i++){
        pthread_create(&threads[i], NULL, multi_test, &mapgen);
    }
    for(uint32_t i = 0 ; i < 16 ; i++) pthread_join(threads[i], NULL);

    map_print(&map);

    free_timestamp_gen(&tsgen);
    map_free(&map);
    key_free(&key);
    return 0;
}
