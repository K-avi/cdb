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


    s_value val_lookupu64;
    transaction_lookup(&txn, &key_u64, &val_lookupu64);

    printf("val u64 before update\n");
    value_print(&val_lookupu64);
    printf("========\n");

    s_value val_lookupstr;
    transaction_lookup(&txn, &key_str, &val_lookupstr);


    s_value new_val ; 
    new_val.val.u64 = 10 ; 
    new_val.as = U64;
    new_val.value_size = sizeof(U64);

    transaction_update(&txn, &key_str,&new_val);

    value_free(&val_lookupu64);
    transaction_lookup(&txn, &key_str, &val_lookupu64);

    printf("val u64 after update\n");
    value_print(&val_lookupu64);
    printf("========\n");

    transaction_remove(&txn, &key_u64);
    transaction_lookup(&txn,  &key_u64, &val_lookupu64);

    transaction_commit(&txn);

    transaction_print(&txn);

    value_print(&val_lookupu64);
    printf("========\n");
    value_print(&val_u64);

    printf("testing illegal commit : \n");
    transaction_commit(&txn);
    
    transaction_print(&txn);

    value_print(&val_lookupstr);
    printf("========\n");
    value_print(&val_str);

    value_free(&val_lookupu64);
    value_free(&val_lookupstr);
    transaction_free(&txn);
    return 0 ; 
}
