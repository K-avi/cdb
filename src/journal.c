#include "journal.h"
#include "common.h"
#include "err_handler.h"

#include <pthread.h>
#include <stdint.h>
#include <string.h>


uint32_t glob_page_size = DEFAULT_PAGE_SIZE;

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
}//not tested

errflag_t journal_init (uint32_t page_size, s_journal *journal){
    def_err_handler(!journal, "journal_init journal", ERR_NULL);
    warning_handler(
        !page_size, 
        "journal_init page_size",
        ERR_VALS, 
        errflag_t failure=journal_init(DEFAULT_PAGE_SIZE, journal); 
        return failure;
    );//if page size is 0, set it to the default value and init w that value
    
    journal->current_page = malloc(sizeof(s_journal_page));
    def_err_handler(!journal->current_page, "journal_init current_page", ERR_ALLOC);
    
    errflag_t failure = journal_page_init(page_size, journal->current_page);
    error_handler(failure, "journal_init journal_page_init", failure, free(journal->current_page););

    journal->page_size = page_size;
    journal->filled_pages = NULL;
    pthread_mutex_init(&journal->lock_current_page,NULL);
    
    return ERR_OK;
}//not tested;


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
}//not tested


errflag_t journal_add(s_journal *journal, s_journal_entry *entry){
    def_err_handler(!journal, "journal_add journal", ERR_NULL);
    def_err_handler(!entry, "journal_add entry", ERR_NULL);

    def_err_handler(!entry->data, "journal_add entry data", ERR_NULL);
    def_err_handler(!entry->size, "journal_add entry size", ERR_VALS);

    def_err_handler(entry->size > glob_page_size, "journal_add size too large", ERR_VALS);

    pthread_mutex_lock(&journal->lock_current_page);

    s_journal_page *current_page = journal->current_page;
    if(current_page->used + entry->size > current_page->size){
        s_journal_page *new_page = malloc(sizeof(s_journal_page));
        error_handler(!new_page, "journal_add new_page", ERR_ALLOC, pthread_mutex_unlock(&journal->lock_current_page););

        errflag_t failure = journal_page_init(journal->page_size, new_page);
        error_handler(failure, "journal_add", failure, free(new_page); pthread_mutex_unlock(&journal->lock_current_page););

        //zero out the rest of the page (not optimal but simple enough)
        memset(&(current_page->entries[current_page->used]), OP_NOOP, current_page->size - current_page->used);
        current_page->used = current_page->size;
        journal_full_pages_append(journal, current_page);

        current_page->next = new_page;
        journal->current_page = new_page;
        current_page = new_page;
    }

    memcpy(&(current_page->entries[current_page->used]), entry->data, entry->size);
    current_page->used += entry->size;

    pthread_mutex_unlock(&journal->lock_current_page);
    return ERR_OK;
}//not tested
//doesnt support adding entries that are larger than the page size

