#include "stdafx.h"
#include <tex_atlas/Space.h>

#include <tex_atlas/TexturePackingLists.h>
#include <tex_atlas/Atlas.h>
#include <attk/attkut/ImageRotate.h>

using namespace TexAtlas;

std::string DefaultAtlasName(const std::string& baseName, U32 atlasIndex, U32 layerIndex)
{
	std::stringstream ss;
	ss 
		<< Tu::RemoveFileExtension(baseName) 
		<< "_baseatlas" 
		<< atlasIndex
		<< "_layer" 
		<< layerIndex
		<< '.' << Tu::FileExtension(baseName);
	return ss.str();
}

Atlas::AtlasNameCallback Atlas::m_atlasNameCallback = &DefaultAtlasName;

Atlas::Atlas()
{
	OneTimeInit();
}

Atlas::~Atlas()
{
	delete[] m_pAtlasList;
}

Attk::C_Status Atlas::Pack
(
	U32 atlasIndex,
	U32 width, 
	U32 height,
	Attk::E_PixelFormat format,
	bool allowRotation,
	U32 border,
	Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > layerBackgroundColourList,
	Database& database,
	List<U32>& sortedTextureList,
	TexturePackingLists& texturePackingLists,
	ImageCache& imageCache,
	Progress* pProgressCallback
)
{
	Clear();
	m_mode = PACK;
	m_pTexturePackingLists = &texturePackingLists;
	m_nLayers = m_pTexturePackingLists->NumLayers();

	Attk::C_Status status;

	//Create atlas images, one per layer.
	m_pAtlasList = new Attk::C_Image[m_pTexturePackingLists->NumLayers()];
	memset(m_textureCountList, 0, sizeof(U32)*m_pTexturePackingLists->NumLayers());
	memset(m_textureAreaList, 0, sizeof(U32)*m_pTexturePackingLists->NumLayers());
	//Initialise atlases.
	for (U32 iAtlasImage = 0; iAtlasImage < m_pTexturePackingLists->NumLayers(); iAtlasImage++)
	{
		status = m_pAtlasList[iAtlasImage].NewPixelData(width, height, 1, Attk::E_PixelFormat_R8G8B8A8U);
		if (!status) return status;
		
		if (iAtlasImage < (U32)layerBackgroundColourList.Size())
		{
			LiteMaths::Vec4U8 colour = layerBackgroundColourList[iAtlasImage];
			Attk::C_PixelA32R32G32B32F pixel;
			pixel.red = colour.X();
			pixel.green = colour.Y();
			pixel.blue = colour.Z();
			pixel.alpha = colour.W();
			m_pAtlasList[iAtlasImage].Fill(pixel);
		}
		else
		{
			Attk::C_PixelA32R32G32B32F pixel;
			pixel.red = 255;
			pixel.green = 0;
			pixel.blue = 0;
			pixel.alpha = 0;
			//Fill unused space with red pixels by default.
			m_pAtlasList[iAtlasImage].Fill(pixel);
		}
	}

	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Packing atlas...\n");
		if (pProgressCallback->OnProgress(0))
			return Attk::E_Status_Cancelled;
	}

	Space space;
	space.Set(0,0,width, height);
	m_spaces.Insert(Ut::MultiMap<U32, Space>::DataType(space.Area(), space));
	
	ListItem<U32>* pTextureIndexItem = sortedTextureList.Begin();
	while(pTextureIndexItem)
	{
		U32 textureIndex = pTextureIndexItem->GetData();
		bool couldNotPack;
		Insert(
			couldNotPack,
			atlasIndex,
			allowRotation,
			border,
			textureIndex,
			&database,
			&imageCache,
			pProgressCallback);
		if (!couldNotPack)
		{
			ListItem<U32>* pNext = pTextureIndexItem->GetNext();
			//Remove our packed texture from the list to pack.
			sortedTextureList.Remove(pTextureIndexItem);
			pTextureIndexItem = pNext;
		}
		else
		{
			pTextureIndexItem = pTextureIndexItem->GetNext();
		}
	}

	return Attk::E_Status_Success;
}

