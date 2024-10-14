#include "transaction.h"
#include "common.h"
#include "err_handler.h"
#include "key.h"
#include "timestamp.h"
#include "value.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*********************************STATIC DYNAMIC ARRAY MANIPULATION*********************************/
/***************************************************************************************************/

#define DEFAULT_KVP_DYNARR_SIZE 16
uint32_t glob_kvp_dynarr_size = DEFAULT_KVP_DYNARR_SIZE;
#define DEFAULT_KVP_REALLOC_COEFF 1.5
double glob_kvp_realloc_coeff = DEFAULT_KVP_REALLOC_COEFF;

#define DEFAULT_TXN_DYNARR_SIZE 16
uint32_t glob_txn_dynarr_size = DEFAULT_TXN_DYNARR_SIZE;
#define DEFAULT_TXN_REALLOC_COEFF 1.5
double glob_txn_realloc_coeff = DEFAULT_TXN_REALLOC_COEFF;

static errflag_t kvp_dynarr_init(s_kvp_dynarr *kvp_arr, uint32_t size){
    def_err_handler(!kvp_arr, "kvp_dynarr_init kvp_arr", ERR_NULL);
    kvp_arr->cur_size = 0;
    kvp_arr->max_size = size;

    kvp_arr->kvp_array = malloc(size*sizeof(s_kvp));
    def_err_handler(!kvp_arr->kvp_array, "kvp_dynarr_init kvp_array", ERR_ALLOC);

    return ERR_OK;
}

static errflag_t kvp_dynarr_realloc(s_kvp_dynarr *kvp_arr){
    def_err_handler(!kvp_arr, "kvp_dynarr_realloc kvp_arr", ERR_NULL);

    s_kvp *new_kvp_array = realloc(kvp_arr->kvp_array, kvp_arr->max_size*glob_kvp_realloc_coeff*sizeof(s_kvp));
    def_err_handler(!new_kvp_array, "kvp_dynarr_realloc new_kvp_array", ERR_ALLOC);

    kvp_arr->kvp_array = new_kvp_array;
    kvp_arr->max_size *= glob_kvp_realloc_coeff;
    return ERR_OK;
}

static errflag_t kvp_dynarr_append(s_kvp_dynarr *kvp_arr, s_key* key, s_value* value){
    def_err_handler(!kvp_arr, "kvp_dynarr_append kvp_arr", ERR_NULL);
    def_err_handler(!kvp_arr->kvp_array, "kvp_dynarr_append kvp_array", ERR_NULL);

    if(kvp_arr->cur_size == kvp_arr->max_size){
        errflag_t failure = kvp_dynarr_realloc(kvp_arr);
        error_handler(failure, "kvp_dynarr_append kvp_dynarr_realloc", failure, return failure;);
    }

    errflag_t failure = key_duplicate(key, &kvp_arr->kvp_array[kvp_arr->cur_size].key);
    error_handler(failure, "kvp_dynarr_append key_duplicate", failure, return failure;);

    failure = value_dup(value, &kvp_arr->kvp_array[kvp_arr->cur_size].value);
    error_handler(failure, "kvp_dynarr_append value_duplicate", failure, return failure;);

    kvp_arr->cur_size++;
    return ERR_OK;
}
/*
this should not be necessary bc the map already returns copies of the keys and values
so technically we could just copy the pointers and not allocate any new memory
however, this would make the transaction dependent on the map implementation

so ATM I will stick to duplicating everything 
but this might change in the future
*/

static void kvp_dynarr_free(s_kvp_dynarr *kvp_arr){
    if(kvp_arr){
        if(kvp_arr->kvp_array){
            for(uint32_t i = 0; i < kvp_arr->cur_size; i++){
                key_free(&kvp_arr->kvp_array[i].key);
                value_free(&kvp_arr->kvp_array[i].value);
            }
            free(kvp_arr->kvp_array);
        }
        kvp_arr->kvp_array = NULL;
        kvp_arr->cur_size = 0;
        kvp_arr->max_size = 0;
    }
}
/**************************************************************************************************** */
/*****************************TXN_DYNARR STUFF******************************************************* */

