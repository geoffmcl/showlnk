// showlnk2.cxx

#include <Windows.h>
#include <stdio.h>
#include <string>
#include <direct.h> // for _getcwd()

#include "sl_utils.hxx"
#include "showlnk2.hxx"
#include "sl_load.hxx"
#include "sprtf.hxx"
#include "conversions.hxx"
int showlinkinfo = 0;

/* ================================================================================
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
   ================================================================================= */
/* =================================================================================
   Alternate information
   from : http://fileanalysis.net/lnk/
   ================================================================================= */

typedef struct tagShellLinkHeader {
    UINT32 HeaderSize; // The size of the structure
	UINT8 LinkClsid[16]; // A class identifier with the following value: 00021401-0000-0000-C000-000000000046
    UINT32 LinkFlags;   // UInt32 		See LinkFlags
    UINT32 FileAttributes; // UInt32	See FileAttributes
    FILETIME CreationTime; // FILETIME	A 64-bit time value
    FILETIME AccessTime; //	FILETIME	A 64-bit time value
    FILETIME WriteTime; //	FILETIME	A 64-bit time value
    UINT32 FileSize; // 	UInt32	    The size of the link target. This will only specify the least significant 32-bits for link targets bigger than 0xFFFFFFFF
    UINT32 IconIndex; //	UInt32	    Index of an icon within a given icon location
    UINT32 ShowCommand; //	UInt32	    Determines how the link target is shown on screen. Can be one of the following values: SW_SHOWNORMAL, SW_SHOWMAXIMIZED, SW_SHOWMINNOACTIVE
    UINT16 Hotkey;  //  	UInt16	    Specifies the keystroke to use to launch the link target
    UINT8 Reserved[10]; // 	UInt8[10]	Reserved space
}ShellLinkHeader, *PShellLinkHeader;
/* =================================================================================
    LinkFlags
    Flag name	Value	Description
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
   ================================================================================= */
typedef struct tagLINKFLAGS {
    unsigned int bit;
    const char *name;
    const char *desc;
}LINKFLAGS, *PLINKFLAGS;

static LINKFLAGS linkFlags[] = {
    { HasLinkTargetIdList, "HasLinkTargetIDList", "The shell link is saved with an item ID list (IDList). If this bit is set, a LinkTargetIDList structure (section 2.2) MUST follow the ShellLinkHeader. If this bit is not set, this structure MUST NOT be present." },
    { HasLinkInfo, "HasLinkInfo", "The shell link is saved with link information. If this bit is set, a LinkInfo structure (section 2.3) MUST be present. If this bit is not set, this structure MUST NOT be present." },
    { HasName, "HasName", "The shell link is saved with a name string. If this bit is set, a NAME_STRINGStringData structure (section 2.4) MUST be present. If this bit is not set, this structure MUST NOT be present." },
    { HasRelativePath, "HasRelativePath", "The shell link is saved with a relative path string. If this bit is set, a RELATIVE_PATH StringData structure (section 2.4) MUST be present. If this bit is not set, this structure MUST NOT be present." },
    { HasWorkingDir, "HasWorkingDir", "The shell link is saved with a working directory string. If this bit is set, a WORKING_DIR StringData structure (section 2.4) MUST be present. If this bit is not set, this structure MUST NOT be present." },
    { HasArguments, "HasArguments", "The shell link is saved with command line arguments. If this bit is set, a COMMAND_LINE_ARGUMENTS StringData structure (section 2.4) MUST be present. If this bit is not set, this structure MUST NOT be present." },
    { HasIconLocation, "HasIconLocation", "The shell link is saved with an icon location string. If this bit is set, an ICON_LOCATION StringData structure (section 2.4) MUST be present. If this bit is not set, this structure MUST NOT be present." },
    { IsUnicode, "IsUnicode", "The shell link contains Unicode encoded strings. This bit SHOULD be set. If this bit is set, the StringData section should contain Unicode-encoded strings; otherwise, it should contain strings that are encoded using the system default code page." },
    { ForceNoLinkInfo, "ForceNoLinkInfo", "The LinkInfo structure (section 2.3) is ignored." },
    { HasExpString, "HasExpString", "The shell link is saved with an EnvironmentVariableDataBlock (section 2.5.4)." },
    { 0x00000400, "RunInSeparateProcess", "The target is run in a separate virtual machine when launching a link target that is a 16-bit application." },
    // { 0x0000800, "Unused1", "A bit that is undefined and MUST be ignored." },
    { 0x00001000, "HasDarwinID", "The shell link is saved with a DarwinDataBlock (section 2.5.3)." },
    { 0x00002000, "RunAsUser", "The application is run as a different user when the target of the shell link is activated." },
    { HasExpIcon, "HasExpIcon", "The shell link is saved with an IconEnvironmentDataBlock (section 2.5.5)." },
    { 0x00008000, "NoPidlAlias", "The file system location is represented in the shell namespace when the path to an item is parsed into an IDList." },
    //{ 0x0010000, "Unused2", "A bit that is undefined and MUST be ignored." },
    { 0x00020000, "RunWithShimLayer", "The shell link is saved with a ShimDataBlock (section 2.5.8)." },
    { 0x00040000, "ForceNoLinkTrack", "The TrackerDataBlock (section 2.5.10) is ignored." },
    { EnableTargetMetadata, "EnableTargetMetadata", "The shell link attempts to collect target properties and store them in the PropertyStoreDataBlock (section 2.5.7) when the link target is set." },
    { 0x00100000, "DisableLinkPathTracking", "The EnvironmentVariableDataBlock is ignored." },
    { 0x00200000, "DisableKnownFolderTracking", "The SpecialFolderDataBlock (section 2.5.9) and the KnownFolderDataBlock (section 2.5.6) are ignored when loading the shell link. If this bit is set, these extra data blocks SHOULD NOT be saved when saving the shell link." },
    { 0x00400000, "DisableKnownFolderAlias", "If the link has a KnownFolderDataBlock (section 2.5.6), the unaliased form of the known folder IDList SHOULD be used when translating the target IDList at the time that the link is loaded." },
    { 0x00800000, "AllowLinkToLink", "Creating a link that references another link is enabled. Otherwise, specifying a link as the target IDList SHOULD NOT be allowed." },
    { 0x01000000, "UnaliasOnSave", "When saving a link for which the target IDList is under a known folder, either the unaliased form of that known folder or the target IDList SHOULD be used." },
    { PreferEnvironmentPath, "PreferEnvironmentPath", "The target IDList SHOULD NOT be stored; instead, the path specified in the EnvironmentVariableDataBlock (section 2.5.4) SHOULD be used to refer to the target." },
    { 0x04000000, "KeepLocalIDListForUNCTarget", "When the target is a UNC name that refers to a location on a local machine, the local path IDList in the PropertyStoreDataBlock (section 2.5.7) SHOULD be stored, so it can be used when the link is loaded on the local machine." },
    // LAST ENTRY
    { 0,         0,                             0 }
};

