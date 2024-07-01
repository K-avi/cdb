#include "../src/timestamp.h"
#include <stdlib.h>


void* test_timestamp(void* arg){
    s_timegen* ts_gen = (s_timegen*)arg;
    timestamp_t ts;

    sleep(rand()%2+1);
    get_timestamp(ts_gen, &ts);
    printf("Thread %lu got timestamp %lu\n", pthread_self(), ts);
    return NULL;
}

int main(void){

    s_timegen ts_gen ; 
    init_timestamp_gen(&ts_gen);

    pthread_t threads[10];

    for(int i = 0; i < 10; i++){
            pthread_create(&threads[i], NULL, test_timestamp, &ts_gen);
    }
    
    sleep(2);
    for(int i = 0; i < 10; i++){
        pthread_join(threads[i], NULL);
    }

    



    return 0;
}
