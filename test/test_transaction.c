#include "../src/transaction.h"

int main(void){
    s_transaction txn;
    transaction_init(&txn);

    transaction_begin(&txn); 
    transaction_commit(&txn);
    
    transaction_print(&txn);

    transaction_free(&txn);
    return 0 ; 
}
