#include "journal.h"
#include "common.h"
#include "err_handler.h"
#include "key.h"
#include "timestamp.h"
#include "transaction.h"
#include "value.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PAGE_SIZE 4096
uint32_t glob_page_size = DEFAULT_PAGE_SIZE;

#define JOURNAL_FILE_SIGNATRUE 0x6976616e00 //it's me in hex :)

#define init_metadata(page,num,size,start,end) \
   {(page)->metadata->page_number = (num); \
    (page)->metadata->page_size = (size); \
    (page)->metadata->start_time = (start); \
    (page)->metadata->end_time = (end);}

static errflag_t journal_page_init(uint32_t page_size, s_journal_page *page){
    def_err_handler(!page, "journal_page_init page", ERR_NULL);
    warning_handler(
        !page_size, 
        "journal_init page_size", 
        ERR_VALS,
        errflag_t failure=journal_page_init(DEFAULT_PAGE_SIZE, page); 
        return failure;
    );//if page size is 0, set it to the default value and init w that value

    page->entries= malloc(page_size*sizeof(byte_t));
    def_err_handler(!page->entries, "journal_page_init data", ERR_ALLOC);

    page->metadata = malloc(sizeof(s_jmetadata));
    error_handler(!page->metadata, "journal_page_init metadata", ERR_ALLOC,free(page->entries););

    page->next = NULL;
    page->size = page_size;
    page->used = 0;
 
    return ERR_OK;
}//tested; seems ok 

errflag_t journal_init (uint32_t page_size, s_journal *journal){
    def_err_handler(!journal, "journal_init journal", ERR_NULL);
    warning_handler(
        !page_size, 
        "journal_init page_size",
        ERR_VALS, 
        errflag_t failure = journal_init(DEFAULT_PAGE_SIZE, journal); 
        return failure;
    );//if page size is 0, set it to the default value and init w that value
    
    journal->current_page = malloc(sizeof(s_journal_page));
    def_err_handler(!journal->current_page, "journal_init current_page", ERR_ALLOC);
    
    errflag_t failure = journal_page_init(page_size, journal->current_page);
    error_handler(failure, "journal_init journal_page_init", failure, free(journal->current_page););

    journal->first_page = journal->current_page;

    init_metadata(journal->first_page,0,page_size, TIMESTAMP_MAX, 0);

    journal->page_size = page_size;
    journal->filled_pages = NULL;
    pthread_mutex_init(&journal->lock_current_page,NULL);
    
    return ERR_OK;
}//tested; ok

static errflag_t journal_full_pages_append(s_journal *journal, s_journal_page *filled_page){
    def_err_handler(!journal, "journal_full_pages_append journal", ERR_NULL);
    def_err_handler(!filled_page, "journal_full_pages_append filled_page", ERR_NULL);

    s_journal_page *current = journal->filled_pages;
    if(!current){
        journal->filled_pages = filled_page;
        return ERR_OK;
    }

    while(current->next) current = current->next;
    
    current->next = filled_page;
    return ERR_OK;
}//tested; ok 
//I want the oldest full page first bc they 
//should be the ones written to disk first

