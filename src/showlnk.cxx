// showlnk.cxx
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning( disable : 4995 )

#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>
#include <atlstr.h>
#include <shobjidl.h>
#include <shlguid.h> 
#include <strsafe.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "sprtf.hxx"
#include "conversions.hxx"
#include "sl_utils.hxx"
#include "showlnk2.hxx"

#ifndef _SPRTF_HXX_
#define sprtf printf
#endif
#ifndef SL_VERSION
#define SL_VERSION "0.0.3"
#endif

typedef std::vector<std::string> vSTG;

static vSTG files;

static bool excludecount = true;
static bool bumpunisize  = true;

/* =================================================================================
    from : http://msdn.microsoft.com/en-us/library/dd891253.aspx
    The Shell Link Binary File Format consists of a sequence of structures that conform to the following ABNF rules [RFC5234].

    SHELL_LINK = SHELL_LINK_HEADER [LINKTARGET_IDLIST] [LINKINFO]
                 [STRING_DATA] *EXTRA_DATA

    SHELL_LINK_HEADER: A ShellLinkHeader structure (section 2.1), which contains identification information, timestamps, 
        and flags that specify the presence of optional structures.

    LINKTARGET_IDLIST: An optional LinkTargetIDList structure (section 2.2), which specifies the target of the link. 
        The presence of this structure is specified by the HasLinkTargetIDList bit (LinkFlags section 2.1.1) in the ShellLinkHeader.
        typedef struct _SHITEMID { 
            USHORT cb; 
            BYTE   abID[1]; 
        } SHITEMID, * LPSHITEMID;

    LINKINFO: An optional LinkInfo structure (section 2.3), which specifies information necessary to resolve the link target. 
        The presence of this structure is specified by the HasLinkInfo bit (LinkFlags section 2.1.1) in the ShellLinkHeader.

    STRING_DATA: Zero or more optional StringData structures (section 2.4), which are used to convey user interface 
        and path identification information. The presence of these structures is specified by bits (LinkFlags section 2.1.1) 
        in the ShellLinkHeader.

    EXTRA_DATA: Zero or more ExtraData structures (section 2.5).

    Note  Structures of the Shell Link Binary File Format can define strings in fixed-length fields. In fixed-length fields, 
        strings MUST be null-terminated. If a string is smaller than the size of the field that contains it, the bytes 
        in the field following the terminating null character are undefined and can have any value. 
        The undefined bytes MUST NOT be used.

    STRING_DATA = [NAME_STRING] [RELATIVE_PATH] [WORKING_DIR]
              [COMMAND_LINE_ARGUMENTS] [ICON_LOCATION]


    NAME_STRING: An optional structure that specifies a description of the shortcut that is displayed to end users 
        to identify the purpose of the shell link. This structure MUST be present if the HasName flag is set.

    RELATIVE_PATH: An optional structure that specifies the location of the link target relative to the file that 
        contains the shell link. When specified, this string SHOULD be used when resolving the link. This structure 
        MUST be present if the HasRelativePath flag is set.

    WORKING_DIR: An optional structure that specifies the file system path of the working directory to be used when 
        activating the link target. This structure MUST be present if the HasWorkingDir flag is set.

    COMMAND_LINE_ARGUMENTS: An optional structure that stores the command-line arguments that should be specified when 
        activating the link target. This structure MUST be present if the HasArguments flag is set.

    ICON_LOCATION: An optional structure that specifies the location of the icon to be used when displaying a shell 
        link item in an icon view. This structure MUST be present if the HasIconLocation flag is set.

    All StringData structures have the following structure.
    struct StringData {
        short CountCharacters;
        char  String;
    }
   ================================================================================= */