void show_link_flags( unsigned int flag )
{
    PLINKFLAGS plf = &linkFlags[0];
    sprtf("LinkFlags: %#x ",flag);
    show_bits32(flag);
    while (plf->name) {
        if (plf->bit & flag) {
            sprtf("%s ", plf->name);
            flag &= ~plf->bit;
        }
        plf++;
    };
    sprtf("\n");
    if (flag) {
        sprtf("Bits missed: 0x%x",flag);
        show_bits32(flag);
    }
}

/* =================================================================================
    LinkTargetIDList

    This structure specifies the target of the link. The presence of this optional section requires 
    ShellLinkHeader::LinkFlags to contain the HasLinkTargetIdList flag.

    Field name	Field type	Description
  ================================================================================== */
typedef struct tagLinkTargetIDList {
    UINT16 IDListSize;  //	UInt16	The size of the structure
    UINT8 ItemIDList[1];//	Array of bytes	Array of ItemID structures
    UINT16 Terminator;  //	UInt16	This field is set to 0
}LinkTargetIDList, *PLinkTargetIDList;
/* =================================================================================
    ItemID

    The official documentation describes the content of this structure as follows:
    The data stored in a given ItemID is defined by the source that corresponds to the location in the target namespace of the preceding ItemIDs. This data uniquely identifies the items in that part of the namespace.
    The actual content is not documented and may vary between different versions of the operating system.

    Field name	Field type	Description
    ItemIDSize	UInt16	The size of the structure
    Data	Array of bytes	The data associated with this segment
   ================================================================================= */
typedef struct tagItemID {
    UINT16 ItemIDSize;  //	UInt16	The size of the structure
    UINT8 Data[1];      //	Array of bytes	The data associated with this segment
}ItemID, *PItemID;

/* ================================================================================
    LinkInfo
    Field name	Field type	Requirements	Description
  ================================================================================= */
typedef struct tagLinkInfo {
    UINT32 LinkInfoSize;    //	UInt32	-	The size, in bytes, of this section. All the offsets specified here are relative to the start of this structure and must not go past the the size specified.
    UINT32 LinkInfoHeaderSize; //	UInt32	-	The size, in bytes, of the non-data part of the section
    UINT32 LinkInfoFlags;   //	UInt32	-	See LinkInfoFlags
    UINT32 VolumeIDOffset;  //	UInt32	-	The relative offset of the VolumeID structure
    UINT32 LocalBasePathOffset; //	UInt32	-	Part of the path to be used with the CommonPath. 
        // Should be ignored if set to 0x00000014
    UINT32 CommonNetworkRelativeLinkOffset; //	UInt32	-	The relative offset for the CommonNetworkRelativeLink structure
    UINT32 CommonPathSuffixOffset;  //	UInt32	-	The path to be appended to the local path to build the full path. 
        // Should be ignored if set to 0x00000014
    UINT32 LocalBasePathOffsetUnicode; //	UInt32	Present if LinkInfoSize >= 0x24	See LocalBasePathOffset
    UINT32 CommonPathSuffixOffsetUnicode; //	UInt32	Present if LinkInfoSize >= 0x24	See CommonPathSuffixOffset
    // UINT8 Data[1]; //	Array of bytes	-
}LinkInfo, *PLinkInfo;
/* ================================================================================
    LinkInfoSize (4 bytes): A 32-bit, unsigned integer that specifies the size, in bytes, of the 
      LinkInfo structure. All offsets specified in this structure MUST be less than this value, 
      and all strings contained in this structure MUST fit within the extent defined by this size.
    LinkInfoHeaderSize (4 bytes): A 32-bit, unsigned integer that specifies the size, in bytes, of the 
      LinkInfo header section, which is composed of the LinkInfoSize, LinkInfoHeaderSize, LinkInfoFlags, 
      VolumeIDOffset, LocalBasePathOffset, CommonNetworkRelativeLinkOffset, CommonPathSuffixOffset fields, 
      and, if included, the LocalBasePathOffsetUnicode and CommonPathSuffixOffsetUnicode fields.<1>
      <1> Section 2.3: In Windows, Unicode characters are stored in this structure if the data cannot be 
       represented as ANSI characters due to truncation of the values. In this case, the value of the 
       LinkInfoHeaderSize field is greater than or equal to 36.
      Value	       Meaning
      0x0000001C   Offsets to the optional fields are not specified.
      0x00000024 ≤ value Offsets to the optional fields are specified.
    LinkInfoFlags (4 bytes): Flags that specify whether the VolumeID, LocalBasePath, LocalBasePathUnicode, 
      and CommonNetworkRelativeLink fields are present in this structure.
  ================================================================================= */
