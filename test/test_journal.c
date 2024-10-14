#include "../src/journal.h"
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>



struct journal_tgen{
    s_journal *journal;
    s_timegen *tsgen;
};

void* test_journal_add(void*args){
    
    s_journal *journal = ((struct journal_tgen*)args)->journal;
    s_timegen *tsgen = ((struct journal_tgen*)args)->tsgen;

    s_transaction txn;
    transaction_init(&txn);

    timestamp_t ts;
    get_timestamp(tsgen, &ts);

    transaction_begin(&txn, ts);

    s_key key;
    key_init("test", ts, &key);

    s_value val;
    u_value uval;
    uval.u64 = rand();
    value_init(uval, U64, &val);

    transaction_insert(&txn, &key, &val);    
    transaction_commit(&txn);

    for(uint32_t i = 0 ; i < 16 ; i++){
        printf("thread %lu inserted %lu \n", pthread_self(), val.val.u64);
        sleep(1);
        journal_add(journal, &txn);
    }

    transaction_free(&txn);
    value_free(&val);
    key_free(&key);

    return NULL;
}

int main(void){
    
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
