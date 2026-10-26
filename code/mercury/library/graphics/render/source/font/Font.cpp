#include "stdafx.h"
#include "Render/Font/Font.h"
#include "Render/Shaders/Constants.h"
#include "System/Serialise/C_FileBlock.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

EXTERN_SHADER(vsFont);
EXTERN_SHADER(psFont);

const float FontGradientStart = 0.5f;

namespace Render
{
namespace Font
{

Debug::Bool		g_debugFontEnabled		("Graphics.Render.Font.Enabled",		true);


C_FontSystem* g_pFontSystem = NULL;


//Font Renderer

C_FontRenderer::C_FontRenderer()
{
	m_pFontResource = NULL;
	
	SetToDefaults();
}

void C_FontRenderer::SetToDefaults()
{
	SetSpaceWidth();
	SetSpacingH();
	SetSpacingV();
	SetScaleX();
	SetScaleY();
	SetFeather();
	SetShadowEnable();
	SetShadowOffset();
	SetColourTop();
	SetColourBottom();
	SetColourOutline();
	SetOutlineSize();
}

Maths::HVec2 C_FontRenderer::GetDimensions( U32 nWrapWidth, const char *text )
{
	Maths::HVec2 currPos(0.0f, 0.0f);
	
	U8* input = (U8*)text;

	U16 nOutputBuffer[512];
	U16* nOutput = nOutputBuffer;
	U16* nLastSpaceOutput = nOutput;
	U16* nLineStart = nOutput;

	F32 fWordLength = 0;

	F32 fLineLength = 0;
	U32 nSpacesCount = 0;

	U16 charcode = (U16)*input++;
	while(charcode)
	{
		*nOutput = charcode;

		switch(charcode)
		{
			case ' ':
			{
				nLastSpaceOutput = nOutput;

				fLineLength += fWordLength + (m_SpaceWidth) * m_ScaleX;
				fWordLength = 0;
				nSpacesCount++;
			}
			break;

			case '\n':
			{
				{
					fLineLength += fWordLength;
					fWordLength = 0;
					*nOutput = 0;
				}

				currPos.X(Maths::Max( (F32)currPos.X(), (F32)fLineLength) );
				currPos.Y(currPos.Y() + (m_SpacingV * m_ScaleY));

				fLineLength = 0;	
				nLineStart = nOutput+1;			

				nSpacesCount = 0;
			}
			break;

			default:
			{
				fWordLength += (m_pFontResource->GetCharWidth(charcode) + m_SpacingH) * m_ScaleX;

				if(((fLineLength + fWordLength) > nWrapWidth) && (nWrapWidth > 0) && (nSpacesCount > 0))
				{
					if(nSpacesCount)
					{
						fLineLength -= (m_SpaceWidth) * m_ScaleX;
						*nLastSpaceOutput = 0;
						nSpacesCount--;
					}

					currPos.X( Maths::Max( (F32)currPos.X(), (F32)fLineLength) );
					currPos.Y(currPos.Y() + (m_SpacingV * m_ScaleY));
					
					nSpacesCount = 0;
					fLineLength = 0;	
					nLineStart = nLastSpaceOutput+1;			
				}
			}
			break;
		}
		
		nOutput++;
		charcode = (U16)*input++;
	}

	*nOutput = 0;
	fLineLength += fWordLength;
	currPos.X( Maths::Max( (F32)currPos.X(), (F32)fLineLength) );
	currPos.Y( currPos.Y() + m_ScaleY );
	
	return currPos;
}

void C_FontRenderer::Print( const Maths::HVec2 &vPos, const char *text, E_Justification justification /*= E_Justification_Left*/, U32 nWrapWidth /*= 0*/ )
{
	using namespace Maths;
	
	if (!g_debugFontEnabled)	
		return;

	assert(m_pFontResource);

	if (m_ShadowEnable)
	{
		//Draw shadow
		RGBA black(0);
		black.A(m_ColourTop.A());
		m_pFontResource->DrawText(vPos + HVec2(m_ShadowOffset * m_ScaleX,m_ShadowOffset * m_ScaleY), justification, nWrapWidth, text, m_SpaceWidth, m_SpacingH, m_SpacingV, m_ScaleX, m_ScaleY, 1.f, 1.f, black, black, Display::E_BlendMode_Alpha );
	}

	//Calculate font pixel size for antialiasing. 30 font size -> feather size pixel -> 0.33f in a 3 pixel gradient
	float fontFeather = (30.f * m_Feather * 0.33f) / Max(m_ScaleX, m_ScaleY);
	fontFeather = Min(fontFeather, Max(FontGradientStart - m_OutlineSize/2.f, 1.f - (FontGradientStart + m_OutlineSize/2.f)));
	float outlineFontFeather = Min(fontFeather*3.f, Max(FontGradientStart - m_OutlineSize/2.f, 1.f - (FontGradientStart + m_OutlineSize/2.f)));

	if (m_OutlineSize > 0.001f)
	{
		//Draw outline
		m_pFontResource->DrawText(vPos, justification, nWrapWidth, text, m_SpaceWidth, m_SpacingH, m_SpacingV, m_ScaleX, m_ScaleY, FontGradientStart - m_OutlineSize/2.f, fontFeather, m_ColourOutline, m_ColourOutline );
	}

	m_pFontResource->DrawText(vPos, justification, nWrapWidth, text, m_SpaceWidth, m_SpacingH, m_SpacingV, m_ScaleX, m_ScaleY, FontGradientStart + m_OutlineSize/2.f, (m_OutlineSize > 0.001f)?outlineFontFeather:fontFeather, m_ColourTop, m_ColourBottom );
}

void C_FontRenderer::Print3D( const Maths::HVec3 &vPos, const char *text, E_Justification justification /*= E_Justification_Left*/, U32 nWrapWidth /*= 0*/ )
{
	const Display::C_ViewFrustum& viewFrustum = Display::Device()->GetViewFrustum();
	const Display::C_ViewSettings2D viewSettings2D = Display::Device()->GetViewSettings2D();
	const Display::C_ViewSettings3D viewSettings3D = Display::Device()->GetViewSettings3D();

	if( viewFrustum.CullSphere(Maths::C_Sphere(vPos, 0.1f)) )	
	{
		const Maths::HVec3 windowPos = viewSettings3D.TransformWorldToWindow(vPos);
		if(windowPos.Z() > 0.0f)
		{
			Maths::C_Rect frame = viewSettings2D.m_ViewExtents;
			if(windowPos.X() < frame.Min().X())
				return;
			if(windowPos.Y() < frame.Min().Y())
				return;

			if(windowPos.X() > frame.Max().X())
				return;
			if(windowPos.Y() > frame.Max().Y())
				return;

			Print(Maths::HVec2(windowPos), text, justification, nWrapWidth);
		}
	}
}

void C_FontRenderer::Print3D( const Maths::HVec3 &vPos, const F32 size, const char *text, E_Justification justification /*= E_Justification_Left*/, U32 nWrapWidth /*= 0*/ )
{
	const Display::C_ViewFrustum& viewFrustum = Display::Device()->GetViewFrustum();
	const Display::C_ViewSettings2D viewSettings2D = Display::Device()->GetViewSettings2D();
	const Display::C_ViewSettings3D viewSettings3D = Display::Device()->GetViewSettings3D();

	Maths::C_Sphere sphere(vPos, size);
	if( viewFrustum.CullSphere(sphere) )	
	{
		const Maths::HVec3 windowPos = viewSettings3D.TransformWorldToWindow(vPos);		
		if(windowPos.Z() > 0.0f)
		{
			Maths::C_Rect frame = viewSettings2D.m_ViewExtents;
			if(windowPos.X() < frame.Min().X())
				return;
			if(windowPos.Y() < frame.Min().Y())
				return;

			if(windowPos.X() > frame.Max().X())
				return;
			if(windowPos.Y() > frame.Max().Y())
				return;
			
			F32 windowSize = viewSettings3D.GetScreenSize(sphere);
			const F32 minSize = 0.01f;
			const F32 maxSize = 256.0f;			
			if(windowSize < minSize)
				windowSize = minSize;
			if(windowSize > maxSize)
				return;
				//windowSize = maxSize;

			//F32 windowScale = windowSize/maxSize;
			//F32 screenSize = windowScale;

			F32 oldScaleX = m_ScaleX;
			F32 oldScaleY = m_ScaleY;

			F32 aspect = oldScaleX / oldScaleY;

			m_ScaleX = windowSize * aspect;
			m_ScaleY = windowSize;

			Print(Maths::HVec2(windowPos), text, justification, nWrapWidth);

			m_ScaleX = oldScaleX;
			m_ScaleY = oldScaleY;
		}
	}
}

//Font Resource
C_FontResource::C_FontResource()
{
	m_pCharInfo = NULL;
	m_pTexturePages = NULL;

	for (U32 i =0 ; i < 65536; i++)
		m_pCharLookup[i] = 0xFFFF;

	m_CharacterCount = 0;
}

C_FontResource::~C_FontResource()
{
	
}

void C_FontResource::Destroy()
{
	if (m_pCharInfo)
	{
		delete [] m_pCharInfo;

		m_pCharInfo = NULL;
	}
	if (m_pTexturePages)
	{
		for (U32 i = 0; i < m_TexturePageCount; i++)
			Display::Device()->DestroyTexture2D(&m_pTexturePages[i]);

		//Destroy textures
		delete [] m_pTexturePages;

		m_pTexturePages = NULL;
	}	
}

bool C_FontResource::Load()
{
	//Open file
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open( m_FontFile, Sys::E_FileAccess_Read );
	
	if(pFile != NULL)
	{
		//Read data from file
		RESOURCE_MEMORY_ID("Font");

		{

			Sys::C_FileBlock rootBlock( pFile );
			while ( rootBlock.GetPosition() < rootBlock.Length() )
			{
				Sys::C_FileBlock block( &rootBlock );
				if(block.HeaderId() == "Header")
				{
					Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
					
					U16 fileMayorVersion;
					U16 fileMinorVersion;
					ar >> fileMayorVersion;
					ar >> fileMinorVersion;

				}
				else
				if(block.HeaderId() == "Data")
				{
					Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );

					// Load Character Information
					ar >> m_CharacterCount;
					if(m_CharacterCount)
					{                        
						m_pCharInfo = new C_CharInfo[m_CharacterCount];

						for(U16 currCharacter = 0; currCharacter < m_CharacterCount; currCharacter++)
						{
							ar >> m_pCharInfo[currCharacter];
						}

						for(U16 i = 0; i < m_CharacterCount; i++)
						{
							m_pCharLookup[m_pCharInfo[i].Unicode()] = i;
						}
					}
				}
				else
				if(block.HeaderId() == "Texture")
				{
					Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
					//Read Texture count
					ar >> m_TexturePageCount;

					m_pTexturePages = new Display::C_Texture2D[m_TexturePageCount];

					//Read Textures
					for (U32 i = 0; i < m_TexturePageCount; i++)
					{
						//Load texture from I_Stream
						Display::LoadTexture2D(&m_pTexturePages[i], &block);

						#if TRACK_DISPLAY2_RESOURCES
						Display2::Device()->SetTextureName(&m_pTexturePages[i], "_FontResource");
						#endif
					}
		
				}
				else
				{
					LOGWARN(g_Log, DBSTR "Unknown File Header\n");
				}
				
			}
		}

		delete pFile;
	}
	else
		return false;
	

