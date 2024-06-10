#ifndef cdb_timestamp_h
#define cdb_timestamp_h

#include "common.h"
#include "err_handler.h"


typedef uint64_t timestamp_t;

typedef struct s_timestamp_generator{
    _Atomic timestamp_t current_time;
}s_timegen; //a timestamp generator is simply an atomic integer that is incremented each time a new timestamp is requested

errflag_t init_timestamp_gen(s_timegen* generator); 
/*
non null generator pointer -> errflag_t
initializes the timestamp generator 
*/

errflag_t get_timestamp(s_timegen* generator, timestamp_t* ts);
/*
non null & initialized generator , non null timestamp 
returns a new timestamp in ts and increments the current time in generator
(thread safe)
*/

errflag_t peek_timestamp(s_timegen* generator, timestamp_t* ts);
/*
non null & initialized generator , non null timestamp
returns the current timestamp in ts 
(thread safe)
*/

errflag_t free_timestamp_gen(s_timegen* generator);
/*
destroys the content of the generator
(not thread safe)
*/

#endif 
