#include "stdafx.h"

using namespace TexAtlas;

//#define IMAGE_CACHE_VERSION 100 // original version
#define IMAGE_CACHE_VERSION 102 // alpha contrast enhancement

ImageCache::ImageCache()
{
	OneTimeInit();
}

ImageCache::~ImageCache()
{
}

Attk::C_Status ImageCache::Build
(
	const TextureKeyArray& textureSet,
	U32 border, 
	Progress* pProgressCallback
)
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Building image cache...\n");
		if (pProgressCallback->OnProgress(0))
			return Attk::E_Status_Cancelled;
	}

	Clear();
	Attk::C_Status status;
	m_border = border;

	m_records.Resize((U32)textureSet.Size());
	U32 imageIndex = 0;
	for (U32 i = 0; i < (U32)textureSet.Size(); i++)
	{
		const TextureKey& key = textureSet[i];
		//Get a reference to the image cache record for this texture.
		ICRecord& r = m_records[imageIndex];
		r.key = key;
		r.loaded = false;
		if (key.FileName() == CS_GEN_RESERVED_TEXTURE_SPACE_NAME)
		{
			r.missing = false;
			r.lowDate = 0;
			r.highDate = 0;
			r.widthScale = 1.f;
			r.heightScale = 1.f;
		}
		else
		{
			if (!Tu::FileExists(key.FileName().c_str()))
			{
				r.missing = true;
				r.lowDate = 0;
				r.highDate = 0;
				r.widthScale = 0;
				r.heightScale = 0;
			}
			else
			{
				r.missing = false;
				Tu::FileDate(key.FileName().c_str(), r.lowDate, r.highDate);
				r.widthScale = 1.f;
				r.heightScale = 1.f;
			}
		}

		imageIndex++;
		if (pProgressCallback)
			if (pProgressCallback->OnProgress((F32)imageIndex / (F32)m_records.Size()*100.f))
				return Attk::E_Status_Cancelled;
	}
	return Attk::E_Status_Success;
}

Attk::C_Status ImageCache::Update
(
	const TextureKeyArray& textureList, 
	U32 border, 
	Progress* pProgressCallback
)
{
	if (m_border != border)
	{
		//border has changed, rebuild entire cache.
		return Build(textureList, border, pProgressCallback);
	}

	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Updating image cache...\n");
		if (pProgressCallback->OnProgress(0))
			return Attk::E_Status_Cancelled;
	}

	Attk::C_Status status;

			
	//Count how many layers each image is used by. Image slots with a layer usage count of zero will be
	//removed.
	Ut::UtVector<Ut::DynamicVector<U32> > unusedImages;
	{
		U32 workToDo = (U32)m_records.Size();
		U32 workDone = 0;
		for (U32 iImage = 0; iImage < (U32)m_records.Size(); iImage++)
		{
			ICRecord& entry = m_records[iImage];

			S32 textureListKeyIndex = FindTextureKey(textureList, entry.key);
			if (textureListKeyIndex == -1)
			{
				//Cache contains an unused texture - flag for deletion.
				unusedImages.PushBack(iImage);
			}
			else
			{
				const TextureKey& key = textureList[textureListKeyIndex];

				//Get current file date.
				U32 currentLowFileDate;
				U32 currentHighFileDate;
				if (key.FileName() == CS_GEN_RESERVED_TEXTURE_SPACE_NAME)
				{
					entry.loaded = false;
				}
				else
				{
					if (!Tu::FileDate(key.FileName().c_str(), currentLowFileDate, currentHighFileDate))
					{
						entry.missing = true;
					}
					else if ((entry.lowDate != currentLowFileDate) || (entry.highDate != currentHighFileDate))
					{
						//Texture has changed - update file date for entry.
						entry.lowDate = currentLowFileDate;
						entry.highDate = currentHighFileDate;
						//Note that we no longer load the image at this point. Instead we load the image
						//during the process of inserting it in to the atlas. This is because it is only at 
						//that point that we know what procsssing needs to be performed upon the image.
						entry.loaded = false;
					}
				}
			}
			workDone++;
			if (pProgressCallback)
				if (pProgressCallback->OnProgress((((F32)workDone) / (F32)workToDo)*50.f))
					return Attk::E_Status_Cancelled;
		}
	}

	for (U32 iImageIndex = 0; iImageIndex < (U32)unusedImages.Size(); iImageIndex++)
	{
		m_records.EraseByIndex(unusedImages[iImageIndex]-iImageIndex);
	}
	//Add any new textures not present in the cache.
	{
		U32 workDone = 0;
		U32 workToDo = (U32)textureList.Size();
		
		for (U32 iImage = 0; iImage < (U32)textureList.Size(); iImage++)
		{
			S32 cacheRecordIndex = FindICRecord(textureList[iImage]);
			if (cacheRecordIndex == -1)
			{
				//Cache doesn't contain this texture - add new texture.
				m_records.PushBack();
				ICRecord& newEntry = m_records[m_records.Size()-1];

				newEntry.key = textureList[iImage];
				newEntry.loaded = false;
				newEntry.missing = false;

				if (newEntry.key.FileName() == CS_GEN_RESERVED_TEXTURE_SPACE_NAME)
				{
					newEntry.lowDate = 0;
					newEntry.highDate = 0;
				}
				else
				{
					if (!Tu::FileDate(
						newEntry.key.FileName().c_str(), 
						newEntry.lowDate, 
						newEntry.highDate))
					{
						newEntry.missing = true;
					}
				}
			}

			workDone++;
			if (pProgressCallback)
				if (pProgressCallback->OnProgress(50.f + ((F32)workDone / (F32)workToDo)*50.f))
				{
					return Attk::E_Status_Cancelled;
				}
		}
	}

	return Attk::E_Status_Success;
}

