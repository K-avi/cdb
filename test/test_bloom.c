#include "../src/bloom.h"
#include <stdint.h>
#include <stdio.h>

int main(void){
    s_bloom_filter filter;
    bloom_init(5096,1e4+100, &filter);

    s_key key;
    timestamp_t ts = 1 ;


    for(uint32_t i = 0; i < 1e4; i++){
        char buff[40]; 
        sprintf(buff, "%d", i);
        key_init(buff, ts++, &key);
        
        bloom_insert(&filter, &key);

        key_free(&key);
    }

    bloom_print(&filter);

    uint32_t nb_false_neg = 0 ; 
    for(uint32_t i = 0 ; i < 1e4; i++){
        
        char buff[40]; 
        sprintf(buff, "%d", i);
        key_init(buff, ts++, &key);
        
        bool result;
        bloom_lookup(&filter, &key, &result);
        if(!result){
            nb_false_neg++;
            printf("false negative in a bloom filter??? how??\n");
        }
        key_free(&key);
    }

    printf("nb_false_neg: %d\n", nb_false_neg);
   


    uint32_t nb_false_pos = 0;
    for(uint32_t i = 1e4+1; i < 1e4+100; i++){
        
        char buff[40]; 
        sprintf(buff, "%d", i);
        key_init(buff, ts++, &key);
          
        bool result;
        bloom_lookup(&filter, &key, &result);
        if(result)
            nb_false_pos++;
        key_free(&key);
    }

    printf("nb_false_pos: %d\n", nb_false_pos);

    bloom_free(&filter);
    return 0;

}
