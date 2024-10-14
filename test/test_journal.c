#include "../src/journal.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/types.h>

struct journal_tgen{
    s_journal *journal;
    s_timegen *tsgen;
};

void* test_journal_add(void*args){
    
    s_journal *journal = ((struct journal_tgen*)args)->journal;
    s_timegen *tsgen = ((struct journal_tgen*)args)->tsgen;

    for(uint32_t i = 0 ; i < 16; i++){
        s_transaction txn;
        transaction_init(&txn);

        timestamp_t ts;
        get_timestamp(tsgen, &ts);

        transaction_begin(&txn, ts);

        s_key key;
        char buff[256];
        //init buff with the value of pthread_self()

        snprintf(buff, 255, "%lu%d", pthread_self(),rand());

        key_init(buff, ts, &key);

        s_value val;
        u_value uval;
        if(i%2){
            char buff2[256];
            snprintf(buff2, 255, "test%d%d",i,rand());
            uval.str = strdup(buff2);
            value_init(uval, STR, &val);
        }else{
            uval.u64 = i;
            value_init(uval, U64, &val);
        }

        transaction_insert(&txn, &key, &val);  
        transaction_commit(&txn);

        if(i%2){
            printf("thread %lu inserted %s \n", pthread_self(), val.val.str);
        }else{
            printf("thread %lu inserted %lu \n", pthread_self(), val.val.u64);
        }
        //sleep(1);
        journal_add(journal, &txn);
    

        transaction_free(&txn);
        value_free(&val);
        key_free(&key);

        s_key key2;
        get_timestamp(tsgen, &ts);
        key_init(buff, ts, &key2);

        s_value value2;

        journal_lookup(journal, &key2, &value2);
        value_print(&value2);

    }

    return NULL;
}

int main(void){
    srand(time(NULL));
    
    s_journal journal;
    journal_init(128, &journal);

    s_timegen tsgen;
    init_timestamp_gen(&tsgen);

    struct journal_tgen j_tgen;
    j_tgen.journal = &journal;
    j_tgen.tsgen = &tsgen;

    
    pthread_t t1, t2;

    pthread_create(&t1, NULL, test_journal_add,  &j_tgen);
    pthread_create(&t2, NULL, test_journal_add,  &j_tgen);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    journal_free(&journal, 0x0);
    free_timestamp_gen(&tsgen);

    return 0;
}
