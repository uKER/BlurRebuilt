#include "LogicalFsPch.h"
#include <Core/Threads/Atomic.h>
#include <System/Debug/Debug.h>
#include <System/Serialise/FilePathHashOps.h>
#include <Btl/Containers/OpenHashSet.h>

#include <OverlayFs/OverlayFs.h>
#include <OverlayFs/OverlayLayer.h>


namespace LogicalFs
{

Debug::Bool g_fileMappingDebug("Util.LogicalFs.OverlayFs.FileMappingDebug", false, Debug::Persist());
//TODO: sort this out :/
extern Debug::Bool g_masterDiscEmulation;

//From Sys::I_FileSystem

Sys::I_File* C_OverlayFs::Open( 
	const Sys::C_FilePath& filePath, 
	const Sys::E_FileAccess fileAccess,
	const Sys::C_OpenParams& params)
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	bool writing = (
		fileAccess == Sys::E_FileAccess_Write || 
		fileAccess == Sys::E_FileAccess_Append) ? true : false;
	C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, filePath, writing);
	if (pBestLayer)
	{
		if (writing && !pBestLayer->GetParams().flags.writable)
		{
			BIZ_ASSERT_FAIL("Writable layer logic broken!");
			//Bail if we're attempting to write to a read-only layer.
			return 0;
		}

		return pBestLayer->GetFs()->Open(layerFilePath, fileAccess, params);
	}

	Sys::FileOpenError(filePath.Chars(), fileAccess != Sys::E_FileAccess_Read, params.m_IsCritical);
	return 0;
}

C_OverlayFs::~C_OverlayFs()
{
	MT::WriteLock writeLock(m_rwLock);

	for (S32 i = 0; i < E_MediaLayer_Count; ++i)
	{
		for (S32 j = 0; j < m_layers[i].Size(); ++j)
				SAFE_DELETE_PTR(m_layers[i][j]);
	}

	for (S32 i = 0; i < m_phantomDirs.Size(); ++i)
		SAFE_DELETE_PTR(m_phantomDirs[i]);
}

bool C_OverlayFs::FileExists(const Sys::C_FilePath& filePath)
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, filePath);
	if (pBestLayer)
		return pBestLayer->GetFs()->FileExists(layerFilePath);
	return false;
}

bool C_OverlayFs::FileInfo(const Sys::C_FilePath& filePath, Sys::C_FileInfo* p_FileInfo)
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, filePath);
	if (pBestLayer && pBestLayer->GetFs()->FileInfo(layerFilePath, p_FileInfo))
	{
		ConvertToOverlayFsFilePath(p_FileInfo->m_FileName, *pBestLayer);
		return true;
	}
	return false;
}

bool C_OverlayFs::FileAccess(const Sys::C_FilePath& filePath, U32 fileAccess)
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, filePath);
	if (pBestLayer)
		return pBestLayer->GetFs()->FileAccess(layerFilePath, fileAccess);
	return false;
}

bool C_OverlayFs::FileTime(const Sys::C_FilePath& filePath, const Time::C_Time &fileTime)
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, filePath);
	if (pBestLayer)
		return pBestLayer->GetFs()->FileTime(layerFilePath, fileTime);
	return false;
}

bool C_OverlayFs::DeleteFile(const Sys::C_FilePath& filePath)
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, filePath);
	if (pBestLayer)
		return pBestLayer->GetFs()->DeleteFile(layerFilePath);
	return false;
}

