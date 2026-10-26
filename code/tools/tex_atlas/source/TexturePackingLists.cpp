#include "stdafx.h"
#include <tools_common/FileUtilities.h>
#include <tex_atlas/TexturePackingLists.h>

using namespace TexAtlas;

TexturePackingLists::TexturePackingLists()
{
	OneTimeInit();
}

TexturePackingLists::TexturePackingLists(const TexturePackingLists& rhs)
{
	OneTimeInit();
	*this = rhs;
}

const TexturePackingLists& TexturePackingLists::operator =(const TexturePackingLists& rhs)
{
	m_records = rhs.m_records;
	return *this;
}

TexturePackingLists::~TexturePackingLists()
{
}

Attk::C_Status TexturePackingLists::Build
(
	const TextureKeyArray& textureList,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallbackFunc)(const std::string& text),
	void (*pErrorCallbackFunc)(const std::string& text),
	Progress* pProgressCallback
)
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Reading image headers...\n");
		pProgressCallback->OnProgress(0);
	}

	Attk::C_Status status;
	S32 layerCount = 0;
	S32 baseImageCount = 0;
	for (U32 i = 0; i < (U32)textureList.Size(); i++)
	{
		if (textureList[i].LayerIndex() == CS_GEN_PACKING_LAYER)
		{
			baseImageCount++;
		}
		if (textureList[i].LayerIndex() > layerCount)
			layerCount = textureList[i].LayerIndex();
	}
	layerCount++;
	m_records.Resize(layerCount);
	//Size each layer.
	for (U32 iLayer = 0; iLayer < (U32)layerCount; iLayer++)
	{
		m_records[iLayer].Resize(baseImageCount);
		for (U32 jImage = 0; jImage < (U32)baseImageCount; jImage++)
		{
			//Initialise all records.
			m_records[iLayer][jImage].height = 0;
			m_records[iLayer][jImage].highDate = 0;
			m_records[iLayer][jImage].lowDate = 0;
			m_records[iLayer][jImage].missing = false;
			m_records[iLayer][jImage].used = false;
			m_records[iLayer][jImage].width = 0;
			m_records[iLayer][jImage].originalWidth = 0;
			m_records[iLayer][jImage].originalHeight = 0;
		}
	}

	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Reading image headers...\n");
		pProgressCallback->OnProgress(0);
	}
	for (U32 i = 0; i < (U32)textureList.Size(); i++)
	{
		std::string filenametest = textureList[i].FileName();

		const TextureKey& key = textureList[i];
		assert(key.LayerIndex() != -1);
		assert(key.BaseIndex() != -1);
		Ut::UtVector< Ut::DynamicVector<TexRecord> >& texRecords = 
			m_records[key.LayerIndex()];

		TexRecord& r = texRecords[key.BaseIndex()];
		r.key = key;
		assert(!r.used);
		r.used = true;

		Attk::C_Image tempImage;
		if (textureList[i].FileName() == CS_GEN_RESERVED_TEXTURE_SPACE_NAME)
		{
			r.width = 1;
			r.height = 1;
			r.originalHeight = 1;
			r.originalWidth = 1;
			r.missing = false;
			r.lowDate = 0;
			r.highDate = 0;
		}
		else
		{
			TexAtlas::ImageHeader header;
			status = header.Read(textureList[i].FileName().c_str());
			if (status)
			{
				r.width = header.Width();
				r.height = header.Height();
				r.originalHeight = r.height;
				r.originalWidth = r.width;
				r.missing = false;
				//Get the date stamp for the file.
				Tu::FileDate(r.key.FileName().c_str(), r.lowDate, r.highDate);
			}
			else
			{
				if (pWarningCallbackFunc)
				{
					if (Tu::FileExists(textureList[i].FileName().c_str()))
					{
						std::string warn;
						warn += "CS-Gen does not understand the image type [" + 
							Tu::FileExtension(textureList[i].FileName()) + 
							"] as used by [" +
							textureList[i].FileName() + 
							"]";
						pWarningCallbackFunc(warn);
					}
				}
				r.width = 0;
				r.height = 0;
				r.originalHeight = 0;
				r.originalWidth = 0;
				r.missing = true;
				r.lowDate = 0;
				r.highDate = 0;
			}
		}
		if (pProgressCallback)
		{
			F32 workDone = ((F32)i+1.f) / ((F32)textureList.Size());
			pProgressCallback->OnPrintProgress("Reading image headers...\n");
			pProgressCallback->OnProgress(workDone*100.f);
		}
	}
/*
for (U32 layer = 0; layer < 4; layer++)
{
		Ut::UtVector< Ut::DynamicVector<TexRecord> >& texRecords = 
			m_records[layer];
	for (U32 i = 0; i < texRecords.Size(); i++)
	{
		TexRecord& r = texRecords[i];
		if (!r.used)
			int x = 5;
	}
}
*/
	return Attk::E_Status_Success;
}

const TexRecord& TexturePackingLists::Record(U32 layerIndex, U32 imageIndex) const
{
	return m_records[layerIndex][imageIndex];
}

TexRecord& TexturePackingLists::Record(U32 layerIndex, U32 imageIndex)
{
	return m_records[layerIndex][imageIndex];
}

U32 TexturePackingLists::NumRecords() const
{
	return (U32)m_records[0].Size();
}

U32 TexturePackingLists::NumLayers() const
{
	return (U32)m_records.Size();
}

U32 TexturePackingLists::Width(U32 layerIndex, U32 imageIndex) const
{
	return m_records[layerIndex][imageIndex].width;
}

U32 TexturePackingLists::Height(U32 layerIndex, U32 imageIndex) const
{
	return m_records[layerIndex][imageIndex].height;
}

void TexturePackingLists::Clear()
{
	m_records.Clear();
}

void TexturePackingLists::OneTimeInit()
{
	Clear();
}