errflag_t journal_add(s_journal *journal, s_transaction* transaction){
    def_err_handler(!journal, "journal_add journal", ERR_NULL);
    def_err_handler(!transaction, "journal_add entry", ERR_NULL);

    def_err_handler(!transaction->txn_array.txn_array, "journal_add entry size", ERR_VALS);
    def_err_handler(transaction->txn_array.cur_size > glob_page_size, "journal_add size too large", ERR_VALS);

    pthread_mutex_lock(&journal->lock_current_page);

    s_journal_page *current_page = journal->current_page;

    if(current_page->used + transaction->txn_array.cur_size > current_page->size){
        s_journal_page *new_page = malloc(sizeof(s_journal_page));
        error_handler(!new_page, "journal_add new_page", ERR_ALLOC, pthread_mutex_unlock(&journal->lock_current_page););

        errflag_t failure = journal_page_init(journal->page_size, new_page);
        error_handler(failure, "journal_add", failure, free(new_page); pthread_mutex_unlock(&journal->lock_current_page););

        //zero out the rest of the page (not optimal but simple enough)
        memset(&(current_page->entries[current_page->used]), OP_NOOP, current_page->size - current_page->used);
        current_page->used = current_page->size;
        journal_full_pages_append(journal, current_page);

        current_page->next = new_page;
        error_handler(!new_page->metadata, "journal_add new_page metadata", ERR_ALLOC, free(new_page); pthread_mutex_unlock(&journal->lock_current_page););

        init_metadata(new_page, current_page->metadata->page_number + 1, journal->page_size, 0, 0);

        journal->current_page = new_page;
        current_page = new_page;
    }

    memcpy(&(current_page->entries[current_page->used]), transaction->txn_array.txn_array, transaction->txn_array.cur_size);
    current_page->used += transaction->txn_array.cur_size;

    //update metadata
    if(transaction->start_time < current_page->metadata->start_time){
        current_page->metadata->start_time = transaction->start_time;
    }
    if(transaction->end_time > current_page->metadata->end_time){
        current_page->metadata->end_time = transaction->end_time;
    }

    pthread_mutex_unlock(&journal->lock_current_page);
    return ERR_OK;
}//tested; seems ok ; more testing needed
//doesnt support adding entries that are larger than the page size
//adds an entry to the current page, if the entry is too large for the current page, it will create a new page and add the entry to that pageA
//One way to handle huge entries would be to just allocate them a huge ass pagge 


static void journal_page_free(s_journal_page *page){
    if(page){
        free(page->entries);
        free(page->metadata);
        free(page);
    }
}//ok

void journal_free(s_journal *journal, uint8_t flags){

    if(flags){
        printf("flags are not supported yet\n");
    }else{
        if(journal){
            s_journal_page *current = journal->first_page;
            while(current){
                s_journal_page *next = current->next;
                journal_page_free(current);
                current = next;
            }
            pthread_mutex_destroy(&journal->lock_current_page);
        }
    }
}//tested; ok

static const uint8_t* sub_mem (const void* big, const void* small, size_t blen, size_t slen){
    /*
    @param big : the big array that is searched for an occurence of small
    @param small : the small array that is searched for in big
    @param blen : the size of big in bytes
    @param slen : the size of small in bytes

    @return : a pointer to the first occurence of small in big or NULL if not found
    */
    //Could implement optimisations w word size but I won't
    uint8_t* s = (uint8_t*)big;
    uint8_t* find = (uint8_t*)small;

	for(int32_t i = blen - slen  ; i >= 0 ; --i){
        if(s[i] == find[0]){  
            if(memcmp(s+i, find, slen) == 0){
                return s+i;
            }
        }
    }
    return NULL;
}/*Think about it like strstr but for two void ptrs */

static errflag_t init_barray(s_byte_array* barray, size_t max){
    def_err_handler(!barray, "init_barray barray", ERR_NULL);

    barray->data = calloc(max, sizeof(byte_t));
    def_err_handler(!barray->data, "init_barray barray->data", ERR_ALLOC);

    barray->cur = 0; 
    barray->max = max; 

    return ERR_OK;
}

#ifdef debug
static void journal_page_print(s_journal_page *page);
#endif