#define VolumeIDAndLocalBasePath                0x00000001 // If set, the VolumeID and LocalBasePath fields are present, 
// and their locations are specified by the values of the VolumeIDOffset and LocalBasePathOffset fields, respectively. 
// If the value of the LinkInfoHeaderSize field is greater than or equal to 0x00000024, the LocalBasePathUnicode field is 
// present, and its location is specified by the value of the LocalBasePathOffsetUnicode field.
// If not set, the VolumeID, LocalBasePath, and LocalBasePathUnicode fields are not present, and the values of the 
// VolumeIDOffset and LocalBasePathOffset fields are zero. 
// If the value of the LinkInfoHeaderSize field is greater than or equal to 0x00000024, the value of the 
// LocalBasePathOffsetUnicode field is zero.
#define CommonNetworkRelativeLinkAndPathSuffix  0x00000002 // If set, the CommonNetworkRelativeLink field is present, 
// and its location is specified by the value of the CommonNetworkRelativeLinkOffset field.
// If not set, the CommonNetworkRelativeLink field is not present, and the value of the CommonNetworkRelativeLinkOffset field is zero.
/* ================================================================================
    VolumeIDOffset (4 bytes): A 32-bit, unsigned integer that specifies the location of the VolumeID field. 
       If the VolumeIDAndLocalBasePath flag is set, this value is an offset, in bytes, from the start of the LinkInfo structure; 
       otherwise, this value MUST be zero.
    LocalBasePathOffset (4 bytes): A 32-bit, unsigned integer that specifies the location of the LocalBasePath field. 
       If the VolumeIDAndLocalBasePath flag is set, this value is an offset, in bytes, from the start of the LinkInfo structure; 
       otherwise, this value MUST be zero.
    CommonNetworkRelativeLinkOffset (4 bytes): A 32-bit, unsigned integer that specifies the location of the CommonNetworkRelativeLink 
      field. If the CommonNetworkRelativeLinkAndPathSuffix flag is set, this value is an offset, in bytes, from the start of the 
      LinkInfo structure; otherwise, this value MUST be zero.
    CommonPathSuffixOffset (4 bytes): A 32-bit, unsigned integer that specifies the location of the CommonPathSuffix field. 
      This value is an offset, in bytes, from the start of the LinkInfo structure.
    LocalBasePathOffsetUnicode (4 bytes): An optional, 32-bit, unsigned integer that specifies the location of the LocalBasePathUnicode field. 
      If the VolumeIDAndLocalBasePath flag is set, this value is an offset, in bytes, from the start of the LinkInfo structure; 
      otherwise, this value MUST be zero. 
      This field can be present only if the value of the LinkInfoHeaderSize field is greater than or equal to 0x00000024.
    CommonPathSuffixOffsetUnicode (4 bytes): An optional, 32-bit, unsigned integer that specifies the location of the 
      CommonPathSuffixUnicode field. This value is an offset, in bytes, from the start of the LinkInfo structure. This 
      field can be present only if the value of the LinkInfoHeaderSize field is greater than or equal to 0x00000024.
    VolumeID (variable): An optional VolumeID structure (section 2.3.1) that specifies information about the volume that the 
      link target was on when the link was created. This field is present if the VolumeIDAndLocalBasePath flag is set.
    LocalBasePath (variable): An optional, NULL–terminated string, defined by the system default code page, which is used 
      to construct the full path to the link item or link target by appending the string in the CommonPathSuffix field. 
      This field is present if the VolumeIDAndLocalBasePath flag is set.
    CommonNetworkRelativeLink (variable): An optional CommonNetworkRelativeLink structure (section 2.3.2) that specifies 
      information about the network location where the link target is stored.
    CommonPathSuffix (variable): A NULL–terminated string, defined by the system default code page, which is used to 
      construct the full path to the link item or link target by being appended to the string in the LocalBasePath field.
    LocalBasePathUnicode (variable): An optional, NULL–terminated, Unicode string that is used to construct the full path 
      to the link item or link target by appending the string in the CommonPathSuffixUnicode field. 
      This field can be present only if the VolumeIDAndLocalBasePath flag is set and the value of the LinkInfoHeaderSize 
      field is greater than or equal to 0x00000024.
    CommonPathSuffixUnicode (variable): An optional, NULL–terminated, Unicode string that is used to construct the full path 
      to the link item or link target by being appended to the string in the LocalBasePathUnicode field. This field can be 
      present only if the value of the LinkInfoHeaderSize field is greater than or equal to 0x00000024.
  ================================================================================= */



/* ================================================================================
    StringData

    Field name	Field type	Requirements	Description
    NAME_STRING	STRINGDATA_FIELD	ShellLinkHeader::LinkFlags must contain HasName	A description of the link target
    RELATIVE_PATH	STRINGDATA_FIELD	ShellLinkHeader::LinkFlags must contain HasRelativePath	Specifies the location of the link target relative to the file that contains the shell link
    WORKING_DIR	STRINGDATA_FIELD	ShellLinkHeader::LinkFlags must contain HasWorkingDir	Specifies the file system path of the working directory to be used when activating the link target
    COMMAND_LINE_ARGUMENTS	STRINGDATA_FIELD	ShellLinkHeader::LinkFlags must contain HasArguments	The command line arguments to pass to the link target
    ICON_LOCATION	STRINGDATA_FIELD	ShellLinkHeader::LinkFlags must contain HasIconLocation
    ================================================================================= */
typedef struct tagSTRINGDATA_FIELD {
    UINT16 CountCharacters; //	UInt16	String length
    UINT8 String[1];    //	Array of bytes	The characters forming the string, without null terminator
}STRINGDATA_FIELD, *PSTRINGDATA_FIELD;
    
/* ================================================================================
    VolumeID
    Field name	Field type	Description
    ================================================================================= */
typedef struct tagVolumeID {
    UINT32 VolumeIDSize;    //	UInt32	The size, in bytes, of this section. All the offsets specified here are relative to the start of this structure and must not go past the the size specified.
    UINT32 DriveType;       //	UInt32	See DriveType
    UINT32 DriveSerialNumber;   //	UInt32	The drive serial number
    UINT32 VolumeLabelOffset;   //	UInt32	The drive label relative offset. Use the unicode version if this field is set to 0x00000014
    UINT32 VolumeLabelOffsetUnicode; //	UInt32	The drive label relative offset. Use the Ansi version if this field is set to 0x00000014
    UINT8 Data[1];      //	Array of bytes	The data referenced by the structure
}VolumeID, *PVolumeID;
/* ================================================================================
    DriveType
    Flag name	Value	Description
    Already defined in winbase.h
#define DRIVE_UNKNOWN	    0x00000000  //	Unknown drive type
#define DRIVE_NO_ROOT_DIR	0x00000001	// The root path is not valid
#define DRIVE_REMOVABLE	    0x00000002	// The drive type is removable (like a cdrom or a floppy)
#define DRIVE_FIXED	        0x00000003	// The drive is a fixed media, like a hard drive
#define DRIVE_REMOTE	    0x00000004	// The drive is a remote (network) drive
#define DRIVE_CDROM	        0x00000005	// The drive is a CD-ROM
#define DRIVE_RAMDISK	    0x00000006	// The drive is a RAM disk
  ================================================================================= */