	return true;
}

F32 C_FontResource::GetCharWidth(U16 character) const
{
	 if(m_pCharLookup[character] >= m_CharacterCount)
        return 0.0f;
        
    return m_pCharInfo[m_pCharLookup[character]].Width();
}

void C_FontResource::DrawText( const Maths::HVec2 &vPos, E_Justification justification, U32 nWrapWidth, const char *text, F32 spaceWidth, F32 spacingH, F32 spacingV, F32 scaleX, F32 scaleY, F32 startFontBorder, F32 feather, Maths::RGBA colourTop, Maths::RGBA colourBottom, Display::E_BlendMode blendMode )
{
	//Clean memory
	for (int i = 0; i < 256; i++)
		m_FontVertexMemory[i].Clear();
	
	//Pack all the text
	Maths::HVec2 currPos = vPos;

	U8* input = (U8*)text;

	U16 nOutputBuffer[512];
	U16* nOutput = nOutputBuffer;
	U16* nLastSpaceOutput = nOutput;
	U16* nLineStart = nOutput;

	F32 fWordLength = 0;

	F32 fLineLength = 0;
	U32 nSpacesCount = 0;

	U16 charcode = (U16)*input++;
	while(charcode)
	{
		*nOutput = charcode;

		switch(charcode)
		{
			case '\t':
			case ' ':
			{
				nLastSpaceOutput = nOutput;

				fLineLength += fWordLength + (spaceWidth) * scaleX;
				fWordLength = 0;
				nSpacesCount++;
			}
			break;

			case '\n':
			{
				{
					fLineLength += fWordLength;
					fWordLength = 0;
					*nOutput = 0;
				}
				DrawLine(currPos, justification, nWrapWidth, nLineStart, spaceWidth, spacingH, spacingV, scaleX, scaleY, startFontBorder, feather, colourTop, colourBottom, fLineLength, nSpacesCount );
				currPos.Y(currPos.Y() + (spacingV * scaleY));

				fLineLength = 0;	
				nLineStart = nOutput+1;			

				nSpacesCount = 0;
			}
			break;

			default:
			{
				fWordLength += (GetCharWidth(charcode) + spacingH) * scaleX;

				if(((fLineLength + fWordLength) > nWrapWidth) && (nWrapWidth > 0) && (nSpacesCount > 0))
				{
					if(nSpacesCount)
					{
						fLineLength -= (spaceWidth) * scaleX;
						*nLastSpaceOutput = 0;
						nSpacesCount--;
					}
				
					DrawLine(currPos, justification, nWrapWidth, nLineStart, spaceWidth, spacingH, spacingV, scaleX, scaleY, startFontBorder, feather, colourTop, colourBottom, fLineLength, nSpacesCount );
					currPos.Y(currPos.Y() + (spacingV * scaleY));
					
					nSpacesCount = 0;
					fLineLength = 0;	
					nLineStart = nLastSpaceOutput+1;			
				}
			}
			break;
		}
		
		nOutput++;
		charcode = (U16)*input++;
	}

	*nOutput = 0;
	fLineLength += fWordLength;
	DrawLine(currPos, justification, nWrapWidth, nLineStart, spaceWidth, spacingH, spacingV, scaleX, scaleY, startFontBorder, feather, colourTop, colourBottom, fLineLength, nSpacesCount );
	
	
	//Render states
	FontSystem()->SetRenderState(startFontBorder, feather, blendMode );

	//Draw Buffers
	for (int i = 0; i < 256; i++)
	{
		if (m_FontVertexMemory[i].Size() > 0)
		{
			//Set Texture
			Display::Device()->SetTexture(0, &m_pTexturePages[i]);
			Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);

			//Draw buffer
			U8 * pCommandBufferMemory = (U8*)FontSystem()->BeginVertices(Display::E_PrimitiveType_TriangleList,m_FontVertexMemory[i].Size());

			if (pCommandBufferMemory)
			{

				memcpy(pCommandBufferMemory, &m_FontVertexMemory[i][0], m_FontVertexMemory[i].Size() * sizeof(VertexFont));

				FontSystem()->EndVertices(pCommandBufferMemory + m_FontVertexMemory[i].Size() * sizeof(VertexFont));
			}
		}
		
	}
	FontSystem()->UnsetRenderState();

}

