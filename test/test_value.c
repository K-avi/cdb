//#include "../src/common.h"
#include "../src/value.h"
#include <string.h>
int main(void){
    s_value val;

   
    u_value v; 
    v.str =  strdup("hello");

    value_init(v, STR, &val);
    value_print(&val);
    value_free(&val);


    u_value v2;
    v2.u64 = 12;
    value_init(v2, U64, &val);
    value_print(&val);
    value_free(&val);

    u_value v3;
    v3.u64 = 12;
    value_init(v3, UNKNOWKN, &val);
    value_print(&val);
    value_free(&val);
    
    return 0 ; 
}
