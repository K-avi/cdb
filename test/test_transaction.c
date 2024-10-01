#include "../src/transaction.h"
#include <stdint.h>
#include <sys/types.h>
#include <string.h>

int main(void){
    s_transaction txn;
    transaction_init(&txn);

    transaction_begin(&txn,12); 

    s_key key_u64; 
    key_u64.key = "key_test1" ;
    key_u64.key_size = strlen(key_u64.key);
    key_u64.ts = 1 ;
  
    s_value val_u64 ; 
    val_u64.val.u64 = 4; 
    val_u64.as = U64 ;
    val_u64.value_size = sizeof(uint64_t);

    transaction_insert(&txn, &key_u64, &val_u64);

    
    s_key key_str; 
    key_str.key = "key_test2" ;
    key_str.key_size = strlen(key_u64.key);
    key_str.ts = 1 ;
    
    s_value val_str ; 
    val_str.val.str = "value_test2"; 
    val_str.as = STR ;
    val_str.value_size = strlen(val_str.val.str);

    transaction_insert(&txn, &key_str, &val_str);


    s_value val_lookup1;
    transaction_lookup(&txn, &key_u64, &val_lookup1);
    transaction_commit(&txn);
    
    transaction_print(&txn);

    value_print(&val_lookup1);
    printf("========\n");
    value_print(&val_u64);

    transaction_free(&txn);
    return 0 ; 
}
