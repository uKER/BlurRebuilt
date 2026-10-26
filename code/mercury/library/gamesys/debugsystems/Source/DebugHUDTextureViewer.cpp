#include "DebugSystemsPch.h"
#include <DebugSystems/DebugHUDTextureViewer.h>
#include <Render/font/Font.h>
#include <Render/debug/ImmediateDebugRender.h>
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psTextureViewer);
EXTERN_SHADER(vsTextureViewer);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "DebugSystems/Shaders/DebugSystems/PS3/psPrimitive2D.shader"
#include "DebugSystems/Shaders/DebugSystems/PS3/vsPrimitive2D.shader"
#endif
#endif

#define KeyToTex(_key) ((Display2::C_Texture*)_key)

struct C_TexturedVertex2D
{
	F32 px, py;
	U32 c;
	F32 tu, tv;

	void Pos(const F32 x, const F32 y) { px = x; py = y; }
	void Pos(const Maths::HVec2& p) { px = p.X(); py = p.Y(); }
	void Col(const Maths::RGBA col) { c = Display::GPU_RGBA(col); }
	void Tex(const F32 u, const F32 v) { tu = u; tv = v; }
	void Tex(const Maths::HVec2& t) { tu = t.X(); tv = t.Y(); }
};

static Debug::Logger &g_TextureViewerLogger = Debug::GetLogger("Graphics.Debug.TextureViewer");

Debug::Bool g_TextureViewerEnable("Graphics.Debug.TextureViewer.Enable", false );
Debug::Bool g_TextureViewerDumpTextureList("Graphics.Debug.TextureViewer.DumpTextureList", false );
Debug::Int g_TextureViewerIndex("Graphics.Debug.TextureViewer.Options.Index", 0);
Debug::Int g_TextureViewerSizeX("Graphics.Debug.TextureViewer.Options.SizeX", 5, Debug::Range(3, 20) );
Debug::Int g_TextureViewerSizeY("Graphics.Debug.TextureViewer.Options.SizeY", 5, Debug::Range(3, 20) );
Debug::Float g_TextureViewerSpeed("Graphics.Debug.TextureViewer.Options.Speed", 5.f, Debug::Range(0.1f, 30.f) );
Debug::Float g_TextureViewerBorder("Graphics.Debug.TextureViewer.Options.Border", 3.f, Debug::Range(0.0f, 25.f) );
Debug::Float g_TextureViewerFullScreenBorder("Graphics.Debug.TextureViewer.Options.FullScreenBorder", 50.f, Debug::Range(0.0f, 200.f) );
Debug::Float g_TextureInfoStart("Graphics.Debug.TextureViewer.Options.InfoStart", 648.f, Debug::Range(0.0f, 768.f) );
Debug::Colour g_TextureViewerBackground("Graphics.Debug.TextureViewer.Options.Background", 0.f,0.f,0.f,0.5f );
Debug::Float g_TextureViewerOffsetX("Graphics.Debug.TextureViewer.Options.1x1OffsetX", 0.f, Debug::Range(-1024.f, 1024.f) );
Debug::Float g_TextureViewerOffsetY("Graphics.Debug.TextureViewer.Options.1x1OffsetY", 0.f, Debug::Range(-768.f, 768.f) );
Debug::Float g_TextureViewerOffsetSpeed("Graphics.Debug.TextureViewer.Options.1x1OffsetSpeed", 10.f, Debug::Range(1.f, 40.f) );
Debug::Bool g_TextureViewerSortByName("Graphics.Debug.TextureViewer.Options.SortByName", false, Debug::Persist());
Debug::Bool g_TextureViewerSortBySize("Graphics.Debug.TextureViewer.Options.SortBySize", false, Debug::Persist());
Debug::Bool g_TextureViewerLookForName("Graphics.Debug.TextureViewer.Options.LookForName", true, Debug::Persist());
Debug::Bool g_TextureViewerShowOnlyStreamed("Graphics.Debug.TextureViewer.Options.ShowOnlyStreamedTextures", false, Debug::Persist());
Debug::Bool g_TextureViewerShowOnlyNonStreamed("Graphics.Debug.TextureViewer.Options.ShowOnlyNonStreamedTextures", false, Debug::Persist());