Attk::C_Status Atlas::PackDryRun
(
	U32 width, 
	U32 height,
	bool allowRotation,
	U32 border,
	const TexturePackingLists& texturePackingLists,
	List<U32>& sortedTextureList
)
{
	Clear();
	m_mode = PACK_DRY_RUN;
	//Constness of below variable is dependant upon whether we are packing, or performing a dry-run.
	//Hence the necessity of such ugly const-casting.
	m_pTexturePackingLists = const_cast<TexturePackingLists*>((TexturePackingLists*)&texturePackingLists);

	memset(m_textureCountList, 0, sizeof(U32)*m_pTexturePackingLists->NumLayers());
	memset(m_textureAreaList, 0, sizeof(U32)*m_pTexturePackingLists->NumLayers());

	Space space;
	space.Set(0,0,width, height);
	m_spaces.Insert(Ut::MultiMap<U32, Space>::DataType(space.Area(), space));

	ListItem<U32>* pTextureIndexItem = sortedTextureList.Begin();
	while(pTextureIndexItem)
	{
		U32 textureIndex = pTextureIndexItem->GetData();
		bool couldNotPack;
		Insert(
			couldNotPack,
			0,
			allowRotation,
			border,
			textureIndex,
			0,
			0,
			0);
		if (!couldNotPack)
		{
			ListItem<U32>* pNext = pTextureIndexItem->GetNext();
			//Remove our packed texture from the list to pack.
			sortedTextureList.Remove(pTextureIndexItem);
			pTextureIndexItem = pNext;
		}
		else
		{
			pTextureIndexItem = pTextureIndexItem->GetNext();;
		}
	}

	return Attk::E_Status_Success;
}