typedef struct tagMY_SHELLLINKHDR {
    unsigned int HeaderSize; // (4 bytes): The size, in bytes, of this structure. This value MUST be 0x0000004C.
    CLSID LinkCLSID; // (16 bytes): A class identifier (CLSID). This value MUST be 00021401-0000-0000-C000-000000000046.
    unsigned int LinkFlags;  // (4 bytes): A LinkFlags structure (section 2.1.1) that specifies information about the shell link and the presence of optional portions of the structure.
    unsigned int FileAttributes; // (4 bytes): A FileAttributesFlags structure (section 2.1.2) that specifies information about the link target.
    FILETIME CreationTime;   // (8 bytes): A FILETIME structure ([MS-DTYP] section 2.3.3) that specifies the creation time of the link target in UTC (Coordinated Universal Time). If the value is zero, there is no creation time set on the link target.
    FILETIME AccessTime; // (8 bytes): A FILETIME structure ([MS-DTYP] section 2.3.3) that specifies the access time of the link target in UTC (Coordinated Universal Time). If the value is zero, there is no access time set on the link target.
    FILETIME WriteTime;  // (8 bytes): A FILETIME structure ([MS-DTYP] section 2.3.3) that specifies the write time of the link target in UTC (Coordinated Universal Time). If the value is zero, there is no write time set on the link target.
    unsigned int FileSize;   // (4 bytes): A 32-bit unsigned integer that specifies the size, in bytes, of the link target. If the link target file is larger than 0xFFFFFFFF, this value specifies the least significant 32 bits of the link target file size.
    int IconIndex;  // (4 bytes): A 32-bit signed integer that specifies the index of an icon within a given icon location.
    unsigned int ShowCommand; // (4 bytes): A 32-bit unsigned integer that specifies the expected window state of an application launched by the link. This value SHOULD be one of the following.
    unsigned short HotKey; // (2 bytes): A HotKeyFlags structure (section 2.1.3) that specifies the keystrokes used to launch the application referenced by the shortcut key. This value is assigned to the application after it is launched, so that pressing the key activates that application.
    unsigned short Reserved1; // (2 bytes): A value that MUST be zero.
    int Reserved2;  // (4 bytes): A value that MUST be zero.
    int Reserved3;  // (4 bytes): A value that MUST be zero.
}MY_SHELLLINKHDR, *PMY_SHELLLINKHDR;

typedef struct tagIDList {
    unsigned short Size;   // (2 bytes): The size, in bytes, of the IDList field.
} IDList, *PIDList;
typedef struct tagStringData {
    unsigned short Size;   // (2 bytes):  A 16-bit, unsigned integer that specifies either the number of characters, 
    // defined by the system default code page, or the number of Unicode characters found in the String field. 
    // A value of zero specifies an empty string.
}StringData, *PStringData;



#define str_size (4 + 16 + 4 + 4 + 8 + 8 + 8 + 4 + 4 + 4 + 2 + 2 + 4 + 4)

static MY_SHELLLINKHDR ShellLinkHdr;

// #define DO_TEST

static const char *file1 =  "C:\\Users\\user\\Desktop\\Beyond Compare 2.lnk";
static const char *file2 =  "C:\\Users\\user\\Desktop\\Command Prompt.lnk";
static const char *file3 =  "C:\\Users\\user\\Desktop\\Dv32.lnk";
static const char *file4 =  "C:\\Users\\user\\Desktop\\EditPlus 3.lnk";

//HRESULT getDescription(IShellLink *psl, WCHAR *pszDescription, int size);
HRESULT ResolveIt(HWND hwnd, LPCSTR lpszLinkFile, LPWSTR lpszPath, int iPathBufferSize) ;


BOOL CreateShortCut(CString file , CString shortCut)
{
	IShellLink*   pISL;
	IPersistFile* pIPF;
	HRESULT hr;
	hr = CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&pISL);
    if(!SUCCEEDED(hr))
	{
		return FALSE;
	}
	else
    {
        hr = pISL->SetPath(file);
        if(!SUCCEEDED(hr))
		{
			return FALSE;
		}
		else
        {       
			hr = pISL->QueryInterface(IID_IPersistFile,(void**)&pIPF);
			if(SUCCEEDED(hr))
            {
                hr = pIPF->Save(CA2W(shortCut),FALSE);               
				pIPF->Release();
            }
			else
			{
				return FALSE;
			}
        }
        pISL->Release();
    }
	return TRUE;
}

