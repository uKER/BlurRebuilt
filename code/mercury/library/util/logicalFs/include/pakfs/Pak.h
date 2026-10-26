#ifndef LOGICALFS_PAK_H
#define LOGICALFS_PAK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Set.h>
#include <PakFs/Impl/PakBuildHelpers.h>
#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakFile.h>

namespace Security
{
	class I_Security;
}

namespace Pak
{
#ifdef CFG_CONVERTER
	struct C_PakOptions
	{
		C_PakOptions()
		{
			sectorSize = PAK_SECTOR_SIZE_DEFAULT;
			codec = E_Codec_ZLib;
			codecFlags = 0;
			recurse = false;
			updateType = E_UpdateType_None;
			convertFilePathsForDelete = true;
			generateRootPath = false;
			bigEndian = false;
			pathSeparator = '/';
		}

		C_PakOptions(const C_PakOptions& rhs)
		{
			*this = rhs;
		}

		const C_PakOptions& operator=(const C_PakOptions& rhs)
		{
			sectorSize = rhs.sectorSize;
			codec = rhs.codec;
			codecFlags = rhs.codecFlags;
			pakFilePath = rhs.pakFilePath;
			pakRootPath = rhs.pakRootPath;
			pakRemapPath = rhs.pakRemapPath;
			metadata = rhs.metadata;
			recurse = rhs.recurse;
			updateType = rhs.updateType;
			convertFilePathsForDelete = rhs.convertFilePathsForDelete;
			generateRootPath = rhs.generateRootPath;
			bigEndian = rhs.bigEndian;
			pathSeparator = rhs.pathSeparator;
			return *this;
		}

		U32				sectorSize;
		E_Codec			codec;
		U32				codecFlags;
		Sys::C_FilePath pakFilePath;
		Sys::C_FilePath pakRootPath;
		Sys::C_FilePath	pakRemapPath;
		Btl::String		metadata;
		bool			recurse;
		E_UpdateType	updateType;
		bool			convertFilePathsForDelete;
		bool			generateRootPath;
		bool			bigEndian;
		char			pathSeparator;
	};

	bool CreatePak(
		U8* pSecurityHash,
		Sys::I_FileSystem& pakFs,
		Sys::I_FileSystem& srcDataFs,
		const C_PakOptions& options,
		const C_FilePathsToProcess& filePaths);

	bool UpdatePak(
		Sys::I_FileSystem& pakFs,
		Sys::I_FileSystem& srcDataFs,
		const C_PakOptions& options,
		const C_FilePathsToProcess& filePaths);
#endif

	C_PakFile* ReadPak(
		Sys::I_FileSystem& pakFs,
		const Sys::C_FilePath& pakFilePath,
		Sys::E_FileAccess access = Sys::E_FileAccess_Read,
		const Security::I_Security* pSecurity = NULL,
		const Sys::C_OpenParams& params = Sys::C_OpenParams());

	bool ReadPak(
		Pak::C_PakFile& pak,
		const Security::I_Security* pSecurity = NULL);

	bool Verify(
		Sys::I_FileSystem& pakFs,
		const Sys::C_FilePath& pakFilePath);

	bool Verify(
		C_PakFile& pak);

	bool Verify2(
		C_PakFile& pak);

	bool Extract(
		Sys::I_FileSystem& outputFs,
		const Sys::C_FilePath& outputPath,
		Sys::I_FileSystem& pakFs,
		const Sys::C_FilePath& pakFilePath,
		const Btl::Vector<Sys::C_FilePath>* pFilePaths = 0);

	bool Extract(
		Sys::I_FileSystem& outputFs,
		const Sys::C_FilePath& outputPath,
		C_PakFile& pak,
		const Btl::Vector<Sys::C_FilePath>* pFilePaths = 0);
}

#endif