Attk::C_Status ImageCache::Read
(
	const std::string& fileName,
	Progress* pProgressCallback
)
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Reading image cache...\n");
		if (pProgressCallback->OnProgress(0))
			return Attk::E_Status_Cancelled;
	}

	Clear();

	Tu::GenericDiskReader reader;
	try
	{
		reader.Open(fileName, TU_GENERIC_DISK_ACCESS_ID('ICDB'));
		if (reader.GetVersion() != IMAGE_CACHE_VERSION)
		{
			//Return an empty cache, we'll purge!
			reader.Close();
			return Attk::E_Status_Success;
		}
		reader.EnterBlock();
		{
			reader >> m_border;
			U32 numEntries;
			reader >> numEntries;
			U32 workToDo = numEntries;
			m_records.Resize(numEntries);
			for (U32 i = 0; i < numEntries; i++)
			{
				ICRecord& entry = m_records[i];
				entry.key.Deserialise(reader);
				reader >> entry.widthScale;
				reader >> entry.heightScale;
				reader >> entry.lowDate;
				reader >> entry.highDate;
				reader >> entry.missing;
				Deserialise(entry.image, &reader);
				//Flag this image as being loaded in to memory.
				if (entry.missing)
					entry.loaded = false;
				else
					entry.loaded = true;
				if (pProgressCallback)
				{
					if (pProgressCallback->OnProgress( ( ((F32)i+1) / (F32)numEntries ) * 100.f))
					{
						reader.Close();
						return Attk::E_Status_Cancelled;
					}
				}
			}
		}

		reader.ExitBlock();
		reader.Close();
	}
	catch (Tu::GenericDiskReader::ReadException e)
	{
		return Attk::E_Status_Failure;
	}

	return Attk::E_Status_Success;
}