bool C_OverlayFs::DirectoryInfo(const Sys::C_FilePath& filePath, Sys::C_DirectoryInfo* pDirInfo)
{
	MT::ReadLock readLock(m_rwLock);
	bool found = false;

	Sys::FileInfoArr& dirResultArr = pDirInfo->FileInfoArray();
	dirResultArr.Clear();

	//Set of unique filepaths.
	Btl::OpenHashSet<Sys::C_FilePath, Sys::HashKeyOpFilePath> uniqueFilePaths;
	
	//Overlay layers matching path.
	Btl::Vector<C_LayerResult> matchingLayers;
	if (GetLayersContainingPath(matchingLayers, filePath))
	{
		found = true;
		for (S32 i = 0; i < matchingLayers.Size(); ++i)
		{
			C_LayerResult& r = matchingLayers[i];
			if (!r.pLayer)
			{
				BIZ_ASSERT_FAIL("Null overlay layer in dir query!");
				continue;
			}
			Sys::C_DirectoryInfo matchDirInfo;
			if (!r.pLayer->GetFs()->DirectoryInfo(r.layerFilePath, &matchDirInfo))
			{
				//Path does not exist within this layer.
				continue;
			}

			Sys::FileInfoArr& arr = matchDirInfo.FileInfoArray();
			for (S32 i = 0; i < arr.Size(); ++i)
			{
				ConvertToOverlayFsFilePath(arr[i].m_FileName, *r.pLayer);
				uniqueFilePaths.Insert(arr[i].m_FileName);
			}
		}
	}
	//Add phantom dir contents.
	{
		Sys::C_FilePath absolutePath = 
			Sys::ConvertToCurrentDirectory( GetWorkingDirectory(), filePath);
		Sys::C_FilePath noTrailing = absolutePath.Length() > 1 ? Sys::RemoveTrailingSlash(absolutePath) : absolutePath;
		bool isRoot = noTrailing == Sys::GetLocalDirToken();

		const C_PhantomDir* pPhantomDir = 0;
		for (S32 i = 0; i < m_phantomDirs.Size(); ++i)
		{
			if (m_phantomDirs[i]->path == noTrailing)
			{
				pPhantomDir = m_phantomDirs[i];
				found = true;
				break;
			}
		}
		if (isRoot || pPhantomDir)
		{
			//Add contents of phantom dir.
			for (S32 i = 0; i < m_phantomDirs.Size(); ++i)
			{
				const C_PhantomDir* pCurDir = m_phantomDirs[i];
				if (pCurDir->pParent == pPhantomDir)
					uniqueFilePaths.Insert(pCurDir->path);
			}
		}
	}
	for (Btl::OpenHashSet<Sys::C_FilePath, Sys::HashKeyOpFilePath>::Iterator it =
		uniqueFilePaths.Begin(); it != uniqueFilePaths.End(); ++it)
	{
		const Sys::C_FilePath& overlayFilePath = *it;
		Sys::C_FileInfo fileInfo;
		{
			Sys::C_FilePath layerFilePath;
			E_MediaLayer mediaLayer;
			C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, overlayFilePath);
			if (pBestLayer && pBestLayer->GetFs()->FileInfo(layerFilePath, &fileInfo))
			{
				ConvertToOverlayFsFilePath(fileInfo.m_FileName, *pBestLayer);
			}
			//Phantom dir.
			else
			{
				fileInfo.m_FileName = overlayFilePath;
				fileInfo.m_FileAccess = Sys::E_FileAccess_Read;
				fileInfo.m_FileType = Sys::E_FileType_Directory;
				fileInfo.m_FileTime = 0;
				fileInfo.m_FileSize = 0;
			}
			dirResultArr.PushBack(fileInfo);
		}
	}

	return found;
}

bool C_OverlayFs::CreateSingleDirectory(const Sys::C_FilePath& path)
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, path, true);
	if (pBestLayer && pBestLayer->GetFs()->CreateSingleDirectory(layerFilePath))
		return true;
	return false;
}

bool C_OverlayFs::SetWorkingDirectory(const Sys::C_FilePath& path)
{
	MT::WriteLock writeLock(m_rwLock);
	m_workingPath = path;
	return true;
}

Sys::C_FilePath C_OverlayFs::GetWorkingDirectory() const
{
	MT::ReadLock readLock(m_rwLock);
	return m_workingPath;
}

Sys::C_FilePath C_OverlayFs::GetSystemFilePath( const Sys::C_FilePath& filePath ) const
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	const C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, filePath, false);
	if (pBestLayer)
	{
		return pBestLayer->GetFs()->GetSystemFilePath(layerFilePath);
	}
	return "";
}

//From C_OverlayFs

C_OverlayFs::C_OverlayFs()
:
m_preferredMediaLayerId(0)
{
}

void C_OverlayFs::AddLayer(
	const Sys::C_FilePath& overlayPath,
	Sys::I_FileSystem* pFs,
	const Sys::C_FilePath& fsPath,
	C_OverlayLayer::C_Params params,
	E_MediaLayer mediaLayer,
	const char* name)
{
	MT::WriteLock writeLock(m_rwLock);

	CreatePhantomDirs(overlayPath);

	C_OverlayLayer* pLayer = new C_OverlayLayer(
		overlayPath,
		pFs,
		fsPath,
		params,
		name ? name : "");

	m_layers[mediaLayer].PushBack(pLayer);
}