const void* DebugSystems::C_DebugHUDTextureViewer::s_pStreamDebugTexture = 0;

DebugSystems::C_DebugHUDTextureViewer::C_DebugHUDTextureViewer(I_ControllerTextureViewer* pController)
{
	m_pFontRenderer = NULL;
	m_pController =  pController;
	m_ViewerState = E_State_Browser;
	m_FullScreenState = E_FullScreen_State_Colour;
	g_TextureViewerIndex = 0;
	m_ZoomPosition = 0.f;
	m_CurrentMipMap = 0;
	m_ColourGammaEnabled = false;
	m_CurrentTextureName = "";
	m_CurrentTexturePointer = 0;
	//m_pStreamDebugTexture = NULL;


	Display::C_VertexFormat vertexFormat2DTex;
	vertexFormat2DTex.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_Position, 0);
	vertexFormat2DTex.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
	vertexFormat2DTex.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0);
	m_pVertexDeclaration2DTex = Display::Device()->CreateVertexDeclaration(vertexFormat2DTex);

	Display::CreatePixelShaderFromFunc("Shaders/psTextureViewer", 0, &m_PixelShader, REF_SHADER(psTextureViewer));
	Display::CreateVertexShaderFromFunc("Shaders/vsTextureViewer", 0, &m_VertexShader, REF_SHADER(vsTextureViewer));

	m_psTextureViewerShaderParams = m_PixelShader.GetParameter(CP_RENDER_SETTINGS, "g_TextureViewerSetup");
}

