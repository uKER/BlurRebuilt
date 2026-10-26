#ifndef PAK_BUILD_HELPERS_H
#define PAK_BUILD_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef CFG_CONVERTER

#include <Btl/Containers/Set.h>
#include <Btl/Containers/Vector.h>

namespace Pak
{
	class C_PakFile;
	struct C_PakFatEntry;

	struct C_FileToPak
	{
		C_FileToPak() {}
		C_FileToPak(
			const Sys::C_FilePath& originalFilePath_,
			const Sys::C_FilePath& relativeFilePath_,
			const Sys::E_FileType fileType_,
			U64 fileTime_,
			U64 fileSize_,
			C_PakFile* pBasePak_,
			const C_PakFatEntry* pPakEntry_) :
			originalFilePath(originalFilePath_),
			relativeFilePath(relativeFilePath_),
			fileType(fileType_),
			fileTime(fileTime_),
			fileSize(fileSize_),
			pBasePak(pBasePak_),
			pBasePakFatEntry(pPakEntry_) {}

		C_FileToPak(const C_FileToPak& rhs)
		{
			*this = rhs;
		}

		const C_FileToPak& operator=(const C_FileToPak& rhs)
		{
			originalFilePath = rhs.originalFilePath;
			relativeFilePath = rhs.relativeFilePath;
			fileType = rhs.fileType;
			fileTime = rhs.fileTime;
			fileSize = rhs.fileSize;
			pBasePak = rhs.pBasePak;
			pBasePakFatEntry = rhs.pBasePakFatEntry;
			return *this;
		}

		bool operator < (const C_FileToPak& rhs) const
		{
			return relativeFilePath < rhs.relativeFilePath;
		}

		bool operator > (const C_FileToPak& rhs) const
		{
			return relativeFilePath > rhs.relativeFilePath;
		}

		Sys::C_FilePath originalFilePath;
		Sys::C_FilePath relativeFilePath;
		Sys::E_FileType fileType;
		U64				fileTime;
		U64				fileSize;
		C_PakFile*				pBasePak;
		const C_PakFatEntry*	pBasePakFatEntry;
	};

	struct C_FilePathsToProcess
	{
		C_FilePathsToProcess() {}
		C_FilePathsToProcess(const C_FilePathsToProcess& rhs)
		{
			*this = rhs;
		}
		const C_FilePathsToProcess& operator=(const C_FilePathsToProcess& rhs)
		{
			filePaths = rhs.filePaths;
			excludedPaths = rhs.excludedPaths;
			return *this;
		}
		Btl::Vector<Sys::C_FilePath> filePaths;
		Btl::Vector<Sys::C_FilePath> excludedPaths;

	private:
	};

	INLINE int FileToPakCompareAlphabetical(const void *lhs, const void *rhs)
	{
		const C_FileToPak& a(*(const C_FileToPak*)lhs);
		const C_FileToPak& b(*(const C_FileToPak*)rhs);

		if (a < b)
			return -1;
		if (a > b)
			return 1;

		return 0;
	}

	bool BuildFileListToPak(
		Btl::Vector<C_FileToPak>& fileList,
		Sys::I_FileSystem& fs,
		const Sys::C_FilePath& pakRootPath,
		const Sys::C_FilePath& pakRemapPath,
		bool generateRootPaths,
		bool recurse,
		const C_FilePathsToProcess& workList);

	bool BuildFileListToDelete(
		Btl::Vector<C_FileToPak>& fileList,
		const Sys::C_FilePath& pakRootPath,
		const Sys::C_FilePath& pakRemapPath,
		bool convertPaths,
		const C_FilePathsToProcess& workList);

	bool IsChildOf(const C_PakFile& pak, const C_PakFatEntry* pParent, const C_PakFatEntry* pChild);
}

#endif

namespace Pak
{
	class C_PakIoMediator;

	namespace Sha1
	{
		class C_HashContext;
	}

	bool VerifyFileHash(Sha1::C_HashContext* pHashContext, C_PakIoMediator* pMediator, Sys::C_StreamPos physicalStartPos, S64 knownPhysicalLength = -1);	// [Blur2PC] knownPhysicalLength for v2 paks

}

#endif
