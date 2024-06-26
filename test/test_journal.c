#include "../src/journal.h"
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

void* test_journal_add(void*args){
    
    s_journal *journal = (s_journal*)args;
    s_journal_entry entry;

    byte_t dt[] = {0,0,0,0,0,0,0,0};

    entry.data = (byte_t*)dt;
    entry.size = 8;
    for(uint32_t i = 0 ; i < 16 ; i++){
        printf("thread %lu inserted \n", pthread_self());
        sleep(1);
        journal_add(journal, &entry);
    }
    return NULL;

}

int main(void){
    s_journal journal;
    journal_init(128, &journal);
    
    pthread_t t1, t2;

    pthread_create(&t1, NULL, test_journal_add, &journal);
    pthread_create(&t2, NULL, test_journal_add, &journal);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    journal_free(&journal, 0x0);
    return 0;
}
