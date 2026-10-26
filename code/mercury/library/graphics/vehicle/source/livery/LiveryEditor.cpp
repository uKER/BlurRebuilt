/////////////////////////////////////////////////////////////
// LiveryEditor
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Livery/LiveryEditor.h"
#include "Vehicle/Livery/Debug.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

namespace Livery
{

#define LIVERY_VERSION 1

/* 
===============================================================
					class C_LiveryEditor

NOTES: 
===============================================================
*/

C_LiveryEditor C_LiveryEditor::s_instance;

/*
===============
NAME: C_LiveryEditor

NOTE: 
===============
*/
C_LiveryEditor::C_LiveryEditor()
{
	m_Initialised = false;

	m_pLiveryResource = NULL;
}

/*
===============
NAME: ~C_LiveryEditor

NOTE: 
===============
*/
C_LiveryEditor::~C_LiveryEditor()
{
	
}

/*
===============
NAME: Initialise

NOTE: Initialise textures etc
===============
*/
void C_LiveryEditor::Initialise()
{
	if (!m_Initialised)
	{
		if (!m_pLiveryResource)
		{
			RESOURCE_MEMORY_ID("Bundles");

			m_BundleName	= "";
			m_pLiveryResource		= new Display::ResourceBundler::C_ResourceBundleSingle();

		#if defined(PLATFORM_PC)
			m_BundleName = "Graphics/Textures/livery_resource.bpr";
		#elif defined(PLATFORM_PS3)
			m_BundleName = "Graphics/Textures/livery_resource.bpr";
		#endif

			if(!m_pLiveryResource->Create(m_BundleName))
			{
				LOGERROR(g_Log, "Failed to open file '%s'\n", m_BundleName.Chars());
			}
		}

		m_Initialised = true;

		m_Canvases[E_Canvas_Front].Create(E_Canvas_Front, 384, 256);
		m_Canvases[E_Canvas_Rear].Create(E_Canvas_Rear, 384, 256);
		m_Canvases[E_Canvas_Left].Create(E_Canvas_Left, 768, 256);
		m_Canvases[E_Canvas_Right].Create(E_Canvas_Right, 768, 256);
		m_Canvases[E_Canvas_Top].Create(E_Canvas_Top, 384, 768);

		m_CurrentCanvas = E_Canvas_None;
	}

	m_Version = LIVERY_VERSION;

	m_ForceLookAt = false;

	m_MenuActive = false;

	m_FlashLayer = 0.0f;
	m_FlashLayerMinTime = 60.0f;
	m_FlashLayerMaxTime = 65.0f;

	m_RenderOnce = true;
}

/*
===============
NAME: Shutdown

NOTE: Kill everything - textures etc
===============
*/
void C_LiveryEditor::Shutdown()
{
	m_Initialised = false;

	for (int iCanvas = 0; iCanvas < m_Canvases.Size(); iCanvas++)
	{
		m_Canvases[iCanvas].Destroy();
	}

	m_MenuActive = false;

	m_pLiveryResource->Destroy();

	SAFE_DELETE_PTR(m_pLiveryResource);
}

/*
===============
NAME: ClearAll

NOTE: Clears all canvases
===============
*/
void C_LiveryEditor::ClearAll()
{
	ClearCanvas(E_Canvas_Front);
	ClearCanvas(E_Canvas_Rear);
	ClearCanvas(E_Canvas_Left);
	ClearCanvas(E_Canvas_Right);
	ClearCanvas(E_Canvas_Top);
}

/*
===============
NAME: ClearCanvas

NOTE: Clears a given canvas
===============
*/
void C_LiveryEditor::ClearCanvas(E_Canvas p_Canvas)
{
	if (p_Canvas <= E_Canvas_None || p_Canvas >= E_Canvas_Max)
		return;

	m_Canvases[p_Canvas].Clear();

	EditCanvas(p_Canvas);

	m_RenderOnce = true;
}

/*
===============
NAME: Update

NOTE: Updates current canvas
===============
*/
void C_LiveryEditor::Update()
{
	if (m_CurrentCanvas >= m_Canvases.Size())
		return;

	SetActiveCanvas(m_CurrentCanvas);

	m_FlashLayer++;

	if (m_FlashLayer > m_FlashLayerMaxTime)
		m_FlashLayer = 0;

	EditCanvas(m_CurrentCanvas);
}

/*
===============
NAME: GetCurrentColour

NOTE: Returns current colour for selected layer
===============
*/
MathsStorage::Float4 C_LiveryEditor::GetCurrentColour()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return MathsStorage::Float4(1.0f, 1.0f, 1.0f, 1.0f);