BOOL test_IShellLink()
{
    BOOL iret = TRUE;
	IShellLink*   pISL;
	//IPersistFile* pIPF;
	HRESULT hr;
	hr = CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&pISL);
    if(!SUCCEEDED(hr)) {
		iret = FALSE;
        goto exit;
	}
    // 
    pISL->Release();
exit:
	return iret;
}

#define MY_MX_PATH 1024
static WCHAR pathw[MY_MX_PATH];
static char path[MY_MX_PATH];

void test_Resolve(LPCSTR lpszLinkFile)
{
    HRESULT hres = ResolveIt(NULL, lpszLinkFile, pathw, MY_MX_PATH); 
    if (UnicodeToAscii(pathw, path)) {
        sprtf("PATH: %s\n", path);
    } else {
        size_t len = wcslen(pathw);
        HexDump((PBYTE)pathw, len);
    }

}

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
void out_indent(int i) {
    char tmp[256];
    if (i) {
        tmp[0] = 0;
        while(i--) {
            strcat(tmp,"  ");
        }
        SPRTF("%s",tmp);
    }
}

static bool add_num_values = false;
static bool add_null_values = false;

void QueryKey(HKEY hKey, int level) 
{ 
    WCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    WCHAR    achClass[MAX_PATH] = L"";  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 
    char *tmp = GetNxtBuf();
    DWORD i, retCode; 
 
    WCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
 
    // Get the class name and the value count. 
    retCode = RegQueryInfoKeyW(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 
 
    // Enumerate the subkeys, until RegEnumKeyEx fails.
    if (cSubKeys) {
        out_indent(level);
        SPRTF( "Number of subkeys: %d\n", cSubKeys);
        for (i = 0; i < cSubKeys; i++) { 
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyExW(hKey, i,
                     achKey, 
                     &cbName, 
                     NULL, 
                     NULL, 
                     NULL, 
                     &ftLastWriteTime); 
            if (retCode == ERROR_SUCCESS) {
                out_indent(level);
                size_t len = wcslen(achKey);
                //wprintf(L"%d: [%s]%ld\n", i+1, achKey, len);
                wsprtf(L"%d: [%s] len %ld\n", i+1, achKey, len);
                if (level < 10) {
                    HKEY hSubKey;
                    retCode = RegOpenKeyExW (hKey, achKey, 0, KEY_READ, &hSubKey);
                    if (retCode == ERROR_SUCCESS) {
                        QueryKey(hSubKey, level + 1);
                        RegCloseKey(hSubKey);
                    }
                }
            }
        }
    } 
 
    // Enumerate the key values. 
    if (cValues) 
    {
        if (add_num_values) {
            out_indent(level);
            SPRTF( "Number of values: %d\n", cValues );
        }
        for (i = 0; i < cValues; i++) 
        { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            retCode = RegEnumValueW(hKey, i, 
                achValue, 
                &cchValue, 
                NULL, 
                NULL,
                NULL,
                NULL);
 
            if (retCode == ERROR_SUCCESS ) { 
                if (*achValue) {
                    out_indent(level);
                    //wprintf(L" %d: %s\n", i+1, achValue); 
                    wsprtf(L" %d: %s\n", i+1, achValue); 
                } else if (add_null_values) {
                    out_indent(level);
                    //wprintf(L" %d: <null>\n", i+1, achValue); 
                    wsprtf(L" %d: <null>\n", i+1, achValue); 
                }
            } 
        }
    }

}


void test_Registry()
{
    // enumerate CLSID from registry
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"CLSID", 0, KEY_READ, &hKey);
    //LONG lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"CLSID", 0, KEY_ENUMERATE_SUB_KEYS, &hKey ); 
    bool bok (lRes == ERROR_SUCCESS);
    if (bok) {
        //dwSize = MAX_PATH;
        //lRes = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
        //                       NULL, NULL, &ftWrite);
        //lRes = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwSize);
        QueryKey(hKey, 0);
        RegCloseKey(hKey);
    } else {
        SPRTF("Failed to open registry...\n");
    }
    exit(1);

}

