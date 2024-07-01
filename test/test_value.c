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

    timestamp_t tsrc = 1 , tdst;

    s_byte_array barray;

    barray.cur = 0; 
    barray.max = 64; 
    barray.data = malloc(64*sizeof(byte_t));
    
    v.str = strdup("hello");
    value_init(v, STR, &val);
    value_to_byte_array(&val,  &tsrc, &barray);
    value_free(&val);

    s_value val2;
    
    value_from_byte_array(&val2, &tdst, &barray);
    value_print(&val2);
    value_free(&val2);

    memset(barray.data, 0, 64);
    barray.cur=0;

    value_init(v2, U64, &val);
    value_to_byte_array(&val,  &tsrc, &barray);
    value_free(&val);

    value_from_byte_array(&val2, &tdst, &barray);
    value_print(&val2);
    value_free(&val2);

    free(barray.data);
    return 0 ; 
}
