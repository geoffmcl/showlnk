// sl_load.hxx
#ifndef _SL_LOAD_HXX_
#define _SL_LOAD_HXX_

extern lnkfileflag get_last_error();
extern void set_last_error( lnkfileflag flag );
extern int showlnk_unload( char * file );
extern char *showlnk_load( char * file, int verb = 0 );
extern size_t get_file_size();


#endif // #ifndef _SL_LOAD_HXX_
// eof - sl_load.hxx