static int verbosity = 0;
static int recursive = 0;

void show_help(char *name)
{
    SPRTF("%s [Options] lnk_file [or directory] [or wild card] [more files/directories...]\n",
        get_filename_from_path(name));
    SPRTF("Options\n");
    SPRTF(" --help (or -h or -?) = This help and exit.\n");
    SPRTF(" --recursive     (-r) = Process subdirectories, if a directory or wild card given.\n");
    SPRTF(" --link          (-l) = Dump link info section, if present.\n");
    SPRTF(" --verb[nn]      (-v) = Bump. or set verbosity to nn value.\n");
    SPRTF("\n");
    SPRTF(" Basically just an exercise is 'decoding' a Microsoft LINK (shortcut) file. (*.lnk)\n");
    SPRTF(" and if verbosity set, then display the contents. All if -v9\n");
    SPRTF(" Will exit(0) if no error found in any file processed, otherwise exit(1)\n");
    SPRTF(" Project 'showlnk' started 20140201, update 20140204. current version " SL_VERSION "\n");
}

int get_lnk_files( char *file );

typedef struct tagSPLITPATH {
    char drv[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
}SPLITPATH, *PSPLITPATH;
static SPLITPATH sp;

int process_wild( char *file )
{
    WIN32_FIND_DATA fd;
    std::string s(file);
    vSTG dirs;
    if (verbosity > 5) SPRTF("Processing wild card [%s].\n",file);
    HANDLE hfind = FindFirstFile( s.c_str(), &fd );
    if (hfind == INVALID_HANDLE_VALUE) {
        return 1;
    }
    memset(&sp,0,sizeof(SPLITPATH));
    _splitpath( s.c_str(), sp.drv, sp.dir, sp.fname, sp.ext );

    do {
        if (strcmp(fd.cFileName,".") && strcmp(fd.cFileName,"..")) {
            std::string dir(sp.drv);
            dir += sp.dir;
            //dir += "\\";
            dir += fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                dirs.push_back(dir);
            } else {
                std::string file(fd.cFileName);
                size_t pos = file.rfind('.');
                if (pos != -1) {
                    file = file.substr(pos);
                    if (stricmp(file.c_str(),".lnk") == 0) {
                        files.push_back(dir);
                        if (verbosity) SPRTF("Added file [%s] to list to process.\n",dir.c_str());
                    }
                }
            }
        }
    } while ( FindNextFile( hfind, &fd ) );
    FindClose(hfind);
    if (recursive) {
        size_t ii;
        for (ii = 0; ii < dirs.size(); ii++) {
            s = dirs[ii];
            if (sp.fname[0]) {
                s += "\\";
                s += sp.fname;
                s += sp.ext;
            }
            if (has_wild((char *)s.c_str())) {
                if (process_wild((char *) s.c_str())) {
                    return 1;
                }
            } else {
                if (get_lnk_files( (char *)s.c_str() )) {
                    return 1;
                }
            }
        }
    }
   
    return 0;


}

int get_lnk_files( char *file )
{
    std::string s(file);
    s += "\\*.*";
    return process_wild((char *)s.c_str());
}

#define IS_DIGIT(a) ((a >= '0') && (a <= '9'))

int is_digits(char *stg)
{
    size_t len = strlen(stg);
    size_t ii;
    int c;
    for (ii = 0; ii < len; ii++) {
        c = stg[ii];
        if (!IS_DIGIT(c))
            return 0;
    }
    return 1;
}



