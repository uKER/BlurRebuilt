#include "LogicalFsPch.h"
#include <Core/Allocators/CoreMemoryFunctions.h>
#include <System/Debug/Debug.h>
#include <System/Serialise/FilePathHashOps.h>

#include <LogFs/LogFs.h>
#include <OverlayFs/OverlayFs.h>
#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include <PakFs/PakFsManager.h>
#include <MemoryFs/MemoryFs.h>

#include <GameFs/GameHelpers.h>

#if defined(PLATFORM_PS3)
#include <sys/paths.h>
#endif

namespace LogicalFs
{

		//Don't need to worry about two layer DVDs.
		#define NUM_MEDIA_LAYERS 1


namespace GameHelpers
{
void MasterDiscEmulationCb(Debug::VarBase* pVar, void* pContext);
}

Debug::Bool g_masterDiscEmulation("Util.LogicalFs.OverlayFs.MasterDiscEmulation", false, Debug::Notify(&LogicalFs::GameHelpers::MasterDiscEmulationCb), Debug::Persist());
Debug::Bool g_enableRamDisk("Util.LogicalFs.OverlayFs.EnableRamDisk", true, Debug::Persist());

namespace GameHelpers
{

C_OverlayFs* g_pGameFs = NULL;

C_OverlayFs* CreateGameFs()
{
	if (g_pGameFs)
	{
		BIZ_ASSERT_FAIL("Game overlay file system already created!");
		return 0;
	}

	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();

	if (!C_PakFsManager::Instance().Create())
		return 0;

	g_pGameFs = new LOGFS_WRAPPER(LogicalFs::C_OverlayFs, "-OverlayFs-");
	g_pGameFs->SetWorkingDirectory("/");

	{
		const Sys::C_FilePath kOverlayDstPath = "/";
		const Sys::C_FilePath kOverlaySrcPath = nativeFs.GetWorkingDirectory();
		LogicalFs::C_OverlayLayer::C_Params params;
		params.flags.layerType = LogicalFs::GameHelpers::GetDefaultOverlayType();
		params.flags.writable = true;
		g_pGameFs->AddLayer(
			kOverlayDstPath,
			&nativeFs,
			kOverlaySrcPath,
			params,
			E_MediaLayer_0,
			"CFileSystem");
	}


	return g_pGameFs;
}

C_PakFs* MountPakLayer(
	 Sys::I_FileSystem& nativeFs, 
	 C_OverlayFs& overlayFs,
	 const Sys::C_FilePath& pakFilePath, 
	 const Sys::C_FilePath& dstPath, 
	 const Sys::C_FilePath& srcPath,
	 const Security::I_Security* pSecurity,
	 const Sys::C_OpenParams& params,
	 E_MediaLayer mediaLayer)
{
	if (nativeFs.FileExists(pakFilePath))
	{
		C_PakFs* pPakFs = PakHelpers::OpenPak(nativeFs, pakFilePath, pSecurity, params);
		if (pPakFs)
		{
			C_OverlayLayer::C_Params params;
			params.flags.layerType =  GetDefaultOverlayType();
			params.flags.writable = false;
			overlayFs.AddLayer(dstPath, pPakFs, srcPath, params, mediaLayer, pakFilePath.Chars());
			LOGINFO(g_log, "Mounted %spak %s (src %s dst %s)", 
				pSecurity ? "SECURE " : "", pakFilePath.Chars(), srcPath.Chars(), dstPath.Chars());
			return pPakFs;
		}
		else
		{
			if (!pSecurity)
			{
				LOGWARN(g_log, "Failed to mount pak %s", pakFilePath.Chars());
			}
			else
			{
				LOGFATAL(g_log, "Failed to mount SECURE pak %s", pakFilePath.Chars());
			}
			return NULL;
		}
	}
	else
	{
		LOGWARN(g_log, "Not mounting missing pak %s", pakFilePath.Chars());
	}

	return NULL;
}

C_PakFs* InsertPakLayer(
	 Sys::I_FileSystem& nativeFs, 
	 C_OverlayFs& overlayFs,
	 U32 index,
	 const Sys::C_FilePath& pakFilePath, 
	 const Sys::C_FilePath& dstPath, 
	 const Sys::C_FilePath& srcPath,
	 const Security::I_Security* pSecurity,
	 E_MediaLayer mediaLayer)
{
	if (nativeFs.FileExists(pakFilePath))
	{
		C_PakFs* pPakFs = PakHelpers::OpenPak(nativeFs, pakFilePath, pSecurity);
		if (pPakFs)
		{
			C_OverlayLayer::C_Params params;
			params.flags.layerType =  GetDefaultOverlayType();
			params.flags.writable = false;
			overlayFs.InsertLayer(index, dstPath, pPakFs, srcPath, params, mediaLayer, pakFilePath.Chars());
			LOGINFO(g_log, "Mounted %spak %s (src %s dst %s)", 
				pSecurity ? "SECURE " : "", pakFilePath.Chars(), srcPath.Chars(), dstPath.Chars());
			return pPakFs;
		}
		else
		{
			if (!pSecurity)
			{
				LOGWARN(g_log, "Failed to mount pak %s", pakFilePath.Chars());
			}
			else
			{
				LOGFATAL(g_log, "Failed to mount SECURE pak %s", pakFilePath.Chars());
			}
			return NULL;
		}
	}
	else
	{
		LOGWARN(g_log, "Not mounting missing pak %s", pakFilePath.Chars());
	}

	return NULL;
}


bool RemountPakLayer(
	E_MediaLayer mediaLayer,
	U32 layerIdx,
	Sys::I_FileSystem& nativeFs,
	C_OverlayFs& overlayFs,
	const Sys::C_FilePath& srcPath)
{
	C_OverlayLayer layer;
	if (!overlayFs.GetLayerByIndex(layer, mediaLayer, layerIdx))
		return false;

	C_PakFs* pPakFs = (C_PakFs*)layer.GetFs();
	return pPakFs->SwitchPakFile(&nativeFs, srcPath);
}

bool ResumePakLayer(
	E_MediaLayer mediaLayer,
	U32 layerIdx,
	C_OverlayFs& overlayFs)
{
	C_OverlayLayer layer;
	if (!overlayFs.GetLayerByIndex(layer, mediaLayer, layerIdx))
		return false;

	C_PakFs* pPakFs = (C_PakFs*)layer.GetFs();
	return pPakFs->Resume();
}

bool SuspendPakLayer(
	E_MediaLayer mediaLayer,
	U32 layerIdx,
	C_OverlayFs& overlayFs)
{
	C_OverlayLayer layer;
	if (!overlayFs.GetLayerByIndex(layer, mediaLayer, layerIdx))
		return false;

	C_PakFs* pPakFs = (C_PakFs*)layer.GetFs();
	pPakFs->Suspend();

	return true;
}

bool StopReadsPakLayer(
	E_MediaLayer mediaLayer,
	U32 layerIdx,
	C_OverlayFs& overlayFs)
{
	C_OverlayLayer layer;
	if (!overlayFs.GetLayerByIndex(layer, mediaLayer, layerIdx))
		return false;

	C_PakFs* pPakFs = (C_PakFs*)layer.GetFs();
	pPakFs->StopReads();	
	return true;
}

bool UnmountPakLayer(
	E_MediaLayer mediaLayer,
	const Sys::C_FilePath& pakFilePath,
	C_OverlayFs& overlayFs)
{
	C_OverlayLayer layer;
	if (!overlayFs.GetLayerByName(layer, mediaLayer, pakFilePath.Chars()))
		return false;

	C_PakFs* pPakFs = (C_PakFs*)layer.GetFs();
	SAFE_DELETE_PTR(pPakFs);

	overlayFs.DeleteLayerByName(mediaLayer, pakFilePath.Chars());
	return true;
}

typedef Btl::OpenHashMap<Sys::C_FilePath, U8*, Sys::HashKeyOpFilePath> RamPakMap;
RamPakMap g_pRamPakDataByOriginalPath;

bool MountRamPakLayer(
	 Sys::I_FileSystem& nativeFs, 
	 C_MemoryFs& memoryFs,
	 C_OverlayFs& overlayFs,
	 const Sys::C_FilePath& pakFilePath, 
	 const Sys::C_FilePath& relativePakFilePath, 
	 const Sys::C_FilePath&  dstPath, 
	 const Sys::C_FilePath&  srcPath,
	 const Security::I_Security* pSecurity)
{
	if (!g_enableRamDisk)
		return false;
	if (!nativeFs.FileExists(pakFilePath))
	{
		LOGWARN(g_log, "Failed to load rampak %s, file does not exist.", pakFilePath.Chars());
		return false;
	}

	U8* pData = NULL;
	{
		Sys::C_OpenParams openParams;
		openParams.m_BufferSizeHint = 0;
		Sys::I_File* pFile = nativeFs.Open(pakFilePath, Sys::E_FileAccess_Read, openParams);
		if (!pFile) 
		{
			LOGWARN(g_log, "Failed to load rampak %s", pakFilePath.Chars());
			goto fail;
		}

		U32 sizeBytes = (U32)pFile->Length();
		//Paks ought to be 2k aligned, but just in case...
		const U32 kTwoKilobytes = 2*1024;
		U32 readSize2kAligned = (sizeBytes + (kTwoKilobytes - 1)) & ~(kTwoKilobytes - 1);
				
		pData = new U8[readSize2kAligned];
		if (!pData) 
		{
			BIZ_ASSERT_FAIL_SPEED("Out of memory allocating rampak %s", relativePakFilePath.Chars());
			goto fail;
		}

		Sys::I_BinaryStream::UnderflowContext ctx;
		ctx.readTarget = pData;
		ctx.readSize = readSize2kAligned;
		if (!pFile->Underflow(ctx)) 
		{
			BIZ_ASSERT_FAIL_SPEED("Underflow failed for rampak %s", relativePakFilePath.Chars());
			goto fail;
		}
		SAFE_DELETE_PTR(pFile);

		memoryFs.CreateDirectory(Sys::ExtractDirectory(Sys::C_FilePath("/") + relativePakFilePath));

		Sys::C_FileInfo fileInfo;
		fileInfo.Clear();
		fileInfo.m_FileName = relativePakFilePath;
		fileInfo.m_FileType = Sys::E_FileType_File;
		fileInfo.m_FileSize = sizeBytes;

	#ifdef CFG_GOLD
		bool isMaster = true;
	#else
		bool isMaster = g_masterDiscEmulation;
	#endif
		
		//Store timestamp in non master ver.
		if (!isMaster)
		{
			Sys::C_FileInfo tmp;
			if (nativeFs.FileInfo(relativePakFilePath, &tmp))
			{
				fileInfo.m_FileTime = tmp.m_FileTime;
			}
		}

		if (!memoryFs.InjectFile(fileInfo, pData)) 
		{
			BIZ_ASSERT_FAIL_SPEED("Inject file failed for rampak %s.", relativePakFilePath.Chars());
			goto fail;
		}

		//Duplicate across both media layers.
		for (U32 i = 0; i < NUM_MEDIA_LAYERS; ++i)
			if (!MountPakLayer(memoryFs, overlayFs, relativePakFilePath, dstPath, srcPath, pSecurity, Sys::C_OpenParams(), (E_MediaLayer)i))
			{
				BIZ_ASSERT_FAIL_SPEED("Failed to mount rampak layer %", relativePakFilePath.Chars());
				goto fail;
			}

		g_pRamPakDataByOriginalPath.Insert(relativePakFilePath, (U8*)pData);
		
	}
	LOGINFO(g_log, "Mounted rampak %s", relativePakFilePath.Chars());
	return true;

fail:
	SAFE_DELETE_ARY(pData);
	return false;
}

bool UnmountRamPakLayer(
	C_MemoryFs& memoryFs,
	C_OverlayFs& overlayFs,
	const Sys::C_FilePath& pakFilePath)
{
	RamPakMap::Iterator it = g_pRamPakDataByOriginalPath.Find(pakFilePath);
	if (it != g_pRamPakDataByOriginalPath.End())
	{
		bool result = true;
		//Unmount pak layers
		for (U32 i = 0; i < NUM_MEDIA_LAYERS; ++i)
			result &= UnmountPakLayer(E_MediaLayer(i), pakFilePath.Chars(), overlayFs);

		//Erase pak entry in memory fs.
		if (result)
		{
			memoryFs.DeleteFile(pakFilePath);
			U8* pData = it->GetValue();
			SAFE_DELETE_ARY(pData);
			g_pRamPakDataByOriginalPath.Erase(it);
			LOGINFO(g_log, "Unmounted rampak %s", pakFilePath.Chars());
			return true;
		}
		BIZ_ASSERT_FAIL_SPEED("Failed to unmount rampak layer. Memory will have leaked.");
	}

	return false;
}

C_OverlayLayer::E_Type GetDefaultOverlayType()
{
#ifdef CFG_GOLD
	//Avoid time comparisons in gold build, use paked data not loose files.
	return  C_OverlayLayer::E_Type_Overlay;
#else
	return g_masterDiscEmulation ? C_OverlayLayer::E_Type_Overlay : C_OverlayLayer::E_Type_Newer;
#endif
}

C_OverlayFs* GetGameFs()
{
	return g_pGameFs;
}

Sys::C_FilePath g_gameHelpersTitleId;

void SetTitleId(const char* titleId)
{
	g_gameHelpersTitleId = titleId;
}

Sys::C_FilePath GetPotentiallyInstalledFilePath(const Sys::C_FilePath& sourcePath)
{
#ifdef PLATFORM_PS3
	//We've found a cache file that's being loaded from the disc. Remount if present on HDD.
	Sys::C_FilePath hdd0CandidateFsPath = Sys::C_FilePath(SYS_DEV_HDD0) + "/game/" + g_gameHelpersTitleId + "/USRDIR/";
	hdd0CandidateFsPath += sourcePath;

	Sys::C_FileInfo sourceInfo;
	Sys::C_FileInfo destInfo;

	if (Sys::C_FileSystem::Instance().FileInfo(sourcePath, &sourceInfo ) &&
		Sys::C_FileSystem::Instance().FileInfo(hdd0CandidateFsPath, &destInfo) &&
		destInfo.m_FileSize == sourceInfo.m_FileSize)
	{
		//No timestamp check, assume this is dealt with by installer for safety.
		return hdd0CandidateFsPath;
	}
	else
	{
		LOGINFO(g_log, "Not switching to hdd file %s", hdd0CandidateFsPath.Chars());
	}
#endif
	//360 and PS3 in the case of no hdd install present.
	return sourcePath;
}

void MasterDiscEmulationCb(Debug::VarBase* pVar, void* pContext)
{
	LOGINFO(g_log, "Master disc emulation %s", g_masterDiscEmulation ? "enabled" : "disabled");
	if (!g_pGameFs)
		return;

	for (U32 mediaLayer = 0; mediaLayer < NUM_MEDIA_LAYERS; ++mediaLayer)
	{
		LogicalFs::E_MediaLayer eMediaLayer = (LogicalFs::E_MediaLayer)mediaLayer;
		for (U32 i = 0; i < g_pGameFs->GetNumLayers(eMediaLayer); ++i)
		{
			C_OverlayLayer layer;
			if (!g_pGameFs->GetLayerByIndex(layer, eMediaLayer, i))
				continue;

			layer.GetParams().flags.layerType = GetDefaultOverlayType();

			bool result = g_pGameFs->SetLayerByIndex(layer, eMediaLayer, i);
			BIZ_ASSERT(result && "Failed to set overlay layer!");
		}
	}
}

}
}
