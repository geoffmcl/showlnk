// sl_utils.cxx
#pragma warning( disable : 4995 ) // '...': name was marked as #pragma deprecated
#include <sys/types.h>
#include <sys/stat.h>
#include <Windows.h>
#include <stdio.h>
#include <string>
#include "sprtf.hxx"
#include "sl_utils.hxx"

const char *pszLinkCLSID = "00021401-0000-0000-C000-000000000046";
//                                     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16
const unsigned char lnkCLSID[16] = { 0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 };


static struct stat stat_buf;
// return: 0 = not file or directory, 1 = file, 2 = directory, 
int is_file_or_directory( char *file )
{
    int iret = 0;
    if (stat(file,&stat_buf) == 0) {
        if (stat_buf.st_mode &  _S_IFDIR) 
            iret = 2;
        else
            iret = 1;
    }
    return iret;
}

size_t get_last_file_size() { return stat_buf.st_size; }

/////////////////////////////////////////////////////////////////////////
// Number of hex values displayed per line
#define HEX_DUMP_WIDTH 16
//
// Dump a region of memory in a hexadecimal format
//
void HexDump(PBYTE ptr, DWORD length, bool addhdr, bool addascii, bool addspace)
{
    char buffer[256];
    PSTR buffPtr, buffPtr2;
    unsigned cOutput, i;
    DWORD bytesToGo = length;

    while ( bytesToGo  )
    {
        memset(buffer,0,256);
        cOutput = bytesToGo >= HEX_DUMP_WIDTH ? HEX_DUMP_WIDTH : bytesToGo;

#if 0 // chop this
        if( Try_HD_Width( ptr, cOutput ) )
        {
           sprtf( "WARNING: Abandoning HEX DUMP, bad ptr %p, length %d!\n",
               ptr, cOutput );
            return;
        }
        if (add_2_ptrlist(ptr)) {
            bytesToGo -= cOutput;
            ptr += HEX_DUMP_WIDTH;
            continue;
        }
#endif // 0 - chopped code

        buffPtr = buffer;
        if (addhdr) {
            buffPtr += sprintf(buffPtr, "%08X:  ", length-bytesToGo );
        }
        buffPtr2 = buffPtr + (HEX_DUMP_WIDTH * 3) + 1;
        *buffPtr2 = 0;

        for ( i=0; i < HEX_DUMP_WIDTH; i++ )
        {
            BYTE value = *(ptr+i);

            if ( i >= cOutput )
            {
                // On last line.  Pad with spaces
                *buffPtr++ = ' ';
                *buffPtr++ = ' ';
                *buffPtr++ = ' ';
            }
            else
            {
                if ( value < 0x10 )
                {
                    *buffPtr++ = '0';
                    _itoa( value, buffPtr++, 16);
                }
                else
                {
                    _itoa( value, buffPtr, 16);
                    buffPtr+=2;
                }
 
                *buffPtr++ = ' ';
                if (addascii) {
                    *buffPtr2++ = isprint(value) ? value : '.';
                    if ( value == '%' )
                       *buffPtr2++ = '%';  // insert another
                }
            }
            
            // Put an extra space between the 1st and 2nd half of the bytes
            // on each line.
            if (addspace) {
                if ( i == (HEX_DUMP_WIDTH/2)-1 )
                    *buffPtr++ = ' ';
            }
        }

        *buffPtr2 = 0;  // Null terminate it.
        //puts(buffer);   // Can't use sprtf(), since there may be a '%'
                        // in the string.
        SPRTF("%s\n", buffer);   // Have add 2nd % if one found in ASCII

        bytesToGo -= cOutput;
        ptr += HEX_DUMP_WIDTH;
    }
}

////////////////////////////////////////////////////////////////////////////

void show_bits32( unsigned int flag, bool showleadingzeros )
{
    if ((flag == 0) && !showleadingzeros) {
        sprtf("0");
    } else {
        int i;
        unsigned int bit = 0x80000000;
        bool had_bit = false;
        for (i = 0; i < 32; i++) {
            if (flag & bit) {
                if (i && !had_bit) {
                    sprtf("0");
                }
                sprtf("1");
                had_bit = true;
            } else {
                if (showleadingzeros || had_bit) {
                    sprtf("0");
                }
            }
            bit = bit >> 1;
        }
    }
    sprtf("b ");
}