Attk::E_Status Atlas::Insert
(
	bool& couldNotPack,
	U32 atlasIndex,
	bool allowRotation,
	U32 border,
	U32 textureIndex,
	Database* pDatabase,
	ImageCache* pImageCache,
	Progress* pProgressCallback
)
{
	TexRecord& baseTexRecord = m_pTexturePackingLists->Record(
		CS_GEN_PACKING_LAYER, textureIndex);

	//Abort this insertion - texture is missing.
	if (baseTexRecord.missing)
	{
		couldNotPack = false;
		m_textureCountList[CS_GEN_PACKING_LAYER]++;
		return Attk::E_Status_Success;
	}

	assert(IsPowerOf2(baseTexRecord.width));
	assert(IsPowerOf2(baseTexRecord.height));

	//Alloate a texture pointer per layer.
	Attk::C_Image* pImageList[CS_GEN_MAX_LAYERS];
	memset(pImageList, 0, sizeof(pImageList));
	Attk::C_Image* pRotatedImageList[CS_GEN_MAX_LAYERS];
	memset(pRotatedImageList, 0, sizeof(pRotatedImageList));

	ContainershipPossiblities bestFitContainership;
	const Space* pBestFitSpace = 0;
	couldNotPack = true;
	{
		Ut::MultiMap<U32, Space>::Iterator it;
		for (it = m_spaces.Begin(); it != m_spaces.End(); ++it)
		{
			const Space& candidateSpace = it->GetValue();
			ContainershipPossiblities candidateContainership(CANNOT_CONTAIN);

			if (candidateSpace.CanContain(
				Vec2S32(
					baseTexRecord.width,
					baseTexRecord.height)))
			{
				candidateContainership = CAN_CONTAIN;
			}
			else if (allowRotation)
			{
				if (candidateSpace.CanContain(
					Vec2S32(
						baseTexRecord.height,
						baseTexRecord.width)))
				{
					candidateContainership = CAN_CONTAIN_ROTATED;
				}
			}

			if (candidateContainership == CANNOT_CONTAIN)
			{
				continue;
			}
			//If we can contain this image then...
			else
			{
				if (!pBestFitSpace)
				{
					bestFitContainership = candidateContainership;
					pBestFitSpace = &candidateSpace;
				}
				else
				{
					//Favour leftmost space
					if (candidateSpace.X() < pBestFitSpace->X())
					{
						bestFitContainership = candidateContainership;
						pBestFitSpace = &candidateSpace;
					}
				}
			}
		}

		//We've found a space into which we can insert the given texture.
		if (pBestFitSpace)
		{
			//Increase our texture count.
			m_textureCountList[CS_GEN_PACKING_LAYER]++;

			if (m_mode == PACK)
			{
				for (U32 iLayer = 0; iLayer < m_pTexturePackingLists->NumLayers(); iLayer++)
				{
					const TexRecord& texRecord = 
					m_pTexturePackingLists->Record(
						iLayer, 
						textureIndex);
					if (texRecord.used)
					{
						S32 cacheIndex = pImageCache->RecordIndex(texRecord.key);
						assert(cacheIndex != -1);

						pImageList[iLayer] = &pImageCache->Record(cacheIndex).image;
						if (iLayer != CS_GEN_PACKING_LAYER)
							m_textureCountList[iLayer]++;
					}
					else
					{
						pImageList[iLayer] = 0;
					}
				}
			}
			
			//Accumulate texture area per layer in order to calculate efficiency later on.
			for (U32 iLayer = 0; iLayer < m_pTexturePackingLists->NumLayers(); iLayer++)
			{
				const TexRecord& texRecord = 
				m_pTexturePackingLists->Record(
					iLayer, 
					textureIndex);
				if (texRecord.used)
				{
					m_textureAreaList[iLayer] += texRecord.width * texRecord.height;
				}
			}
			
			//Remove this texture from our list to pack.
			couldNotPack = false;
		}
	}
	if (couldNotPack)
	{
		//Cannot pack this image.
		return Attk::E_Status_Success;
	}

	if (m_mode == PACK)
	{
		//Insert image in to atlas.
		{
			//Update progress
			if (pProgressCallback)
			{
				if (pProgressCallback->OnProgress(
					((F32)m_textureCountList[CS_GEN_PACKING_LAYER]/(F32)m_pTexturePackingLists->NumRecords())*100.f))
						return Attk::E_Status_Cancelled;
			}

			//Add a new database Record for this image.
			Database::DBImageRecord dbRecord;
			dbRecord.lowDate = m_pTexturePackingLists->Record(
				CS_GEN_PACKING_LAYER, textureIndex).lowDate;
			dbRecord.highDate = m_pTexturePackingLists->Record(
				CS_GEN_PACKING_LAYER, textureIndex).highDate;
			dbRecord.key = m_pTexturePackingLists->Record(
				CS_GEN_PACKING_LAYER, textureIndex).key;
			F32 trueImageWidth = (F32)baseTexRecord.width - (F32)border * 2;
			F32 trueImageHeight = (F32)baseTexRecord.height - (F32)border * 2;
			dbRecord.uScale = trueImageWidth / (F32)m_pAtlasList[CS_GEN_PACKING_LAYER].GetWidth();
			dbRecord.vScale = trueImageHeight / (F32)m_pAtlasList[CS_GEN_PACKING_LAYER].GetHeight();
			dbRecord.atlasIndex = atlasIndex;

			//Insert without rotation.
			if (bestFitContainership == CAN_CONTAIN)
			{
				dbRecord.uOffset = (F32)pBestFitSpace->TopLeftCorner().X() / (F32)m_pAtlasList[CS_GEN_PACKING_LAYER].GetWidth();
				dbRecord.vOffset = (F32)pBestFitSpace->TopLeftCorner().Y() / (F32)m_pAtlasList[CS_GEN_PACKING_LAYER].GetHeight();
				dbRecord.rotated = false;
			}
			//Insert with rotation.
			else
			{
				dbRecord.uOffset = (F32)pBestFitSpace->TopLeftCorner().X() / (F32)m_pAtlasList[CS_GEN_PACKING_LAYER].GetWidth();
				dbRecord.vOffset = (F32)pBestFitSpace->TopLeftCorner().Y() / (F32)m_pAtlasList[CS_GEN_PACKING_LAYER].GetHeight();
				for (U32 iLayer = 0; iLayer < m_pTexturePackingLists->NumLayers(); iLayer++)
				{
					//If we've got an image for this layer...
					if (pImageList[iLayer])
					{			
						pRotatedImageList[iLayer] = new Attk::C_Image(*pImageList[iLayer]);
						AttkUt::C_ImageRotate::Rotate90CCW(*pRotatedImageList[iLayer]);
						pImageList[iLayer] = pRotatedImageList[iLayer];
					}
				}
				dbRecord.rotated = true;
			}
			dbRecord.uOffset += (F32)border / (F32)m_pAtlasList[CS_GEN_PACKING_LAYER].GetWidth();
			dbRecord.vOffset += (F32)border / (F32)m_pAtlasList[CS_GEN_PACKING_LAYER].GetHeight();

			S32 newRecordIndex = pDatabase->AddImageRecord(dbRecord);
			//Insert appropriate texture for each layer.
			for (U32 iLayer = 0; iLayer < m_pTexturePackingLists->NumLayers(); iLayer++)
			{
				//If we've got an image for this layer then insert it.
				if (pImageList[iLayer])
				{
					if (!baseTexRecord.missing)
					{
						m_pAtlasList[iLayer].Insert(
							pBestFitSpace->TopLeftCorner().X(),
							pBestFitSpace->TopLeftCorner().Y(),
							*pImageList[iLayer]);
						if (bestFitContainership == CAN_CONTAIN)
						{
							assert(pImageList[iLayer]->GetWidth() == baseTexRecord.width);
							assert(pImageList[iLayer]->GetHeight() == baseTexRecord.height);
						}
						else
						{
							assert(pImageList[iLayer]->GetWidth() == baseTexRecord.height);
							assert(pImageList[iLayer]->GetHeight() == baseTexRecord.width);
						}
					}
				}
			}
		}
//			std::string fileName = GetAtlasName("c:/temp/pwnage.tga", 0, 0);
//			m_pAtlasList[0].Write(fileName);
	}

	if (pRotatedImageList[CS_GEN_PACKING_LAYER])
	{
		//delete rotated copies.
		for (U32 iLayer = 0; iLayer < m_pTexturePackingLists->NumLayers(); iLayer++)
		{
			delete pRotatedImageList[iLayer];
		}
	}

	m_dirty = true;

	Space newImageSpace;
	newImageSpace.Set(
		pBestFitSpace->X(),
		pBestFitSpace->Y(),
		baseTexRecord.width,
		baseTexRecord.height);

	ClipSpaces(newImageSpace);
	return Attk::E_Status_Success;
}

