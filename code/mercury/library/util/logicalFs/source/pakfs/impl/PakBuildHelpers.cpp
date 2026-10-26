#include "LogicalFsPch.h"


#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakFatEntry.h>
#include <PakFs/Impl/PakFile.h>
#include <PakFs/Impl/PakBuildHelpers.h>
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/Impl/PakIoMediator.h>

#ifdef CFG_CONVERTER

namespace Pak
{

const U32 kDriveLength = 2;

Sys::C_FilePath ConvertToPakRootPath(
	const Sys::C_FilePath& rootPath,
	const Sys::C_FilePath& remapPath,
	const Sys::C_FilePath& filePath, 
	S32 rootOffset)
{
	S32 rootPathLength = rootPath.Length()-rootOffset;
	S32 filePathLength = filePath.Length()-rootOffset;
	//Find root path in given filepath
	if (rootPathLength > filePathLength)
		return "";
	//Find relative path from absolute root
	if (memcmp(rootPath.Chars()+rootOffset, filePath.Chars()+rootOffset, rootPathLength))
		//Failure.
		return "";

	//Make overlay path relative filepath
	Sys::C_FilePath tempPath = Sys::C_FilePath("/") + filePath.String()(rootPathLength+rootOffset, filePath.Length());
	return remapPath + tempPath;
	//return Sys::C_FilePath("/") + filePath.String()(rootPathLength+rootOffset, filePath.Length());
}

void CreatePakDirectoryChain(
	Btl::Set<C_FileToPak>& filePaths, const C_FileToPak& filePath);

bool DirFiltered( const Btl::Vector<Sys::C_FilePath>& filePaths, 
					  Sys::C_FilePath filePath) 
{
	filePath += "/";

	for (S32 i = 0; i < filePaths.Size(); ++i)
	{
		Sys::C_FilePath filter = Sys::C_FilePath("/") + filePaths[i] + "/";

		if (filePath.Search(filter) == 0)
		{
			LOGINFO(g_log, "Excluded dir %s using filter %s", filePath.Chars(), filter.Chars());
			//we've found an excluded match completely enclosed by dir tokens
			return true;
		}
	}
	return false;
}

bool BuildFileSetFromDir(
	Btl::Set<C_FileToPak>& fileSet,
	Sys::I_FileSystem& fs,
	const Sys::C_FilePath& dir,
	const Sys::C_FilePath& pakRootPath,
	const Sys::C_FilePath& pakRemapPath,
	const Btl::Vector<Sys::C_FilePath>& excludedPaths,
	bool recurse)
{
	Sys::C_FilePath rootMappedDir(ConvertToPakRootPath(pakRootPath, pakRemapPath, dir, kDriveLength));
	if (DirFiltered(excludedPaths, rootMappedDir))
		return true;

	Sys::C_DirectoryInfo dirInfo;
	if (fs.DirectoryInfo(dir, &dirInfo))
	{
		const Sys::FileInfoArr& fileInfoArray = dirInfo.FileInfoArray();
		for (S32 i = 0; i < fileInfoArray.Size(); ++i)
		{
			const Sys::C_FileInfo& fileInfo = fileInfoArray[i];
			Sys::C_FilePath relativeFilePath(
				ConvertToPakRootPath(pakRootPath, pakRemapPath, fileInfo.m_FileName, kDriveLength));
			if (relativeFilePath == "")
			{
				LOGERROR(g_log, "Could not convert filepath %s to working dir %s!",
					fileInfo.m_FileName.Chars(), pakRootPath.Chars());
				return false;
			}

			if (fileInfo.m_FileType == Sys::E_FileType_File)
				fileSet.Insert(C_FileToPak(
					fileInfo.m_FileName, 
					relativeFilePath, 
					fileInfo.m_FileType,
					fileInfo.m_FileTime,
					fileInfo.m_FileSize,
					0,
					0));
			else if (fileInfo.m_FileType == Sys::E_FileType_Directory && recurse)
				BuildFileSetFromDir(fileSet, fs, fileInfo.m_FileName, pakRootPath, pakRemapPath, excludedPaths, true);
			else
				continue;
		}
	}

	return true;
}

bool BuildFileSet(
	Btl::Set<C_FileToPak>& fileSet,
	Sys::I_FileSystem& fs,
	const Sys::C_FilePath& pakRootPath,
	const Sys::C_FilePath& pakRemapPath,
	bool generateRootPath,
	bool recurse,
	const C_FilePathsToProcess& workList)
{
	Btl::Vector<Sys::C_FilePath> dirs;
	for (S32 i = 0; i < workList.filePaths.Size(); ++i)
	{
		const Sys::C_FilePath& originalFilePath = workList.filePaths[i];

		Sys::C_FileInfo fileInfo;
		Sys::C_DirectoryInfo dirInfo;
		if (fs.DirectoryInfo(originalFilePath, &dirInfo))
			dirs.PushBack(originalFilePath);
		else if (fs.FileInfo(originalFilePath, &fileInfo))
		{
			Sys::C_FilePath relativeFilePath;
			
			if (generateRootPath)		// Generate root path?
			{
				// Get the path of the 'file' and use that as the pakRootPath
				Sys::C_FilePath filePath = Sys::ExtractDirectory(originalFilePath);
				relativeFilePath = ConvertToPakRootPath(filePath, pakRemapPath, originalFilePath, kDriveLength);
			}
			else
				relativeFilePath = ConvertToPakRootPath(pakRootPath, pakRemapPath, originalFilePath, kDriveLength);

			if (relativeFilePath == "")
			{
				LOGERROR(g_log, "Could not convert filepath %s to working dir %s!",
					originalFilePath.Chars(), pakRootPath.Chars());
				return false;
			}

			if (fileInfo.m_FileType == Sys::E_FileType_File)
				fileSet.Insert(C_FileToPak(
					originalFilePath, 
					relativeFilePath,
					fileInfo.m_FileType,
					fileInfo.m_FileTime,
					fileInfo.m_FileSize,
					0,
					0));
			else if (fileInfo.m_FileType == Sys::E_FileType_Directory)
				dirs.PushBack(originalFilePath + "\\");
			else
				continue;
		}
		else
		{
			LOGERROR(g_log, "File or directory %s does not exist!",	originalFilePath.Chars());
			return false;
		}
	}

	Sys::C_FilePath rootPath;
	rootPath = pakRootPath;

	for (S32 i = 0; i < dirs.Size(); ++i)
	{
		if (generateRootPath)
			rootPath = Sys::ExtractDirectory(dirs[i]);

		dirs[i] += "\\";

		if (!BuildFileSetFromDir(fileSet, fs, dirs[i], rootPath, pakRemapPath, workList.excludedPaths, recurse))
			return false;
	}

	return true;
}

bool BuildFileListToPak(
	Btl::Vector<C_FileToPak>& fileList,
	Sys::I_FileSystem& fs,
	const Sys::C_FilePath& pakRootPath,
	const Sys::C_FilePath& pakRemapPath,
	bool generateRootPaths,
	bool recurse,
	const C_FilePathsToProcess& workList)
{
	if (pakRootPath.Length() == 0 && !generateRootPaths)
	{
		LOGERROR(g_log, "No pak root path specified!");
		return false;
	}

	Btl::Set<C_FileToPak> inputFileSet;

	//Add/Update/Create incremental
	inputFileSet.Insert(C_FileToPak("", "\\", Sys::E_FileType_Directory, 0, 0, 0, 0));
	if (!BuildFileSet(inputFileSet, fs, pakRootPath, pakRemapPath, generateRootPaths, recurse, workList))
		return false;

	Btl::Vector<C_FileToPak> inputFileCopy;
	for (Btl::Set<C_FileToPak>::ConstIterator it = inputFileSet.Begin(); 
		it != inputFileSet.End(); ++it)
	{
		inputFileCopy.PushBack(*it);
	}

	for (S32 i = 0; i < inputFileCopy.Size(); ++i)
	{
		CreatePakDirectoryChain(inputFileSet, inputFileCopy[i]);
	}

	for (Btl::Set<C_FileToPak>::ConstIterator it = inputFileSet.Begin(); 
		it != inputFileSet.End(); ++it)
	{
		fileList.PushBack(*it);
	}

	return true;
}

void CreatePakDirectoryChain(
	Btl::Set<C_FileToPak>& filePaths, const C_FileToPak& filePath)
{
	Btl::Vector<Sys::C_FilePath> tokenised;
	filePath.relativeFilePath.TokeniseByDir(tokenised);

	S32 lastDirIndex = tokenised.Size()-1;
	if (filePath.fileType == Sys::E_FileType_Directory)
		--lastDirIndex;
	
	Sys::C_FilePath dir("\\");
	for (S32 i = 0 ; i < lastDirIndex; i++)
	{
		dir += tokenised[i] + "\\";
		C_FileToPak toInsert(
			"",
			dir, 
			Sys::E_FileType_Directory,
			0,
			0,
			0, 
			0);

		Btl::Set<C_FileToPak>::Iterator it = filePaths.Find(toInsert);
		if (it == filePaths.End())
			filePaths.Insert(C_FileToPak(
				"",
				Sys::RemoveTrailingSlash(dir),
				Sys::E_FileType_Directory,
				0,
				0,
				0,
				0));
	}
}

bool BuildFileListToDelete(
	Btl::Vector<C_FileToPak>& fileList,
	const Sys::C_FilePath& pakRootPath,
	const Sys::C_FilePath& pakRemapPath,
	bool convertPaths,
	const C_FilePathsToProcess& workList)
{
	if (pakRootPath.Length() == 0 && !convertPaths)
	{
		LOGERROR(g_log, "No pak root path specified!");
		return false;
	}
	Btl::Set<C_FileToPak> inputFileSet;

	for (S32 i = 0; i < workList.filePaths.Size(); ++i)
	{
		const Sys::C_FilePath& originalFilePath = workList.filePaths[i];
		Sys::C_FilePath relativeFilePath;

		if (convertPaths)
		{
			relativeFilePath = ConvertToPakRootPath(pakRootPath, pakRemapPath, originalFilePath, kDriveLength);
			if (relativeFilePath == "")
			{
				LOGERROR(g_log, "Could not convert filepath %s to working dir %s!",
					originalFilePath.Chars(), pakRootPath.Chars());
				return false;
			}
		}
		else
			relativeFilePath = originalFilePath;

		inputFileSet.Insert(C_FileToPak(
			originalFilePath,
			relativeFilePath, 
			Sys::E_FileType_NotPresent,
			0,
			0,
			0,
			0));
	}

	for (Btl::Set<C_FileToPak>::ConstIterator it = inputFileSet.Begin(); 
		it != inputFileSet.End(); ++it)
	{
		fileList.PushBack(*it);
	}

	return true;
}

bool IsChildOf(const C_PakFile& pak, const C_PakFatEntry* pParent, const C_PakFatEntry* pChild)
{
	const C_PakFatEntry* pCurParent = (const C_PakFatEntry*)(pChild->parentDirectoryOffsetBytes == PAK_FAT_ENTRY_NO_PARENT ? 0 : pak.pFatEntries + pChild->parentDirectoryOffsetBytes);

	while(pCurParent)
	{
		if (pCurParent == pParent)
			return true;

		pCurParent = (const C_PakFatEntry*)(pCurParent->parentDirectoryOffsetBytes == PAK_FAT_ENTRY_NO_PARENT ? 0 : pak.pFatEntries + pCurParent->parentDirectoryOffsetBytes);
	}

	return false;
}

}