int main( int argc, char **argv )
{
	int iret = 0;
    lnkfileflag lf = is_ms_lnk;
    char *log = set_logfile();
    // test_Registry();
    // CoInitialize ( NULL );  
#ifdef DO_TEST
    //test_IShellLink();
    //test_Resolve(file1);
    verbosity = 9;
    if (verbosity) sprtf("\nProcessing file [%s]\n",file1);
    lf = Is_MS_LNK_File((char *)file1,verbosity);
    if (lf != is_ms_lnk)
        iret |= 1;
#if 0 // ==========================
    if (verbosity) sprtf("\nProcessing file [%s]\n",file2);
    lf = Is_MS_LNK_File((char *)file2,verbosity);
    if (lf != is_ms_lnk)
        iret |= 1;
    if (verbosity) sprtf("\nProcessing file [%s]\n",file3);
    lf = Is_MS_LNK_File((char *)file3,verbosity);
    if (lf != is_ms_lnk)
        iret |= 1;
    if (verbosity) sprtf("\nProcessing file [%s]\n",file4);
    lf = Is_MS_LNK_File((char *)file4,verbosity);
    if (lf != is_ms_lnk)
        iret |= 1;
#endif // 0 ========================================
#else
    int i, res;
    char *arg;
    char *sarg;
    if (argc < 2) {
        show_help(argv[0]);
        return 1;
    }
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        if (*arg == '-') {
            sarg = &arg[1];
            while (*sarg && (*sarg == '-')) sarg++;
            if (*sarg == 'v') {
                sarg++;
                if (*sarg) {
                    while (*sarg && !IS_DIGIT(*sarg)) sarg++;
                    if (is_digits(sarg)) {
                        verbosity += atoi(sarg);
                    } else {
                        verbosity++;
                    }
                } else {
                    verbosity++;
                }
                if (verbosity) {
                    if (*log)
                        SPRTF("Set LOG file to [%s]\n",log);
                    SPRTF("Set verbosity to %d.\n",verbosity);
                }
            } else if ((*sarg == 'h') || (*sarg == '?')) {
                show_help(argv[0]);
                return 0;
            } else if (*sarg == 'r') {
                recursive = 1;
                if (verbosity) SPRTF("Set recursive into subdirectories.\n");
            } else if (*sarg == 'l') {
                showlinkinfo = 1;
                if (verbosity) SPRTF("Set to show link info, if pesent.\n");
            } else {
                SPRTF("Unknown argument [%s]! Try -? for help\n",arg);
                return 1;
            }
        } else {
            if (has_wild(arg)) {
                if (process_wild(arg)) {
                    SPRTF("Failed using argument [%s]\n",arg);
                    return 1;
                }
            } else {
                res = is_file_or_directory(arg); 
                if ( res == 1) {
                    if (verbosity) SPRTF("Added file [%s] to list to process.\n",arg);
                    files.push_back(arg);
                } else if (res == 2) {
                    if (get_lnk_files(arg)) {
                        SPRTF("Unable to open directory [%s]\n",arg);
                        return 1;
                    }
                } else {
                    SPRTF("Argument [%s] is NOT a valid file or directory! Use -? for help\n", arg);
                    return 1;
                }
            }
        }
    }
    size_t max = files.size();
    if (max == 0) {
        SPRTF("No files to process found in command!\n");
        return 1;
    }
    if (verbosity) SPRTF("Got %ld files to process...\n", max);
    for (size_t ii = 0; ii < max; ii++) {
        std::string s = files[ii];
        lf = Is_MS_LNK_File((char *)s.c_str(),verbosity);
        if (lf != is_ms_lnk) {
            if (verbosity) SPRTF("File %ld of %ld: [%s] FAILED lnk test!\n", (ii+1), max, s.c_str());
            iret |= 1;
        } else if (verbosity > 5) {
            SPRTF("File %ld of %ld: [%s] PASSED lnk test!\n", (ii+1), max, s.c_str());
        }
        if (((ii + 1) < max) && (verbosity || showlinkinfo)) {
            SPRTF("\n");
        }
    }
#endif
    //CoUninitialize();
	return iret;
}

// eof