void C_OverlayFs::InsertLayer(
	U32 idx,
	const Sys::C_FilePath& overlayPath,
	Sys::I_FileSystem* pFs,
	const Sys::C_FilePath& fsPath,
	C_OverlayLayer::C_Params params,
	E_MediaLayer mediaLayer,
	const char* name)
{
	MT::WriteLock writeLock(m_rwLock);

	CreatePhantomDirs(overlayPath);

	C_OverlayLayer* pLayer = new C_OverlayLayer(
		overlayPath,
		pFs,
		fsPath,
		params,
		name ? name : "");
	if ((S32)idx >= m_layers[mediaLayer].Size())
		m_layers[mediaLayer].PushBack(pLayer);
	else
	{
		Btl::Vector<C_OverlayLayer*>::Iterator it = m_layers[mediaLayer].MakeIterator(idx);
		m_layers[mediaLayer].InsertBefore(pLayer, it);
	}
}

void C_OverlayFs::DeleteLayerByName(E_MediaLayer mediaLayer, const char* name)
{
	MT::WriteLock writeLock(m_rwLock);

	if (name)
	{
		for (S32 i = 0; i < m_layers[mediaLayer].Size(); ++i)
		{
			if (m_layers[mediaLayer][i]->GetName() == name)
			{
				SAFE_DELETE_PTR(m_layers[mediaLayer][i])
				m_layers[mediaLayer].EraseByIndex(i);
				return;
			}
		}
	}
	BIZ_ASSERT_FAIL("Failed to find layer to delete!");
}

void C_OverlayFs::DeleteLayerByIndex(E_MediaLayer mediaLayer, U32 index)
{
	MT::WriteLock writeLock(m_rwLock);

	SAFE_DELETE_PTR(m_layers[mediaLayer][index]);
	m_layers[mediaLayer].EraseByIndex(index);
}

U32 C_OverlayFs::GetNumLayers(E_MediaLayer mediaLayer) const
{
	MT::ReadLock readLock(m_rwLock);

	return (U32)m_layers[mediaLayer].Size();
}

bool C_OverlayFs::GetLayerByName(C_OverlayLayer& layer, E_MediaLayer mediaLayer, const char* name) const
{
	MT::ReadLock readLock(m_rwLock);

	if (name)
	{
		for (S32 i = 0; i < m_layers[mediaLayer].Size(); ++i)
			if (m_layers[mediaLayer][i]->GetName() == name)
			{
				BIZ_ASSERT(m_layers[mediaLayer][i]);
				layer = *m_layers[mediaLayer][i];
				return true;
			}
	}
	return false;
}

bool C_OverlayFs::GetLayerByIndex(C_OverlayLayer& layer, E_MediaLayer mediaLayer, U32 idx) const
{
	MT::ReadLock readLock(m_rwLock);
	if ((S32)idx > m_layers[mediaLayer].Size())
		return false;

	BIZ_ASSERT(m_layers[mediaLayer][idx]);
	layer = *m_layers[mediaLayer][idx];
	return true;
}

bool C_OverlayFs::SetLayerByName(const C_OverlayLayer& layer, E_MediaLayer mediaLayer, const char* name)
{
	MT::ReadLock readLock(m_rwLock);

	if (name)
	{
		for (S32 i = 0; i < m_layers[mediaLayer].Size(); ++i)
			if (m_layers[mediaLayer][i]->GetName() == name)
			{
				BIZ_ASSERT(m_layers[mediaLayer][i]);
				*m_layers[mediaLayer][i] = layer;
				return true;
			}
	}
	return false;
}

bool C_OverlayFs::SetLayerByIndex(const C_OverlayLayer& layer, E_MediaLayer mediaLayer, U32 idx)
{
	MT::ReadLock readLock(m_rwLock);
	if ((S32)idx > m_layers[mediaLayer].Size())
		return false;

	BIZ_ASSERT(m_layers[mediaLayer][idx]);
	*m_layers[mediaLayer][idx] = layer;
	return true;
}

