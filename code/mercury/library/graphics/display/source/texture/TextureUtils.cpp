#include "stdafx.h"
#include "Display/Texture/TextureUtils.h"
#include "Display/Texture/Texture2DUtils.h"
#include "Display/Device/Device.h"

#include <System/Serialise.h>
#include <System/Misc/StringUtil.h>
#include <System/Serialise/C_FileBlock.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{

enum E_TextureFileVersion
{
	E_TextureFileVersion_Initial = 0,
	E_TextureFileVersion_Current = E_TextureFileVersion_Initial
};

bool LoadTexture(C_Texture* pTexture, Sys::I_File* pFile, U32& flags)
{
	BIZ_ASSERT(pTexture);
	BIZ_ASSERT(!pTexture->IsCreated());
	BIZ_ASSERT(pFile);

	Sys::C_FileBlock rootBlock(pFile);
	if(rootBlock.HeaderId() != "Texture")
	{
		LOGWARN(g_Log, "Expecting a 'Texture' block");
		return false;
	}

	const U32 prevFileVersion = pFile->GetVersionNumber();
	
	while(rootBlock.GetPosition() < rootBlock.Length())
	{
		Sys::C_FileBlock block(&rootBlock);

		if(block.HeaderId() == "Header")
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(block);

			U32 fileVersion = E_TextureFileVersion_Initial;
			ar >> fileVersion;
			pFile->SetVersionNumber(fileVersion);

			ar >> flags;
		}
		else			
		if(block.HeaderId() == "Data")
		{
			Display::LoadTexture2D(pTexture, &block);
		}
		else
		{
			LOGWARN(g_Log, "Unknown block header '%s'", block.HeaderId().Chars());
		}
	}

	pFile->SetVersionNumber(prevFileVersion);

	return true;
}

void RefreshTextures(const Util::C_String& dir)
{
#if TRACK_DISPLAY2_RESOURCES

	Display2::Device()->LockMutex();

	#if PLATFORM_PS3
	Display::Device()->BlockUntilIdle();
	#endif

	Sys::C_DirectoryInfo dirInfo;
	if(Display::gFileSystem->DirectoryInfo(dir, &dirInfo))
	{
		for(U32 fileIndex=0; fileIndex<dirInfo.NumFiles(); fileIndex++)
		{
			Sys::C_FileInfo& fileInfo = dirInfo.FileInfo(fileIndex);

			if(fileInfo.m_FileType == Sys::E_FileType_Directory)
			{
				RefreshTextures(fileInfo.m_FileName.Chars());
			}

			if(fileInfo.m_FileType == Sys::E_FileType_File)
			{
				Sys::C_FilePath filepathOnDisk = fileInfo.m_FileName;
				Sys::C_FilePath filenameOnDisk = 
					Sys::RemoveFileExtension(Sys::ExtractFilename(filepathOnDisk));
				
				if(filenameOnDisk == "")
					continue;

				Display2::C_ResourceListIt it = Display2::Device()->GetTextureBegin();
				while(it != Display2::Device()->GetTextureEnd())
				{
					Display2::C_Texture* pTexture = (Display2::C_Texture*)it->GetKey();
					Display2::C_ResourceInfo* pResourceInfo = it->GetValue();

					it++;

					if(pTexture->GetFlags() & E_Usage_DoNotReload)
						continue;

					Sys::C_FilePath filenameInGame = Sys::RemoveFileExtension(
						Sys::ExtractFilename(pResourceInfo->m_Name));
					if(filenameInGame == "")
						continue;

					if(filenameInGame != filenameOnDisk)
						continue;

					if(fileInfo.m_FileTime <= pResourceInfo->m_Time)
						continue;

					Display::C_Texture texture;
					
					if(pTexture->GetType() == Display2::E_TextureType_2D)
						Display::CreateTexture2DFromFile(filepathOnDisk.Chars(), &texture);

					if(!texture.IsCreated())
					{
						LOGINFO(g_Log, "Failed to reload texture '%s'\n", filepathOnDisk.Chars());
						continue;
					}

					pTexture->SetFlags(pTexture->GetFlags() & ~Display2::E_Flags_DoNotTrack);

					Display2::Device()->Destroy(pTexture);

					pTexture->SetFlags(pTexture->GetFlags() | Display2::E_Flags_DoNotTrack);

					*pTexture = texture;

					if(pTexture->GetFlags() & Display2::E_Flags_SRGB)
						pTexture->SetSRGB(true);

					pResourceInfo->m_Time = fileInfo.m_FileTime;

					LOGINFO(g_Log, "Successfully reloaded texture '%s'\n", filepathOnDisk.Chars());
				}
			}
		}
	}

	Display2::Device()->UnlockMutex();

#if 0
	Sys::C_DirectoryInfo dirInfo;
	const bool success = Display::gFileSystem->DirectoryInfo(dir, &dirInfo);
	if(success)
	{
		for(U32 fileIndex = 0; fileIndex < dirInfo.NumFiles(); fileIndex++)
		{
			Display2::C_ResourceInfo& fileInfo = dirInfo.FileInfo(fileIndex);

			if(fileInfo.m_FileType == Sys::E_FileType_Directory)
			{
				RefreshTextures(fileInfo.m_FileName.Chars());
			}
			
			if(fileInfo.m_FileType == Sys::E_FileType_File)
			{
				Sys::C_FilePath fileOnDisk = fileInfo.m_FileName;
				Sys::C_FilePath filenameOnDisk = Sys::RemoveFileExtension(Sys::ExtractFilename(fileOnDisk));
				if(filenameOnDisk != "")
				{
					C_Device::SetType<C_Texture2D*>::Iterator it = Device()->m_Texture2Ds.Begin();
					while(it != Device()->m_Texture2Ds.End())
					{
						C_Texture2D* pTexture = *it;
						it++;

						if( (pTexture->GetFlags() & E_Usage_DoNotReload) == 0 )
						{
							Sys::C_FilePath filenameInGame = Sys::RemoveFileExtension(Sys::ExtractFilename(pTexture->GetFileName()));
							if(filenameInGame != "")
							{
								if( filenameInGame == filenameOnDisk )
								{
									if(fileInfo.m_FileTime > pTexture->GetFileTime())
									{
									}
								}
							}
						}
					}
				}
			}
		}
	}
#endif

#endif
}

}; // namespace Display
