#include "value.h"
#include "err_handler.h"

errflag_t value_init(u_value val,  value_as as, s_value *value_struct){
    def_err_handler(!value_struct, "value_init value_struct", ERR_NULL);
    
    value_struct->val = val;
    value_struct->as = as;
    
    return ERR_OK;
}//tested; ok;

typedef void (*free_value_fn)(u_value value_struct) ;
//typedef for function pointer of value destruction functions

static void free_unknwon_value(u_value value_struct){return;}
static void free_u64_value(u_value value_struct){return;}
static void free_str_value(u_value value_struct){ free(value_struct.str); return;}
//free functions for each type in the union ; more complex types can be supported by 
//adding the function here, the fn ptr in free_tab and a new element in the enum

static free_value_fn free_tab[] = {
    free_unknwon_value,
    free_str_value,
    free_u64_value,
};//static tab with each value destruction function; the index is the enum value of the type 


errflag_t value_free(s_value *value_struct){
    def_err_handler(!value_struct, "value_destroy", ERR_NULL);
    free_tab[value_struct->as]((value_struct->val));
    return ERR_OK;
}//tested; ok; basically just a wrapper for the function pointer

#ifdef debug 
void value_print(s_value *value_struct){
    if(!value_struct){
        printf("NULL\n");
        return;
    }
    switch(value_struct->as){
        case UNKNOWKN:
            printf("UNKNOWN\n");
            break;
        case STR:
            printf("%s\n", value_struct->val.str);
            break;
        case U64:
            printf("%lu\n", value_struct->val.u64);
            break;
    }
    return;
}//tested;ok;
#endif //debug 