static errflag_t txn_dynarr_init(s_txn_dynarr *txn_arr, uint32_t size){
    def_err_handler(!txn_arr, "txn_dynarr_init txn_arr", ERR_NULL);
    txn_arr->cur_size = 0;
    txn_arr->max_size = size;

    txn_arr->txn_array = malloc(size*sizeof(byte_t));
    def_err_handler(!txn_arr->txn_array, "txn_dynarr_init txn_array", ERR_ALLOC);

    return ERR_OK;
}

static errflag_t txn_dynarr_realloc(s_txn_dynarr *txn_arr){
    def_err_handler(!txn_arr, "txn_dynarr_realloc txn_arr", ERR_NULL);

    byte_t *new_txn_array = realloc(txn_arr->txn_array, txn_arr->max_size*glob_txn_realloc_coeff*sizeof(byte_t));
    def_err_handler(!new_txn_array, "txn_dynarr_realloc new_txn_array", ERR_ALLOC);

    txn_arr->txn_array = new_txn_array;
    txn_arr->max_size *= glob_txn_realloc_coeff;
    return ERR_OK;
}

static errflag_t txn_dynarr_append(s_txn_dynarr *txn_arr, byte_t elem){
    def_err_handler(!txn_arr, "txn_dynarr_append txn_arr", ERR_NULL);
    def_err_handler(!txn_arr->txn_array, "txn_dynarr_append txn_array", ERR_NULL);

    if(txn_arr->cur_size == txn_arr->max_size){
        errflag_t failure = txn_dynarr_realloc(txn_arr);
        error_handler(failure, "txn_dynarr_append txn_dynarr_realloc", failure, return failure;);
    }

    txn_arr->txn_array[txn_arr->cur_size] = elem;
    txn_arr->cur_size++;
    return ERR_OK;
}

static errflag_t txn_dynarr_copy(s_txn_dynarr *txn_arr, byte_t* elements, uint32_t nb_bytes ){
    def_err_handler(!txn_arr, "txn_dynarr_copy txn arr", ERR_NULL);
    def_err_handler(!elements, "txn_dynarr_copy elements", ERR_NULL);

    while(txn_arr->cur_size + nb_bytes > txn_arr->max_size){
        errflag_t failure = txn_dynarr_realloc(txn_arr);
        error_handler(failure, "txn_dynarr_copy txn_dynarr_realloc", failure, return failure;);
    }

    memcpy(txn_arr->txn_array + txn_arr->cur_size, elements, nb_bytes);
    txn_arr->cur_size += nb_bytes;

    return ERR_OK;
}//not tested; copies nb_bytes from elements to txn_arr

static void txn_dynarr_free(s_txn_dynarr *txn_arr){
    if(txn_arr){
        if(txn_arr->txn_array)
            free(txn_arr->txn_array);
        txn_arr->txn_array = NULL;
        txn_arr->cur_size = 0;
        txn_arr->max_size = 0;
    }
}
/***************************************************************************************************/
/**********************************TRANSACTION STUFF************************************************/

errflag_t transaction_init(s_transaction* txn){
    def_err_handler(!txn, "transaction_init txn", ERR_NULL);

    errflag_t failure = kvp_dynarr_init(&txn->kvp_array, glob_kvp_dynarr_size);
    error_handler(failure, "transaction_init kvp_dynarr_init", failure, return failure;);

    failure = txn_dynarr_init(&txn->txn_array, glob_txn_dynarr_size);
    error_handler(failure, "transaction_init txn_dynarr_init", failure, return failure;);

    txn->start_time = TIMESTAMP_MAX;
    txn->end_time = 0;

    txn->flags = 0;


    return ERR_OK;
}//tested;ok

void transaction_free(s_transaction* txn){
    if(txn){

        if(txn->txn_array.txn_array)
            txn_dynarr_free(&txn->txn_array);
        if(txn->kvp_array.kvp_array)
            kvp_dynarr_free(&txn->kvp_array);
        txn->kvp_array = (s_kvp_dynarr){0};
        txn->txn_array = (s_txn_dynarr){0};
        txn->flags = 0;
    }
}//tested; ok