/* ================================================================================
    CommonNetworkRelativeLink
    Field name	Field type	Description
  ================================================================================= */
typedef struct tagCommonNetworkRelativeLink {
    UINT32 CommonNetworkRelativeLinkSize; //	UInt32	The size, in bytes, of this section. All the offsets specified here are relative to the start of this structure and must not go past the the size specified. Must be greater than or equal to 0x00000014
    UINT32 CommonNetworkRelativeLinkFlags; //	UInt32	See CommonNetworkRelativeLinkFlags
    UINT32 NetNameOffset; //	UInt32	Relative offset for the NetName field
    UINT32 DeviceNameOffset; //	UInt32	Relative offset for the DeviceName field. Should be zero if the ValidDevice flag is not set
    UINT32 NetworkProviderType; //	UInt32	See NetworkProviderType. Specifies the type of network provider. Should be ignored if ValidNetType is not specified in CommonNetworkRelativeLinkFlags
    UINT32 NetNameOffsetUnicode; //	UInt32	Specifies the location of the NetNameUnicode. This field must be present if the value of the NetNameOffset field is greater than 0x00000014
    UINT32 DeviceNameOffsetUnicode; //	UInt32	Specifies the location of the DeviceNameOffsetUnicode. This field must be present if the value of the NetNameOffset field is greater than 0x00000014
    UINT8 Data[1];      //	Array of bytes	The data referenced by the structure
}CommonNetworkRelativeLink, *PCommonNetworkRelativeLink;
/* ================================================================================
    CommonNetworkRelativeLinkFlags
    Flag name	Value	Description
  ================================================================================= */
#define ValidDevice	    0x00000001	// The DeviceNameOffset is valid
#define ValidNetType	0x00000002	// The NetProviderType is valid
/* ================================================================================
    NetworkProviderType
    Flag name	Value
   ================================================================================= */
#define WNNC_NET_AVID	0x001A0000
#define WNNC_NET_DOCUSPACE	0x001B0000
#define WNNC_NET_MANGOSOFT	0x001C0000
#define WNNC_NET_SERNET	0x001D0000
//#define WNNC_NET_RIVERFRONT1	0x001E0000
//#define WNNC_NET_RIVERFRONT2	0x001F0000
//#define WNNC_NET_DECORB	0x00200000
//#define WNNC_NET_PROTSTOR	0x00210000
//#define WNNC_NET_FJ_REDIR	0x00220000
//#define WNNC_NET_DISTINCT	0x00230000
//#define WNNC_NET_TWINS	0x00240000
//#define WNNC_NET_RDR2SAMPLE	0x00250000
//#define WNNC_NET_CSC	0x00260000
//#define WNNC_NET_3IN1	0x00270000
//#define WNNC_NET_EXTENDNET	0x00290000
#define WNNC_NET_STAC	0x002A0000
#define WNNC_NET_FOXBAT	0x002B0000
#define WNNC_NET_YAHOO	0x002C0000
#define WNNC_NET_EXIFS	0x002D0000
#define WNNC_NET_DAV	0x002E0000
#define WNNC_NET_KNOWARE	0x002F0000
//#define WNNC_NET_OBJECT_DIRE	0x00300000
//#define WNNC_NET_MASFAX	0x00310000
//#define WNNC_NET_HOB_NFS	0x00320000
//#define WNNC_NET_SHIVA	0x00330000
//#define WNNC_NET_IBMAL	0x00340000
//#define WNNC_NET_LOCK	0x00350000
//#define WNNC_NET_TERMSRV	0x00360000
//#define WNNC_NET_SRT	0x00370000
//#define WNNC_NET_QUINCY	0x00380000
//#define WNNC_NET_OPENAFS	0x00390000
//#define WNNC_NET_AVID1	0x003A0000
#define WNNC_NET_DFS	0x003B0000
#define WNNC_NET_KWNP	0x003C0000
#define WNNC_NET_ZENWORKS	0x003D0000
#define WNNC_NET_DRIVEONWEB	0x003E0000
#define WNNC_NET_VMWARE	0x003F0000
//#define WNNC_NET_RSFX	0x00400000
//#define WNNC_NET_MFILES	0x00410000
//#define WNNC_NET_MS_NFS	0x00420000
//#define WNNC_NET_GOOGLE	0x0043000
/* ================================================================================
    ExtraData

    This section is made of data block structures; each one of them always starts with two DWORDs containing the block size and a signature.
    After the last block, a DWORD with a value between 0 and 4 is found.
    As the blocks are several in number, refer to the official documentation.

    Block #0
    Block #1
    Block #n
    UInt32 terminator set to a value between zero and four.
   ================================================================================= */
typedef struct tagBLOCK {
    DWORD   size;
    DWORD   signature;
}BLOCK, *PBLOCK;
/* ================================================================================
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
   ================================================================================= */

/* ================================================================================
    20140204 - Yet another source of information found
    from : http://www.sweetscape.com/010editor/templates/files/LNKTemplate.bt

    // http://msdn.microsoft.com/en-us/library/cc144089%28VS.85%29.aspx#unknown_74413
    typedef struct
    {
	    WORD ItemIDSize;
	    if (ItemIDSize == 0x14)
	    {
		    BYTE Type;
		    BYTE Unknown;
		    BYTE GUID[ItemIDSize - 4];
	    }
	    else
		    BYTE Data[ItemIDSize - 2];
    } IDList <read=ReadIDList>;

		SPrintf(sGUID, "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        sIDList.GUID[3], sIDList.GUID[2], sIDList.GUID[1], sIDList.GUID[0], sIDList.GUID[5], sIDList.GUID[4], 
        sIDList.GUID[7], sIDList.GUID[6], sIDList.GUID[8], sIDList.GUID[9], sIDList.GUID[10], 
        sIDList.GUID[11], sIDList.GUID[12], sIDList.GUID[13], sIDList.GUID[14], sIDList.GUID[15]);

  ================================================================================= */