#endif // CFG_CONVERTER

namespace Pak
{

bool VerifyFileHash(Sha1::C_HashContext* pHashContext, C_PakIoMediator* pMediator, Sys::C_StreamPos physicalStartPos, S64 knownPhysicalLength)
{
	U8 actualHash[20];
	Sha1::C_HashContext hashContext;

	Sys::C_StreamPos curPos = physicalStartPos;

	U32 physicalLength(0);
	if (knownPhysicalLength >= 0)	// [Blur2PC] v2: the hash covers the stored bytes, no prefix
		physicalLength = (U32)knownPhysicalLength;
	else
	pMediator->Read(physicalLength, &curPos);
	
	// Process the data in chunks to reduce memory spikes
	U8 compressedData[32768];

	U32 leftToHash = physicalLength;

	while (leftToHash > 0)
	{
		U32 readSize = (leftToHash >= sizeof(compressedData)) ? sizeof(compressedData) : leftToHash;

		pMediator->Read(compressedData, readSize, &curPos);
		
		if (!hashContext.Valid() || !hashContext.UpdateHash(compressedData, readSize)) 
			return false;

		leftToHash -= readSize;
	}

	hashContext.GetHash(actualHash);

	if (!pHashContext->VerifyHash(actualHash, sizeof(actualHash)))
	{
		LOGERROR(g_log, "Failed to verify file hash. File data altered since given hash generated!");
		return false;
	}
	
	return true;
}

}
