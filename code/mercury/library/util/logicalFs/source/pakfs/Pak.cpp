#include "LogicalFsPch.h"
#include <Btl/Containers/OpenHashMap.h>
#include <Btl/Containers/OpenHashSet.h>

#include <System/Serialise/BinarySerialiseAutoEndian.h>
#include <System/Serialise/C_MemoryStream.h>
#include <System/Serialise/FileSystemGlobals.h>

#include <System/Misc/Security.h>

#include <Misc_Shared/DataStructure/StringTable.h>

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakBuildHelpers.h>
#include <PakFs/Impl/PakCreateFiles.h>
#include <PakFs/Impl/PakEntryFile.h>
#include <PakFs/Impl/PakFile.h>
#include <PakFs/Impl/PakIoMediator.h>
#include <PakFs/Impl/PakRead.h>
#include <PakFs/Impl/PakSha1.h>
#include <PakFs/Impl/PakUtil.h>
#include <PakFs/Impl/PakWrite.h>
#include <PakFs/Pak.h>

namespace Pak
{

#ifdef CFG_CONVERTER

//Creates a pak file without expanding the toProcess list folders.
bool CreatePak(
	U8* pSecurityHash,
	Sys::I_FileSystem& pakFs,
	Sys::I_FileSystem& srcDataFs,
	const C_PakOptions& options,
	const Btl::Vector<C_FileToPak>& toProcess_)
{
	Btl::Vector<C_FileToPak> toProcess = (toProcess_);
	if (toProcess.Size() == 0) return true;

	qsort(&toProcess[0], toProcess.Size(), sizeof(C_FileToPak), &FileToPakCompareAlphabetical);

	Btl::OpenHashSet<U32> uniqueCrcSet;

	struct C_FatInfo
	{
		C_FatInfo() {}
		C_FatInfo(const C_FatInfo& rhs) { *this = rhs; }
		const C_FatInfo& operator=(const C_FatInfo& rhs)
		{
			if (&rhs != this)
			{
				sortedFatIdx = rhs.sortedFatIdx;
				stringOffset = rhs.stringOffset;
			}
			return *this;
		}
		U32					sortedFatIdx;
		U32					stringOffset;
	};
	typedef Btl::OpenHashMap<U32, C_FatInfo> CrcFatInfoMap;

	Pak::C_PakFile pak;
	if (!pak.pMediator->Open(&pakFs, options.pakFilePath, Sys::E_FileAccess_Write))
	{
		LOGERROR(g_log, "Failed to create output file %s.\n", options.pakFilePath.Chars());
		return false;
	}
	Sys::C_FileInfo pakInfo;
	if (!pakFs.FileInfo(options.pakFilePath, &pakInfo))
	{
		LOGERROR(g_log, "Failed to query output file %s.\n", options.pakFilePath.Chars());
		return false;
	}
	pak.pMediator->SetEndianSwapFlag(options.bigEndian);

	pak.pMediator->SetSectorSize(options.sectorSize);
	CrcFatInfoMap fatInfoByCrc;

	//Set some initial header info.
	pak.header.pathSeparator = options.pathSeparator;

	//Populate string table.
	Util::C_StringTable strTable;
	Util::C_StringId metaStrId;
	{
		Util::T_StringTableBuilder<Btl::String> stringTableBuilder;
		metaStrId = stringTableBuilder.AddString(options.metadata);

		for (Btl::Vector<C_FileToPak>::ConstIterator it = toProcess.Begin(); 
			it != toProcess.End(); ++it)
		{
			const C_FileToPak& fileToPak = *it;
			Btl::String260 localisedRelativePath;
			ConvertToPakDirToken(localisedRelativePath, pak.header.pathSeparator, fileToPak.relativeFilePath);
			stringTableBuilder.AddString(localisedRelativePath.Chars());
		}
		strTable.Build(stringTableBuilder);
	}
	{
		//Build header remainder.
		{
			if (pSecurityHash)
				pak.header.fatEntryType = E_FatEntryType_Secure;
			pak.header.codec = options.codec;
			pak.header.codecFlags = options.codecFlags;
			pak.header.metaStrOffset = strTable.GetOffsetByIndex(metaStrId);
			pak.header.fileTime = pakInfo.m_FileTime;
		}
		//Copy string table.
		{
			//Copy str data.
			pak.stringTable.dataSize = strTable.GetSizeBytes();
			pak.stringTable.pData = new U8[strTable.GetSizeBytes()];
			memcpy(pak.stringTable.pData, strTable.GetData(), strTable.GetSizeBytes());
		}

		Sys::C_StreamPos versionPosition;
		if (!WritePreHeader(pak, versionPosition))	return false;

		//Write space for header.
		Sys::C_StreamPos headerStartPos;
		{
			U32 headerSize(sizeof(C_PakHeader));
			if (!pak.pMediator->Write(headerSize)) return false;
			headerStartPos = pak.pMediator->GetWritePosition();
			if (!WritePadding(*pak.pMediator, headerSize)) return false;
		}

		//Write string table.
		if (!WriteStringTable(pak)) return false;

		//Write fat heading and reserve space...
		U32 fatEntrySize = GetFatEntrySize((E_FatEntryType)pak.header.fatEntryType);
		pak.nFatEntries = (U32)toProcess.Size();
		U32 fatSize = fatEntrySize * pak.nFatEntries;
		//Store size information for FAT
		if (!pak.pMediator->Write(pak.nFatEntries)) return false;
		if (!pak.pMediator->Write(fatSize)) return false;
		//Write placeholder space for fat entries and pad to next sector.
		Sys::C_StreamPos fatStartPos = pak.pMediator->GetWritePosition();
		{
			if (!WritePadding(*pak.pMediator, fatSize)) return false;
			U32 pos = (U32)pak.pMediator->GetWritePosition();
			U32 padding = pak.pMediator->GetSectorSize() - (pos % pak.pMediator->GetSectorSize());
			if (!WritePadding(*pak.pMediator, padding)) return false;
		}
		Sys::C_StreamPos dataStartPos = pak.pMediator->GetWritePosition();
		//Build fat entries
		{
			C_PakCrc32 crc32;
			//Allocate fat storage.
			pak.pFatEntries = new U8[fatSize];

			//Iterate files in alphabetical order.
			{
				U32 fatIdx = 0;
				for (Btl::Vector<C_FileToPak>::ConstIterator it = toProcess.Begin(); 
					it != toProcess.End(); ++it, ++fatIdx)
				{
					const C_FileToPak& fileToPak = *it;

					//Begin initialisation of fat entry.
					C_PakFatEntry& fatEntry = 
						*(C_PakFatEntry*)(((U8*)pak.pFatEntries)+fatEntrySize*fatIdx);

					if (pak.header.fatEntryType == E_FatEntryType_Secure)
					{
						Pak::C_PakFatEntrySecure& fatEntrySecure = (Pak::C_PakFatEntrySecure&)fatEntry;
						fatEntrySecure.padding = 0;
						memset(fatEntrySecure.fileHash, 0, sizeof(fatEntrySecure.fileHash));
					}

					if (fileToPak.fileType == Sys::E_FileType_File)
					{
						//Copy file in-memory from another pak.
						if (fileToPak.pBasePak)
						{
							BIZ_ASSERT(fileToPak.pBasePakFatEntry);
							if (!ProcessMemoryFile(
								pak,
								&fatEntry,
								fileToPak.pBasePak,
								fileToPak.pBasePakFatEntry))
								return false;
						}
						else 
						{
							Sys::C_FileInfo fileInfo;
							if (!srcDataFs.FileInfo(fileToPak.originalFilePath, &fileInfo)) return false;

							Btl::String260 localisedRelativePath;
							ConvertToPakDirToken(localisedRelativePath, pak.header.pathSeparator, fileToPak.relativeFilePath);
							LOGINFO(g_log, "Creating %u/%u : %s (%.2f Mb)\n", 
								fatIdx+1, toProcess.Size(),
								localisedRelativePath.Chars(), 
								(F32)fileInfo.m_FileSize / (F32)(1024*1024));
							if (!ProcessPhysicalFile(
								pak,
								srcDataFs,
								&fatEntry,
								fileToPak.originalFilePath))
								return false;
						}
					}
					else if (fileToPak.fileType == Sys::E_FileType_Directory)
					{
						fatEntry.dataStartSector = PAK_DATA_ISDIRECTORY;
						fatEntry.dataLogicalLengthBytes = 0;
						fatEntry.fileTime = 0;

						Btl::String260 localisedRelativePath;
						ConvertToPakDirToken(localisedRelativePath, pak.header.pathSeparator, fileToPak.relativeFilePath);
						LOGINFO(g_log, "Creating %u/%u : %s\n", 
							fatIdx+1, pak.nFatEntries, localisedRelativePath.Chars());
					}
					else
						return false;

					Btl::String260 localisedRelativePath;
					ConvertToPakDirToken(localisedRelativePath, pak.header.pathSeparator, fileToPak.relativeFilePath);
					fatEntry.filePathCrc			= crc32.CRCFromMemory(	
						(const U8*)localisedRelativePath.Chars(), localisedRelativePath.Length());
					if (uniqueCrcSet.Find(fatEntry.filePathCrc) != uniqueCrcSet.End())
					{
						LOGERROR(g_log, "Found hash collision for path %s! Please rename this file.", localisedRelativePath.Chars());
						return false;
					}
					uniqueCrcSet.Insert(fatEntry.filePathCrc);

					fatEntry.filePathStrTableOffset = strTable.GetOffset(localisedRelativePath);
					if (fatEntry.filePathStrTableOffset == -1)
					{
						LOGERROR(g_log, "Failed to find string table offset for %s", localisedRelativePath.Chars());
						return false;
					}

					//Store path string id in hash table
					{
						CrcFatInfoMap::InsertionReturnType result = 
							fatInfoByCrc.Insert(fatEntry.filePathCrc);
						if (!result.inserted)
						{
							LOGERROR(g_log, "Collision in file info by crc!");
							return false;
						}

						C_FatInfo& fatInfo = result.iterator->GetValue();
						fatInfo.sortedFatIdx = -1;
						fatInfo.stringOffset = fatEntry.filePathStrTableOffset;
					}
				}
			}
			//Sort FAT
			qsort(pak.pFatEntries, pak.nFatEntries, fatEntrySize, &FatEntryCompare);
			//Store fat indices against crc mapping table.
			{
				U8* pData = (U8*)pak.pFatEntries;
				for (U32 i = 0; i < pak.nFatEntries; ++i, pData+=fatEntrySize)
				{
					//Cast to base FAT type.
					C_PakFatEntry& fatEntry = *(C_PakFatEntry*)pData;
					//Store dir crc -> fat id mapping.
					CrcFatInfoMap::Iterator it = fatInfoByCrc.Find(fatEntry.filePathCrc);
					if (it == fatInfoByCrc.End())
					{
						LOGERROR(g_log, "Failed to find file info by crc!");
						return false;
					}

					C_FatInfo& fatInfo = it->GetValue();
					fatInfo.sortedFatIdx = i;
				}
			}
			//Resolve parent folders by traversing our now sorted FAT.
			for (U32 i = 0; i < pak.nFatEntries; ++i)
			{
				C_PakFatEntry& fatEntry = *(C_PakFatEntry*)(((U8*)pak.pFatEntries)+fatEntrySize*i);
				const char* pFilePath = (const char*)(pak.stringTable.pData + fatEntry.filePathStrTableOffset);

				Btl::String filePathStr = pFilePath;//strTable(i);
				S32 len = (S32)filePathStr.Length();
				S32 parentDirEnd = len-2;
				for (; parentDirEnd >= 0; --parentDirEnd)
					if (filePathStr[parentDirEnd] == pak.header.pathSeparator)
						break;
				if (parentDirEnd >= 0)
				{
					Btl::String parentDir;
					if (parentDirEnd > 0)
						parentDir = filePathStr(0, parentDirEnd);
					else
						parentDir = pak.header.pathSeparator;

					U32 parentDirCrc = crc32.CRCFromMemory(
						(const U8*)parentDir.Chars(), parentDir.Length());

					//Find parent fat index.
					CrcFatInfoMap::Iterator it = fatInfoByCrc.Find(parentDirCrc);
					if (it == fatInfoByCrc.End())
					{
						LOGERROR(g_log, "Failed to find file info by crc!");
						return false;
					}

					const C_FatInfo& fatInfo = it->GetValue();
					//Store parent fat offset.
					fatEntry.parentDirectoryOffsetBytes = fatEntrySize*fatInfo.sortedFatIdx;
				}
				else
				{
					//Only the root should drop in here.
					fatEntry.parentDirectoryOffsetBytes = PAK_FAT_ENTRY_NO_PARENT;
				}
			}
		}
		Sys::C_StreamPos highestPos = pak.pMediator->GetWritePosition();
		//Write FAT
		if (!WriteFat(pak, fatStartPos, pSecurityHash)) return false;

		if (!WriteHeader(pak, headerStartPos)) return false;
		pak.pMediator->SetWritePosition(versionPosition);
		U32 version = PAK_VERSION;
		if (!pak.pMediator->Write(version)) return false;
		//Set position to top of file to ensure we terminate correctly.
		pak.pMediator->SetWritePosition(highestPos);
	}
	return true;
}

bool CreatePak(
	U8* pSecurityHash,
	Sys::I_FileSystem& pakFs,
	Sys::I_FileSystem& srcDataFs,
	const C_PakOptions& options,
	const C_FilePathsToProcess& filePaths)
{
	Btl::Vector<Pak::C_FileToPak> toProcess;
	if (!Pak::BuildFileListToPak(
		toProcess,
		srcDataFs,
		options.pakRootPath, 
		options.pakRemapPath, 
		options.generateRootPath,
		options.recurse, 
		filePaths))
		return false;

	return CreatePak(
		pSecurityHash,
		pakFs,
		srcDataFs,
		options,
		toProcess);
}

bool UpdatePak(
	Sys::I_FileSystem& pakFs,
	Sys::I_FileSystem& srcDataFs,
	const C_PakOptions& options_,
	const C_FilePathsToProcess& filePaths)
{
	C_PakOptions options(options_);

	Sys::C_FilePath tmpFilePath = options.pakFilePath + ".$bizpak$";

	Pak::C_PakFile pak;
	if (!pak.pMediator->Open(&pakFs, options.pakFilePath, Sys::E_FileAccess_Read))
	{
		if (options.updateType == E_UpdateType_CreateIncremental)
		{
			LOGINFO(g_log, "Creating missing pak file...");
			if (!CreatePak(
				0,
				pakFs,											//File system containing output pak
				srcDataFs,										//Filesystem containing src data
				options,										//Creation params
				filePaths))										//List of files and folders to pak
				return false;
			return true;
		}
		else
		{
			LOGERROR(g_log, "Failed to open pak %s!\n", options.pakFilePath.Chars());
			return false;
		}
	}

	Sys::C_StreamPos fileLength = pak.pMediator->Length();
	Sys::C_StreamPos physicalPos(0);
	Sys::C_StreamPos versionPos(0);
	Sys::C_StreamPos headerStartPos(0);
	Sys::C_StreamPos fatStartPos(0);
	
	C_PreHeader preHeader;
	if (!ReadPreHeader(pak, preHeader, versionPos, &physicalPos))
		return false;

	if (preHeader.version == PAK_VERSION_CORRUPT || preHeader.version > PAK_VERSION)
	{
		if (options.updateType == E_UpdateType_CreateIncremental)
		{
			LOGINFO(g_log, "Recreating pak...");
			pak.pMediator->Close();
			if (!CreatePak(
				0,
				pakFs,											//File system containing output pak
				srcDataFs,										//Filesystem containing src data
				options,										//Creation params
				filePaths))										//List of files and folders to pak
				return false;
			return true;
		}
		if (preHeader.version == PAK_VERSION_CORRUPT)
		{
			LOGERROR(g_log, "Pak file is only partially complete! Please recreate it.");
		}
		else
		{
			LOGERROR(g_log, "Failed to read pak version %u as highest version we can handle is %u.", 
				preHeader.version, PAK_VERSION);
		}
		return false;
	}

	if (!ReadHeader(pak, headerStartPos, &physicalPos))
		return false;

	if (pak.header.fatEntryType == E_FatEntryType_Secure)
	{
		LOGERROR(g_log, "Cannot update a secure pak file.");
		return false;
	}

	//Validate header.
	if (options.updateType == E_UpdateType_CreateIncremental)
	{
		if (preHeader.sectorSize != options.sectorSize ||
			(options.bigEndian && !pak.pMediator->GetEndianSwapFlag()) ||
			((E_FatEntryType)pak.header.fatEntryType) != E_FatEntryType_Normal ||
			pak.header.codec != options.codec ||
			pak.header.codecFlags != options.codecFlags ||
			pak.header.pathSeparator != options.pathSeparator)
		{
			LOGINFO(g_log, "Attempting to update pak using different creation parameters. Pak must be recreated.");
			pak.pMediator->Close();
			if (!CreatePak(
				0,
				pakFs,											//File system containing output pak
				srcDataFs,										//Filesystem containing src data
				options,										//Creation params
				filePaths))										//List of files and folders to pak
				return false;
			return true;
		}
	}
	else if (options.updateType == E_UpdateType_AddUpdate)
	{
		//Assume properties of existing pak.
		options.sectorSize = preHeader.sectorSize;
		options.bigEndian = pak.pMediator->GetEndianSwapFlag();
		options.codec = (Pak::E_Codec)pak.header.codec;
		options.codecFlags = pak.header.codecFlags;
		options.pathSeparator = pak.header.pathSeparator;
	}

	if (!ReadStringTable(pak, &physicalPos)) return false;
	if (!ReadFat(pak, fatStartPos, &physicalPos)) return false;

	Btl::Vector<C_FileToPak> filesNotInPak, updatedFiles;
	Btl::Set<const C_PakFatEntry*> fatEntriesToErase;
	{
		Btl::Vector<C_FileToPak> fileList;
		if (options.updateType == E_UpdateType_Delete)
		{
			if (!BuildFileListToDelete(
				fileList,
				options.pakRootPath,
				options.pakRemapPath,
				options.convertFilePathsForDelete,
				filePaths))
				return false;
		}
		else if (!BuildFileListToPak(
			fileList, 
			srcDataFs,
			options.pakRootPath,
			options.pakRemapPath,
			options.generateRootPath,
			options.recurse, 
			filePaths))
			return false;

		for (Btl::Vector<C_FileToPak>::Iterator it = fileList.Begin(); it != fileList.End(); ++it)
		{
			C_FileToPak& fileToPak = *it;
			C_PakFatEntry* pFatEntry = pak.GetFatEntry(fileToPak.relativeFilePath);
			if (pFatEntry)
			{
				//Flagged for deletion?
				if (fileToPak.fileType == Sys::E_FileType_NotPresent)
				{
					fatEntriesToErase.Insert(pFatEntry);
					continue;
				}
				if (fileToPak.fileType == Sys::E_FileType_Directory)
				{
					if (!pFatEntry->IsDirectory())
					{
						//FAT entry marked as file clashes with a dir to update.
						fatEntriesToErase.Insert(pFatEntry);
						filesNotInPak.PushBack(fileToPak);
						continue;
					}
					else
						continue; //Nothing to update for folders.
				}
				else if (fileToPak.fileType == Sys::E_FileType_File)
				{
					if (pFatEntry->IsDirectory())
					{
						//FAT entry marked as dir clashes with a file to update.
						fatEntriesToErase.Insert(pFatEntry);
						filesNotInPak.PushBack(fileToPak);
						continue;
					}

					//If files same, ignore.
					if (pFatEntry->fileTime == fileToPak.fileTime)
						continue;

					//Attempt to fit into space.
					if (pak.header.codec == E_Codec_Raw)
					{
						U32 oldPhysicalSize = pFatEntry->dataLogicalLengthBytes;
						//Can we fit this file within a sector gap?
						U64 oldSectorCount = oldPhysicalSize / pak.pMediator->GetSectorSize() + (oldPhysicalSize % pak.pMediator->GetSectorSize() > 0 ? 1 : 0);
						U64 newSectorCount = fileToPak.fileSize / pak.pMediator->GetSectorSize() + (fileToPak.fileSize % pak.pMediator->GetSectorSize() > 0 ? 1 : 0);
						if (oldSectorCount == newSectorCount)
						{
							const char* pFilePath = 
								(const char*)(pak.stringTable.pData + pFatEntry->filePathStrTableOffset);
							LOGINFO(g_log, "Updating in place for : %s (old size %u bytes, new size %u bytes)\n", 
								pFilePath, oldPhysicalSize, fileToPak.fileSize);
							//Update in place.
							updatedFiles.PushBack(fileToPak);
							continue;
						}
					}

					//Cannot update, must recreate.
					fatEntriesToErase.Insert(pFatEntry);
					filesNotInPak.PushBack(fileToPak);
				}
				else
					return false;
			}
			else
				filesNotInPak.PushBack(fileToPak);
		}
	}
	if (filesNotInPak.Size() == 0 && updatedFiles.Size() == 0 && fatEntriesToErase.Size() == 0)
		return true;

	Btl::Vector<C_FileToPak> filesForNewPak;
	if (options.updateType != E_UpdateType_Delete)
	{
		//Populate with data from existing pak + new files.
		filesForNewPak = filesNotInPak;
	}
	if (filesForNewPak.Size() > 0 || fatEntriesToErase.Size() > 0)
	{
		LOGINFO(g_log, "Creating new pak...\n");

		U32 fatEntrySize = Pak::GetFatEntrySize(
			(Pak::E_FatEntryType)pak.header.fatEntryType);
		U32 fatSize = fatEntrySize * pak.nFatEntries;

		for (U32 i = 0; i < pak.nFatEntries; i++)
		{
			const Pak::C_PakFatEntry* pFatEntry = 
				(const Pak::C_PakFatEntry*)(pak.pFatEntries + fatEntrySize*i);

			//If we've been flagged for erasure, or one of our parents has...
			bool deleteMe = false;
			{
				Btl::Set<const C_PakFatEntry*>::Iterator it = fatEntriesToErase.Find(pFatEntry);
				if (it != fatEntriesToErase.End())
				{
					if (pFatEntry == it->GetKey())
						deleteMe = true;
				}
				else
					for (it = fatEntriesToErase.Begin(); it != fatEntriesToErase.End(); ++it)
						if (IsChildOf(pak, it->GetKey(), pFatEntry))
						{
							deleteMe = true;
							break;
						}
			}
			if (deleteMe)
			{
				const char* pFilePath = 
					(const char*)(pak.stringTable.pData + pFatEntry->filePathStrTableOffset);
				if (pFatEntry->IsDirectory())
				{
					LOGINFO(g_log, "Flagged for delete: %u/%u : %s\n", 
						i+1, pak.nFatEntries, pFilePath);
				}
				else
					LOGINFO(g_log, "Flagged for delete: %u/%u : %s (%.2f Mb)\n", 
						i+1, pak.nFatEntries, pFilePath, 
						(F32)pFatEntry->dataLogicalLengthBytes / (F32)(1024*1024));
			}
			else
			{
				//(phew!) we've not been flagged for deletion.
				filesForNewPak.PushBack();
				const char* pFilePath = 
					(const char*)(pak.stringTable.pData + pFatEntry->filePathStrTableOffset);

				C_FileToPak& memFile = filesForNewPak.Back();
				memFile.relativeFilePath = pFilePath;
				memFile.originalFilePath = pFilePath;
				memFile.fileType = pFatEntry->IsDirectory() ? Sys::E_FileType_Directory : Sys::E_FileType_File;
				memFile.fileTime = 0;
				memFile.fileSize = 0;
				memFile.pBasePak = &pak;
				memFile.pBasePakFatEntry = pFatEntry;
			}
		}
		
		Sys::C_FilePath originalPakFile = options.pakFilePath;
		options.pakFilePath = tmpFilePath;
		if (!CreatePak(0, pakFs, srcDataFs, options, filesForNewPak))
			return false;

		pak.pMediator->Close();
		if (!pakFs.DeleteFile(originalPakFile))
		{
			LOGERROR(g_log, "Failed to delete %s", originalPakFile.Chars());
			return false;
		}

		if (!pakFs.MoveFile(tmpFilePath, originalPakFile))
		{
			LOGERROR(g_log, "Failed to move %s to %s", tmpFilePath.Chars(), originalPakFile.Chars());
			return false;
		}
	}
	else
	{
		LOGINFO(g_log, "Updating content within existing pak!\n");

		pak.pMediator->Close();

		if (!pak.pMediator->SwitchFile(&pakFs, options.pakFilePath, Sys::E_FileAccess_Append))
		{
			LOGERROR(g_log, "Failed to open pak %s!\n", options.pakFilePath.Chars());
			return false;
		}
		pak.pMediator->SetEndianSwapFlag(options.bigEndian);

		//Update time info
		{
			Sys::C_FileInfo fileInfo;
			if (!pakFs.FileInfo(options.pakFilePath, &fileInfo))
			{
				LOGERROR(g_log, "Failed to quert file info for %s!\n", options.pakFilePath.Chars());
				return false;
			}
			pak.header.fileTime = fileInfo.m_FileTime;
		}

		pak.pMediator->SetWritePosition(versionPos);
		if (!pak.pMediator->Write((U32)PAK_VERSION_CORRUPT)) return false;

		//Loop thru updated FAT entries and serialise.
		for (S32 i = 0; i < updatedFiles.Size(); ++i)
		{	
			C_FileToPak& fileToUpdate = updatedFiles[i];
			C_PakFatEntry* pFatEntry = pak.GetFatEntry(fileToUpdate.relativeFilePath);
			if (!pFatEntry)
				//Shouldn't be possible.
				return false;

			if (fileToUpdate.fileType == Sys::E_FileType_File)
			{
				Sys::C_FileInfo fileInfo;
				if (!srcDataFs.FileInfo(fileToUpdate.originalFilePath, &fileInfo)) return false;

				LOGINFO(g_log, "%u/%u : %s (%.2f Mb)\n", 
					i+1,
					updatedFiles.Size(),
					fileToUpdate.relativeFilePath.Chars(), 
					(F32)fileInfo.m_FileSize / (F32)(1024*1024));

				U64 dataStartSector = pFatEntry->dataStartSector;
				pak.pMediator->SetWriteSector(dataStartSector);
				if (!ProcessPhysicalFile(
					pak,
					srcDataFs,
					pFatEntry,
					fileToUpdate.originalFilePath))
					return false;
				if (pFatEntry->dataStartSector != dataStartSector)
					return false;
			}
			else if (fileToUpdate.fileType == Sys::E_FileType_Directory)
			{
				continue;
			}
			else
			{
				return false;
			}
		}
		//Write out updated FAT
		if (!WriteFat(pak, fatStartPos, 0)) return false;

		//For speed, remove ability to verify updated file integrity
		memset(pak.header.wholeFileIntegrityHash, 0, sizeof(pak.header.wholeFileIntegrityHash));
		//Write header
		if (!WriteHeader(pak, headerStartPos)) return false;
	
		pak.pMediator->SetWritePosition(versionPos);
		U32 version = PAK_VERSION;
		if (!pak.pMediator->Write(version)) return false;
		//Set position to top of file to ensure we terminate correctly.
		pak.pMediator->SetWritePosition(fileLength);
		pak.pMediator->Close();
	}

	return true;
}

#endif

Pak::C_PakFile* ReadPak(
	Sys::I_FileSystem& pakFs,
	const Sys::C_FilePath& filePath,
	Sys::E_FileAccess access,
	const Security::I_Security* pSecurity,
	const Sys::C_OpenParams& params)
{
	Pak::C_PakFile* pPak = new Pak::C_PakFile();
	if (!pPak) return NULL;

	if (!pPak->pMediator->Open(&pakFs, filePath, access, params))
	{
		SAFE_DELETE_PTR(pPak);
		return NULL;
	}

	if (ReadPak(*pPak, pSecurity))
		return pPak;

	SAFE_DELETE_PTR(pPak);
	return NULL;
}

bool ReadPak(
	Pak::C_PakFile& pak,
	const Security::I_Security* pSecurity)
{
	Sys::C_StreamPos physicalPos(0);
	Sys::C_StreamPos versionPos(0);
	Sys::C_StreamPos headerStartPos(0);
	Sys::C_StreamPos fatStartPos(0);
	C_PreHeader preHeader;
	if (!ReadPreHeader(pak, preHeader, versionPos, &physicalPos))
		return false;
	if (preHeader.version == PAK_VERSION_CORRUPT)
	{
		FSERROR(g_log, "ReadPak - Pak file is only partially complete! Please recreate it. %s", pak.pMediator->GetFilePath().Chars());
		Sys::FileRwError(pak.pMediator->GetFilePath().Chars(), false, true);
		return false;
	}
	else if (preHeader.version > PAK_VERSION_READ_MAX)	// [Blur2PC] was PAK_VERSION
	{
		FSERROR(g_log, "ReadPak - Failed to read pak version %u as highest version we can handle is %u %s", preHeader.version, PAK_VERSION_READ_MAX, pak.pMediator->GetFilePath().Chars());
		Sys::FileRwError(pak.pMediator->GetFilePath().Chars(), false, true);
		return false;
	}
	//Header.
	if (!ReadHeader(pak, headerStartPos, &physicalPos))	return false;
	if (!ReadStringTable(pak, &physicalPos)) return false;
	if (!ReadFat(pak, fatStartPos, &physicalPos)) return false;

	//Verify FAT hash.
	if (pSecurity)
	{
		U32 fatEntrySize = Pak::GetFatEntrySize((Pak::E_FatEntryType)pak.header.fatEntryType);
		U32 fatSize = fatEntrySize * pak.nFatEntries;
		U8 actualFileTableHash[20];
		memset(actualFileTableHash, 0, sizeof(actualFileTableHash));
		Sha1::C_HashContext hashContext;
		if (!hashContext.Valid()) return false;
		// [Blur2PC] a version 2 signature covers the FAT as stored on disk
		if (pak.pRawFatV2)
		{
			if (!hashContext.UpdateHash(pak.pRawFatV2, pak.rawFatV2Size)) return false;
		}
		else
		if (!hashContext.UpdateHash(pak.pFatEntries, fatSize)) return false;
		if (!hashContext.GetHash(actualFileTableHash)) return false;

		if (!pSecurity->Verify(actualFileTableHash, sizeof(actualFileTableHash), pak.pMediator->GetFilePath().Chars()))
		{
			FSERROR(g_log, "ReadPak - Pak FAT is corrupt! %s", pak.pMediator->GetFilePath().Chars());
			Sys::FileRwError(pak.pMediator->GetFilePath().Chars(), false, true);
			return false;
		}
	}
	return true;
}

bool Verify(
	Sys::I_FileSystem& pakFs,
	const Sys::C_FilePath& pakFilePath)
{
	Pak::C_PakFile pak;
	if (!pak.pMediator->Open(&pakFs, pakFilePath, Sys::E_FileAccess_Read, Sys::C_OpenParams()))
		return false;
	if (!Verify(pak))
		return false;
	return true;
}

bool Verify(Pak::C_PakFile& pak)
{
	U32 fatEntrySize = Pak::GetFatEntrySize((Pak::E_FatEntryType)pak.header.fatEntryType);
	U32 fatSize = fatEntrySize * pak.nFatEntries;
	U8 sha1WholeFileResult[20];
	memset(sha1WholeFileResult, 0, sizeof(sha1WholeFileResult));
	//If we're dealing with a pak that's been updated, we lose verification capability in the interest of performance.
	if (memcmp(sha1WholeFileResult, 
		pak.header.wholeFileIntegrityHash, 
		sizeof(sha1WholeFileResult)) == 0)
		return true;

	for (U32 i = 0; i < pak.nFatEntries; i++)
	{
		const Pak::C_PakFatEntry* pFatEntry = 
			(const Pak::C_PakFatEntry*)(pak.pFatEntries + fatEntrySize*i);
		if (pFatEntry->dataStartSector != PAK_DATA_ISDIRECTORY)
		{
			C_PakEntryFile file;
			if (!file.Open(
				&pak,
				pFatEntry,
				(char*)(pak.stringTable.pData + pFatEntry->filePathStrTableOffset),
				Sys::E_FileAccess_Read)) return false;

			U8 sha1FileResult[20];
			if (!GenerateSha1HashForFile(
				sha1FileResult, file)) return false;
			for (U32 j = 0; j < sizeof(sha1FileResult); ++j)
				sha1WholeFileResult[j] += sha1FileResult[j];
		}
	}
	if (memcmp(sha1WholeFileResult, pak.header.wholeFileIntegrityHash, sizeof(sha1WholeFileResult)) == 0)
		return true;
	return false;
}

bool Verify2(Pak::C_PakFile& pak)
{
	U32 fatEntrySize = Pak::GetFatEntrySize((Pak::E_FatEntryType)pak.header.fatEntryType);
	U32 fatSize = fatEntrySize * pak.nFatEntries;

	for (U32 i = 0; i < pak.nFatEntries; i++)
	{
		const Pak::C_PakFatEntry* pFatEntry = 
			(const Pak::C_PakFatEntry*)(pak.pFatEntries + fatEntrySize*i);
		if (pFatEntry->dataStartSector != PAK_DATA_ISDIRECTORY)
		{
			C_PakEntryFile file;
			if (!file.Open(
				&pak,
				pFatEntry,
				(char*)(pak.stringTable.pData + pFatEntry->filePathStrTableOffset),
				Sys::E_FileAccess_Read)) return false;

			U8 sha1FileResult[20];
			if (!GenerateSha1HashForFile(
				sha1FileResult, file)) return false;
		}
	}

	return true;
}


bool ExtractFile(Sys::I_File& dstFile, Sys::I_File& srcFile)
{
	U32 bytesRemaining = (U32)srcFile.Length();
	U8 buffer[PAK_IFILE_BUFFER_SIZE];
	U32 bytesTotal = 0;
	while(bytesRemaining)
	{
		U32 bytesToRead = bytesRemaining > sizeof(buffer) ? sizeof(buffer) : (U32)bytesRemaining;
		if (!srcFile.Read(buffer, bytesToRead)) return false;
		if (!dstFile.Write(buffer, bytesToRead)) return false;
		bytesRemaining -= bytesToRead;
		bytesTotal += bytesToRead;
	}
	return true;
}

bool ExtractAll(Sys::I_FileSystem& dstDataFs, const Sys::C_FilePath& outputPath, C_PakFile& pak)
{
	U32 fatEntrySize = Pak::GetFatEntrySize(
		(Pak::E_FatEntryType)pak.header.fatEntryType);
	U32 fatSize = fatEntrySize * pak.nFatEntries;

	for (U32 i = 0; i < pak.nFatEntries; i++)
	{
		const Pak::C_PakFatEntry* pFatEntry = (const Pak::C_PakFatEntry*)(pak.pFatEntries + fatEntrySize*i);
		const char* pSrcFilePath = (char*)(pak.stringTable.pData + pFatEntry->filePathStrTableOffset);

		Sys::C_FilePath dstFilePath = outputPath + pSrcFilePath;
		if (pFatEntry->dataStartSector == PAK_DATA_ISDIRECTORY)
		{
			if (!dstDataFs.FileExists(dstFilePath))
				if (!dstDataFs.CreateDirectory(dstFilePath))
					return false;
		}
		else
		{
			Sys::C_FilePath dstDir = Sys::ExtractDirectory(dstFilePath);
			if (!dstDataFs.FileExists(dstDir) && (!dstDataFs.CreateDirectory(dstDir))) return false;

			Sys::I_File* pDstFile = dstDataFs.Open(
				dstFilePath, 
				Sys::E_FileAccess_Write);
			if (!pDstFile) return false;

			C_PakEntryFile srcFile;
			if (!srcFile.Open(
				&pak, 
				pFatEntry,
				(char*)(pak.stringTable.pData + pFatEntry->filePathStrTableOffset),
				Sys::E_FileAccess_Read)) return false;

			LOGINFO(g_log, "%u/%u : %s (%.2f Mb)\n", 
				i+1,
				pak.nFatEntries,
				(char*)(pak.stringTable.pData + pFatEntry->filePathStrTableOffset), 
				(F32)srcFile.Length() / (F32)(1024*1024));

			bool status = ExtractFile(*pDstFile, srcFile);
			SAFE_DELETE_PTR(pDstFile);
			if (!status) return status;

			if (!dstDataFs.FileTime(dstFilePath, pFatEntry->fileTime))
				return status;
		}
	}
	return true;
}

bool Extract(
	Sys::I_FileSystem& outputFs,
	const Sys::C_FilePath& outputPath,
	Sys::I_FileSystem& pakFs,
	const Sys::C_FilePath& pakFilePath,
	const Btl::Vector<Sys::C_FilePath>* pFilePaths)
{
	Pak::C_PakFile pak;
	if (!pak.pMediator->Open(&pakFs, pakFilePath, Sys::E_FileAccess_Read, Sys::C_OpenParams()))
		return false;
	return Extract(outputFs, outputPath, pak, pFilePaths);
}

bool Extract(
	Sys::I_FileSystem& outputFs,
	const Sys::C_FilePath& outputPath,
	C_PakFile& pak,
	const Btl::Vector<Sys::C_FilePath>* pFilePaths)
{
	if (!pFilePaths)
		return ExtractAll(outputFs, outputPath, pak);
	LOGERROR(g_log, "Extraction of listed files not yet implemented.\n");
	return false;
}

}
