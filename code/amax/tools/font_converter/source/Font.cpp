#include "stdafx.h"

#include "Font.h"
#include "system/serialise/TextSerialise.h"
#include "BSPPageNode.h"
#include "texture_export/texture.h"
#include "texture_export/targets/pctex.h"
#include "texture_export/targets/ps3tex.h"
#include "texture_export/targets/xetex.h"
#include "texture_export/converters/atidxt.h"
#include "texture_export/utility.h"
#include "system/perf.h"
namespace Font
{

C_Font::C_Font()
{
	for(U32 currImage = 0; currImage < 256; currImage++)
	{
		m_pSourcePageTexture[currImage] = NULL;
		m_PageIndex[currImage] = 0;
	}
	
	m_BorderSize = 1;

	m_MinU = DEF_TPAGE_SIZE;
	m_MaxU = 0;
	m_MinV = DEF_TPAGE_SIZE;
	m_MaxV = 0;
}

C_Font::~C_Font()
{
	for(U32 currImage = 0; currImage < 256; currImage++)
	{
		SAFE_DELETE_PTR(m_pSourcePageTexture[currImage]);
		m_PageIndex[currImage] = 0;
	}	

	for (S32 i = 0; i < m_pPageTree.Size(); i++)
		SAFE_DELETE_PTR(m_pPageTree[i]);

	for (S32 i = 0; i < m_pNewTexturePage.Size(); i++)
		SAFE_DELETE_PTR(m_pNewTexturePage[i]);
}

void C_CharInfo::AddBorder(U8 borderSize)
{
	m_rectDst.m_UMin += borderSize;
	m_rectDst.m_UMax -= borderSize;
	m_rectDst.m_VMax -= borderSize; 
	m_rectDst.m_VMin += borderSize;
}

void C_CharInfo::CheckExtents(U32 x, U32 y)
{
	if(m_Blank)
	{
		m_Blank = 0;
		m_rectSrc.m_UMin = x;
		m_rectSrc.m_UMax = x + 1;

		m_rectSrc.m_VMin = y;
		m_rectSrc.m_VMax = y + 1;
	}

	// Adjust limits for opaque pixels:
	if(m_rectSrc.m_UMin > x)
		m_rectSrc.m_UMin = x;
	if(m_rectSrc.m_UMax < x + 1)
		m_rectSrc.m_UMax = x + 1;

	if(m_rectSrc.m_VMin > y)
		m_rectSrc.m_VMin = y;
	if(m_rectSrc.m_VMax < y + 1)
		m_rectSrc.m_VMax = y + 1;
}


void C_CharInfo::SizeOutputRect()
{
	m_Output.m_RectOut.m_UMin = m_rectDst.m_UMin / (F32)DEF_TPAGE_SIZE;
	m_Output.m_RectOut.m_VMin = m_rectDst.m_VMin / (F32)DEF_TPAGE_SIZE;
	m_Output.m_RectOut.m_UMax = m_rectDst.m_UMax / (F32)DEF_TPAGE_SIZE;
	m_Output.m_RectOut.m_VMax = m_rectDst.m_VMax / (F32)DEF_TPAGE_SIZE;
}


void C_Font::ReadFontTexture( const Util::C_String& textureFileName )
{
	PERF_EVENT("Read Font Texture");
	U32 filePointPtr = textureFileName.RevSearch('.');
	Util::C_String fileBaseName = textureFileName(0, filePointPtr - 4);

	EXECUTER_INFO( "Base Font Texture Page: %s", fileBaseName.Chars());
	
	//Try to open the 256 max source page textures _XXX.tga
	U32 currentPage = 0;
	for (U32 i = 0; i < 256; i++)
	{
		//Build file name
		Util::C_String pageTextureName;
		pageTextureName.Printf("%s_%3.3i.tga", fileBaseName.Chars(), i);

		if (Sys::C_FileSystem::Instance().FileExists(pageTextureName))
		{
			Attk::C_Image* pImage = new Attk::C_Image();
			pImage->Read(pageTextureName);

			//Process font to implement anti aliasing

			Attk::C_Image* pProcessImage = ProcessImage(pImage);
			delete pImage;


			m_pSourcePageTexture[i] = pProcessImage;
			m_PageIndex[currentPage] = i; 

			currentPage++;
		}
	}
	assert(currentPage != 0);
	EXECUTER_INFO( "Font Texture Page processed: %i", currentPage);

	m_NumSourcePages = currentPage;

}

void C_Font::ReadCharList( const Util::C_String& textureFileName )
{
	PERF_EVENT("Read Char List");
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( textureFileName, Sys::E_FileAccess_Read );
	if(pFile != NULL)
	{
		while( pFile->GetPosition() < pFile->Length() )
		{
			Sys::TextDeserialiser textStream(*pFile);

			Util::C_String input;
			textStream >> input;
			Font::C_CharInfo newChar;
			newChar.m_Output.m_Unicode = atoi(input);
			m_CharInfoList.PushBack(newChar);
		}

		SAFE_DELETE_PTR(pFile);
	}
}

void C_Font::CalculateFontSize()
{
	PERF_EVENT("Calculate Font Sizes");
	//Get Size of the char from the first page texture
	U32 baseCharWidth = m_pSourcePageTexture[m_PageIndex[0]]->GetWidth() / 16;
	U32 baseCharHeight = m_pSourcePageTexture[m_PageIndex[0]]->GetHeight() / 16;

	EXECUTER_INFO( "Font Size: width %d, height %d", baseCharWidth, baseCharHeight);

	U32 currentCharacterIndex = 0;

	for(U16 currentTexturePage = 0; currentTexturePage < m_NumSourcePages; currentTexturePage++)
	{
		Attk::C_Image *currImage = m_pSourcePageTexture[m_PageIndex[currentTexturePage]];
		U32 pageGlyphStart = 0, pageGlyphEnd = 0, baseLoc = 0;

		pageGlyphStart = m_PageIndex[currentTexturePage] * 256;
		pageGlyphEnd = pageGlyphStart + 256;
		
		while((currentCharacterIndex < U16(m_CharInfoList.Size())) && (m_CharInfoList[currentCharacterIndex].m_Output.m_Unicode < pageGlyphEnd) && currImage)
		{
			C_CharInfo& currentCharacter = m_CharInfoList[currentCharacterIndex];

			currentCharacter.m_SourcePageTexture = m_PageIndex[currentTexturePage];

			baseLoc = currentCharacter.m_Output.m_Unicode - pageGlyphStart;
			int Dx = currentCharacter.m_UOffsetSrc = (baseLoc % 16) * baseCharWidth;
			int Dy = currentCharacter.m_VOffsetSrc = (baseLoc / 16) * baseCharHeight;

			for(U32 y = 0; y < baseCharHeight; y++)
			{
				for(U32 x = 0; x < baseCharWidth; x++)
				{
					Attk::C_PixelR32G32B32A32F pixelColour = currImage->GetPixel(Dx + x, Dy + y);
					
					if(pixelColour.alpha != 0.0f) 
					{
						currentCharacter.CheckExtents(x, y);
					}
				}
			}

			if(!currentCharacter.m_Blank)
			{
				//Adjust new sizes

				if(m_MinU > currentCharacter.m_rectSrc.m_UMin)
					m_MinU = currentCharacter.m_rectSrc.m_UMin;
				if(m_MaxU < currentCharacter.m_rectSrc.m_UMax)
					m_MaxU = currentCharacter.m_rectSrc.m_UMax;

				if(m_MinV > currentCharacter.m_rectSrc.m_VMin)
					m_MinV = currentCharacter.m_rectSrc.m_VMin;
				if(m_MaxV < currentCharacter.m_rectSrc.m_VMax)
					m_MaxV = currentCharacter.m_rectSrc.m_VMax;
			}

			currentCharacterIndex++;
		}
	}

	// Setup character scales based upon max extents
	currentCharacterIndex = 0;
	while(currentCharacterIndex < U16(m_CharInfoList.Size()))
	{
		C_CharInfo& currentCharacter = m_CharInfoList[currentCharacterIndex];

		if(!currentCharacter.m_Blank)
		{
			currentCharacter.m_Output.m_Offset = (F32)(currentCharacter.m_rectSrc.m_VMin - m_MinV) / (F32)(m_MaxV - m_MinV);
			currentCharacter.m_Output.m_Width = (F32)(currentCharacter.m_rectSrc.m_UMax - currentCharacter.m_rectSrc.m_UMin) / (F32)(m_MaxU - m_MinU);
			currentCharacter.m_Output.m_Height = (F32)(currentCharacter.m_rectSrc.m_VMax - currentCharacter.m_rectSrc.m_VMin) / (F32)(m_MaxV - m_MinV);
		}
		currentCharacterIndex++;
	}
}

// Add another Texture Page to the List
void C_Font::AddBSPPage()
{
	C_BSPPageNode* pPageNode;

	pPageNode = new C_BSPPageNode();

	pPageNode->m_Rect.m_UMax = DEF_TPAGE_SIZE;
	pPageNode->m_Rect.m_VMax = DEF_TPAGE_SIZE;

	m_pPageTree.PushBack(pPageNode);
}

bool C_Font::ArrangeCharacters()
{
	U8 nReturn = 0;

	// Arrange all the Characters
	U16 currentCharacterIndex = 0;
	while(currentCharacterIndex < U16(m_CharInfoList.Size()))
	{
		C_CharInfo& currentCharacter = m_CharInfoList[currentCharacterIndex];
		
		if (!currentCharacter.m_Blank)
		{
			Rect rectSize = {0,0,0,0};

			// Calculate size of Character plus 1 pixel Border
			rectSize.m_UMax = currentCharacter.m_rectSrc.m_UMax - currentCharacter.m_rectSrc.m_UMin;
			if(rectSize.m_UMax <= 0)
				return false;

			rectSize.m_VMax = currentCharacter.m_rectSrc.m_VMax - currentCharacter.m_rectSrc.m_VMin;
			if(rectSize.m_VMax <= 0)
				return false;

			rectSize.m_UMax += (m_BorderSize * 2);
			rectSize.m_VMax += (m_BorderSize * 2);

			// Try Inserting this Character into the existing Texture Pages
			int pos = 0;
			nReturn = 1;
			for(pos = 0; pos < m_pPageTree.Size(); pos++)
			{
				nReturn = m_pPageTree[pos]->Insert(&currentCharacter.m_rectDst, &rectSize);
				if(nReturn == 0)
					break; // Success!
			}

			if(nReturn)
			{
				// No space in existing Pages, Create additional Texture Page
				AddBSPPage();

				pos = m_pPageTree.Size() - 1;
				if(pos < 0)
					return false;

				// Attempt to Insert Character into the new Texture Page
				nReturn = m_pPageTree[pos]->Insert(&currentCharacter.m_rectDst, &rectSize);
				if(nReturn)
					return false; ///< Failed to insert
			}

			currentCharacter.m_Output.m_TexturePage = pos; ///< Record texture page

			// Add a surrounding border to buffer the character
			currentCharacter.AddBorder(m_BorderSize);
		}	
		currentCharacterIndex++;
	}
    return true;
}

void C_Font::FillNewTexturePages()
{
	PERF_EVENT("Fill new Texture Pages");
	//Allocate new Texture Page
	m_pNewTexturePage.Resize(m_pPageTree.Size());

	//Create and Init new Texture Pages
	for (int i = 0; i < m_pNewTexturePage.Size(); i++)
	{
		m_pNewTexturePage[i] = new Attk::C_Image();
		m_pNewTexturePage[i]->NewPixelData( DEF_TPAGE_SIZE, DEF_TPAGE_SIZE, 1, Attk::E_PixelFormat_B8G8R8A8U );
		m_pNewTexturePage[i]->Fill( Attk::C_PixelR32G32B32A32F(0.0f, 0.0f, 0.0f, 0.0f) );
	}

	//Copy for the Old to the New
	U16 currentCharacterIndex = 0;
	
	while(currentCharacterIndex < U16(m_CharInfoList.Size()))
	{
		C_CharInfo& currentCharacter = m_CharInfoList[currentCharacterIndex];
		if (!currentCharacter.m_Blank)
		{
			Attk::C_Image* pCurrentImage = m_pNewTexturePage[currentCharacter.m_Output.m_TexturePage];
			
			if(pCurrentImage)
			{
				for(U32 y = 0; y < currentCharacter.m_rectDst.m_VMax - currentCharacter.m_rectDst.m_VMin; y++)
				{
					for(U32 x = 0; x < currentCharacter.m_rectDst.m_UMax - currentCharacter.m_rectDst.m_UMin; x++)
					{
						Attk::C_PixelR32G32B32A32F rgba = m_pSourcePageTexture[currentCharacter.m_SourcePageTexture]->GetPixel(currentCharacter.m_UOffsetSrc + x + currentCharacter.m_rectSrc.m_UMin, currentCharacter.m_VOffsetSrc + y + currentCharacter.m_rectSrc.m_VMin);
						pCurrentImage->SetPixel(rgba, currentCharacter.m_rectDst.m_UMin + x, currentCharacter.m_rectDst.m_VMin + y);
					}
				}
			}
		}
		currentCharacterIndex++;
	}

}

void C_Font::Process()
{
	PERF_EVENT("Process");

	//Calculate font size
	CalculateFontSize();
	
	//Arrange Chars
	ArrangeCharacters();

	FillNewTexturePages();

	
	/*for(U32 currOutputPage = 0; currOutputPage < m_pPageTree.Size(); currOutputPage++)
		{
			Util::C_String rawFile;
			rawFile.Printf("c:/Raw%d.tga", currOutputPage);

			m_pNewTexturePage[currOutputPage]->Write(rawFile.Chars());
		}*/
}


void C_Font::Write(Tools::E_Platform platform, Sys::I_File& file)
{
	PERF_EVENT("Write Font");
	{

	Sys::C_FileBlock fileBlock( &file, "Font" );

	{
		Sys::C_FileBlock headerBlock( &fileBlock, "Header" );
		Sys::BinarySerialiserAutoEndian ar( headerBlock );

		ar << U16(E_VersionMajor_Current);
		ar << U16(E_VersionMinor_Current);
	}

	{
		Sys::C_FileBlock dataBlock( &fileBlock, "Data" );
		Sys::BinarySerialiserAutoEndian ar( dataBlock );

		//Output only the not blanks
		U16 numCharInfo = 0;
		for (U32 i = 0; i < U32(m_CharInfoList.Size()); i++)
		{
			if (!m_CharInfoList[i].m_Blank)
				numCharInfo++;
		}
		ar << numCharInfo;

		// Output Character List	
		for (U32 i = 0; i < U32(m_CharInfoList.Size()); i++)
		{
			if (!m_CharInfoList[i].m_Blank)
			{
				m_CharInfoList[i].SizeOutputRect();
				ar << m_CharInfoList[i];
			}
		}
	}
	{
		Sys::C_FileBlock textureBlock( &fileBlock, "Texture" );

		Sys::BinarySerialiserAutoEndian ar( textureBlock );

		ar << U16(m_pNewTexturePage.Size());

		// Output Texture Page Resources
		for(U32 i = 0; i < U32(m_pNewTexturePage.Size()); i++)
		{
			// convert from Attk to export_texture format
			ActTools::Texture *pTexture = TextureExport::TextureFromAttkImage(m_pNewTexturePage[i]);
			
			// convert to DXT5 using ATI library first
			//ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("A8");
			//pTexture->ConvertTo(p);
			
			switch (platform)
			{
			case Tools::E_Platform_360:
				{
					//Convert to DXT5
					NglXeTex texWriter;
					ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat("A8");
					pTexture->ConvertTo(pFormat);
					delete pFormat;

					texWriter.SerialOutTexture(&textureBlock, pTexture);
				}
				break;
			case Tools::E_Platform_PC:
				{
					// convert to DXT5 using ATI library first
					//ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("DXT5");
					//pTexture->ConvertTo(p);

					//Convert to DXT5
					NglPcTex texWriter;
					ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat("A8");
					pTexture->ConvertTo(pFormat);
					delete pFormat;

					texWriter.SerialOutTexture(&textureBlock, pTexture);
				}
				break;
			case Tools::E_Platform_PS3:
				{
					// convert to DXT5 using ATI library first
					//ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("DXT5");
					//pTexture->ConvertTo(p);

					//Convert to DXT5
					NglPs3Tex texWriter;
					ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat("A8");
					pTexture->ConvertTo(pFormat);
					delete pFormat;

					texWriter.SerialOutTexture(&textureBlock, pTexture);
				}
				break;
			}
			

			delete pTexture;
		}
	}
	}
}

Attk::C_Image* C_Font::ProcessImage( Attk::C_Image* pSrcImage )
{
	PERF_EVENT("Blur Image");

	EXECUTER_INFO( "Process Image");

	Attk::C_Image* pImage = new Attk::C_Image(*pSrcImage);

	//Kernel size
	const S32 kernelSize = 3;
	const float kernelRadius = sqrtf(kernelSize * kernelSize * 2.f);
	const float kernelpower = 2.f;

	//Filter, very slow
	for (U32 y = 0 ; y < pImage->GetHeight(); y++)
	for (U32 x = 0 ; x < pImage->GetWidth(); x++)
	{
		Attk::C_PixelR32G32B32A32F pixelColour = pSrcImage->GetPixel(x, y);

		//blur
		float value = 0;
		float range = 0;
		
		for (S32 dy = y - kernelSize; dy <= S32(y + kernelSize); dy++)
		for (S32 dx = x - kernelSize; dx <= S32(x + kernelSize); dx++)
		{
			if ((dx >= 0) && (dx < S32(pImage->GetWidth())) &&
				(dy >= 0) && (dy < S32(pImage->GetHeight())))
			{
				Attk::C_PixelR32G32B32A32F kernelpixelColour = pSrcImage->GetPixel(dx, dy);
				float distantX = float(S32(x) - dx);
				float distantY = float(S32(y) - dy);
				
				float distant = sqrtf(distantX * distantX + distantY * distantY);
				if (distant > kernelRadius)
					distant = kernelRadius;

				distant = (kernelRadius - distant)/kernelRadius;

				distant = powf(distant, kernelpower);

				if (kernelpixelColour.alpha != 0.f)
				{
					value+= distant;
				}
				range += distant;

			}
		}
		value /= range;

		pixelColour.red = pixelColour.green = pixelColour.blue = pixelColour.alpha = value;
		
		pImage->SetPixel(pixelColour,x,y);
	}
	
	//pImage->Write("c:/postprocess.tga");

	return pImage;
}
}