E_MediaLayer C_OverlayFs::GetPreferredMediaLayer() const
{
	return (E_MediaLayer)MT::AtomicGet((volatile U32*)&m_preferredMediaLayerId);
}

void C_OverlayFs::SetPreferredMediaLayer(E_MediaLayer mediaLayer)
{
	MT::AtomicSet(&m_preferredMediaLayerId, (U32)mediaLayer);
	LOGINFO(g_log, "OverlayFs preferred layer set to %i", mediaLayer);
}

E_MediaLayer C_OverlayFs::GetMediaLayer(const Sys::C_FilePath& filePath) const
{
	MT::ReadLock readLock(m_rwLock);

	Sys::C_FilePath layerFilePath;
	E_MediaLayer mediaLayer;
	const C_OverlayLayer* pBestLayer = GetBestLayer(layerFilePath, mediaLayer, filePath, false);
	if (pBestLayer)
		return mediaLayer;
	return E_MediaLayer_0;
}

void C_OverlayFs::ConvertToOverlayFsFilePath(Sys::C_FilePath& filePath, const C_OverlayLayer& layer) const
{
	//Subtract fs path from existing file info.
	filePath = filePath.String()(layer.GetFsPath().Length(), filePath.Length());
	//Prepend overlay path.
	filePath = layer.GetOverlayPath() + Sys::C_FilePath("/") + filePath;
	if (filePath.Length() > 0)
		filePath = Sys::RemoveTrailingSlash(filePath);
}

bool C_OverlayFs::GetLayerAbsoluteFilePath(
	Sys::C_FilePath& layerAbsoluteFilePath, 
	C_OverlayLayer& layer, 
	const Sys::C_FilePath& absoluteFilePath) const
{
	//Find path in layer overlay path.
	{
		if (layer.GetOverlayPath().Length() > absoluteFilePath.Length())
			return false;
		//Find relative path from absolute root -> layerFs root.
		if (memcmp(layer.GetOverlayPath().Chars(), absoluteFilePath.Chars(), layer.GetOverlayPath().Length()))
			//Failure.
			return false;
	}

	//Make overlay path relative filepath
	Sys::C_FilePath overlayRelativePath = 
		absoluteFilePath.String()(layer.GetOverlayPath().Length(), absoluteFilePath.Length());

	//Translate overlay relative filepath to absolute filepath in layer's local fs.
	layerAbsoluteFilePath = layer.GetFsPath() + Sys::C_FilePath("/") + overlayRelativePath;
	return true;
}

bool C_OverlayFs::GetLayersContainingPath(Btl::Vector<C_LayerResult>& results, const Sys::C_FilePath& filePath)
{
	Sys::C_FilePath absoluteFilePath =
		Sys::ConvertToCurrentDirectory(GetWorkingDirectory(), filePath);
	
	for (S32 mediaLayer = 0; mediaLayer < E_MediaLayer_Count; ++mediaLayer)
	{
		//Iterate layers in reverse order so that we ensure we encounter opaque and overlay layers first.
		for (S32 i = m_layers[mediaLayer].Size()-1; i >= 0; --i)
		{
			C_OverlayLayer* pLayer = m_layers[mediaLayer][i];
			BIZ_ASSERT(pLayer);
			Sys::C_FilePath layerAbsoluteFilePath;
			bool layerContainsPath = 
				GetLayerAbsoluteFilePath(layerAbsoluteFilePath, *pLayer, absoluteFilePath);
			if (layerContainsPath)
				results.PushBack(C_LayerResult(layerAbsoluteFilePath, pLayer));
		}
	}
	return results.Size() > 0;
}

