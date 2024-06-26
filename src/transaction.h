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
}s_kvp;

typedef struct s_kvp_dynarr{
    uint32_t cur_size;
    uint32_t max_size;
    s_kvp* kvp_array;
}s_kvp_dynarr;

typedef struct s_txn_dynarr{
    uint32_t cur_size;
    uint32_t max_size;
    byte_t* txn_array;
}s_txn_dynarr;

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

errflag_t transaction_abort(s_transaction* txn);

errflag_t transaction_insert(s_transaction* txn, s_key* key, s_value* value);
errflag_t transaction_remove(s_transaction* txn, s_key* key);

errflag_t transaction_update(s_transaction* txn, s_key* key, s_value* value);
errflag_t transaction_lookup(s_transaction* txn, s_key* key, s_value* value);

errflag_t transaction_delete(s_transaction* txn, s_key* key);

void transaction_free(s_transaction* txn);

#ifdef debug 
void transaction_print(s_transaction* txn);
#endif


#endif
