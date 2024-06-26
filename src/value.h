#ifndef cdb_value_h 
#define cdb_value_h

#include "common.h"
#include "err_handler.h"


typedef union u_value{
    char* str; 
    uint64_t u64;
}u_value; 
/*
union used for the types ; atm a value 
can be a string or integer but you could 
add support for more types
*/

typedef enum value_as{
    UNKNOWKN=0,
    STR,
    U64
}value_as;
/*
simple enum for the types
*/


typedef struct s_value{
    u_value val;
    value_as as; //maybe changing the order is better for the weird padding stuff idk 
} s_value;
//a value is simply an union containing the data, and the type you should cast the data as
//my issue with using strings is that you store a pointer and not the data but it shouldn't be too much 
//of a problem


errflag_t value_init(u_value val,  value_as as, s_value *value_struct);
/*
@param: val -> initialized u_value (of whatever type)
@param: as -> the type of the value 
@param: value_struct -> non null ; possible memleak on already initialized struct

@brief: initializes the fields of value_struct with a copy of val it's type
*/

errflag_t value_free(s_value *value_struct);
/*
@param: any value pointer
free the fields of value_struct (if need be) and zeroes them 
*/

errflag_t value_dup(s_value *src, s_value *dst); 
/*
@params: src -> non null ; initialized
@params: dst -> non null 
@returns: ERR_OK if successful, ERR_NULL if src or dst is NULL

@brief : duplicates the value in src into dst; might allocate memory
*/

#ifdef debug 
void value_print(s_value *value_struct);
/*
prints value to stdout
*/
#endif //debug 

#endif
