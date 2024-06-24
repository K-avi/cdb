#ifndef cdb_journal_h
#define cdb_journal_h

#include "common.h"
#include "err_handler.h"
#include <stdint.h>
#include "timestamp.h"
/*
a journal will be an in memory array 
each array will contain a fixed number of effects ; 

once an array is filled it will be moved to a linked list of filled arrays 
and flushed to disk by a separate thread

the abstract "journal object" will be the collection of files + the in memory array

the file format will (at first) be a simple binary dump of the array
*/

//should cast the data as u8 before writing to disk
//bc enums are untrustworthy
typedef enum journal_op{
    OP_NOOP = 0,
    OP_BEGIN,
    OP_COMMIT,
    OP_ABORT,
    OP_INSERT,
    OP_REMOVE,
    OP_UPDATE,
    OP_LOOKUP,
    OP_DELETE,
}journal_op;
//these are basically "opcode" for the journal entries
//they will be used to determine what to do with the data 

#define DEFAULT_PAGE_SIZE 4096
extern uint32_t glob_page_size;

typedef struct s_journal_metadata s_jmetadata;
//this will contain AT LEAST : 
/*
- bloom filter 
- page number
- page size
- time domain of the page
*/

typedef struct s_journal_entry{
    uint32_t size;
    byte_t *data;
}s_journal_entry;

typedef struct s_journal_metadata{
    uint32_t page_number;
    uint32_t page_size;
    timestamp_t start_time;
    timestamp_t end_time;
    //bloom filter
}s_jmetadata;//not final at all; just a placeholder

typedef struct s_journal_page{
    uint32_t size;
    uint32_t used;
    byte_t *entries;
    s_jmetadata *metadata;

    struct s_journal_page *next;

}s_journal_page;

typedef struct s_journal{
    
    uint32_t page_size;

    s_journal_page *current_page;
    pthread_mutex_t lock_current_page;

    s_journal_page *filled_pages;
    
}s_journal;

errflag_t journal_init(uint32_t page_size, s_journal *journal);
/*
@param: page_size -> non-zero ; the size of the pages in the journal
@param: journal -> non null ; the journal to initialize ; memleak on initialized journal 
@brief : initializes a journal with a single page set to page_size ; initializes every field 
for the journal and it's first page 
*/

errflag_t journal_add(s_journal *journal, s_journal_entry *entry);
/*
@param: journal -> non null ; initialized the journal that will receive the data
@param: data -> non null ; the data to add to the journal
@param: size -> non-zero ; the size of the data to add to the journal

@brief: thread safe; appends the data to the journal ; if the current page is full, it will be moved to the filled pages list; 
a new page will be created and the data will be added to it
*/

errflag_t journal_free(s_journal *journal, uint8_t flags);
/*
@param : journal -> non null ; initialized

@brief : frees the journal and all of it's pages; before doing so , will try to flush the journal to disk if (flags & 0x1)
if (flag & 0x2), the journal will be destroyed but not the pages
*/

errflag_t journal_write(s_journal *journal, char *filename);
/*
@param : journal -> non null ; initialized 
no 
*/

errflag_t journal_read(s_journal *journal, char *filename);
/*
@param : journal -> non null ; initialized | non initialized
@param : filename -> non null ; correct journal file;  the file to read the journal from

@brief : reads the journal from the file and initializes the journal with the data or appends the data to the journal
if the journal is already initialized
*/
#endif