#define TXN_BEGIN 1
#define TXN_COMMIT 2
#define TXN_ABORT 4

/*these three should be trivial*/
errflag_t transaction_begin(s_transaction* txn, uint32_t txn_id){
    def_err_handler(!txn, "transaction_begin txn", ERR_NULL);
    def_err_handler(txn->flags, "transaction_begin illegal op", ERR_VALS);

    errflag_t failure = txn_dynarr_append(&txn->txn_array, OP_BEGIN);
    def_err_handler(failure, "transaction_begin txn_dynarr_append", failure);

    failure = txn_dynarr_append(&txn->txn_array, txn_id);
    def_err_handler(failure, "transaction_begin txn_dynarr_append", failure);

    txn->flags |= TXN_BEGIN;
    txn->txn_id = txn_id;

    return ERR_OK;
}//tested;ok

errflag_t transaction_commit(s_transaction* txn){
    def_err_handler(!txn, "transaction_commit txn", ERR_NULL);
    def_err_handler( (txn->flags & (TXN_COMMIT | TXN_ABORT)), "transaction_commit illegal op already ended", ERR_VALS);
    def_err_handler( !(txn->flags & TXN_BEGIN ), "transaction_commit illegal op no begin", ERR_VALS);

    errflag_t failure = txn_dynarr_append(&txn->txn_array, OP_COMMIT);
    error_handler(failure, "transaction_commit txn_dynarr_append", failure, return failure;);

    failure = txn_dynarr_append(&txn->txn_array, txn->txn_id);
    def_err_handler(failure, "transaction_commit txn_dynarr_append", failure);

    txn->flags |= TXN_COMMIT;

    return ERR_OK;
}// tested;ok

errflag_t transaction_abort(s_transaction* txn){
    def_err_handler(!txn, "transaction_abort txn", ERR_NULL);
    def_err_handler( (txn->flags & (TXN_COMMIT | TXN_ABORT)), "transaction_commit illegal op already ended", ERR_VALS);
    def_err_handler( !(txn->flags & TXN_BEGIN), "transaction_commit illegal op no begin", ERR_VALS);

    errflag_t failure = txn_dynarr_append(&txn->txn_array, OP_ABORT);
    error_handler(failure, "transaction_abort txn_dynarr_append", failure, return failure;);

    failure = txn_dynarr_append(&txn->txn_array, txn->txn_id);
    def_err_handler(failure, "transaction_abort txn_dynarr_append", failure);

    txn->flags |= TXN_ABORT;

    return ERR_OK;
}//not tested


static errflag_t init_barray(s_byte_array* barray, size_t max){
    def_err_handler(!barray, "init_barray barray", ERR_NULL);

    barray->data = calloc(max, sizeof(byte_t));
    def_err_handler(!barray->data, "init_barray barray->data", ERR_ALLOC);

    barray->cur = 0; 
    barray->max = max; 

    return ERR_OK;
}

static inline errflag_t txn_dynarr_append_kvp(s_txn_dynarr* txn_dynarr, s_key* key, s_value* value){

    //I should support a way to write the key and value to the transaction array as well
    //I will have to be careful. It shouldn't be hard (it's writing to a byte array after all) 
    //but if I get this wrong I will be in trouble

    //create a byte array from the key and value and copy it to the txn array
    //inneficient (uses the memory twice) but simple enough
    s_byte_array barray ; 
    init_barray(&barray, key->key_size + sizeof(timestamp_t) + sizeof(uint32_t) + //key size
                  value->value_size + sizeof(value_as) + sizeof(uint32_t) + sizeof(timestamp_t) );
   

    errflag_t failure = key_to_byte_array(key, &barray);
    def_err_handler(failure, "txn_dynarr_append_kvp key_to_byte_array", failure);

    failure = value_to_byte_array(value, &key->ts, &barray);
    def_err_handler(failure, "txn_dynarr_append_kvp value_to_byte_array", failure);

    failure = txn_dynarr_copy(txn_dynarr, barray.data, barray.cur);
    def_err_handler(failure, "txn_dynarr_append_kvp txn_dynarr_copy", failure);

    free(barray.data);

    return ERR_OK;
}