Attk::C_Status ImageCache::Write
(
	const std::string& fileName,
	Progress* pProgressCallback
) const
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Writing image cache...\n");
		if (pProgressCallback->OnProgress(0))
			return Attk::E_Status_Cancelled;
	}

	Tu::GenericDiskWriter writer;
	try
	{
		writer.Open(fileName, TU_GENERIC_DISK_ACCESS_ID('ICDB'), IMAGE_CACHE_VERSION);
		writer.BeginBlock(TU_GENERIC_DISK_ACCESS_ID('MAIN'));
		{
			writer << m_border;
			U32 numEntries = (U32)m_records.Size();
			writer << numEntries;
			for (U32 i = 0; i < (U32)m_records.Size(); i++)
			{
				const ICRecord& entry = m_records[i];
				entry.key.Serialise(writer);
				writer << entry.widthScale;
				writer << entry.heightScale;
				writer << entry.lowDate;
				writer << entry.highDate;
				writer << entry.missing;
				Serialise(entry.image, &writer);
				if (pProgressCallback)
					if (pProgressCallback->OnProgress( ( ((F32)i+1) / (F32)numEntries ) * 100.f))
					{
						writer.Close();
						return Attk::E_Status_Cancelled;
					}
			}
		}
		writer.EndBlock();
		writer.Close();
	}
	catch (Tu::GenericDiskWriter::WriteException e)
	{
		return Attk::E_Status_Failure;
	}

	return Attk::E_Status_Success;
}

const ImageCache::ICRecord& ImageCache::Record(U32 index) const
{
	return m_records[index];
}

ImageCache::ICRecord& ImageCache::Record(U32 index)
{
	return m_records[index];
}

S32 ImageCache::RecordIndex(const TextureKey& key) const
{
	for (U32 i = 0; i < (U32)m_records.Size(); i++)
		if (m_records[i].key == key)
			return i;

	return -1;
}

U32 ImageCache::NumRecords() const
{
	return (U32)m_records.Size();
}

U32 ImageCache::Border() const
{
	return m_border;
}

U32 ImageCache::Width(U32 index) const
{
	return m_records[index].image.GetWidth();
}

U32 ImageCache::Height(U32 index) const
{
	return m_records[index].image.GetHeight();
}

void ImageCache::Clear()
{
	m_records.Clear();
	m_border = 0;
}

void ImageCache::OneTimeInit()
{
	Clear();
}

S32 ImageCache::FindICRecord(const TextureKey& key) const
{
	for (U32 i = 0; i < (U32)m_records.Size(); i++)
		if (m_records[i].key == key)
			return i;
	return -1;
}

S32 ImageCache::FindTextureKey(
	const TextureKeyArray& records, 
	const TextureKey& key) const
{
	for (U32 i = 0; i < (U32)records.Size(); i++)
		if (records[i] == key)
			return i;
	return -1;
}


void ImageCache::Serialise(const Attk::C_Image& image, Tu::GenericDiskWriter* pWriter) const
{
	U32 format;
	//Spoof old enum values for the time-being.
	//if (pReader->GetVersion() == 100)
	{
		switch(image.GetPixelFormat())
		{
		case Attk::E_PixelFormat_R8G8B8U: //RGB
			format = 1;
			break;
		case Attk::E_PixelFormat_R8G8B8A8U: //RGBA
			format = 2;
			break;
		
		default:
		case Attk::E_PixelFormat_Unknown: //UNKNOWN
			format = 0;
			break;
		}
	}
	*pWriter << format;
	*pWriter << image.GetWidth();
	*pWriter << image.GetHeight();
	if (image.GetWidth() * image.GetHeight() > 0)
	{
		assert(format != 0);
		pWriter->BlockWrite((void*)image.GetPixelDataPtr(), image.GetPixelDataSize());
	}
}

void ImageCache::Deserialise(Attk::C_Image& image, Tu::GenericDiskReader* pReader)
{
	image.Clear();
	U32 width, height, format;

	*pReader >> format;
	*pReader >> width;
	*pReader >> height;

	//Remap format enum if required
//	if (pReader->GetVersion() == 100)
	{
		switch(format)
		{
		case 1: //RGB
			format = Attk::E_PixelFormat_R8G8B8U;
			break;
		case 2: //RGBA
			format = Attk::E_PixelFormat_R8G8B8A8U;
			break;
		
		default:
		case 0: //UNKNOWN
			format = Attk::E_PixelFormat_Unknown;
			break;
		}
	}

	if (width * height > 0)
	{
		image.NewPixelData(
			width, 
			height,
			1,
			(Attk::E_PixelFormat)format);
		pReader->BlockRead(image.GetPixelDataPtr(), image.GetPixelDataSize());
	}
}