void Show_FileTime(FILETIME *pft)
{
    SYSTEMTIME stUTC, stLocal;

    // Convert the last-write time to local time.
    FileTimeToSystemTime(pft, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // Build a string showing the date and time.
    sprtf("%02d/%02d/%d  %02d:%02d",
        stLocal.wMonth, stLocal.wDay, stLocal.wYear,
        stLocal.wHour, stLocal.wMinute);
}

typedef struct tagFILEATTS {
    unsigned int bit;
    const char *name;
    const char *desc;
}FILEATTS, *PFILEATTS;

static FILEATTS fileatts[] = {
    { FILE_ATTRIBUTE_READONLY, "FILE_ATTRIBUTE_READONLY", "The file or directory is read-only. For a file, if this bit is set, applications can read the file but cannot write to it or delete it. For a directory, if this bit is set, applications cannot delete the directory." },
    { FILE_ATTRIBUTE_HIDDEN,   "FILE_ATTRIBUTE_HIDDEN", "The file or directory is hidden. If this bit is set, the file or folder is not included in an ordinary directory listing." },
    { FILE_ATTRIBUTE_SYSTEM,   "FILE_ATTRIBUTE_SYSTEM", "The file or directory is part of the operating system or is used exclusively by the operating system." },
    // { reserved1 - A bit that MUST be zero.
    { FILE_ATTRIBUTE_DIRECTORY,"FILE_ATTRIBUTE_DIRECTORY", "The link target is a directory instead of a file." },
    { FILE_ATTRIBUTE_ARCHIVE,  "FILE_ATTRIBUTE_ARCHIVE" , "The file or directory is an archive file. Applications use this flag to mark files for backup or removal." },
    // { Reserved2 - A bit that MUST be zero.
    { FILE_ATTRIBUTE_NORMAL,   "FILE_ATTRIBUTE_NORMAL", "The file or directory has no other flags set. If this bit is 1, all other bits in this structure MUST be clear." },
    { FILE_ATTRIBUTE_TEMPORARY,"FILE_ATTRIBUTE_TEMPORARY","The file is being used for temporary storage." },
    { FILE_ATTRIBUTE_SPARSE_FILE,"FILE_ATTRIBUTE_SPARSE_FILE","The file is a sparse file." },
    { FILE_ATTRIBUTE_REPARSE_POINT, "FILE_ATTRIBUTE_REPARSE_POINT", "The file or directory has an associated reparse point." },
    { FILE_ATTRIBUTE_COMPRESSED, "FILE_ATTRIBUTE_COMPRESSED", "The file or directory is compressed. For a file, this means that all data in the file is compressed. For a directory, this means that compression is the default for newly created files and subdirectories." },
    { FILE_ATTRIBUTE_OFFLINE, "FILE_ATTRIBUTE_OFFLINE", "The data of the file is not immediately available." },
    { FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, "FILE_ATTRIBUTE_NOT_CONTENT_INDEXED", "The contents of the file need to be indexed." },
    { FILE_ATTRIBUTE_ENCRYPTED, "FILE_ATTRIBUTE_ENCRYPTED", "The file or directory is encrypted. For a file, this means that all data in the file is encrypted. For a directory, this means that encryption is the default for newly created files and subdirectories." },
    { 0,                        0,                          0 }
};

void show_file_atts( unsigned int att )
{
    PFILEATTS pfa = &fileatts[0];
    sprtf("FileAttributes: %#x ", att );
    show_bits32(att);
    while (pfa->name) {
        if (pfa->bit & att) 
            sprtf("%s ", pfa->name);
        pfa++;
    };
    sprtf("\n");
}

int has_wild( char *file )
{
    size_t len = strlen(file);
    size_t ii;
    int c;
    for (ii = 0; ii < len; ii++) {
        c = file[ii];
        if (c == '?')
            return 1;
        if (c == '*')
            return 1;
    }
    return 0;
}

char *get_filename_from_path( char *path )
{
    size_t len = strlen(path);
    size_t ii, i2;
    char *name = path;
    int c;
    for (ii = 0; ii < len; ii++) {
        i2 = ii + 1;
        c = path[ii];
        if ((c == '\\') || (c == '/')) {
            if (i2 < len) {
                name = &path[i2];
            }
        }
    }
    return name;
}
char *get_path_from_filename( char *file )
{
    int len = (int)strlen(file);
    int ii;
    char *path = GetNxtBuf();
    int c;
    *path = 0;
    if (len > 0) {
        for (ii = len - 1; ii >= 0; ii--) {
            c = file[ii];
            if ((c == '\\') || (c == '/')) {
                // found first path separator
                break;
            }
        }
        ii++;
        if (ii) {
            for (len = 0; len < ii; len++) {
                path[len] = file[len];
            }
            path[len] = 0;
        }
    }
    return path;

}

char *set_logfile()
{
    static char mod[_MAX_PATH];
    mod[0] = 0;
    DWORD dwd = GetModuleFileName( NULL, mod, _MAX_PATH );
    if (dwd) {
        int c;
        char *cp;
        while(dwd--) {
            c = mod[dwd];
            if ((c == '\\')||(c == '/')) {
                dwd++;
                mod[dwd] = 0;
                if (dwd > 6) {
                    cp = &mod[dwd-6];
                    if (stricmp(cp,"Debug\\") == 0) {
                        dwd -= 6;
                        mod[dwd] = 0;
                    } else if (dwd > 8) {
                        cp = &mod[dwd-8];
                        if (stricmp(cp,"Release\\") == 0) {
                            dwd -= 8;
                            mod[dwd] = 0;
                        }
                    }
                }
                strcat(mod,"templog.txt");
                set_log_file( mod, false );
                break;
            }
        }
    }
    return mod;
}

#define MX_PATH_SEPS 32
// WIP - not completed, or used, YET
int fix_rel_path_in_buf( char *file )
{
    char *seps[MX_PATH_SEPS];
    size_t len = strlen(file);
    size_t ii, i2;
    size_t cnt = 0;
    size_t ncnt = 0;
    int c;
    char *nbuf = GetNxtBuf();
    for (ii = 0; ii < len; ii++) {
        i2 = ii + 1;
        c = file[ii];
        if ((c == '\\') || (c == '/')) {
            seps[cnt++] = &file[ii];
        } else if (c == '.') {
            if (i2 < len) {
                if (file[i2] == '.') {
                    if ((i2 + 1) < len) {
                        if ((file[i2+1] == '\\')||(file[i2+1] == '/')) {

                        } else {
                            // YOWEE! what to do now???
                        }
                    } else {
                        break; // dump trailing '..'
                    }
                } else if ((file[i2] == '\\') || (file[i2] == '/')) {
                    // just a single '.'
                    ii++;
                    c = 0;
                }

            } else {
                break;  // dump trailing '.'
            }
        }
        if (c) {
            nbuf[ncnt++] = (char)c;
        }
    }
    return 0;

}

/////////////////////////////////////////////////////////////////////////////
// not exactly a 'trim', but an important 'split' function

vSTG split_whitespace(const std::string &str, int maxsplit)
{
    vSTG result;
    std::string::size_type len = str.length();
    std::string::size_type i = 0;
    std::string::size_type j;
    int countsplit = 0;
    while (i < len) {
        while (i < len && isspace((unsigned char)str[i])) {
            i++;
        }
        j = i;  // next non-space
        while (i < len && !isspace((unsigned char)str[i])) {
            i++;
        }

        if (j < i) {
            result.push_back(str.substr(j, i - j));
            ++countsplit;
            while (i < len && isspace((unsigned char)str[i])) {
                i++;
            }

            if (maxsplit && (countsplit >= maxsplit) && i < len) {
                result.push_back(str.substr(i, len - i));
                i = len;
            }
        }
    }
    return result;
}
/**
* split a string per a separator - if no sep, use space - split_whitespace
*/
vSTG string_split(const std::string& str, const char* sep, int maxsplit)
{
    if (sep == 0)
        return split_whitespace(str, maxsplit);

    vSTG result;
    int n = (int)strlen(sep);
    if (n == 0) {
        // Error: empty separator string
        return result;
    }
    const char* s = str.c_str();
    std::string::size_type len = str.length();
    std::string::size_type i = 0;
    std::string::size_type j = 0;
    int k;
    int splitcount = 0;
    //while ((i + n) <= len) {
    while ((i + 1) <= len) {
        for (k = 0; k < n; k++) {
            if (s[i] == sep[k])
                break;
        }
        if (k < n) {
            result.push_back(str.substr(j, i - j));
            //i = j = i + n;
            i = j = i + 1;
            ++splitcount;
            if (maxsplit && (splitcount >= maxsplit))
                break;
        }
        else {
            ++i;
        }
    }
    result.push_back(str.substr(j, len - j));
    return result;
}

///////////////////////////////////////////////////////////////

char *fix_relative_path(const char *path, char *buf)
{
    std::string stg(path);
    vSTG vs = string_split(stg, "\\/");
    std::string s;
    size_t ii, max = vs.size();
    size_t i2, max2;
    int c;
    buf[0] = 0;
    if (max == 0)
        return 0;
    for (ii = 0; ii < max; ii++) {
        s = vs[ii];
        if (s == ".") {
            // just remove
            continue;
        }
        else if (s == "..") {
            // need to back up one
            max2 = strlen(buf);
            if (max2) {
                for (i2 = (max2 - 1); i2 > 0; i2--) {
                    c = buf[i2];
                    if ((c == '\\') || (c == '/')) {
                        // have backed up one
                        buf[i2] = 0;
                        break;
                    }
                }
                if (i2 == 0)
                    return 0;   // FAILED
            }
            else {
                return 0;   // FAILED
            }

        }
        else {
            if (strlen(buf))
                strcat(buf, PATH_SEP);
            strcat(buf, s.c_str());
        }
    }
    return buf;
}


// eof - sl_utils.cxx