Attk::C_Status Atlas::Write
(
	const std::string& baseFileName,
	U32 atlasIndex,
	Database& db,
	void (*pMessageCallbackFunc)(const std::string& text)
)
{
	Attk::C_Status status;
	for (U32 iLayer = 0; iLayer < m_nLayers; iLayer++)
	{
		std::string fileName = GetAtlasName(baseFileName, atlasIndex, iLayer);
		status = m_pAtlasList[iLayer].Write(fileName.c_str());
		if (!status) return status;
	
		Database::DBAtlasRecord r;
		r.fileName = fileName;
		db.AddAtlasRecord(r);

		status = m_pAtlasList[iLayer].Write(fileName.c_str());
		if (!status) return status;

		std::stringstream ss;
		ss << "Wrote scale crop sheet [" << fileName << "].\nCrop sheet contains " << NumTextures(iLayer) << " textures with " << Efficiency(iLayer)*100.f << "% efficiency.";
		pMessageCallbackFunc(ss.str());
	}

	m_dirty = false;
	return Attk::E_Status_Success;
}

void Atlas::Clear()
{
	delete[] m_pAtlasList;
	m_pAtlasList = 0;
	memset(m_textureCountList, 0, sizeof(U32)*CS_GEN_MAX_LAYERS);
	memset(m_textureAreaList, 0, sizeof(U32)*CS_GEN_MAX_LAYERS);
	m_nLayers = 0;

	m_dirty = false;
	m_mode = UNKNOWN;
	m_pTexturePackingLists = 0;
}

bool Atlas::Dirty() const
{
	return m_dirty;
}

U32 Atlas::NumTextures(U32 layerIndex) const
{
	return m_textureCountList[layerIndex];
}

F32 Atlas::Efficiency(U32 layerIndex) const
{
	return F32(m_textureAreaList[layerIndex]) / (F32)(m_pAtlasList[layerIndex].GetWidth() * m_pAtlasList[layerIndex].GetHeight());
}

std::string Atlas::GetAtlasName(std::string baseName, U32 atlasIndex, U32 layerIndex)
{
	assert(m_atlasNameCallback);
	if (m_atlasNameCallback)
		return m_atlasNameCallback(baseName, atlasIndex, layerIndex);
	else
		return DefaultAtlasName(baseName, atlasIndex, layerIndex);
}