	C_Layer* pLayer = m_Canvases[m_CurrentCanvas].GetActiveLayer();

	if (!pLayer)
		return MathsStorage::Float4(1.0f, 1.0f, 1.0f, 1.0f);

	return pLayer->m_Colour;
}

/*
===============
NAME: Render

NOTE: Renders current canvas panel
===============
*/
void C_LiveryEditor::Render()
{
	if (m_RenderOnce)
	{
		for (int iCanvas = 0; iCanvas < m_Canvases.Size(); iCanvas++)
		{
			RenderCanvas((E_Canvas)iCanvas);
		}

		m_RenderOnce = false;
	}

	if (m_CurrentCanvas >= m_Canvases.Size())
		return;

	float fFlash = 0.0f;
	fFlash = Maths::Clamp((m_FlashLayer - m_FlashLayerMinTime) / (m_FlashLayerMaxTime - m_FlashLayerMinTime), 0.0f, 1.0f);

	RenderCanvas(m_CurrentCanvas, fFlash);
}

/*
===============
NAME: SetActiveCanvas

NOTE: Sets the active canvas
===============
*/
void C_LiveryEditor::SetActiveCanvas(E_Canvas p_Canvas)
{	
	if (p_Canvas != m_CurrentCanvas)
	{
		if (p_Canvas > E_Canvas_None && p_Canvas < E_Canvas_Max)
		{
			C_Canvas* pCanvas = &m_Canvases[p_Canvas];
			if (pCanvas->GetActiveLayer())
			{
				pCanvas->GetActiveLayer()->UpdateDebugValues();
				pCanvas->UpdateActiveLayer();
			}
		}

		m_FlashLayer = m_FlashLayerMinTime + (m_FlashLayerMaxTime - m_FlashLayerMinTime) * 0.5f;
	}

	m_CurrentCanvas = p_Canvas;
}

/*
===============
NAME: NextLayer

NOTE: Selects next layer
===============
*/
void C_LiveryEditor::NextLayer()
{
	if (!GetActiveCanvas())
		return;

	GetActiveCanvas()->NextLayer();

	m_FlashLayer = m_FlashLayerMinTime + (m_FlashLayerMaxTime - m_FlashLayerMinTime) * 0.5f;
}

/*
===============
NAME: PreviousLayer

NOTE: Selects previous layer
===============
*/
void C_LiveryEditor::PreviousLayer()
{
	if (!GetActiveCanvas())
		return;

	GetActiveCanvas()->PreviousLayer();

	m_FlashLayer = m_FlashLayerMinTime + (m_FlashLayerMaxTime - m_FlashLayerMinTime) * 0.5f;
}

/*
===============
NAME: GetActiveCanvas

NOTE: Returns active canvas
===============
*/
C_Canvas* C_LiveryEditor::GetActiveCanvas()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return NULL;

	return &m_Canvases[m_CurrentCanvas];
}

/*
===============
NAME: CopyLayer

NOTE: Copies current layer
===============
*/
void C_LiveryEditor::CopyLayer()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[m_CurrentCanvas];

	if (pCanvas->GetActiveLayer())
	{
		m_CopiedLayer = *pCanvas->GetActiveLayer();
	}
}

