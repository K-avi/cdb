#ifndef cdb_transaction_h
#define cdb_transaction_h

#include "common.h"
#include "err_handler.h"
#include "timestamp.h"
#include "value.h"
#include "key.h"
#include <stdint.h>

//VERY MUCH A WIP; JUST TRYING THINGS OUT ; MIGHT CHANGE A LOT 

//should cast the data as u8 before writing to disk
//bc enums are untrustworthy
typedef enum txn_op{
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
/*these are basically "opcode" for the txn buffer / journal entries
they will be used to determine what to do with the data ; you can see them as
a bytecode for the transaction

each op is followed by some data and so on
*/


/*

bytecode format: 

op | txn_id | key_size |  key_timestamp | key_string | value_size | value_type | value_data
key_size and so on are optionnal. 

I will have to assume that key_size is 4 bytes , key_timestamp is 8 bytes , 
value_size is 4 bytes and value_type is 1 byte.

the size of the key/value_data can be whatever
*/

typedef struct s_key_value_pair{
    s_key key;
    s_value value;
}s_kvp; //a key value pair is simply a key and a value

typedef struct s_kvp_dynarr{
    uint32_t cur_size;
    uint32_t max_size;
    s_kvp* kvp_array;
}s_kvp_dynarr; 
//the kvp dynnamic array is a simple array of kvp 
//I'm not sure why it exists (I can't remember) but it's here
//I guess it can be used to look up a key ? 

typedef struct s_txn_dynarr{
    uint32_t cur_size;
    uint32_t max_size;
    byte_t* txn_array;
}s_txn_dynarr; 
//the txn dynamic array is a simple array of bytes, this array 
//will be the one that is passed to the journal and written to disk 

typedef struct s_transaction{
    s_txn_dynarr txn_array;
    s_kvp_dynarr kvp_array;

    uint32_t txn_id;

    uint8_t flags; //flags allow to ensure that the transaction is in a valid state (no operation after commit/abort or before begin)
    /*
    b0 -> begin
    b1 -> commit
    b2 -> abort
    */
}s_transaction; 

errflag_t transaction_init(s_transaction* txn);
/*
@param: txn ; non null ; non initialized transaction pointer

@brief: initializes / allocates the field of the transaction; 
memleak on already initialized txn
*/

//idk tbh how do I make txn interact w stores ? god knows
errflag_t transaction_begin(s_transaction* txn, uint32_t txn_id);
/*
@param: txn ; non null ; initialized transaction pointer
@brief: appends a begin operation to the transaction buffer
*/
errflag_t transaction_commit(s_transaction* txn);
/*
@param: txn ; non null ; initialized transaction pointer
@brief: appends a commit operation to the transaction buffer, 
raises an error if the transaction is not in a valid state i.e 
no begin or already committed/aborted
*/
errflag_t transaction_abort(s_transaction* txn);
/*
@param: txn ; non null ; initialized transaction pointer
@brief: appends an abort operation to the transaction buffer, 
raises an error if the transaction is not in a valid state i.e 
no begin or already committed/aborted
*/

errflag_t transaction_insert(s_transaction* txn, s_key* key, s_value* value);
/*
@param: txn ; non null ; initialized transaction pointer
@param: key ; non null ; initialized key to insert
@param: value ; non null ; initialized value to insert


*/

errflag_t transaction_lookup(s_transaction* txn, s_key* key, s_value* value);
/*
@param: txn; non null; initialized transaction pointer
@param: key; non null; initialized key to look up
@param: value; non null; uninitialized value to store the result of the lookup

@brief: looks up the key in the transaction buffer and returns the value in value 
if the key is not found, value->as is set to UNKNOWN and value.val.u64 is set to 0
*/
errflag_t transaction_remove(s_transaction* txn, s_key* key);
/*
will probably be implemented by updating a flag or something 
because I won't move memory around in the journal array
*/

errflag_t transaction_update(s_transaction* txn, s_key* key, s_value* value);
/*
*/

errflag_t transaction_delete(s_transaction* txn, s_key* key);

void transaction_free(s_transaction* txn);

#ifdef debug 
void transaction_print(s_transaction* txn);
#endif


#endif