errflag_t journal_lookup(s_journal *journal, s_key *key, s_value *value_ret){
   
    errflag_t failure; 
    s_journal_page *current = journal->current_page;

    //only need to lock the current page because the filled pages / next pages are read only
    pthread_mutex_lock(&journal->lock_current_page);

    if(current->metadata->start_time > key->ts){
        //if the key is newer than the current page, it's not in the journal
        pthread_mutex_unlock(&journal->lock_current_page);
        value_ret = NULL;
        return ERR_OK;
    }
    const uint8_t* found = sub_mem(current->entries, key->key, current->used, key->key_size);
    if(found){
        s_byte_array value_barray; 
        value_barray.data = (uint8_t*)found + key->key_size;
        //this could be wrong tbh

        value_barray.cur = sizeof(value_as) + sizeof(timestamp_t) + sizeof(uint32_t);

        failure = value_from_byte_array(value_ret, &key->ts,&value_barray);
        pthread_mutex_unlock(&journal->lock_current_page);
        def_err_handler(failure, "journal_lookup value_from_byte_array", failure);
        return ERR_OK;
    }
    current = current->next;// do this before unlocking to avoid another thread overwritting next
    pthread_mutex_unlock(&journal->lock_current_page);

    //general case with a while loop; code is the same as above but without the lock; SHOULD work bc
    //the list is head insert only and only the current page is written to
    while (current) {
        if(current->metadata->end_time < key->ts){
        //if ts is higher than the end time of the page, the key is not in the journal
            value_ret->as = UNKNOWKN;
            return ERR_OK;
        }
        found = sub_mem(current->entries, key->key, current->used, key->key_size);
        if(found){
            s_byte_array value_barray; 
            value_barray.data = (uint8_t*)found + key->key_size;
            //this could be wrong tbh
            

            failure = value_from_byte_array(value_ret, &key->ts,&value_barray);
            def_err_handler(failure, "journal_lookup value_from_byte_array", failure);
            return ERR_OK;
        }
        current = current->next;
    }
    value_ret->as = UNKNOWKN;
    return ERR_OK;
}//not tested; very likely to be wrong ; doesn't support bloom filter yet
//doesn't take journal files into account yet


errflag_t journal_write(s_journal *journal, char *filename){
    def_err_handler(!journal, "journal_write journal", ERR_NULL);
    def_err_handler(!filename, "journal_write filename", ERR_NULL);


    FILE * file = fopen(filename, "rb");
    if(file){
        uint64_t signature = 0;
        fgets((char*)&signature, sizeof(uint64_t), file);
        if(signature != JOURNAL_FILE_SIGNATRUE){
            fclose(file);
            def_err_handler(1, "journal_write signature", ERR_VALS);            
        }
    }
    fclose(file);
    
    file = fopen(filename, "ab+");
    def_err_handler(!file, "journal_write file", ERR_VALS);

    s_journal_page *first_full = journal->filled_pages;

    while(first_full){
        fwrite(first_full->entries, first_full->used, 1, file);
        first_full = first_full->next;
    }
    return ERR_OK;
}//not tested; flushed the filled pages to disk; doesnt write metadata ; kinda bad tbh

errflag_t journal_read(s_journal *journal, char *filename){
    return ERR_OK;
}//not done

#ifdef debug
static void journal_page_print(s_journal_page *page){
    if(page){
        printf("size: %u\n", page->size);
        printf("used: %u\n", page->used);
        printf("next: %p\n", (void*)page->next);
        printf("metadata: %p\n", (void*)page->metadata);
        printf("entries: %p\n", (void*)page->entries);

        for(uint32_t i = 0 ; i < page->used ; i++){
            printf("%c", page->entries[i]);
            //if(i % 16 == 15) printf("\n");

        }
        printf("\n");

        for(uint32_t i = 0 ; i < page->used ; i++){
            printf("%x ", page->entries[i]);
            //if(i % 16 == 15) printf("\n");

        }
        printf("\n");
    }
    return;
}//not done 

void journal_print(s_journal *journal){
    if(journal){
        printf("page_size: %u\n", journal->page_size);
        printf("current_page: %p\n", (void*)journal->current_page);
        printf("first_page: %p\n", (void*)journal->first_page);
        printf("filled_pages: %p\n", (void*)journal->filled_pages);
    }
}//not done
#endif
