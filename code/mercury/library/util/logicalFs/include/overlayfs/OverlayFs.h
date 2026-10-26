#ifndef LOGICALFS_OVERLAY_H
#define LOGICALFS_OVERLAY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Threads/RwLock.h>
#include <Btl/Containers/Vector.h>

#include "OverlayLayer.h"

namespace LogicalFs
{
	enum E_MediaLayer
	{
		E_MediaLayer_0 = 0,
		E_MediaLayer_1,
		E_MediaLayer_Count
	};

	class C_OverlayFs : public Sys::I_FileSystem
	{
	//Sys::I_FileSystem
	public:
		virtual Sys::I_File* Open( 
			const Sys::C_FilePath& filePath, 
			const Sys::E_FileAccess fileAccess = Sys::E_FileAccess_Read,
			const Sys::C_OpenParams& params = Sys::C_OpenParams());

		virtual	~C_OverlayFs();

		virtual bool FileExists(const Sys::C_FilePath& filePath);
	 
		virtual bool FileInfo(const Sys::C_FilePath& filePath, Sys::C_FileInfo* p_FileInfo);

		virtual bool FileAccess(const Sys::C_FilePath& filePath, U32 fileAccess);

		virtual bool FileTime(const Sys::C_FilePath& filePath, const Time::C_Time &fileTime);
	 
		virtual bool DeleteFile(const Sys::C_FilePath& filePath);

		virtual bool DirectoryInfo(const Sys::C_FilePath& filePath, Sys::C_DirectoryInfo* pDirInfo);
	 
		virtual bool CreateSingleDirectory(const Sys::C_FilePath& path);

		virtual bool SetWorkingDirectory(const Sys::C_FilePath& path);

		virtual Sys::C_FilePath GetWorkingDirectory() const;

		virtual Sys::C_FilePath GetSystemFilePath( const Sys::C_FilePath& filePath ) const;


	//C_OverlayFs
	public:
		struct C_LayerResult
		{
			C_LayerResult() : pLayer(0) {}
			C_LayerResult(Sys::C_FilePath layerFilePath_, C_OverlayLayer* pLayer_) : 
				layerFilePath(layerFilePath_), pLayer(pLayer_) {}
			C_LayerResult(const C_LayerResult& rhs) { *this = rhs; }
			const C_LayerResult& operator=(const C_LayerResult& rhs)
			{
				layerFilePath = rhs.layerFilePath;
				pLayer = rhs.pLayer;
				return *this;
			}

			Sys::C_FilePath layerFilePath;
			C_OverlayLayer* pLayer;
		};

		C_OverlayFs();

		void AddLayer(
			const Sys::C_FilePath& overlayPath,		//Where to mount overlay.
			Sys::I_FileSystem* pFs,					//File system for overlay.
			const Sys::C_FilePath& fsPath,			//Path within above file system to use for our overlay.
			C_OverlayLayer::C_Params params,		//Overlay properties.
			E_MediaLayer mediaLayer = E_MediaLayer_0,
			const char* name = 0);					//Optional name for overlay.

		void InsertLayer(
			U32 idx,
			const Sys::C_FilePath& overlayPath,		//Where to mount overlay.
			Sys::I_FileSystem* pFs,					//File system for overlay.
			const Sys::C_FilePath& fsPath,			//Path within above file system to use for our overlay.
			C_OverlayLayer::C_Params params,		//Overlay properties.
			E_MediaLayer mediaLayer = E_MediaLayer_0,
			const char* name = 0);					//Optional name for overlay.

		void DeleteLayerByName(E_MediaLayer mediaLayer, const char* name);
		void DeleteLayerByIndex(E_MediaLayer mediaLayer, U32 idx);

		U32 GetNumLayers(E_MediaLayer mediaLayer) const;

		bool GetLayerByName(C_OverlayLayer& layer, E_MediaLayer mediaLayer, const char* name) const;
		bool GetLayerByIndex(C_OverlayLayer& layer, E_MediaLayer mediaLayer, U32 idx) const;

		bool SetLayerByName(const C_OverlayLayer& layer, E_MediaLayer mediaLayer, const char* name);
		bool SetLayerByIndex(const C_OverlayLayer& layer, E_MediaLayer mediaLayer, U32 idx);

		E_MediaLayer GetPreferredMediaLayer() const;
		void SetPreferredMediaLayer(E_MediaLayer mediaLeyer);

		///Only valid for files to be opened for reading, not writing or appending.
		///Returns 0 if file not found so cannot be used to check for file existence.
		E_MediaLayer GetMediaLayer(const Sys::C_FilePath& filePath) const;

	//C_OverlayFs
	private:
		struct C_PhantomDir
		{
			Sys::C_FilePath path;
			C_PhantomDir*	pParent;
		};

		void ConvertToOverlayFsFilePath(
			Sys::C_FilePath& filePath, 
			const C_OverlayLayer& layer) const;

		bool GetLayerAbsoluteFilePath(
			Sys::C_FilePath& layerAbsoluteFilePath,
			C_OverlayLayer& layer, 
			const Sys::C_FilePath& absoluteFilePath) const;

		bool GetLayersContainingPath(
			Btl::Vector<C_LayerResult>& results, const Sys::C_FilePath& filePath);

		C_OverlayLayer* GetBestLayer(
			Sys::C_FilePath& layerFilePath, E_MediaLayer& mediaLayer, const Sys::C_FilePath& filePath, bool creatingNewContent = false);

		const C_OverlayLayer* GetBestLayer(
			Sys::C_FilePath& layerFilePath, E_MediaLayer& mediaLayer, const Sys::C_FilePath& filePath, bool creatingNewContent = false) const;

		C_OverlayLayer* GetBestLayer(
			Sys::C_FilePath& layerFilePath, Time::C_Time& bestFileTime, const Sys::C_FilePath& filePath, bool creatingNewContent, E_MediaLayer mediaLayer);

		const C_OverlayLayer* GetBestLayer(
			Sys::C_FilePath& layerFilePath, Time::C_Time& bestFileTime, const Sys::C_FilePath& filePath, bool creatingNewContent, E_MediaLayer mediaLayer) const;

		void CreatePhantomDir(const Sys::C_FilePath& overlayPath, bool& found);
		void CreatePhantomDirs(const Sys::C_FilePath& overlayPath);

		//Two sets of layers. One per media layer.
		Btl::Vector<C_OverlayLayer*>	m_layers[E_MediaLayer_Count];
		Sys::C_FilePath					m_workingPath;
		mutable MT::RwLock				m_rwLock;
		Btl::Vector<C_PhantomDir*>		m_phantomDirs;
		volatile U32					m_preferredMediaLayerId;
	};
} 

#endif
