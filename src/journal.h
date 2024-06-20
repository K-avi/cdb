#ifndef cdb_journal_h
#define cdb_journal_h

#include "common.h"
#include "err_handler.h"
#include <stdint.h>

/*
a journal will be an in memory array 
each array will contain a fixed number of effects ; 

once an array is filled it will be moved to a linked list of filled arrays 
and flushed to disk by a separate thread

the abstract "journal object" will be the collection of files + the in memory array


the file format will (at first) be a simple binary dump of the array
*/


//man. 


//should cast the data as u8 before writing to disk
//bc enums are untrustworthy
typedef enum journal_op{
    OP_BEGIN = 0,
    OP_COMMIT = 1,
    OP_ABORT = 2,
    OP_INSERT = 3,
    OP_REMOVE = 4,
    OP_UPDATE = 5,
    OP_LOOKUP = 6,
    OP_DELETE = 7,
}journal_op;
//these are basically "opcode" for the journal entries
//they will be used to determine what to do with the data 

#define PAGE_SIZE 4096

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
    void *data;
}s_journal_entry;


typedef struct s_journal_page{
    uint32_t size;
    void *data;
    s_jmetadata *metadata;

    struct s_journal_page *next;

}s_journal_page;

typedef struct s_journal{
    

    s_journal_page *current_page;
    pthread_mutex_t lock_current_page;

    s_journal_page *filled_pages;
    
    
}s_journal;


errflag_t journal_init(uint32_t max_size, s_journal *journal);
errflag_t journal_add(s_journal *journal, void *data, uint32_t size);
errflag_t journal_free(s_journal *journal);
errflag_t journal_write(s_journal *journal, char *filename);
errflag_t journal_read(s_journal *journal, char *filename);




#endif