/*
===============
NAME: PasteLayer

NOTE: Pastes layer
===============
*/
void C_LiveryEditor::PasteLayer()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[m_CurrentCanvas];

	pCanvas->AddLayer();

	if (pCanvas->GetActiveLayer())
	{
		*pCanvas->GetActiveLayer() = m_CopiedLayer;
		
		pCanvas->GetActiveLayer()->UpdateDebugValues();
		
		pCanvas->UpdateActiveLayer();
	}
}

/*
===============
NAME: FlipHorizontal

NOTE: Flips active layer horizontally
===============
*/
void C_LiveryEditor::FlipHorizontal()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[m_CurrentCanvas];

	if (pCanvas->GetActiveLayer())
	{
		pCanvas->GetActiveLayer()->m_Size.x *= -1.0f;

		pCanvas->GetActiveLayer()->UpdateDebugValues();
		pCanvas->UpdateActiveLayer();
	}
}

/*
===============
NAME: FlipVertical

NOTE: Flips active layer vertically
===============
*/
void C_LiveryEditor::FlipVertical()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[m_CurrentCanvas];

	if (pCanvas->GetActiveLayer())
	{
		pCanvas->GetActiveLayer()->m_Size.y *= -1.0f;

		pCanvas->GetActiveLayer()->UpdateDebugValues();
		pCanvas->UpdateActiveLayer();
	}
}

/*
===============
NAME: MirrorHorizontal

NOTE: Mirrors active layer horizontally
===============
*/
void C_LiveryEditor::MirrorHorizontal()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[m_CurrentCanvas];

	if (pCanvas->GetActiveLayer())
	{
		pCanvas->GetActiveLayer()->m_Position.x *= -1.0f;
		pCanvas->GetActiveLayer()->m_Size.x *= -1.0f;

		pCanvas->GetActiveLayer()->UpdateDebugValues();
		pCanvas->UpdateActiveLayer();
	}
}

/*
===============
NAME: MirrorVertical

NOTE: Mirrors active layer vertically
===============
*/
void C_LiveryEditor::MirrorVertical()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[m_CurrentCanvas];

	if (pCanvas->GetActiveLayer())
	{
		pCanvas->GetActiveLayer()->m_Position.y *= -1.0f;
		pCanvas->GetActiveLayer()->m_Size.y *= -1.0f;

		pCanvas->GetActiveLayer()->UpdateDebugValues();
		pCanvas->UpdateActiveLayer();
	}
}

