#ifndef ER_ERFLAGS_H 
#define ER_ERFLAGS_H

#include "common.h"
//from https://github.com/K-avi/edgerunner/blob/main/src/misc.h

typedef uint8_t errflag_t; //value to know wether a function encountered a problem , 0 ok, {1..255} -> error code
/*
the enum of flags ; I might add stuff to it at some point.
*/
typedef enum errflag_tS{   
    ERR_OK = 0, ERR_NULL , ERR_ALLOC, ERR_REALLOC, ERR_VALS, ERR_NOTNULL,  
    ERR_PTHREAD_INIT, ERR_INVALID, ERR_MAPFULL, ERR_NOTFOUND,
}flags ;


#ifdef debug //if not compiling with debug don't use the error reports to be more efficient

/*functions and macros*/
extern void er_report( FILE * flux, const char * repport_msg, const char * error_msg , errflag_t flag);
/*
    flux -> not null 
    repport_msg -> not null 
    error_msg -> not null 
    flag -> 

    reports an error in the flux given. 

    Some wrappers are defined around this function to avoid code redundancy
*/

//some wrappers arround the er_report function ; I recommand sticking to the def_err/war_handlers 
//bc passing a code block to a macro is very much not safe lol
#define error_handler(cond, msg,flag,handler_code) if((cond)){er_report(stderr, "error", (msg), (flag)); {handler_code;} return (flag);}
/*
    the error_handler macro function checks if cond==TRUE, reports the error 
    associated with flag if it's the case and executes hanlder_code before returning the value of flag.
*/

#define warning_handler(cond,msg,flag, handler_code) if((cond)){er_report(stderr, "warning", (msg), (flag)); {handler_code;}}
/*
same as error_hanler but doesn't return anything
*/
//I really like these macros I just think they're neat the C preprocessor is crazy for this ngl 
/*
    the default macro functions are safer / easier to use. I recommand sticking to them. 
    they don't execute any code and just report the error / warning if cond is true.
*/
#define def_err_handler(cond,msg,flag) error_handler(cond,msg,flag,;)
#define def_war_handler(cond,msg,flag) warning_handler(cond,msg,flag,;)

#else //ifndef debug

#define error_handler(cond, msg,flag,handler_code) ;
#define warning_handler(cond,msg,flag, handler_code) ;
#define def_err_handler(cond,msg,flag) ;
#define def_war_handler(cond,msg,flag) ;

#endif


#endif 