C_OverlayLayer* C_OverlayFs::GetBestLayer(
	Sys::C_FilePath& layerFilePath, E_MediaLayer& mediaLayer, const Sys::C_FilePath& filePath, bool creatingNewContent)
{
#ifdef CFG_GOLD
	bool isMaster = true;
#else
	bool isMaster = g_masterDiscEmulation;
#endif

	U32 preferredMediaLayer = GetPreferredMediaLayer();

	C_OverlayLayer* pResult = NULL;
	mediaLayer = E_MediaLayer_0;

	U32 mediaLayerSearchOrder[E_MediaLayer_Count];
	mediaLayerSearchOrder[0] = preferredMediaLayer;
	mediaLayerSearchOrder[1] = !preferredMediaLayer;

	Time::C_Time bestFileTime = 0;
	for (U32 idx = 0; idx < E_MediaLayer_Count; ++idx)
	{
		E_MediaLayer currentLayer = (E_MediaLayer)mediaLayerSearchOrder[idx];
		Time::C_Time fileTime = 0;
		Sys::C_FilePath layerFilePathTmp;
		C_OverlayLayer* pLayer = GetBestLayer(layerFilePathTmp, fileTime, filePath, creatingNewContent, currentLayer);
		if (pLayer)
		{
			//Return immediately if we find content on our preferred layer.
			if (currentLayer == preferredMediaLayer)
			{
				layerFilePath = layerFilePathTmp;
				bestFileTime = fileTime;
				pResult = pLayer;
				//Store the media layer we've found our path on.
				mediaLayer = currentLayer;
				if (isMaster)
					break;
				else
					continue;
			}
			//continuing to other layers...
			
			//If we've no hits yet, let's use this one.
			if (!pResult)
			{
				layerFilePath = layerFilePathTmp;
				bestFileTime = fileTime;
				pResult = pLayer;
				//Store the media layer we've found our path on.
				mediaLayer = currentLayer;
			}
			//Use newest files in non master build.
			else if (!isMaster && fileTime > bestFileTime)
			{
				layerFilePath = layerFilePathTmp;
				bestFileTime = fileTime;
				pResult = pLayer;
				//Store the media layer we've found our path on.
				mediaLayer = currentLayer;
			}
		}

		//Warn about duplicate content with differing dates.
		if (!isMaster && pResult && pLayer)
		{
			if (fileTime != bestFileTime)
				LOGWARN(g_log, "File %s duplicated across media layers with differing dates. Grabbing a mirrored build will clean this.",
					filePath.Chars());
		}
	}

	if (g_fileMappingDebug)
	{
		if (pResult)
			LOGINFO(g_log, "Mapped path %s to overlayfs media layer %i - %s", 
				filePath.Chars(), mediaLayer, pResult->GetName().Chars());
//			mediaLayer != preferredMediaLayer ? "Not on preferred layer!" : "");
		else 
			LOGERROR(g_log, "Failed to map path %s to overlayfs", filePath.Chars());
	}
	return pResult;
}

C_OverlayLayer* C_OverlayFs::GetBestLayer(
	Sys::C_FilePath& layerFilePath, 
	Time::C_Time& bestFileTime,
	const Sys::C_FilePath& filePath, 
	bool creatingNewContent, E_MediaLayer mediaLayer)
{
	//Grab a reference to our current set of layers.
	Btl::Vector<C_OverlayLayer*>& layers = m_layers[mediaLayer];

	Sys::C_FilePath absoluteFilePath =
		Sys::ConvertToCurrentDirectory(m_workingPath, filePath);

	U8 preferredMediaLayer = GetPreferredMediaLayer();
	
	//Iterate layers in reverse order so that we ensure we encounter opaque and overlay layers first.
	S32 bestLayer = -1;
	bestFileTime = 0;
	Sys::C_FilePath		bestLayerAbsoluteFilePath;
	C_OverlayLayer*		pBestLayer = NULL;
	for (S32 i = layers.Size()-1; i >= 0; --i)
	{
		C_OverlayLayer* pLayer = layers[i];
		BIZ_ASSERT(pLayer);
		Sys::C_FilePath layerAbsoluteFilePath;
		bool layerContainsPath = 
			GetLayerAbsoluteFilePath(layerAbsoluteFilePath, *pLayer, absoluteFilePath);
		//Skip to next layer if our absolute path isn't contained within this layer.
		if (!layerContainsPath)
			continue;

		C_OverlayLayer::E_Type layerType = (C_OverlayLayer::E_Type)pLayer->GetParams().flags.layerType;
		if (layerType == C_OverlayLayer::E_Type_Opaque)
		{
			//If we cannot write here...
			if (creatingNewContent && !pLayer->GetParams().flags.writable)
			{
				bestLayer = -1;
				pBestLayer = NULL;
				break;
			}

			bestLayerAbsoluteFilePath = layerAbsoluteFilePath;
			bestLayer = i;
			pBestLayer = pLayer;
			break;
		}
		else if (layerType == C_OverlayLayer::E_Type_Overlay)
		{
			if (creatingNewContent && !pLayer->GetParams().flags.writable)
				continue;

			//Return if we were able to open/create our file here
			if (creatingNewContent)
			{
				bestLayerAbsoluteFilePath = layerAbsoluteFilePath;
				bestLayer = i;
				pBestLayer = pLayer;
				break;
			}

			//Skip if file not present.
			if (!pLayer->GetFs()->FileExists(layerAbsoluteFilePath))
				continue;

			//Jackpot, we've found an overlay layer containing our file.
			bestLayerAbsoluteFilePath = layerAbsoluteFilePath;
			bestLayer = i;
			pBestLayer = pLayer;
			break;
		}
		else if (layerType == C_OverlayLayer::E_Type_Newer)
		{
			if (creatingNewContent)
				if (pLayer->GetParams().flags.writable)
				{
					bestLayerAbsoluteFilePath = layerAbsoluteFilePath;
					bestLayer = i;
					pBestLayer = pLayer;
					break;
				}
				else
					continue;		//Read only

			//Grab filetime, skip upon failure.
			Sys::C_FileInfo fileInfo;
			if (!pLayer->GetFs()->FileInfo(layerAbsoluteFilePath, &fileInfo))
				continue;

			//If newer...
			if (bestLayer == -1 || fileInfo.m_FileTime > bestFileTime)
			{
				//It's the best layer yet!
				bestLayer = i;
				bestFileTime = fileInfo.m_FileTime;
				bestLayerAbsoluteFilePath = layerAbsoluteFilePath;
				pBestLayer = pLayer;
			}
		}
	}

	if (bestLayer != -1)
	{
		//If we get here then we use our best candidate layer.
		layerFilePath = bestLayerAbsoluteFilePath;
		return pBestLayer;
	}

	return 0;
}

