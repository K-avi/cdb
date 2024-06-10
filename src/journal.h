#ifndef cdb_journal_h
#define cdb_journal_h

#include "common.h"
#include "err_handler.h"

/*
a journal will be an in memory array 
each array will contain a fixed number of effects ; 

once an array is filled it will be moved to a linked list of filled arrays 
and flushed to disk by a separate thread

the abstract "journal object" will be the collection of files + the in memory array


the file format will (at first) be a simple binary dump of the array
*/


#endif