void C_FontResource::DrawLine( const Maths::HVec2 &vPos, E_Justification justification, U32 nWrapWidth, const U16 *text, F32 spaceWidth, F32 spacingH, F32 spacingV, F32 scaleX, F32 scaleY, F32 startFontBorder, F32 feather,  Maths::RGBA colourTop, Maths::RGBA colourBottom, F32 fLineLength, S32 nSpacesCount )
{
	F32 currSpaceWidth = spaceWidth * scaleX;

	Maths::HVec2 printPos = vPos;

	switch(justification)
	{
		default:
		case E_Justification_Left:
		break;

		case E_Justification_Centre:
			printPos.X(printPos.X() - fLineLength * 0.5f);
		break;

		case E_Justification_Right:
			printPos.X(vPos.X() - fLineLength);
		break;

		case E_Justification_Justify:
			if(nSpacesCount)
			{
				currSpaceWidth += ((nWrapWidth - fLineLength) / (F32)(nSpacesCount));
			}
		break;
	}

	while(*text)
	{		
		U16 charcode = *text++;
		if(charcode == 32)
		{
			printPos.X(printPos.X() + currSpaceWidth);
		}
		else
		{	
			if (m_pCharLookup[charcode] == 0xFFFF)
				continue;

			C_CharInfo& characterInfo = m_pCharInfo[m_pCharLookup[charcode]];
	
			Maths::HVec2 currPos = printPos;
			currPos.Y(currPos.Y() + (scaleY * characterInfo.Offset()));
			F32 fWidth = scaleX * characterInfo.Width();
			F32 fHeight = scaleY * characterInfo.Height();
			
			U16 texturePageIndex = characterInfo.TexturePage();

			Maths::HVec2 size(fWidth, fHeight);
			Maths::HVec2 A = currPos;
			Maths::HVec2 B = currPos + Maths::HVecX(size);
			Maths::HVec2 C = currPos + Maths::HVecY(size);
			Maths::HVec2 D = currPos + size;

			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());

				vertex.Pos(A);
				vertex.Col(colourTop);
				vertex.UV(characterInfo.Rect().m_UMin, characterInfo.Rect().m_VMin);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}
		    
		#ifdef PLATFORM_PS3
			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(C);
				vertex.Col(colourBottom);
				vertex.UV(characterInfo.Rect().m_UMin, characterInfo.Rect().m_VMax);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}

			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(B);
				vertex.Col(colourTop);
				vertex.UV(characterInfo.Rect().m_UMax, characterInfo.Rect().m_VMin);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}

			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(C);
				vertex.Col(colourBottom);
				vertex.UV(characterInfo.Rect().m_UMin, characterInfo.Rect().m_VMax);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}

			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(B);
				vertex.Col(colourTop);
				vertex.UV(characterInfo.Rect().m_UMax, characterInfo.Rect().m_VMin);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}
		#else
			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(B);
				vertex.Col(colourTop);
				vertex.UV(characterInfo.Rect().m_UMax, characterInfo.Rect().m_VMin);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}

			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(C);
				vertex.Col(colourBottom);
				vertex.UV(characterInfo.Rect().m_UMin, characterInfo.Rect().m_VMax);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}

			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(B);
				vertex.Col(colourTop);
				vertex.UV(characterInfo.Rect().m_UMax, characterInfo.Rect().m_VMin);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}

			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(C);
				vertex.Col(colourBottom);
				vertex.UV(characterInfo.Rect().m_UMin, characterInfo.Rect().m_VMax);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}
		#endif

			{
				m_FontVertexMemory[texturePageIndex].PushBack();
				VertexFont& vertex(m_FontVertexMemory[texturePageIndex].Back());
				vertex.Pos(D);
				vertex.Col(colourBottom);
				vertex.UV(characterInfo.Rect().m_UMax, characterInfo.Rect().m_VMax);
				//m_FontVertexMemory[texturePageIndex].PushBack(vertex);
			}
		
			//Move
			printPos.X(printPos.X() + (GetCharWidth(charcode) + spacingH) * scaleX);
		}
	}
}