const C_OverlayLayer* C_OverlayFs::GetBestLayer(
	Sys::C_FilePath& layerFilePath, E_MediaLayer& mediaLayer, const Sys::C_FilePath& filePath, bool creatingNewContent) const
{
	return const_cast<C_OverlayFs*>(this)->GetBestLayer(layerFilePath, mediaLayer, filePath, creatingNewContent);
}

const C_OverlayLayer* C_OverlayFs::GetBestLayer(
	Sys::C_FilePath& layerFilePath, Time::C_Time& bestFileTime, const Sys::C_FilePath& filePath, bool creatingNewContent, E_MediaLayer mediaLayer) const
{
	return const_cast<C_OverlayFs*>(this)->GetBestLayer(layerFilePath, bestFileTime, filePath, creatingNewContent, mediaLayer);
}

void C_OverlayFs::CreatePhantomDir(const Sys::C_FilePath& overlayPath, bool& found)
{
	found = false;

	//Already exists?
	for (S32 i = 0; i < m_phantomDirs.Size(); ++i)
		if (m_phantomDirs[i]->path == overlayPath)
		{
			found = true;
			return;
		}

	C_PhantomDir* pParent = 0;
	S32 parentEndDirToken = overlayPath.RevSearch(Sys::GetLocalDirToken());
	if (parentEndDirToken == -1)
	{
		 BIZ_ASSERT_FAIL("Failed to create phantom dir!");
		 return;
	}

	Sys::C_FilePath parent = overlayPath.String()(0, parentEndDirToken);

	//Find parent
	C_PhantomDir* pPhantomParent = 0;
	if (parent.Length() > 0)
	{
		for (S32 i = 0; i < m_phantomDirs.Size(); ++i)
			if (m_phantomDirs[i]->path == parent)
			{
				pPhantomParent = m_phantomDirs[i];
				break;
			}
	}
	C_PhantomDir* pPhantomDir = new C_PhantomDir;
	pPhantomDir->path = overlayPath;
	pPhantomDir->pParent = pPhantomParent;
	m_phantomDirs.PushBack(pPhantomDir);
}

void C_OverlayFs::CreatePhantomDirs(const Sys::C_FilePath& overlayPath)
{
	Btl::Vector<Sys::C_FilePath> dirs;
	overlayPath.TokeniseByDir(dirs);

	Sys::C_FilePath dirChain;
	for (S32 i = 0; i < dirs.Size(); ++i)
	{
		dirChain += "/";
		dirChain += dirs[i];

		bool found;
		CreatePhantomDir(dirChain, found);
		if (found)
			break;
	}
}

}
