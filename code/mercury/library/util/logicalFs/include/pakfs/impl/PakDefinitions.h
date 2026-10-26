#ifndef LOGICALFS_PAKFS_PAK_DEFINITIONS_H
#define LOGICALFS_PAKFS_PAK_DEFINITIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#define PAK_FOURCC_U32 FOURCC_U32('PAK2')
#define PAK_VERSION 1
// [Blur2PC] Retail Blur PC (Apr/May 2010) ships version 2 paks; the reader accepts them
// (docs/analysis.md "Pak version 2"). Writing still produces version 1.
#define PAK_VERSION_READ_MAX 2
#define PAK_VERSION_CORRUPT				-1

#define PAK_SECTOR_SIZE_DEFAULT			2* 1024
#define PAK_SECTOR_SIZE_MAX				32*1024
#define PAK_SECURITY_READ_BUFFER_SIZE	16*1024
#define PAK_IFILE_BUFFER_SIZE			32*1024
#define PAK_DATA_ISDIRECTORY			-1
#define PAK_FAT_ENTRY_NO_PARENT			-1

namespace Pak
{
	enum E_FatEntryType
	{
		E_FatEntryType_Normal = 0,
		E_FatEntryType_Secure = 1
	};

	enum E_Codec
	{
		E_Codec_Unknown = 0,
		E_Codec_Raw = 1,
		E_Codec_ZLib = 2,
		E_Codec_XMem = 3,
		E_Codec_PacketZLib = 4,
		E_Codec_PacketXMem = 5
	};

	enum E_UpdateType
	{
		E_UpdateType_None,
		E_UpdateType_AddUpdate,
		E_UpdateType_CreateIncremental,
		E_UpdateType_Delete
	};

	extern Debug::Logger &g_log;
}

#endif
