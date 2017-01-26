// showlnk2.hxx
#ifndef _SHOWLNK2_HXX_
#define _SHOWLNK2_HXX_

enum lnkfileflag {
    is_ms_lnk = 0,
    no_file_given,
    bad_hdr_str,
    no_stat,
    bad_size,
    no_open,
    no_memory,
    rd_hdr_failed,
    bad_hdr_size,
    bad_CLSID,
    rd_idls_failed,
    rd_idl_failed,
    bad_itemid,
    rd_lis_failed,
    rd_li_failed,
    exceeded_buf,
    // has to be last
    lf_flag_max
};

#define HasLinkTargetIdList	0x00000001  //	A LinkTargetIDList structure must follow the ShellLinkHeader
#define HasLinkInfo	        0x00000002	// A LinkInfo structure must be present
#define HasName	            0x00000004	// The StringData section contains the NAME_STRING field
#define HasRelativePath	    0x00000008	// The StringData section contains the RELATIVE_PATH field
#define HasWorkingDir	    0x00000010  // The StringData section contains the WORKING_DIR field
#define HasArguments	    0x00000020	// The StringData section contains the COMMAND_LINE_ARGUMENTS field
#define HasIconLocation	    0x00000040	// The StringData section contains the ICON_LOCATION field
#define IsUnicode	        0x00000080	// The link will contain Unicode strings. This should always be set
#define ForceNoLinkInfo	    0x00000100	// The LinkInfo structure (even if present) is ignored
#define HasExpString        0x00000200  // The shell link is saved with an EnvironmentVariableDataBlock (section 2.5.4).
#define HasExpIcon	        0x00004000	// The IconEnvironmentDataBlock is present
#define EnableTargetMetadata 0x00080000 // The shell link will attempt to collect target properties into the property list each time it is set
#define PreferEnvironmentPath 0x02000000 // The target IDList SHOULD NOT be stored; instead, the path specified in the EnvironmentVariableDataBlock (section 2.5.4) SHOULD be used to refer to the target.

extern lnkfileflag Is_MS_LNK_File( char * file, int verb = 0 );
extern const char *LnkFlagtoString( lnkfileflag lf );
extern void show_link_flags( unsigned int flag );

extern int showlinkinfo;

#endif // #ifndef _SHOWLNK2_HXX_
// eof - showlnk2.hxx
