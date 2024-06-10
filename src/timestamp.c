#include "timestamp.h"
#include "err_handler.h"
#include <stdatomic.h>


errflag_t init_timestamp_gen(s_timegen* generator){
    def_err_handler(!generator, "init_timestamp_gen", ERR_NULL);
    generator->current_time = 0;

    return ERR_OK;
}//not tested

errflag_t get_timestamp(s_timegen* generator, timestamp_t* ts){
    def_err_handler(!generator, "get_timestamp", ERR_NULL);
    def_err_handler(!ts, "get_timestamp", ERR_NULL);
    
    *ts = atomic_fetch_add(&(generator->current_time), 1);
    
    return ERR_OK;
}//not tested 


errflag_t peek_timestamp(s_timegen* generator, timestamp_t* ts){
    def_err_handler(!generator, "peek_timestamp", ERR_NULL);
    def_err_handler(!ts, "peek_timestamp", ERR_NULL);
    
    *ts = atomic_load(&(generator->current_time));
    
    return ERR_OK;
}//not tested


errflag_t free_timestamp_gen(s_timegen* generator){
    def_err_handler(!generator, "free_timestamp_gen", ERR_NULL);
    
    generator->current_time = 0;
    
    return ERR_OK;
}//not tested
//this function is somewhat awkward sicne the generator doesn't 
//allocate any memory idk if I'll keep it around (maybe for future use if I implement vector clocks or smtg)