//Font System
C_FontSystem::C_FontSystem() : Display::I_Renderer("FontSystem")
{
	
}
C_FontSystem::~C_FontSystem()
{
	
}

bool C_FontSystem::OnCreate()
{
	RESOURCE_MEMORY_ID("Font");

	if(!Display::CreateVertexShaderFromFunc("Shaders/vsFont", 0, &m_VertexShader, REF_SHADER(vsFont)))
		return false;
	if(!Display::CreatePixelShaderFromFunc("Shaders/psFont", 0, &m_PixelShader, REF_SHADER(psFont)))
		return false;
	
	m_psFontShaderParams = m_PixelShader.GetParameter(CP_RENDER_SETTINGS, "g_FontShaderParams");
	
	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_Position,	0);
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_Colour, 0);
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0);
	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(vertexFormat);

	//m_vsClipVector = m_VertexShader.GetParameter(CV_DISPLAY_CLIP_VECTOR, "g_ClipVector");

	return true;
}

void C_FontSystem::OnDestroy()
{
	//Shaders
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);


	//Clean Font Resource Cache
	while (!m_FontResourceCache.Empty())
	{
		C_FontResource& fontResource = *m_FontResourceCache.Begin();
		m_FontResourceCache.Erase(m_FontResourceCache.Begin());

		delete &fontResource;
	}
	
}

