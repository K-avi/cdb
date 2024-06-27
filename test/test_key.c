#include "../src/key.h"
#include <pthread.h>
#include <stdint.h>


void * test_key_create(void* arg){
    s_timegen * generator = (s_timegen*) arg;
    s_key key;

    char buff[256]; 
    snprintf(buff, 255, "key_%lu", pthread_self());
    key_create( buff, generator, &key);
    printf("key: %s, ts: %lu thread: %lu \n", key.key, key.ts, pthread_self());
    key_free(&key);
 
    return NULL;
}

int main(void){
    
    s_timegen timegen;
    init_timestamp_gen(&timegen);

    pthread_t thread_tab[10];

    for(int i = 0; i < 10; i++){
        pthread_create(&thread_tab[i], NULL, test_key_create, &timegen);
    }

    sleep(2); 

    for(int i = 0; i < 10; i++){
        pthread_join(thread_tab[i], NULL);
    }


    s_key dummy_key ;
    key_init("testingwow", UINT64_MAX, &dummy_key);

    s_byte_array byte_array;
    key_to_byte_array(&dummy_key, &byte_array);

    s_key new_key;
    key_from_byte_array(&new_key, &byte_array);

    key_print(&new_key);

    key_free(&dummy_key);
    key_free(&new_key);
    free(byte_array.data);
    free_timestamp_gen(&timegen);

    return 0;
}
