// sl_load.cxx

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "showlnk2.hxx"
#include "sl_utils.hxx"
#include "sl_load.hxx"
#include "sprtf.hxx"

lnkfileflag last_error = is_ms_lnk;
static size_t sl_size = 0;
static FILE *sl_fp = 0;     // is not normally left open
static char *sl_buf = 0;    // allocated with 'new', so use delete to free
static char *sl_file = 0;   // allocated with strdup() which uses malloc, so use free to remove

lnkfileflag get_last_error() { return last_error; }
void set_last_error( lnkfileflag flag ) { last_error = flag; }

size_t get_file_size() { return sl_size; }

int showlnk_unload( char * file )
{
    if (sl_file && strcmp(file,sl_file)) {
        return 1;   // not current file
    }
    if (sl_file)
        free(sl_file);
    sl_file = 0;
    if (sl_buf)
        delete sl_buf;
    sl_buf = 0;
    sl_size = 0;
    if (sl_fp)
        fclose(sl_fp);
    sl_fp = 0;
    return 0;
}

char *showlnk_load( char * file, int verb )
{
    if (file == 0) {
        last_error = no_file_given;
        return 0;
    }
    if (sl_file && sl_buf) {
        if (strcmp(file,sl_file)) {
            // new file
            showlnk_unload(sl_file);
        } else {
            if (verb > 1) SPRTF("Returning previous loaded buffer\n");
            return sl_buf;
        }
    }
    if (is_file_or_directory(file) != 1) {
        if (verb) SPRTF("Error: Can NOT 'stat' file [%s]!\n", file);
        last_error = no_stat;
        return 0;
    }
    sl_size = get_last_file_size();
    if (sl_size < 0x4c) {
        if (verb) SPRTF("Error: File [%s] size %ld less than minimum %ld!\n", file, sl_size, 0x4c);
        last_error = bad_size;
        return 0;
    }
    sl_fp = fopen(file,"rb");
    if (!sl_fp) {
        if (verb) SPRTF("Error: Unable to OPEN File [%s]!\n", file);
        last_error = no_open;
        return 0;
    }
    sl_buf = new char[sl_size];
    if (!sl_buf) {
        if (verb) SPRTF("Error: Memory allocation of %ld FAILED!\n", sl_size);
        showlnk_unload(file);
        last_error = no_memory;
        return 0;
    }
    if (sl_file)
        free(sl_file);
    sl_file = _strdup(file);    // make copy of file name
    size_t res = fread(sl_buf,1,sl_size,sl_fp);
    fclose(sl_fp);
    sl_fp = 0;
    if (res != sl_size) {
        if (verb) SPRTF("Error: Read of %ld bytes from file %s FAILED! read %ld\n", sl_size, file, res);
        showlnk_unload(file);
        last_error = rd_hdr_failed;
        return 0;
    }
    last_error = is_ms_lnk;
    return sl_buf;
}

// eof - sl_load.cxx