/*
===============
NAME: AddLayer

NOTE: Adds a layer to active canvas
===============
*/
void C_LiveryEditor::AddLayer()
{
	if (m_CurrentCanvas <= E_Canvas_None || m_CurrentCanvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[m_CurrentCanvas];

	pCanvas->AddLayer();
	
	C_Layer* pLayer = pCanvas->GetActiveLayer();

	if (pCanvas->m_Layers.Size() == 1)
		pCanvas->SetLayerDefaults();

	if (pLayer)
		pLayer->m_pLiveryTexture = m_pLiveryResource->GetTexture2D(pLayer->m_TextureIndex);
}

/*
===============
NAME: EditCanvas

NOTE: Edits a canvas texture
===============
*/
void C_LiveryEditor::EditCanvas(E_Canvas p_Canvas, bool updateTexture)
{	
	//PixEvent pix(0xffff0000, "C_LiveryEditor::EditCanvas()");

	if (p_Canvas <= E_Canvas_None || p_Canvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[p_Canvas];

	//Add layer, if applicable
	bool addLayer = g_debugAmaxLiveryAddLayer;
	if (addLayer)
	{
		AddLayer();
		g_debugAmaxLiveryAddLayer = false;
		//pCanvas->AddLayer();
		
		//g_debugAmaxLiveryAddLayer = false;
		//C_Layer* pLayer = pCanvas->GetActiveLayer();
		//if (pLayer)
		//	pLayer->m_pLiveryTexture = m_pLiveryResource->GetTexture(0);
	}

	//Remove layer, if applicable
	bool removeLayer = g_debugAmaxLiveryRemoveLayer;
	if (removeLayer)
	{
		pCanvas->RemoveLayer();
		g_debugAmaxLiveryRemoveLayer = false;
	}

	//Next layer
	bool nextLayer = g_debugAmaxLiveryNextLayer;
	if (nextLayer)
	{
		pCanvas->NextLayer();
		g_debugAmaxLiveryNextLayer = false;
	}

	//Previous layer
	bool prevLayer = g_debugAmaxLiveryPrevLayer;
	if (prevLayer)
	{
		pCanvas->PreviousLayer();
		g_debugAmaxLiveryPrevLayer = false;
	}

	//Copy layer
	bool copyLayer = g_debugAmaxLiveryCopyLayer;
	if (copyLayer)
	{
		CopyLayer();
		g_debugAmaxLiveryCopyLayer = false;
	}

	//Paste layer
	bool pasteLayer = g_debugAmaxLiveryPasteLayer;
	if (pasteLayer)
	{
		PasteLayer();
		g_debugAmaxLiveryPasteLayer = false;
	}

	//Flip horiz
	bool flipHoriz = g_debugAmaxLiveryFlipHoriz;
	if (flipHoriz)
	{
		FlipHorizontal();
		g_debugAmaxLiveryFlipHoriz = false;
	}

	//Flip vertical
	bool flipVert = g_debugAmaxLiveryFlipVert;
	if (flipVert)
	{
		FlipVertical();
		g_debugAmaxLiveryFlipVert = false;
	}

	//Mirror horiz
	bool mirrorHoriz = g_debugAmaxLiveryMirrorHoriz;
	if (mirrorHoriz)
	{
		MirrorHorizontal();
		g_debugAmaxLiveryMirrorHoriz = false;
	}

	//Mirror vertical
	bool mirrorVert = g_debugAmaxLiveryMirrorVert;
	if (mirrorVert)
	{
		MirrorVertical();
		g_debugAmaxLiveryMirrorVert = false;
	}

	//Clear layer
	bool clearLayer = g_debugAmaxLiveryClearCanvas;
	if (mirrorVert)
	{
		pCanvas->Clear();
		g_debugAmaxLiveryClearCanvas = false;
	}

	//Check the layer is valid
	if (!pCanvas->GetActiveLayer())
		return;

	//Update texture, if applicable
	//int textureIndex = g_debugAmaxLiveryLiveryTextureIndex;
	C_Layer* pLayer = pCanvas->GetActiveLayer();
	//if (pLayer && (pLayer->m_TextureIndex != textureIndex && m_pLiveryResource) && updateTexture)
	//{
	//	if (textureIndex >= 0 && textureIndex < m_pLiveryResource->ResourceCount())
	//	{
	//		pLayer->m_TextureIndex = textureIndex;

	//		pLayer->m_pLiveryTexture = m_pLiveryResource->GetTexture(pLayer->m_TextureIndex);
	//	}
	//}

	if (pLayer && pLayer->m_TextureIndex >= 0 && pLayer->m_TextureIndex < (S32) m_pLiveryResource->ResourceCount())
		pLayer->m_pLiveryTexture = m_pLiveryResource->GetTexture2D(pLayer->m_TextureIndex);

	//Update it
	pCanvas->UpdateActiveLayer();	
}

/*
===============
NAME: RenderCanvas

NOTE: Renders to a canvas texture
===============
*/
void C_LiveryEditor::RenderCanvas(E_Canvas p_Canvas, float fFlash)
{		
	//PixEvent pix(0xffff0000, "C_LiveryEditor::RenderCanvas()");

	if (p_Canvas <= E_Canvas_None || p_Canvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[p_Canvas];

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = pCanvas->m_Width;
	viewport.m_H = pCanvas->m_Height;

	Display::Device()->SetViewport(viewport);

	Display::Device()->SetColourDepthStencilSurface(&pCanvas->m_RenderTexture.GetSurface(), NULL);
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(255, 255, 255, 255));
	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);

	//Render canvas
	pCanvas->Render(fFlash);


	Display::Device()->SetView(NULL);
}

/*
===============
NAME: DebugDisplay

NOTE: Debug display
===============
*/
void C_LiveryEditor::DebugDisplay()
{
	if (g_debugAmaxLiveryCurrentCanvas >= m_Canvases.Size() || !g_debugDisplayCanvasTexture)
		return;

	Debug_DisplayCanvas(g_debugAmaxLiveryCurrentCanvas);
}

/*
===============
NAME: Debug_DisplayCanvas

NOTE: Displays a canvas
===============
*/
void C_LiveryEditor::Debug_DisplayCanvas(E_Canvas p_Canvas)
{
	if (p_Canvas <= E_Canvas_None || p_Canvas >= E_Canvas_Max)
		return;

	C_Canvas* pCanvas = &m_Canvases[p_Canvas];

	//int x = g_debugAmaxLiveryPosX;
	//int y = g_debugAmaxLiveryPosY;

	//PixEvent pix(0xffff0000, "C_LiveryEditor::Debug_DisplayCanvas()");

	//ColourARGB c(255, 255, 255, 255);

	//DefaultEffect::SetRenderMode(RENDER_MODE_TEXTURE);
	//DefaultEffect::SetBlendMode(BLEND_MODE_ALPHA);
	//DefaultEffect::SetVertexMode(VERTEX_MODE_UISCALE);

	//DefaultEffect::AutoRenderStateChange<D3DRS_ZENABLE, D3DZB_FALSE> autoZ;
	//
	//TextureStateManager::SetTexture(0, pCanvas->m_pTexture);

	//TextureStateManager::SetPixelShaderFlags();

	//Draw::StartQuad2D(x + pCanvas->m_Width/2.0f, y - pCanvas->m_Height/2.0f, 0.f, 1.f, 1.f, 0.f, c);
	//Draw::Vertex2D	 (x - pCanvas->m_Width/2.0f, y - pCanvas->m_Height/2.0f, 0.f, 1.f, 0.f, 0.f);
	//Draw::Vertex2D	 (x - pCanvas->m_Width/2.0f, y + pCanvas->m_Height/2.0f, 0.f, 1.f, 0.f, 1.f);
	//Draw::EndQuad	 (x + pCanvas->m_Width/2.0f, y + pCanvas->m_Height/2.0f, 0.f, 1.f, 1.f, 1.f);
}

/*
===============
NAME: GetTexture

NOTE: Gets texture
===============
*/
//LPDIRECT3DTEXTURE9 C_LiveryEditor::GetTexture(CarClusterCode p_Code)
//{
//	//return m_Canvases[E_Canvas_Top].m_pTexture;
//
//	switch (p_Code)
//	{
//	case LIVERY_FRONT:
//		return m_Canvases[E_Canvas_Front].m_pTexture;
//	case LIVERY_REAR:
//		return m_Canvases[E_Canvas_Rear].m_pTexture;
//	case LIVERY_LEFT:
//		return m_Canvases[E_Canvas_Left].m_pTexture;
//	case LIVERY_RIGHT:
//		return m_Canvases[E_Canvas_Right].m_pTexture;
//	case LIVERY_TOP:
//		return m_Canvases[E_Canvas_Top].m_pTexture;
//	}
//
//	return NULL;
//}

/*
===============
NAME: Save

NOTE: Saves livery
===============
*/
bool C_LiveryEditor::Save(const char* p_pPath, const char* p_pName)
{
	char path[MAX_CHARS];
	_snprintf(path, lengthof(path), "%s%s", p_pPath, p_pName);

	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(path, Sys::E_FileAccess_Write);

	if (!pFile)
		return false;

	pFile->Write(&m_Version, sizeof(int));

	int numCanvases = m_Canvases.Size();

	pFile->Write(&numCanvases, sizeof(int));

	for (int iCanvas = 0; iCanvas < m_Canvases.Size(); iCanvas++)
	{
		C_Canvas* pCanvas = &m_Canvases[iCanvas];

		int numLayers = pCanvas->m_Layers.Size();

		pFile->Write(&numLayers, sizeof(int));

		for (int iLayer = 0; iLayer < pCanvas->m_Layers.Size(); iLayer++)
		{
			C_Layer* pLayer = &pCanvas->m_Layers[iLayer];

			pFile->Write(&pLayer->m_Position.x, sizeof(float));
			pFile->Write(&pLayer->m_Position.y, sizeof(float));

			pFile->Write(&pLayer->m_Size.x, sizeof(float));
			pFile->Write(&pLayer->m_Size.y, sizeof(float));

			pFile->Write(&pLayer->m_Rotation, sizeof(int));

			pFile->Write(&pLayer->m_TextureIndex, sizeof(int));

			pFile->Write(&pLayer->m_Colour.x, sizeof(float));
			pFile->Write(&pLayer->m_Colour.y, sizeof(float));
			pFile->Write(&pLayer->m_Colour.z, sizeof(float));
			pFile->Write(&pLayer->m_Colour.w, sizeof(float));			
		}
	}

	delete pFile;

	return true;
}

/*
===============
NAME: Load

NOTE: Loads livery
===============
*/
bool C_LiveryEditor::Load(const char* p_pPath, const char* p_pName)
{
	char path[MAX_CHARS];
	_snprintf(path, lengthof(path), "%s%s", p_pPath, p_pName);

	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(path, Sys::E_FileAccess_Read);

	if (!pFile)
		return false;

	for (int iCanvas = 0; iCanvas < m_Canvases.Size(); iCanvas++)
		m_Canvases[iCanvas].m_Layers.Clear();

	pFile->Read(&m_Version, sizeof(int));

	int numCanvases = 0;

	pFile->Read(&numCanvases, sizeof(int));

	if (numCanvases != m_Canvases.Size())
	{
		delete pFile;
		return false;
	}

	for (int iCanvas = 0; iCanvas < m_Canvases.Size(); iCanvas++)
	{
		C_Canvas* pCanvas = &m_Canvases[iCanvas];

		int numLayers = 0;
		
		pFile->Read(&numLayers, sizeof(int));

		pCanvas->m_Layers.Resize(numLayers);

		for (int iLayer = 0; iLayer < pCanvas->m_Layers.Size(); iLayer++)
		{
			C_Layer* pLayer = &pCanvas->m_Layers[iLayer];

			pFile->Read(&pLayer->m_Position.x, sizeof(float));
			pFile->Read(&pLayer->m_Position.y, sizeof(float));

			pFile->Read(&pLayer->m_Size.x, sizeof(float));
			pFile->Read(&pLayer->m_Size.y, sizeof(float));

			pFile->Read(&pLayer->m_Rotation, sizeof(int));

			pFile->Read(&pLayer->m_TextureIndex, sizeof(int));

			pLayer->m_pLiveryTexture = m_pLiveryResource->GetTexture2D(pLayer->m_TextureIndex);

			pFile->Read(&pLayer->m_Colour.x, sizeof(float));
			pFile->Read(&pLayer->m_Colour.y, sizeof(float));
			pFile->Read(&pLayer->m_Colour.z, sizeof(float));
			pFile->Read(&pLayer->m_Colour.w, sizeof(float));			
		}

		EditCanvas((E_Canvas)iCanvas);
	}

	m_RenderOnce = true;

	delete pFile;

	return true;
}

} //namespace Livery

} //namespace Vehicle
