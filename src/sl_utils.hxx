// sl_utils.hxx
#ifndef _SL_UTILS_HXX_
#define _SL_UTILS_HXX_
#include <Windows.h>
#include <string>
#include <vector>
typedef std::vector<std::string> vSTG;

#ifndef PATH_SEP
#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
#endif

extern const char *pszLinkCLSID; // = "00021401-0000-0000-C000-000000000046";
extern const unsigned char lnkCLSID[16]; // = { 0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 };

#define ShellLinkHeaderSize 0x4c

extern int is_file_or_directory( char *file );
extern size_t get_last_file_size();

//
// Dump a region of memory in a hexadecimal format
//
extern void HexDump(PBYTE ptr, DWORD length, bool addhdr = false, bool addascii = true, bool addspace = true);
// output a 32-bit value in binary
extern void show_bits32( unsigned int flag, bool showleadingzeros = false );
extern void Show_FileTime(FILETIME *pft);
extern void show_file_atts( unsigned int att );

extern int has_wild( char *file );
extern char *get_filename_from_path( char *path );
extern char *get_path_from_filename( char *file );

extern char *set_logfile();

extern vSTG string_split(const std::string &str, const char *sep = 0, int maxsplit = 0);

extern char *fix_relative_path(const char *path, char *buf);


#endif // _SL_UTILS_HXX_
// eof - sl_utils.cxx