static inline errflag_t txn_dynnar_append_key(s_txn_dynarr* txn_dynarr, s_key* key){
    
    s_byte_array barray ; 
    init_barray(&barray, key->key_size + sizeof(timestamp_t) + sizeof(uint32_t) ); //key size   

    errflag_t failure = key_to_byte_array(key, &barray);
    def_err_handler(failure, "txn_dynarr_append_key key_to_byte_array", failure);
    
    failure = txn_dynarr_copy(txn_dynarr, barray.data, barray.cur);
    def_err_handler(failure, "txn_dynarr_append_key txn_dynarr_copy", failure);

    free(barray.data);

    return ERR_OK;
}

/******these are gonna be a pain*/
errflag_t transaction_insert(s_transaction* txn, s_key* key, s_value* value){
    def_err_handler(!txn, "transaction_insert txn", ERR_NULL);
    def_err_handler(!key, "transaction_insert key", ERR_NULL);
    def_err_handler(!value, "transaction_insert value", ERR_NULL);
    def_err_handler(!(txn->flags & TXN_BEGIN), "transaction_insert illegal op no begin", ERR_VALS);
    def_err_handler( (txn->flags & (TXN_COMMIT | TXN_ABORT)), "transaction_commit illegal op already ended", ERR_VALS);

    errflag_t failure = kvp_dynarr_append(&txn->kvp_array, key, value);
    def_err_handler(failure, "transaction_insert kvp_dynarr_append", failure);

    failure = txn_dynarr_append(&txn->txn_array, OP_INSERT);
    def_err_handler(failure, "transaction_insert kvp_dynarr_append", failure);

    failure = txn_dynarr_append(&txn->txn_array, txn->txn_id);
    def_err_handler(failure, "transaction_insert txn_dynarr_append", failure);

    failure = txn_dynarr_append_kvp(&txn->txn_array, key, value);
    def_err_handler(failure, "append_kvp_txndynarr", failure);

    //update the start and end time of the transaction
    //if need be
    if(key->ts > txn->end_time)
        txn->end_time = key->ts;
    if(key->ts < txn->start_time)
        txn->start_time = key->ts;

    return ERR_OK;
}//done; seems to work 



errflag_t transaction_lookup(s_transaction* txn, s_key* key, s_value* value){
    def_err_handler(!txn, "transaction_lookup txn", ERR_NULL);
    def_err_handler(!key, "transaction_lookup key", ERR_NULL);
    def_err_handler(!value, "transaction_lookup value", ERR_NULL);
    def_err_handler(!(txn->flags & TXN_BEGIN), "transaction_lookup illegal op no begin", ERR_VALS);
    def_err_handler( (txn->flags & (TXN_COMMIT | TXN_ABORT)), "transaction_commit illegal op already ended", ERR_VALS);

    errflag_t failure = txn_dynarr_append(&txn->txn_array, OP_LOOKUP); 
    def_err_handler(failure, "transaction_lookup txn_dynarr_append OP_LOOKUP", failure);

    failure = txn_dynarr_append(&txn->txn_array, txn->txn_id);
    def_err_handler(failure, "transaction_lookup txn_dynarr_append", failure);

    s_kvp* key_val_arr = txn->kvp_array.kvp_array;
    for(uint32_t i = 0; i < txn->kvp_array.cur_size ; i++){
        if( key->key_size == key_val_arr[i].key.key_size){
            if(!strncmp(key_val_arr[i].key.key, key->key, key->key_size)) {
                failure = value_dup(&(key_val_arr[i].value), value);
                def_err_handler(failure, "transaction_lookup value_dup", failure);
                return  ERR_OK;
            }
        }
    }

    value->value_size = 0 ; 
    value->as = UNKNOWKN ;
    value->val.u64 = 0 ;

    return ERR_OK;
}//seems to work