// GUIDs found in shlguid.h
// but this is quite a SHORT list - there seem MANY more defined
// like all those defined in <knownfolders.h>
// legacy CSIDL value: CSIDL_NETWORK
// display name: "Network"
// legacy display name: "My Network Places"
// default path: 
// {D20BEEC4-5CA8-4905-AE3B-BF251EA09B53}
//DEFINE_KNOWN_FOLDER(FOLDERID_NetworkFolder, 0xD20BEEC4, 0x5CA8, 0x4905, 0xAE, 0x3B, 0xBF, 0x25, 0x1E, 0xA0, 0x9B, 0x53);
// {0AC0837C-BBF8-452A-850D-79D08E667CA7}
//DEFINE_KNOWN_FOLDER(FOLDERID_ComputerFolder,   0x0AC0837C, 0xBBF8, 0x452A, 0x85, 0x0D, 0x79, 0xD0, 0x8E, 0x66, 0x7C, 0xA7);
// etc
#define Stricmp stricmp
const char *LookupGUID(char *sGUID)
{
	if (!Stricmp(sGUID, "{208D2C60-3AEA-1069-A2D7-08002B30309D}"))
		return "CLSID_NetworkPlaces";
	else if (!Stricmp(sGUID, "{46e06680-4bf0-11d1-83ee-00a0c90dc849}"))
		return "CLSID_NetworkDomain";
	else if (!Stricmp(sGUID, "{c0542a90-4bf0-11d1-83ee-00a0c90dc849}"))
		return "CLSID_NetworkServer";
	else if (!Stricmp(sGUID, "{54a754c0-4bf1-11d1-83ee-00a0c90dc849}"))
		return "CLSID_NetworkShare";
	else if (!Stricmp(sGUID, "{20D04FE0-3AEA-1069-A2D8-08002B30309D}"))
		return "CLSID_MyComputer";
	else if (!Stricmp(sGUID, "{871C5380-42A0-1069-A2EA-08002B30309D}"))
		return "CLSID_Internet";
	else if (!Stricmp(sGUID, "{F3364BA0-65B9-11CE-A9BA-00AA004AE837}"))
		return "CLSID_ShellFSFolder";
	else if (!Stricmp(sGUID, "{645FF040-5081-101B-9F08-00AA002F954E}"))
		return "CLSID_RecycleBin";
	else if (!Stricmp(sGUID, "{21EC2020-3AEA-1069-A2DD-08002B30309D}"))
		return "CLSID_ControlPanel";
	else if (!Stricmp(sGUID, "{450D8FBA-AD25-11D0-98A8-0800361B1103}"))
		return "CLSID_MyDocuments"; //450D8FBA-AD25-11D0-98A8-0800361B1103
	else
		return sGUID;
}


typedef struct tagLnkFlag2Stg {
    lnkfileflag lf;
    const char *msg;
}LnkFlag2Stg, *PLnkFlag2Stg;

LnkFlag2Stg lf2stg[] = {
    { is_ms_lnk, "Is MS LNK file" },
    { no_file_given, "No file name pointer given" },
    { bad_hdr_str, "INTERNAL ERROR: structure size incorrect" },
    { no_stat, "Unable to stat file" },
    { bad_size, "File size too small" },
    { no_open, "Unable to open file" },
    { no_memory, "Memory allocation failed" },
    { rd_hdr_failed, "Read link header failed" },
    { bad_hdr_size, "Bad link header size" },
    { bad_CLSID, "CLSID not correct" },
    { rd_idls_failed, "IDlist failed!" },
    { rd_idl_failed, "IDitem failed!" },
    { bad_itemid, "Bad ItemID!" },
    { rd_lis_failed, "List Size FAILED!" },
    { rd_li_failed, "List item failed" },
    { exceeded_buf, "Out of buffer range" },
    { lf_flag_max, 0 }
};

const char *LnkFlagtoString( lnkfileflag lf )
{
    PLnkFlag2Stg plf = &lf2stg[0];
    while (plf->msg) {
        if (plf->lf == lf)
            return plf->msg;
    }
    return "Unlisted value";
}