DebugSystems::C_DebugHUDTextureViewer::~C_DebugHUDTextureViewer()
{
	if (m_pFontRenderer)
		delete m_pFontRenderer;

	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

void DebugSystems::C_DebugHUDTextureViewer::Update( const Maths::C_TimeS& timeStep )
{
	if (g_TextureViewerEnable)
	{
		if( m_pController->Cancel())
		{
			Cancel();
		}
		else	
		if( m_pController->Confirm()  )
		{
			Confirm();
		}
		else
		if( m_pController->MoveUp() )
		{
			Up();
		}
		else
		if( m_pController->MoveDown()  )
		{
			Down();
		}
		else
		if( m_pController->MoveLeft()  )
		{
			Left();
		}
		else
		if( m_pController->MoveRight()  )
		{
			Right();
		}

		//Move
		/*if (m_FullScreenState == E_FullScreen_State_1x1Pixel)
		{
			F32 deltaX, deltaY;
			if (m_pController->PositionMove(deltaX, deltaY))
			{
				g_TextureViewerOffsetX = g_TextureViewerOffsetX + deltaX*g_TextureViewerOffsetSpeed;
				g_TextureViewerOffsetY = g_TextureViewerOffsetY + deltaY*g_TextureViewerOffsetSpeed;
			}
		}*/

		//Animation
		if (m_ViewerState == E_State_Unzoom)
		{
			m_ZoomPosition -= timeStep * g_TextureViewerSpeed;
			if (m_ZoomPosition < 0.f)
			{
				m_ZoomPosition = 0.f;
				m_ViewerState = E_State_Browser;
			}
		}
		else if (m_ViewerState == E_State_Zoom)
		{
			m_ZoomPosition += timeStep * g_TextureViewerSpeed;
			if (m_ZoomPosition > 1.f)
			{
				m_ZoomPosition = 1.f;
				m_ViewerState = E_State_SingleTexture;
				//m_FullScreenState = E_FullScreen_State_Colour;
				m_ColourGammaEnabled = false;
			}
		}
	}
}

int NameSortFunc(const void *A, const void *B)
{
	Display2::C_ResourceListIt& itA = *(Display2::C_ResourceListIt*)A;
	Display2::C_ResourceListIt& itB = *(Display2::C_ResourceListIt*)B;
	
	return stricmp((itA->GetValue()) ? itA->GetValue()->m_Name.Chars() : "", (itB->GetValue()) ? itB->GetValue()->m_Name.Chars() : "");
}

int SizeSortFunc(const void *A, const void *B)
{
	Display2::C_ResourceListIt& itA = *(Display2::C_ResourceListIt*)A;
	Display2::C_ResourceListIt& itB = *(Display2::C_ResourceListIt*)B;

	if (KeyToTex(itA->GetKey())->GetSize() > KeyToTex(itB->GetKey())->GetSize())
		return -1;
	else
		return 1;
}

void DebugSystems::C_DebugHUDTextureViewer::Render()
{
#if TRACK_DISPLAY2_RESOURCES
	if (g_TextureViewerEnable)
	{
		Display2::Device()->LockMutex();

		//Get Immediate Debug Render
		Render::Debug::C_ImmediateDebugRender* pImmediateDebugRender = Render::Debug::C_ImmediateDebugRender::GetInstance();
	
		pImmediateDebugRender->SetBlendMode(DbUtil::E_BlendMode_Alpha);
		pImmediateDebugRender->SetDepthEnable(false);
		//Draw black background
		pImmediateDebugRender->Draw2dBar(Maths::HVec2(0.f,0.f), Maths::HVec2(1024.f, 768.f), Maths::RGBA(g_TextureViewerBackground.Value().r, g_TextureViewerBackground.Value().g, g_TextureViewerBackground.Value().b, g_TextureViewerBackground.Value().a));

		S32 numTextures;

		//Copy to local vector
		m_CachedTextureVector.Resize(Display2::Device()->GetTextureCount());
		{
			U32 i = 0;
			
			if (g_TextureViewerDumpTextureList)
			{
				LOGINFO(g_TextureViewerLogger,"Begin TextureViewer Dump");
			}


			Display2::C_ResourceListIt it = Display2::Device()->GetTextureBegin();
			while (it != Display2::Device()->GetTextureEnd())
			{
				Display2::C_Texture* pTexture = KeyToTex(it->GetKey());
				if (pTexture->IsCreated() &&
					pTexture->GetType() == Display2::E_TextureType_2D
					&& (!g_TextureViewerShowOnlyStreamed || (pTexture->GetFlags() & Display::E_Usage_Streamed))
					&& (!g_TextureViewerShowOnlyNonStreamed || !(pTexture->GetFlags() & Display::E_Usage_Streamed))
					)
				{
					m_CachedTextureVector[i++] = it;
					if (g_TextureViewerDumpTextureList)
					{
						LOGINFO(g_TextureViewerLogger,"%s",  (it->GetValue()) ? it->GetValue()->m_Name.Chars() : "");
					}
				}
				
				++it;
			}
			numTextures = i;
		}

		if (g_TextureViewerDumpTextureList)
		{
			LOGINFO(g_TextureViewerLogger,"End TextureViewer Dump");
		}

		//Sort if needed
		if (g_TextureViewerSortByName)
		{
			qsort(&m_CachedTextureVector[0], numTextures, sizeof(m_CachedTextureVector[0]), NameSortFunc);
		}
		else if (g_TextureViewerSortBySize)
		{
			qsort(&m_CachedTextureVector[0], numTextures, sizeof(m_CachedTextureVector[0]), SizeSortFunc);
		}
		


		if (g_TextureViewerLookForName)
		{
			int nameMatch = -1;
			int nameAndPointerMatch = -1;

			//Look by name first
			if (m_CurrentTextureName != "")
			{
				for (int i = 0; i < numTextures; i++)
				{
					if (m_CachedTextureVector[i]->GetValue() && m_CurrentTextureName == m_CachedTextureVector[i]->GetValue()->m_Name.Chars())
					{
						if (nameMatch == -1)
							nameMatch = i;

						if (m_CurrentTexturePointer == KeyToTex(m_CachedTextureVector[i]->GetKey())->GetGPUTexture())
						{
							nameAndPointerMatch = i;
							break;
						}
					}
				}
			}

			if (nameAndPointerMatch != -1)
				g_TextureViewerIndex = nameAndPointerMatch;
			else if (nameMatch != -1)
				g_TextureViewerIndex = nameMatch;
		}

		if (g_TextureViewerIndex < 0)
			g_TextureViewerIndex = 0;

		if (g_TextureViewerIndex >= numTextures)
			g_TextureViewerIndex = numTextures - 1;

		int maxLevel = KeyToTex(m_CachedTextureVector[g_TextureViewerIndex]->GetKey())->GetLevels();

		if (m_CurrentMipMap >= maxLevel)
			m_CurrentMipMap = maxLevel - 1;

		//Size
		float borderX = g_TextureViewerBorder;
		float borderY = g_TextureViewerBorder *(768.f/1024.f);
		Maths::HVec2 size = Maths::HVec2((1024.f / g_TextureViewerSizeX), (768.f / g_TextureViewerSizeY));
		Maths::HVec2 sizeBorder = Maths::HVec2((1024.f / g_TextureViewerSizeX) - borderX*2.f, (768.f / g_TextureViewerSizeY) - borderY*2.f);

		S32 selectPositionX =  g_TextureViewerSizeX/2;
		S32 selectPositionY =  g_TextureViewerSizeY/2;
		S32 selectPosition =  selectPositionX + selectPositionY * g_TextureViewerSizeX;
		bool first = true;
		Display2::C_ResourceListIt pSelectedTexture2D = Display2::Device()->GetTextureEnd();
		
		if (m_ViewerState == E_State_Browser || m_ViewerState == E_State_Zoom || m_ViewerState == E_State_Unzoom)
		{

			for (S32 y = 0; y < g_TextureViewerSizeY; y++)
			{
				for (S32 x = 0; x < g_TextureViewerSizeX; x++)
				{
					//Current slot position
					S32 position = 	x + y * g_TextureViewerSizeX;
					S32 relativePosition = position - selectPosition;
					S32 textureIndexPosition = g_TextureViewerIndex + relativePosition;

					if ((textureIndexPosition >= 0) && (textureIndexPosition < numTextures) )
					{	
						Display2::C_Texture* pTexture2D = KeyToTex(m_CachedTextureVector[textureIndexPosition]->GetKey());
						
						//Position
						Maths::HVec2 pos = Maths::HVec2(x * (1024.f / g_TextureViewerSizeX), y * (768.f / g_TextureViewerSizeY));
						Maths::HVec2 posBorder = Maths::HVec2(x * (1024.f / g_TextureViewerSizeX) + borderX, y * (768.f / g_TextureViewerSizeY) + borderY);
						
						F32 intensity = 0.6f;
						F32 alpha = (1.f - m_ZoomPosition);
						Maths::HVec2 zoomPosition = posBorder;
						Maths::HVec2 zoomSize = sizeBorder;
						if (selectPosition == position)
						{
							//Calculate Zoom
							zoomPosition = pos + (Maths::HVec2(F32(g_TextureViewerFullScreenBorder),F32(g_TextureViewerFullScreenBorder) * (768.f / 1024.f)) - pos)*m_ZoomPosition;
							zoomSize = size + (Maths::HVec2(1024.f - 2.f * F32(g_TextureViewerFullScreenBorder) ,768.f - 2.f * F32(g_TextureViewerFullScreenBorder) * (768.f / 1024.f)) - size)*m_ZoomPosition;

							//Draw select Background
							pImmediateDebugRender->Draw2dBar(zoomPosition, zoomSize, Maths::RGBA(1.f,1.f,1.f,0.4f * (1.f - m_ZoomPosition)));
							pSelectedTexture2D = m_CachedTextureVector[textureIndexPosition];
							intensity = 1.f;
							alpha = 1.f;

							m_CurrentTextureName = pSelectedTexture2D->GetValue() ? pSelectedTexture2D->GetValue()->m_Name.Chars() : "";
							m_CurrentTexturePointer = KeyToTex(pSelectedTexture2D->GetKey())->GetGPUTexture();
						}
												

						//Draw
						RenderTexture(zoomPosition, zoomSize, intensity, alpha, pTexture2D);
					}
				}
			}
		}
		else if ( m_ViewerState == E_State_SingleTexture)
		{
			pSelectedTexture2D = m_CachedTextureVector[g_TextureViewerIndex];
			m_CurrentTextureName = pSelectedTexture2D->GetValue() ? pSelectedTexture2D->GetValue()->m_Name.Chars() : "";
			m_CurrentTexturePointer = KeyToTex(pSelectedTexture2D->GetKey())->GetGPUTexture();

			//Draw
			RenderTexture(Maths::HVec2(F32(g_TextureViewerFullScreenBorder),F32(g_TextureViewerFullScreenBorder) * (768.f / 1024.f)), Maths::HVec2(1024.f - 2.f * F32(g_TextureViewerFullScreenBorder) ,768.f - 2.f * F32(g_TextureViewerFullScreenBorder) * (768.f / 1024.f)), 1.f, 1.f, KeyToTex(pSelectedTexture2D->GetKey()));

		}
		//Print Selected texture 
		if (pSelectedTexture2D != Display2::Device()->GetTextureEnd())
		{
			//Stream debug texture
			if (KeyToTex(pSelectedTexture2D->GetKey())->GetFlags() & Display::E_Usage_DoNotReload)
			{
				s_pStreamDebugTexture = KeyToTex(pSelectedTexture2D->GetKey())->GetGPUTexture();
			}
			else
			{
				//It is not stream texture
				s_pStreamDebugTexture = NULL;
			}

			if (m_pFontRenderer == NULL)
			{
				//Create the font renderer
				m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
			}
			pImmediateDebugRender->Draw2dBar(Maths::HVec2(0.f, F32(g_TextureInfoStart)), Maths::HVec2(1024.f, 768.f - g_TextureInfoStart), Maths::RGBA(0.f,0.f,0.f,0.6f));
			char string[512];
			F32 pos = g_TextureInfoStart;

			sprintf_s(string, sizeof(string), "FileName: %s",
				pSelectedTexture2D->GetValue() ? pSelectedTexture2D->GetValue()->m_Name.Chars() : "");
			m_pFontRenderer->Print(Maths::HVec2(100.f, pos), string);
			pos += m_pFontRenderer->GetScaleY();

			sprintf_s(string, sizeof(string), "Size: (%i,%i) Mips: %i", KeyToTex(pSelectedTexture2D->GetKey())->GetWidth(), KeyToTex(pSelectedTexture2D->GetKey())->GetHeight(), KeyToTex(pSelectedTexture2D->GetKey())->GetLevels());
			m_pFontRenderer->Print(Maths::HVec2(100.f, pos), string );
			pos += m_pFontRenderer->GetScaleY();

			sprintf_s(string, sizeof(string), "Size: %iKb", KeyToTex(pSelectedTexture2D->GetKey())->GetSize() / 1024 );
			m_pFontRenderer->Print(Maths::HVec2(100.f, pos), string );
			pos += m_pFontRenderer->GetScaleY();

			Util::C_String format = Display2::GetTextureFormatAsString(KeyToTex(pSelectedTexture2D->GetKey())->GetFormat());
			sprintf_s(string, sizeof(string), "Format: %s", format.Chars());
			m_pFontRenderer->Print(Maths::HVec2(100.f, pos), string );
			pos += m_pFontRenderer->GetScaleY();


			//Print Current Mode
			if (m_ViewerState == E_State_Browser)
			{
				strcpy_s(string, sizeof(string), "Browser");
				m_pFontRenderer->Print(Maths::HVec2(924.f, 768.f - m_pFontRenderer->GetScaleY() * 3.f), string, Render::Font::E_Justification_Right );
			}
			else if (m_ViewerState == E_State_SingleTexture)
			{
				switch (m_FullScreenState)
				{
				case E_FullScreen_State_Colour:
					sprintf_s(string, sizeof(string), "Colour (%s)", (m_ColourGammaEnabled)?"No Gamma (SRGB)":"Gamma");

					break;
				case E_FullScreen_State_Alpha:
					strcpy_s(string, sizeof(string), "Alpha");
					break;
				case E_FullScreen_State_1x1Pixel:
					strcpy_s(string, sizeof(string), "1x1 Pixel");
					break;
				case E_FullScreen_State_Mipmaps:
					sprintf_s(string, sizeof(string), "MipMaps %i",	m_CurrentMipMap);
					break;
				case E_FullScreen_State_Off:
					strcpy_s(string, sizeof(string), "Off");
					break;
				default:
					strcpy_s(string, sizeof(string), "???");
					break;
				}
				m_pFontRenderer->Print(Maths::HVec2(924.f, 768.f - m_pFontRenderer->GetScaleY() * 3.f), string, Render::Font::E_Justification_Right );
			}

			//Print number
			sprintf_s(string, sizeof(string), "%d/%d", U32(g_TextureViewerIndex), U32(numTextures));
			m_pFontRenderer->Print(Maths::HVec2(924.f, 768.f - m_pFontRenderer->GetScaleY() * 2.f), string, Render::Font::E_Justification_Right );

		}

		Display2::Device()->UnlockMutex();
	}
#endif
}

void DebugSystems::C_DebugHUDTextureViewer::Up()
{
	if (m_ViewerState == E_State_SingleTexture)
	{
		//Previous full screen mode
		if (m_FullScreenState == E_FullScreen_State_Colour)
			m_FullScreenState = (E_FullScreen_State)(E_FullScreen_State_Total - 1);
		else
			m_FullScreenState =(E_FullScreen_State)( U32(m_FullScreenState) - 1);
	
	}
	else if (m_ViewerState == E_State_Browser)
	{
		g_TextureViewerIndex = g_TextureViewerIndex - g_TextureViewerSizeX;
		if (g_TextureViewerIndex < 0)
			g_TextureViewerIndex = 0;
		ClearTextureNameSelected();
	}
}

void DebugSystems::C_DebugHUDTextureViewer::Down()
{	
	if (m_ViewerState == E_State_SingleTexture)
	{
		//Next full screen mode
		m_FullScreenState =(E_FullScreen_State)( U32(m_FullScreenState) + 1);
		if (m_FullScreenState == E_FullScreen_State_Total)
			m_FullScreenState = E_FullScreen_State_Colour;
	}
	else if (m_ViewerState == E_State_Browser)
	{
		g_TextureViewerIndex = g_TextureViewerIndex +  g_TextureViewerSizeX;
		ClearTextureNameSelected();
	}
}

void DebugSystems::C_DebugHUDTextureViewer::Left()
{
	if (m_FullScreenState == E_FullScreen_State_Mipmaps)
	{
		m_CurrentMipMap--;
		if (m_CurrentMipMap < 0) m_CurrentMipMap = 0;
	}
	if (m_FullScreenState == E_FullScreen_State_Colour && m_ViewerState == E_State_SingleTexture)
	{
		m_ColourGammaEnabled = !m_ColourGammaEnabled;
	}
	else if (m_ViewerState == E_State_Browser)
	{
		g_TextureViewerIndex = g_TextureViewerIndex - 1;
		if (g_TextureViewerIndex < 0)
			g_TextureViewerIndex = 0;

		ClearTextureNameSelected();
	}
}

void DebugSystems::C_DebugHUDTextureViewer::Right()
{
	if (m_FullScreenState == E_FullScreen_State_Mipmaps)
	{
		m_CurrentMipMap++;
	}
	if (m_FullScreenState == E_FullScreen_State_Colour && m_ViewerState == E_State_SingleTexture)
	{
		m_ColourGammaEnabled = !m_ColourGammaEnabled;
	}
	else if (m_ViewerState == E_State_Browser)
	{
		g_TextureViewerIndex = g_TextureViewerIndex + 1;
		ClearTextureNameSelected();
	}
}

void DebugSystems::C_DebugHUDTextureViewer::Confirm()
{
	if (m_ViewerState == E_State_Browser)
	{
		//Switch to zoom
		m_ZoomPosition = 0.f;
		m_ViewerState = E_State_Zoom;
	}
}

void DebugSystems::C_DebugHUDTextureViewer::Cancel()
{
	if (m_ViewerState == E_State_Browser)
	{
		//Exit
		g_TextureViewerEnable = FALSE;
		m_ZoomPosition = 0.f;
		m_ViewerState = E_State_Browser;
	}
	else if (m_ViewerState == E_State_SingleTexture)
	{
		//Return  to Browser
		m_ZoomPosition = 1.f;
		m_ViewerState = E_State_Unzoom;
		//m_FullScreenState = E_FullScreen_State_Colour;
	}
}

void DebugSystems::C_DebugHUDTextureViewer::RenderTexture( Maths::HVec2 PCREF position, Maths::HVec2 PCREF size, F32 intensity, F32 alpha, Display::C_Texture2D* pTexture2D )
{
	//Calculate the position to center the texture in the rect
	F32 screenWidth = F32(Display::Device()->GetBackBuffer().GetWidth());
	F32 screenHeight = F32(Display::Device()->GetBackBuffer().GetHeight());
	F32 aspectRatio = screenWidth / screenHeight;
	//GetTexture Size in aspectratio
	F32 width = F32(pTexture2D->GetWidth()) / (aspectRatio /(4.f/3.f));
	F32 height = F32(pTexture2D->GetHeight());

	//Adjust
	Maths::HVec2 adjustPos;
	Maths::HVec2 adjustSize;

	if ((m_ViewerState == E_State_SingleTexture) && (m_FullScreenState == E_FullScreen_State_1x1Pixel))
	{
		//Calculate 1x1 pixel
		adjustPos = position + Maths::HVec2(F32(g_TextureViewerOffsetX), F32(g_TextureViewerOffsetY));
		adjustSize = Maths::HVec2(pTexture2D->GetWidth() * 1024.f / screenWidth, pTexture2D->GetHeight() * 768.f / screenHeight);
	}
	else
	//Look for the adjust face
	if ((size.X() / width) > (size.Y() / height))
	{
		//Adjust to Y
		adjustSize = Maths::HVec2( size.Y() * width / height ,size.Y());
		adjustPos = Maths::HVec2(position.X() + (size.X() - adjustSize.X())/2.f, position.Y());
	}
	else
	{
		adjustSize = Maths::HVec2( size.X(), size.X() * height / width);
		adjustPos = Maths::HVec2(position.X(), position.Y() + (size.Y() - adjustSize.Y())/2.f);
	}
	Maths::RGBA colour;

	if (m_ViewerState == E_State_SingleTexture && m_FullScreenState == E_FullScreen_State_Alpha)
	{
		colour = Maths::RGBA(0.f,0.f,0.f,1.f);
	}
	else
	{
		colour = Maths::RGBA(intensity,intensity,intensity,alpha);
	}

	//Draw Texture
	if (m_ViewerState != E_State_SingleTexture || m_FullScreenState != E_FullScreen_State_Off)
	{
		//Set States		
		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetDepthEnable(Display::E_Bool_False);

		F32 textureShaderParams[4] = {1.f, 0.f, 0.f, (pTexture2D->GetFlags() & Display::E_Format_SRGB)? 1.f : 0.f };

		if (m_ViewerState == E_State_SingleTexture)
		{
			switch (m_FullScreenState)
			{
			case E_FullScreen_State_Alpha:
				textureShaderParams[0] = 0.f;
				break;
			case E_FullScreen_State_Mipmaps:
				textureShaderParams[0] = 0.f;
				textureShaderParams[1] = 1.f;
				textureShaderParams[2] = F32(m_CurrentMipMap);
				break;
			}
		}

		Display::Device()->SetPixelShaderParameterF(m_psTextureViewerShaderParams, (F32*)&textureShaderParams);

		Display::C_Matrix worldMatrix;
		Display::MatrixSetIdentity(&worldMatrix);
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);

		Display::Device()->SetVertexShader(&m_VertexShader);
		Display::Device()->SetPixelShader(&m_PixelShader);
		Display::Device()->SetPixelShaderParameters(&m_PixelShader);


		//Set Texture
		Display::Device()->SetTexture(0, pTexture2D);
#ifdef PLATFORM_PS3
		if (Display::IsFloatFormat(pTexture2D))
			Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point);
		else
			Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
#else
		Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
#endif

		if ((m_FullScreenState == E_FullScreen_State_Colour) && (m_ViewerState == E_State_SingleTexture))
		{
			Display::Device()->SetTextureGamma(0, Display::E_Bool(m_ColourGammaEnabled));
		}

		//Fill Vertices
		C_TexturedVertex2D* pTextureVertex =  (C_TexturedVertex2D*) Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleStrip, 4, sizeof(C_TexturedVertex2D), m_pVertexDeclaration2DTex);
		if (pTextureVertex)
		{

			pTextureVertex->Pos(Maths::HVec2(adjustPos.X(), adjustPos.Y()));
			pTextureVertex->Col(colour);
			pTextureVertex->Tex(Maths::HVec2(0.f,0.f));
			pTextureVertex++;

			pTextureVertex->Pos(Maths::HVec2(adjustPos.X() + adjustSize.X(), adjustPos.Y()));
			pTextureVertex->Col(colour);
			pTextureVertex->Tex(Maths::HVec2(1.f,0.f));
			pTextureVertex++;

			pTextureVertex->Pos(Maths::HVec2(adjustPos.X(), adjustPos.Y() + adjustSize.Y()));
			pTextureVertex->Col(colour);
			pTextureVertex->Tex(Maths::HVec2(0.f,1.f));
			pTextureVertex++;

			pTextureVertex->Pos(Maths::HVec2(adjustPos.X() + adjustSize.X(), adjustPos.Y() + adjustSize.Y()));
			pTextureVertex->Col(colour);
			pTextureVertex->Tex(Maths::HVec2(1.f,1.f));
			pTextureVertex++;

			Display::Device()->EndVertices(pTextureVertex);
		}

		Display::Device()->SetVertexShader(NULL);
		Display::Device()->SetPixelShader(NULL);
		Display::Device()->SetTexture(0, NULL);
		Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

		if (m_ViewerState == E_State_SingleTexture)
		{
			/*//Draw border in black
			Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(0.f,0.f), Maths::HVec2(1024.f, adjustPos.Y()), Maths::RGBA(0.f,0.f,0.f,m_ZoomPosition * 0.5f));
			Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(0.f,adjustPos.Y() + adjustSize.Y()), Maths::HVec2(1024.f,768.f - (adjustPos.Y() + adjustSize.Y())), Maths::RGBA(0.f,0.f,0.f,m_ZoomPosition * 0.5f));
			Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(0.f,adjustPos.Y()), Maths::HVec2(adjustPos.X(), adjustSize.Y()), Maths::RGBA(0.f,0.f,0.f,m_ZoomPosition * 0.5f));
			Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(adjustPos.X() + adjustSize.X(),adjustPos.Y()), Maths::HVec2(1024.f - (adjustPos.X() + adjustSize.X()), adjustSize.Y()), Maths::RGBA(0.f,0.f,0.f,m_ZoomPosition * 0.5f));*/

			//Draw a border
			Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(adjustPos.X(),adjustPos.Y()), Maths::HVec2(adjustPos.X() + adjustSize.X(),adjustPos.Y()), Maths::RGBA(1.f,1.f,1.f,1.f));
			Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(adjustPos.X(),adjustPos.Y() + adjustSize.Y()), Maths::HVec2(adjustPos.X() + adjustSize.X(),adjustPos.Y() + adjustSize.Y()), Maths::RGBA(1.f,1.f,1.f,1.f));
			Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(adjustPos.X(),adjustPos.Y()), Maths::HVec2(adjustPos.X(),adjustPos.Y() + adjustSize.Y()), Maths::RGBA(1.f,1.f,1.f,1.f));
			Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(adjustPos.X() + adjustSize.X(),adjustPos.Y()), Maths::HVec2(adjustPos.X() + adjustSize.X(),adjustPos.Y() + adjustSize.Y()), Maths::RGBA(1.f,1.f,1.f,1.f));

		}
	}
}

void DebugSystems::C_DebugHUDTextureViewer::ClearTextureNameSelected()
{
	m_CurrentTextureName = "";
	m_CurrentTexturePointer = 0;
}