errflag_t transaction_remove(s_transaction* txn, s_key* key){
    
    errflag_t failure = txn_dynarr_append(&txn->txn_array, OP_REMOVE); 
    def_err_handler(failure, "transaction_remove txn_dynarr_append OP_REMOVE", failure);

    failure = txn_dynarr_append(&txn->txn_array, txn->txn_id); 
    def_err_handler(failure, "transaction_remove txn_dynarr_append txn_id", failure);

    failure = txn_dynnar_append_key(&txn->txn_array, key);
    def_err_handler(failure, "transaction_remove txn_dynarr_append_key", failure);

    s_kvp* kvp_arr = txn->kvp_array.kvp_array ;
    for(uint32_t i = 0 ; i < txn->kvp_array.cur_size; i++){
        if( key->key_size == kvp_arr[i].key.key_size){
            if((!strncmp(kvp_arr[i].key.key, key->key, key->key_size)) ){
                kvp_arr[i].key.key_size = 0 ; //A key with a length of zero will 
                //never work bc an 0 sized string doesn't really make sense tbh 
                //if this doesn't work; I guess I'll do some memmove thing idk
                return ERR_OK;
            }
        }
    }
    return ERR_OK;
}//tested; might be wrong ; kinda odd

errflag_t transaction_update(s_transaction* txn, s_key* key, s_value* value){
    def_err_handler(!txn, "transaction_update txn", ERR_NULL);
    def_err_handler(!key, "transaction_update key", ERR_NULL);
    def_err_handler(!value, "transaction_update value", ERR_NULL);
    def_err_handler(!(txn->flags & TXN_BEGIN), "transaction_update illegal op no begin", ERR_VALS);
    def_err_handler( (txn->flags & (TXN_COMMIT | TXN_ABORT)), "transaction_commit illegal op already ended", ERR_VALS);

    errflag_t failure = txn_dynarr_append(&txn->txn_array, OP_UPDATE); 
    def_err_handler(failure, "transaction_update txn_dynarr_append OP_LOOKUP", failure);

    failure = txn_dynarr_append(&txn->txn_array, txn->txn_id);
    def_err_handler(failure, "transaction_update txn_dynarr_append", failure);   

    s_kvp* kvp_arr = txn->kvp_array.kvp_array ;
    for(uint32_t i = 0 ; i < txn->kvp_array.cur_size; i++){
        if( key->key_size == kvp_arr[i].key.key_size){
            if((!strncmp(kvp_arr[i].key.key, key->key, key->key_size)) ){
               
                failure = txn_dynarr_append_kvp(&txn->txn_array, key, value);
                def_err_handler(failure, "transaction_update txn_dynarr_append_kvp", failure);   

                kvp_arr[i].key.ts = key->ts ;
                
                failure = value_free(&kvp_arr[i].value);
                def_err_handler(failure, "transaction_update value_free", failure);   

                failure = value_dup(value, &kvp_arr[i].value);//unsure abt that
                def_err_handler(failure, "transaction_update value_dup", failure);

                return ERR_OK;
            }
        }
    }

    //behaves like insert on non existing key ; unsure if this is good behavior
    failure = txn_dynarr_append_kvp(&txn->txn_array, key, value);
    def_err_handler(failure, "transaction_update txn_dynarr_append_kvp", failure); 

    failure = kvp_dynarr_append(&txn->kvp_array, key, value);
    def_err_handler(failure, "transaction_update kvp_dynarr_append", failure); 

    return ERR_OK;
}// seems to work

/***************************************************************************************************/

#ifdef debug 
void transaction_print(s_transaction* txn){
    if(txn){
        printf("Transaction:\n");
        printf("flags: %x\n", txn->flags);
        printf("KVP Array:\n");
        printf("Cur Size: %d\n", txn->kvp_array.cur_size);
        for(uint32_t i = 0; i < txn->kvp_array.cur_size; i++){
            printf("Key %d:\n", i);
            key_print(&txn->kvp_array.kvp_array[i].key);
            printf("Value %d:\n", i);
            value_print(&txn->kvp_array.kvp_array[i].value);
        }
        printf("Transaction Array:\n");
        printf("Cur Size: %d\n", txn->txn_array.cur_size);
        for(uint32_t i = 0; i < txn->txn_array.cur_size; i++){
            printf("%2.2x",  txn->txn_array.txn_array[i]);
        }
        printf("\n");
    }
}
#endif
