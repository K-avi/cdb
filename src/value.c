#include "value.h"
#include "common.h"
#include "err_handler.h"
#include <string.h>

errflag_t value_init(u_value val,  value_as as, s_value *value_struct){
    def_err_handler(!value_struct, "value_init value_struct", ERR_NULL);
    
    value_struct->val = val;
    value_struct->as = as;
    
    switch(as){
        case  UNKNOWKN:
            value_struct->value_size = 0;
            break;
        case STR:
            value_struct->value_size = strlen(val.str);
            break;
        case U64:
            value_struct->value_size = sizeof(uint64_t);
            break;
        default: 
            def_err_handler(ERR_VALS, "value_init", ERR_VALS); 
    }
    
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


typedef u_value (*dup_value_fn)(u_value value_struct) ;

static u_value dup_unknwon_value(u_value value_struct){return value_struct;}
static u_value dup_u64_value(u_value value_struct){return value_struct;}
static u_value dup_str_value(u_value value_struct){
    u_value ret; 
    ret.str = strdup(value_struct.str);
    return ret;
}

static dup_value_fn dup_tab[] = {
    dup_unknwon_value,
    dup_str_value,
    dup_u64_value,
};//static tab with each value duplication function; the index is the enum value of the type

errflag_t value_dup(s_value *src, s_value *dst){
    def_err_handler(!src, "value_dup src", ERR_NULL);
    def_err_handler(!dst, "value_dup dst", ERR_NULL);
    
    dst->as = src->as;
    dst->val = dup_tab[src->as](src->val);
    dst->value_size = src->value_size;
    
    return ERR_OK;
}//tested; ok; just a simple copy of the struct 

errflag_t value_to_byte_array(s_value* value, s_byte_array* barray){
    def_err_handler(!value, "value_to_byte_array value", ERR_NULL);
    def_err_handler(!barray, "value_to_byte_array barray", ERR_NULL);
    
    uint32_t size = value->value_size + sizeof(value_as) + sizeof(uint32_t);
    def_err_handler(size > barray->max, "value_to_byte_array size", ERR_VALS);

    barray->cur = size;

    memcpy(barray->data, &value->as, sizeof(value_as));
    memcpy(barray->data + sizeof(value_as), &value->value_size, sizeof(uint32_t));

    switch (value->as) {
        case STR:
            memcpy(barray->data + sizeof(value_as) + sizeof(uint32_t), value->val.str, value->value_size);
            break;
        case U64:
            memcpy(barray->data + sizeof(value_as) + sizeof(uint32_t), &value->val.u64, value->value_size);
            break;
        case UNKNOWKN:
            break;
        default:
            def_err_handler(ERR_VALS, "value_to_byte_array", ERR_VALS);
    }
    return ERR_OK;
}//not tested; might be wrong

errflag_t value_from_byte_array(s_value* value, s_byte_array* barray){
    def_err_handler(!value, "value_from_byte_array value", ERR_NULL);
    def_err_handler(!barray, "value_from_byte_array barray", ERR_NULL);
    
    memcpy(&value->as, barray->data, sizeof(value_as));
    memcpy(&value->value_size, barray->data + sizeof(value_as), sizeof(uint32_t));

    switch (value->as) {
        case STR:
            value->val.str = strndup((char*)barray->data + sizeof(value_as) + sizeof(uint32_t), value->value_size);
            break;
        case U64:
            memcpy(&value->val.u64, barray->data + sizeof(value_as) + sizeof(uint32_t), value->value_size);
            break;
        case UNKNOWKN:
            break;
        default:
            def_err_handler(ERR_VALS, "value_from_byte_array", ERR_VALS);
    }
    return ERR_OK;
}//not tested; might be wrong


#ifdef debug 
void value_print(s_value *value_struct){
    if(!value_struct){
        printf("NULL\n");
        return;
    }
    printf("value: ");
    switch(value_struct->as){
        case UNKNOWKN:
            printf("UNKNOWN\n");
            break;
        case STR:
            printf("str: %s\n", value_struct->val.str);
            break;
        case U64:
            printf("u64: %lu\n", value_struct->val.u64);
            break;
    }
    return;
}//tested;ok;
#endif //debug 
