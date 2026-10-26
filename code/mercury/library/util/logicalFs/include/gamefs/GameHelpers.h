#ifndef LOGICALFS_GAMEHELPERS_H
#define LOGICALFS_GAMEHELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Security
{
	class I_Security;
}

namespace LogicalFs
{

class C_PakFs;
class C_MemoryFs;

namespace GameHelpers
{
	C_PakFs* MountPakLayer(
		 Sys::I_FileSystem& nativeFs, 
		 C_OverlayFs& overlayFs,
		 const Sys::C_FilePath& pakFilePath, 
		 const Sys::C_FilePath& dstPath, 
		 const Sys::C_FilePath& srcPath,
		 const Security::I_Security* pSecurity = NULL,
		 const Sys::C_OpenParams& params = Sys::C_OpenParams(),
		 E_MediaLayer mediaLayer = E_MediaLayer_0);

	C_PakFs* InsertPakLayer(
		 Sys::I_FileSystem& nativeFs, 
		 C_OverlayFs& overlayFs,
		 U32 index,
		 const Sys::C_FilePath& pakFilePath, 
		 const Sys::C_FilePath& dstPath, 
		 const Sys::C_FilePath& srcPath,
		 const Security::I_Security* pSecurity = NULL,
		 const Sys::C_OpenParams& params = Sys::C_OpenParams(),
		 E_MediaLayer mediaLayer = E_MediaLayer_0);

	bool RemountPakLayer(
		E_MediaLayer mediaLayer,
		U32 layerIdx,
		Sys::I_FileSystem& nativeFs,
		C_OverlayFs& overlayFs,
		const Sys::C_FilePath& srcPath);

	bool ResumePakLayer(
		E_MediaLayer mediaLayer,
		U32 layerIdx,
		C_OverlayFs& overlayFs);

	bool SuspendPakLayer(
		E_MediaLayer mediaLayer,
		U32 layerIdx,
		C_OverlayFs& overlayFs);
		
	bool StopReadsPakLayer(
		E_MediaLayer mediaLayer,
		U32 layerIdx,
		C_OverlayFs& overlayFs);		

	bool UnmountPakLayer(
		E_MediaLayer mediaLayer,
		const Sys::C_FilePath& pakFilePath,
		C_OverlayFs& overlayFs);

	bool MountRamPakLayer(
		 Sys::I_FileSystem& nativeFs, 
		 C_MemoryFs& memoryFs,
		 C_OverlayFs& overlayFs,
		 const Sys::C_FilePath& pakFilePath, 
		 const Sys::C_FilePath& pakFileRelativePath, 
		 const Sys::C_FilePath& dstPath, 
		 const Sys::C_FilePath& srcPath,
		 const Security::I_Security* pSecurity = NULL);

	bool UnmountRamPakLayer(
		C_MemoryFs& memoryFs,
		C_OverlayFs& overlayFs,
		const Sys::C_FilePath& pakFilePath);

	C_OverlayFs* CreateGameFs();

	C_OverlayLayer::E_Type GetDefaultOverlayType();

	C_OverlayFs* GetGameFs();

	void SetTitleId(const char* titleId);

	Sys::C_FilePath GetPotentiallyInstalledFilePath(const Sys::C_FilePath& sourcePath);
}
}

#endif