/////////////////////////////////////////////////////////////////
// lnkfileflag Is_MS_LNK_File( char * file, int verb )
// Test (and show) a .LNK file contents
////////////////////////////////////////////////////////////////
 lnkfileflag Is_MS_LNK_File( char * file, int verb )
{
    std::string s;
    char offset[64];
    lnkfileflag lf = is_ms_lnk;
    size_t slhsize = sizeof(ShellLinkHeader);
    if (slhsize != ShellLinkHeaderSize) {
        if (verb) SPRTF("Internal Error: Structure NOT size 0x4c! It is 0x%x\n", slhsize);
        set_last_error(bad_hdr_str);
        return bad_hdr_str;
    }
    char *sl_buf = showlnk_load(file,verb);
    if (!sl_buf) {
        return  get_last_error();
    }
    size_t fsize = get_file_size();
    if (fsize < ShellLinkHeaderSize) {
        set_last_error(bad_size);
        return bad_size;
    }

    if (verb || showlinkinfo) SPRTF("Processing file [%s], size %ld bytes...\n", file, fsize);

    PShellLinkHeader pslh = (PShellLinkHeader)sl_buf;
    if (pslh->HeaderSize != ShellLinkHeaderSize) {
        if (verb) SPRTF("Header size %d (0x%x) NOT %d (0x%x)\n", pslh->HeaderSize, pslh->HeaderSize, ShellLinkHeaderSize, ShellLinkHeaderSize);
        lf = bad_hdr_size;
        goto exit;
    }
    UINT32 LinkFlags = pslh->LinkFlags;
    bool bIsUnicode = (LinkFlags & IsUnicode) ? true : false; // 0x00000080 // The link will contain Unicode strings. This should always be set
    LinkFlags &= ~IsUnicode;

    if (verb) SPRTF("Header size is %d is correct (0x%x)\n", pslh->HeaderSize, pslh->HeaderSize);
    unsigned char *clsid = (unsigned char *)&pslh->LinkClsid[0];
    int i;
    for (i = 0; i < 16; i++) {
        if (clsid[i] != lnkCLSID[i])
            break;
    }
    if (verb) {
        SPRTF("CLSID: ");
        HexDump((PBYTE)clsid, 16, false, false, false);
    }
    if (i < 16) {
        SPRTF("Error: LinkCLSID NOT correct! at off %d of 16. got 0x%02x instead of 0x%02x\n", (i+1), (clsid[i] & 0xff), (lnkCLSID[i] & 0xff));
        lf = bad_CLSID;
        goto exit;
    }
    if (verb) {
        SPRTF("LinkCLSID is correct. [%s]\n", pszLinkCLSID);
        show_link_flags( pslh->LinkFlags );

        bool bIsUni =  (pslh->LinkFlags & 0x0000080) ? true : false; //"IsUnicode"
        if (bIsUni)
            sprtf( "Unicode bit is set\n" );
        else
            sprtf( "Unicode bit is NOT set. Uses default code page.\n" );
        show_file_atts( pslh->FileAttributes );

        int shown_time = 0;
        if ( !( (pslh->AccessTime.dwHighDateTime == 0) && (pslh->AccessTime.dwLowDateTime == 0) ) ) {
            sprtf("AccessTime: ");
            Show_FileTime( &pslh->AccessTime);
            shown_time++;
        }
        if ( !( (pslh->CreationTime.dwHighDateTime == 0) && (pslh->CreationTime.dwLowDateTime == 0) ) ) {
            if (shown_time) SPRTF(" ");
            sprtf("CreationTime: ");
            Show_FileTime( &pslh->CreationTime );
            shown_time++;
        }
        if ( !( (pslh->WriteTime.dwHighDateTime == 0) && (pslh->WriteTime.dwLowDateTime == 0) ) ) {
            if (shown_time) SPRTF(" ");
            sprtf("WriteTime: ");
            Show_FileTime( &pslh->WriteTime );
            shown_time++;
        }
        if (shown_time)
            sprtf("\n");
        else
            sprtf("Appears NO file times!\n");

        sprtf("Indicated file size %d\n", pslh->FileSize);

        sprtf("Icon Index %d\n", pslh->IconIndex);

        sprtf("ShowCommand: %s\n", (pslh->ShowCommand == SW_SHOWNORMAL) ? "SW_SHOWNORMAL" :
            (pslh->ShowCommand == SW_SHOWMAXIMIZED) ? "SW_SHOWMAXIMIZED" :
            (pslh->ShowCommand == SW_SHOWMINNOACTIVE) ? "SW_SHOWMINNOACTIVE" :
            "SW_SHOWNORMAL (as def)" );

    }
    //#define ForceNoLinkInfo	    0x00000100	// The LinkInfo structure (even if present) is ignored
    if (LinkFlags & ForceNoLinkInfo) {
        LinkFlags &= ~ForceNoLinkInfo;
        if (verb > 5) SPRTF("Has ForceNoLinkInfo flag.\n");
    }
    //#define EnableTargetMetadata 0x00080000 // The shell link will attempt to collect target properties into the property list each time it is set
    if (LinkFlags & EnableTargetMetadata) {
        LinkFlags &= ~EnableTargetMetadata;
        if (verb > 5) SPRTF("Has EnableTargetMetadata flag.\n");
    }
    //#define PreferEnvironmentPath 0x02000000 // The target IDList SHOULD NOT be stored; instead, 
    // the path specified in the EnvironmentVariableDataBlock (section 2.5.4) SHOULD be used to refer to the target.
    if (LinkFlags & PreferEnvironmentPath) {
        LinkFlags &= ~PreferEnvironmentPath;
        if (verb > 5) SPRTF("Has PreferEnvironmentPath flag.\n");
    }

    UINT32 flag;
    char *bgn = 0;
    char *cp = sl_buf;
    char *ptr = cp + pslh->HeaderSize; // end of header
    char *endbuf = sl_buf + fsize;    // end of buffer
    char *end;
    int cnt = 0;
    size_t dsize;
    char *nbuf;
    char *ps;
    bool isfound = false;
    // check the link target ID list integrety
    bgn = ptr;
    cp  = ptr;
    sprintf(offset,"%08x", (ptr - sl_buf));

    if (LinkFlags & HasLinkTargetIdList) {
        cnt = 0;
        LinkFlags &= ~HasLinkTargetIdList;
        PLinkTargetIDList pl = (PLinkTargetIDList)ptr;
        ptr = cp + sizeof(pl->IDListSize) + pl->IDListSize;  // set ptr to end this structure
        cp += sizeof(pl->IDListSize);   // bump cp past size item
        end = cp + pl->IDListSize;  // pointer to end
        if (verb > 1) SPRTF("Got IDList of length %d (0x%x)\n", pl->IDListSize, pl->IDListSize);
        if (pl->IDListSize) {
            PItemID pid = (PItemID)&pl->ItemIDList[0];
            end = cp + pl->IDListSize;
            while (pid->ItemIDSize) {
                cnt++;
                if (verb > 1) {
                    SPRTF("ID %d: len %d (%04x)\n", cnt, pid->ItemIDSize, pid->ItemIDSize);
                    HexDump((PBYTE)pid, pid->ItemIDSize);
                    if (pid->ItemIDSize == 0x14) {
                        char *tmp = GetNxtBuf();
                        sprintf(tmp, "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                            pid->Data[3], pid->Data[2], pid->Data[1], pid->Data[0], pid->Data[5], pid->Data[4], 
                            pid->Data[7], pid->Data[6], pid->Data[8], pid->Data[9], pid->Data[10], 
                            pid->Data[11], pid->Data[12], pid->Data[13], pid->Data[14], pid->Data[15]);
                        const char *fold = LookupGUID(tmp);
                        SPRTF("GUID: %s\n",fold);
                    }
                }
                cp += pid->ItemIDSize;
                if (cp >= end) {
                    lf = bad_itemid;
                    goto exit;
                }
                pid = (PItemID)cp;  // to next ItemID
            }
        }
        cp += sizeof(UINT16);
        if (cp != end) {
            if (verb) SPRTF("Error: Iteration of ItemIDList FAILED!\n");
            lf = bad_itemid;
            goto exit;
        }
        if (verb) SPRTF("Done IDList of length %d (0x%x), count %d\n", pl->IDListSize, pl->IDListSize, cnt);
        ptr = end;
    }

    sprintf(offset,"%08x", (ptr - sl_buf));
    if (LinkFlags & HasLinkInfo) {
        LinkFlags &= ~HasLinkInfo;
        PLinkInfo pli = (PLinkInfo)ptr;
        UINT32 LinkInfoSize = pli->LinkInfoSize;
        ptr += LinkInfoSize;
        if (verb || showlinkinfo) SPRTF("Got HasLinkInfo flag: len %d (0x%x)\n", LinkInfoSize, LinkInfoSize);
        if (LinkInfoSize) {
            end = cp + LinkInfoSize;
            if ((verb >= 5) || showlinkinfo) {
                dsize = LinkInfoSize;
                if (end >= endbuf) {
                    dsize = endbuf - cp;
                }
                HexDump((PBYTE)pli, dsize);
            }
            flag = pli->LinkInfoFlags;
            s = "";
            if (pli->LocalBasePathOffset) {
                bgn = cp + pli->LocalBasePathOffset;
                sprintf(offset,"%08x", (bgn - sl_buf));
                if (bgn < end) {
                    s = bgn;
                    //HexDump((PBYTE)bgn, (end - bgn));
                }
            }
            if (pli->CommonPathSuffixOffset) {
                 bgn = cp + pli->CommonPathSuffixOffset;
                 sprintf(offset,"%08x", (bgn - sl_buf));
                 if (bgn < end) {
                     s += bgn;
                 }
            }
            if (s.size()) {
                // if ((verb || showlinkinfo) && s.size()) {
                if (is_file_or_directory((char *)s.c_str()) == 1) {
                    SPRTF("%s ok\n",s.c_str());
                } else {
                    SPRTF("%s NF\n",s.c_str());
                }
            }
            if (flag & VolumeIDAndLocalBasePath) { // If set, the VolumeID and LocalBasePath fields are present, 
                if (pli->VolumeIDOffset) {
                    bgn = cp + pli->VolumeIDOffset;
                    sprintf(offset,"%08x", (bgn - sl_buf));
                    //if (bgn < end) {
                    //    s = bgn;
                    //}
                }
                flag &= ~VolumeIDAndLocalBasePath;
            }
            if (flag & CommonNetworkRelativeLinkAndPathSuffix) { // If set, the CommonNetworkRelativeLink field is present, 
                if (pli->CommonNetworkRelativeLinkOffset) {
                    bgn = cp + pli->CommonNetworkRelativeLinkOffset;
                    sprintf(offset,"%08x", (bgn - sl_buf));
                }
                flag &= ~CommonNetworkRelativeLinkAndPathSuffix;
            }
            if (pli->LinkInfoSize >= 0x24) {
                if (pli->LocalBasePathOffsetUnicode) { //	UInt32	Present if LinkInfoSize >= 0x24	See LocalBasePathOffset
                    bgn = cp + pli->LocalBasePathOffsetUnicode;
                    sprintf(offset,"%08x", (bgn - sl_buf));
                }
                if (pli->CommonPathSuffixOffsetUnicode) { //	UInt32	Present if LinkInfoSize >= 0x24	See CommonPathSuffixOffset
                    bgn = cp + pli->CommonPathSuffixOffsetUnicode;
                    sprintf(offset,"%08x", (bgn - sl_buf));
                }
            }
        }
    } else if (showlinkinfo) {
        SPRTF("No LinkInfo flag present\n");
    }
    sprintf(offset,"%08x", (ptr - sl_buf));
    //#define HasName	            0x00000004	// The StringData section contains the NAME_STRING field
    cp = ptr;
    PSTRINGDATA_FIELD psd;
    size_t add, ii, max;
    if (LinkFlags & HasName) {
        LinkFlags &= ~HasName;
        psd = (PSTRINGDATA_FIELD)ptr;
        add = sizeof(psd->CountCharacters);
        add += psd->CountCharacters;
        if (bIsUnicode) add += psd->CountCharacters;
        if (verb) SPRTF("HasName flag: len %ld (0x%x) val %#x\n", add, add, psd->CountCharacters);
        if (verb >= 5) HexDump((PBYTE)psd, add);
        ptr += add;
        sprintf(offset,"%08x", (ptr - sl_buf));

    }
    // #define HasRelativePath	    0x00000008	// The StringData section contains the RELATIVE_PATH field
    if (LinkFlags & HasRelativePath) {
        LinkFlags &= ~HasRelativePath;
        psd = (PSTRINGDATA_FIELD)ptr;
        add = sizeof(psd->CountCharacters);
        add += psd->CountCharacters;
        if (bIsUnicode) add += psd->CountCharacters;
        if (verb) SPRTF("HasRelativePath flag: len %ld (0x%x) val %#x\n", add, add, psd->CountCharacters);
        if (verb >= 5) HexDump((PBYTE)psd, add);
        nbuf = GetNxtBuf(); // = MX_ONE_BUF (bytes)
        ps = GetNxtBuf();
        max = add - sizeof(psd->CountCharacters);
        // collect the UNICODE using string length in bytes
        for (ii = 0; ii < max; ii++) {
            if (ii >= (MX_ONE_BUF - 2))
                break;
            nbuf[ii] = psd->String[ii];
        }
        nbuf[ii++] = 0; // add unicode nul terminations
        nbuf[ii] = 0;
        i = Wide2UTF8( (wchar_t *)nbuf, ps, MX_ONE_BUF );
        if (i) {
            char *qpath = GetNxtBuf();
            DWORD dwd = GetFullPathName(file, MX_ONE_BUF, qpath, NULL);
            if (dwd) {
                nbuf = get_path_from_filename(qpath);
            }
            else {
                nbuf = get_path_from_filename(file);
            }
            if ((*nbuf == 0) || (!strcmp(nbuf, ".\\")) || (!strcmp(nbuf, "./"))) {
                // _getcwd returns a string that represents the path of the current working directory. 
                // If the current working directory is the root, the string ends with a backslash ( \ ). 
                // If the current working directory is a directory other than the root, the string ends 
                // with the directory name and not with a backslash.
                nbuf = _getcwd(nbuf, MX_ONE_BUF);
                if (nbuf) {
                    ii = strlen(nbuf);
                    if (ii && (nbuf[ii - 1] != '\\') && (nbuf[ii - 1] != '/')) {
                        strcat(nbuf, "\\");
                    }
                }
            }
            isfound = false;
            if (nbuf) {
                strcat(nbuf, ps);
                if (is_file_or_directory(nbuf) == 1) {
                    // could try to do better if a relative path
                    char *tmp = GetNxtBuf();
                    char *res = fix_relative_path(nbuf, tmp);
                    if (res) {
                        if (is_file_or_directory(res) == 1) {
                            SPRTF("AbsPath: %s ok\n", res);
                        }
                        else {
                            SPRTF("RelPath: %s ok\n", nbuf);
                        }
                    }
                    else {
                        SPRTF("RelPath: %s ok\n", nbuf);
                    }
                    isfound = true;
                }
                else {
                    SPRTF("RelPath: %s NOT FOUND\n", nbuf);
                }
            }
            // maybe only show relpath if can NOT output full path
            if (!isfound)
                SPRTF("RelPath: [%s]\n", ps);
        }
        ptr += add;
        sprintf(offset,"%08x", (ptr - sl_buf));
    }
    //#define HasWorkingDir	    0x00000010  // The StringData section contains the WORKING_DIR field
    if (LinkFlags & HasWorkingDir) {
        LinkFlags &= ~HasWorkingDir;
        psd = (PSTRINGDATA_FIELD)ptr;
        add = sizeof(psd->CountCharacters);
        add += psd->CountCharacters;
        if (bIsUnicode) add += psd->CountCharacters;
        if (verb) SPRTF("HasWorkingDir flag: len %ld (0x%x) val %#x\n", add, add, psd->CountCharacters);
        if (verb >= 5) HexDump((PBYTE)psd, add);
        ptr += add;
        sprintf(offset,"%08x", (ptr - sl_buf));
    }
    //#define HasArguments	    0x00000020	// The StringData section contains the COMMAND_LINE_ARGUMENTS field
    if (LinkFlags & HasArguments) {
        LinkFlags &= ~HasArguments;
        psd = (PSTRINGDATA_FIELD)ptr;
        add = sizeof(psd->CountCharacters);
        add += psd->CountCharacters;
        if (bIsUnicode) add += psd->CountCharacters;
        if (verb) SPRTF("HasArguments flag: len %ld (0x%x) val %#x\n", add, add, psd->CountCharacters);
        if (verb >= 5) HexDump((PBYTE)psd, add);    // dump for length
        nbuf = GetNxtBuf(); // = MX_ONE_BUF (bytes)
        ps = GetNxtBuf();
        max = add - sizeof(psd->CountCharacters);
        // collect the UNICODE using string length in bytes
        for (ii = 0; ii < max; ii++) {
            if (ii >= (MX_ONE_BUF - 2))
                break;
            nbuf[ii] = psd->String[ii];
        }
        nbuf[ii++] = 0; // add unicode nul terminations
        nbuf[ii] = 0;
        i = Wide2UTF8((wchar_t *)nbuf, ps, MX_ONE_BUF);
        if (i) {
            SPRTF("Argument: [%s]\n", ps);
        }
        ptr += add;
        sprintf(offset,"%08x", (ptr - sl_buf));
    }
    //#define HasIconLocation	    0x00000040	// The StringData section contains the ICON_LOCATION field
    if (LinkFlags & HasIconLocation) {
        LinkFlags &= ~HasIconLocation;
        psd = (PSTRINGDATA_FIELD)ptr;
        add = sizeof(psd->CountCharacters);
        add += psd->CountCharacters;
        if (bIsUnicode) add += psd->CountCharacters;
        if (verb) SPRTF("HasIconLocation flag: len %ld (0x%x) val %#x\n", add, add, psd->CountCharacters);
        if (verb >= 5) HexDump((PBYTE)psd, add);
        ptr += add;
        sprintf(offset,"%08x", (ptr - sl_buf));
    }
    //#define HasExpString        0x00000200  // The shell link is saved with an EnvironmentVariableDataBlock (section 2.5.4).
    if (LinkFlags & HasExpString) {
        LinkFlags &= ~HasExpString;
        psd = (PSTRINGDATA_FIELD)ptr;
        add = sizeof(psd->CountCharacters);
        add += psd->CountCharacters;
        if (bIsUnicode) add += psd->CountCharacters;
        if (verb) SPRTF("HasExpString flag: len %ld (0x%x) val %#x\n", add, add, psd->CountCharacters);
        if (verb >= 5) HexDump((PBYTE)psd, add);
        ptr += add;
        sprintf(offset,"%08x", (ptr - sl_buf));
    }

    //#define HasExpIcon	        0x00004000	// The IconEnvironmentDataBlock is present
    if (LinkFlags & HasExpIcon) {
        LinkFlags &= ~HasExpIcon;
        psd = (PSTRINGDATA_FIELD)ptr;
        add = sizeof(psd->CountCharacters);
        add += psd->CountCharacters;
        if (bIsUnicode) add += psd->CountCharacters;
        if (verb) SPRTF("HasExpIcon flag: len %ld (0x%x) val %#x\n", add, add, psd->CountCharacters);
        if (verb >= 5) HexDump((PBYTE)psd, add);
        ptr += add;
        sprintf(offset,"%08x", (ptr - sl_buf));
    }

    PBLOCK pb;
    pb = (PBLOCK)ptr;
    cnt = 0;
    while (pb->size) {
        add = pb->size;
        // SPECIAL EXCEPTION
        if (add == 0xfeeefeee) {
            break;
        }
        cnt++;
        ptr += add;
        sprintf(offset,"%08x", (ptr - sl_buf));
        if (verb) SPRTF("Block %d: len %d (0x%x)\n", cnt, add, add);
        if (ptr >= endbuf) {
            if (verb) SPRTF("Error in block enumeration! Exceeding buffer\n");
            lf = exceeded_buf;
            goto exit;
        }
        if (verb >= 5) HexDump((PBYTE)pb,add);
        pb = (PBLOCK)ptr;
    }
    if (verb && LinkFlags) {
        SPRTF("UNUSED ");
        show_link_flags(LinkFlags);
    }
    if (ptr >= endbuf) {
        if (verb) SPRTF("Warning: pointer beyond end of buffer!\n");
    } else {
        pb = (PBLOCK)ptr;
        add = endbuf - ptr;
        if (verb) {
            if (add >= 4) {
                if (pb->size <= 4) {
                    SPRTF("Have reached termination at EOF\n");
                }
            } else {
                SPRTF("Remaining %ld bytes in buffer\n", add);
            }
        }
    }

exit:
    if (verb) {
        if (lf == is_ms_lnk) {
            SPRTF("Returning is_ms_lnk flag for [%s]\n", file);
        } else {
            SPRTF("Decode of [%s] FAILED\n", file);
        }
    }
    set_last_error(lf);
	return lf;
}


// eof - showlnk2.cxx