C_FontRenderer* C_FontSystem::GetFontRenderer(const char* font)
{
	//Look in cache for the font resource
	C_FontResource* pFontResource = NULL;


	//Create a new resource
	if (pFontResource == NULL)
	{
		Btl::List<C_FontResource>::Iterator fontResourceIt = m_FontResourceCache.Begin();

		while (fontResourceIt != m_FontResourceCache.End())
		{
			C_FontResource* pCurrentFontResource = (C_FontResource*)fontResourceIt.Pointer();
			if (pCurrentFontResource->m_FontFile == font)
			{
				pFontResource = pCurrentFontResource;
				break;
			}
			++fontResourceIt;
		}

	}
	
	if (pFontResource == NULL)
	{
		//Create a new Resource
		pFontResource = new C_FontResource();

		pFontResource->m_FontFile = font;
		if (!pFontResource->Load())  //Check if font resource exists
		{
			delete pFontResource;
			return NULL;
		}

		//Add to the cache list
		m_FontResourceCache.PushBack(*pFontResource);
	}

	//Create a FontRenderer
	C_FontRenderer* pFontRenderer = new C_FontRenderer();

	pFontRenderer->m_pFontResource = pFontResource;
	
	//return
	return pFontRenderer;
}

void* C_FontSystem::BeginVertices(const Display::E_PrimitiveType primitiveType, const U32 vertexCount)
{	
	return Display::Device()->BeginVertices(primitiveType, vertexCount, sizeof(VertexFont), m_pVertexDeclaration, true); // This can fail
}

void C_FontSystem::EndVertices(void* pVertex)
{
	Display::Device()->EndVertices(pVertex);
}

void C_FontSystem::SetRenderState(float startFontBorder, float alphaGradientSize, Display::E_BlendMode blendMode)
{
	Display::Device()->SetCullMode(Display::E_CullMode_None);

	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	
	Display::Device()->SetBlendMode( blendMode );

	Display::C_Float4 psFontParams(alphaGradientSize, startFontBorder + alphaGradientSize/2.f, 0.f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_psFontShaderParams, (F32*)&psFontParams);

	Display::Device()->SetVertexShader(&m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader);

	Display::Device()->SetPixelShaderParameters(&m_PixelShader);

}

void C_FontSystem::UnsetRenderState()
{
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);

	Display::Device()->SetVertexShader(NULL);
	Display::Device()->SetPixelShader(NULL);
}
}; // namespace Font
}; // namespace Render
