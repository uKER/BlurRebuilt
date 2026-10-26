#ifndef _SYSTEM_FILESYSTEMHELPERTYPES_H__
#define _SYSTEM_FILESYSTEMHELPERTYPES_H__

namespace Sys
{

enum E_FileAccess
{
    E_FileAccess_Read		= 0x00000001,
    E_FileAccess_Write		= 0x00000002,
    E_FileAccess_Append		= 0x00000003,
};

enum E_FileCreate
{
	E_FileCreate_Unused = 100,
    E_FileCreate_CreateAlways = 0,
    E_FileCreate_CreateNew,
    E_FileCreate_OpenAlways,
    E_FileCreate_OpenExisting,
};

enum E_FileType
{
    E_FileType_NotPresent = 0,
    E_FileType_File,
    E_FileType_Directory
};

typedef U64 C_StreamPos;
typedef U64 C_StreamSize;

}

#endif