void Atlas::SetAtlasNameCallback(AtlasNameCallback callback)
{
	if (callback)
		m_atlasNameCallback = callback;
	else
		m_atlasNameCallback = &DefaultAtlasName;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void Atlas::OneTimeInit()
{
	m_pAtlasList = 0;
	Clear();
}

void Atlas::ClipSpaces(const Space& newSpace)
{
	Space clipSpace;

	Ut::MultiMap<U32, Space>::Iterator it;
	int iw = newSpace.Width();
	int ih = newSpace.Height();

	Ut::UtVector<Ut::DynamicVector<Space> > clipSpaceList;

	for (it = m_spaces.Begin(); it != m_spaces.End();)
	{
		const Space& space = it->GetValue();

		// check whether the two areas touch at all
		if ((space.X() + space.Width()) <= newSpace.X()) 
		{
			++it;
			continue; 
		}
		if ((space.Y() + space.Height()) <= newSpace.Y())
		{ 
			++it;
			continue;
		}
		if ((newSpace.X() + iw) <= space.X())
		{
			++it;
			continue;
		}
		if ((newSpace.Y() + ih) <= space.Y())
		{ 
			++it;
			continue; 
		}

		// generate free rect to the left of the chart
		if (newSpace.X() > space.X())
		{
			clipSpace.Set(
				space.X(),
				space.Y(),
				newSpace.X() - space.X(),
				space.Height());
			clipSpaceList.PushBack(clipSpace);
		}
		// generate free rect to the right of the chart
		if ((newSpace.X() + iw) < (space.X() + space.Width()))
		{
			clipSpace.Set(
				newSpace.X() + iw,
				space.Y(),
				(space.X() + space.Width()) - (newSpace.X() + iw),
				space.Height());
			clipSpaceList.PushBack(clipSpace);
		}
		// generate free rect above the chart
		if (newSpace.Y() > space.Y())
		{
			clipSpace.Set(
				space.X(),
				space.Y(),
				space.Width(),
				newSpace.Y() - space.Y());
			clipSpaceList.PushBack(clipSpace);
		}
		// generate free rect below the chart
		if ((newSpace.Y() + ih) < (space.Y() + space.Height()))
		{
			clipSpace.Set(
				space.X(),
				newSpace.Y() + ih,
				space.Width(),
				(space.Y() + space.Height()) - (newSpace.Y() + ih));
			clipSpaceList.PushBack(clipSpace);
		}

		Ut::MultiMap<U32, Space>::Iterator itToErase = it;
		++it; 
		m_spaces.Erase(itToErase);
	}

	//Add our clip spaces to our sorted list of free space.
	for (U32 i = 0; i < (U32)clipSpaceList.Size(); i++)
	{
		AddSpace(clipSpaceList[i]);
	}
}

void Atlas::AddSpace(const Space& newSpace)
{
	bool bAdd = true;

	Ut::MultiMap<U32, Space>::Iterator it;
	for (it = m_spaces.Begin(); it != m_spaces.End(); )
	{
		const Space& space = it->GetValue();

		// do the two overlap at all?
		if ((newSpace.X() + newSpace.Width()) <= space.X())	
		{
			++it;
			continue; 
		}
		if ((newSpace.Y() + newSpace.Height()) <= space.Y())	
		{
			++it;
			continue; 
		}
		if ((space.X() + space.Width()) <= newSpace.X())
		{
			++it;
			continue; 
		}
		if ((space.Y() + space.Height()) <= newSpace.Y())	
		{
			++it;
			continue; 
		}

		// Rects are overlapping..
		if (space.X() >= newSpace.X())
		{
			if ((space.X() + space.Width()) > (newSpace.X() + newSpace.Width()))	
			{ 
				++it;
				continue; 
			}
			if (space.Y() < newSpace.Y())
			{ 
				++it;
				continue; 
			}
			if ((space.Y() + space.Height()) > (newSpace.Y() + newSpace.Height()))
			{ 
				++it;
				continue; 
			}
		}
		else
		{
			if ((newSpace.X() + newSpace.Width()) > (space.X() + space.Width()))
			{ 
				++it;
				continue; 
			}
			if (newSpace.Y() < space.Y())
			{ 
				++it;
				continue; 
			}
			if ((newSpace.Y() + newSpace.Height()) > (space.Y() + space.Height()))
			{ 
				++it;
				continue; 
			}
		}

		// Ok.. One overlaps the other..
		if ((space.Width() * space.Height()) < (newSpace.Width() * newSpace.Height()))
		{
			Ut::MultiMap<U32, Space>::Iterator itToErase = it;
			++it; 
			m_spaces.Erase(itToErase);
			bAdd = true;
		}
		else
		{
			++it;
			bAdd = false;
		}
		break;
	}

	// add the new rectangle to our free list
	if (bAdd)
	{
		m_spaces.Insert(Ut::MultiMap<U32, Space>::DataType(-newSpace.Area(), newSpace));
	}
}

#endif
