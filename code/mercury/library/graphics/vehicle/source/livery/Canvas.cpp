/////////////////////////////////////////////////////////////
// LiveryEditor
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Livery/Canvas.h"
#include "Vehicle/Livery/Debug.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

namespace Livery
{

/* 
===============================================================
					class C_Canvas

NOTES: 
===============================================================
*/

/*
===============
NAME: C_Canvas

NOTE: C_Canvas
===============
*/
C_Canvas::C_Canvas()
{
	m_Width				= 0;
	m_Height			= 0;

	m_CurrentActiveLayer = 0;
}

/*
===============
NAME: ~C_Canvas

NOTE: Destructor
===============
*/
C_Canvas::~C_Canvas()
{

}

/*
===============
NAME: Create

NOTE: Creates texture
===============
*/
bool C_Canvas::Create(E_Canvas p_CanvasIndex, U32 p_Width, U32 p_Height)
{
	m_CanvasIndex			= p_CanvasIndex;

	m_Width					= p_Width;
	m_Height				= p_Height;

	m_CurrentActiveLayer	= 0;
	
	//if (!Display::Device()->CreateTexture2D(m_Width, m_Height, 1, Display::E_Usage_Dynamic, Display::E_Format_A8R8G8B8, &m_Texture, NULL))
	//	return false;
	if (!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, NULL, &m_RenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_CanvasRT");
	#endif
	
	return true;
}

/*
===============
NAME: Destroy

NOTE: Clears out canvas
===============
*/
void C_Canvas::Destroy()
{
	Display::Device()->DestroyRenderTexture2D(&m_RenderTexture);
	
	m_Layers.Clear();
}

/*
===============
NAME: Clear

NOTE: Clears the layers
===============
*/
void C_Canvas::Clear()
{
	m_Layers.Clear();

	m_CurrentActiveLayer = 0;
}

/*
===============
NAME: AddLayer

NOTE: Adds a new layer
===============
*/
void C_Canvas::AddLayer()
{
	//TODO: Insert after the current active layer instead of tacking on the end

	m_Layers.PushBack();
	
	m_CurrentActiveLayer = m_Layers.Size() - 1;
}

/*
===============
NAME: RemoveLayer

NOTE: Removes active layer
===============
*/
void C_Canvas::RemoveLayer()
{
	if (m_CurrentActiveLayer < 0 || m_CurrentActiveLayer >= m_Layers.Size())
		return;

	Ut::Vector<C_Layer> tempLayers;

	for (int iLayer = 0; iLayer < m_Layers.Size(); iLayer++)
	{
		if (iLayer == m_CurrentActiveLayer)
			continue;

		tempLayers.PushBack(m_Layers[iLayer]);
	}

	m_Layers.Clear();

	for (int iLayer = 0; iLayer < tempLayers.Size(); iLayer++)
	{
		m_Layers.PushBack(tempLayers[iLayer]);
	}

	tempLayers.Clear();
	
	if (m_CurrentActiveLayer >= m_Layers.Size())
	{
		m_CurrentActiveLayer = m_Layers.Size() - 1;
		PreviousLayer();
	}	
}

/*
===============
NAME: NextLayer

NOTE: Sets next layer as active
===============
*/
void C_Canvas::NextLayer()
{
	m_CurrentActiveLayer++;

	if (m_CurrentActiveLayer >= m_Layers.Size())
		m_CurrentActiveLayer = m_Layers.Size() - 1;

	if (C_Layer* pLayer = GetActiveLayer())
		pLayer->UpdateDebugValues();
}

/*
===============
NAME: PreviousLayer

NOTE: Sets previous layer as active
===============
*/
void C_Canvas::PreviousLayer()
{
	m_CurrentActiveLayer--;

	if (m_CurrentActiveLayer < 0)
		m_CurrentActiveLayer = 0;

	if (C_Layer* pLayer = GetActiveLayer())
		pLayer->UpdateDebugValues();
}

/*
===============
NAME: UpdateActiveLayer

NOTE: Updates the currently active layer
===============
*/
void C_Canvas::UpdateActiveLayer()
{
	//if (!g_debugAmaxLiveryUpdateCanvas)
	//	return;

	//C_Layer* pLayer = GetActiveLayer();

	//if (!pLayer)
	//{
	//	if (m_Layers.Size() == 0)
	//		return;
	//	else
	//	{
	//		pLayer = &m_Layers[0];
	//		m_CurrentActiveLayer = 0;
	//		pLayer->UpdateDebugValues();
	//	}
	//}

	////Rotate layer
	//pLayer->m_Rotation = g_debugAmaxLiveryLiveryRotationZ;

	////Position layer
	//pLayer->m_Position = MathsStorage::Float2(g_debugAmaxLiveryLiveryPositionX, g_debugAmaxLiveryLiveryPositionY);

	////Resize layer
	//pLayer->m_Size = MathsStorage::Float2(g_debugAmaxLiveryLiverySizeX, g_debugAmaxLiveryLiverySizeY);

	////Set colour
	//pLayer->m_Colour.x = g_debugAmaxLiveryLiveryColour->r;
	//pLayer->m_Colour.y = g_debugAmaxLiveryLiveryColour->g;
	//pLayer->m_Colour.z = g_debugAmaxLiveryLiveryColour->b;
	//pLayer->m_Colour.w = g_debugAmaxLiveryLiveryColour->a;

	////Promote layer
	//bool promoteLayer = g_debugAmaxLiveryPromoteLayer;
	//if (promoteLayer)
	//{
	//	PromoteLayer();
	//	g_debugAmaxLiveryPromoteLayer = false;
	//}

	////Demote layer
	//bool demoteLayer = g_debugAmaxLiveryDemoteLayer;
	//if (demoteLayer)
	//{
	//	DemoteLayer();
	//	g_debugAmaxLiveryDemoteLayer = false;
	//}
}

/*
===============
NAME: SetLayerDefaults

NOTE: Sets default layer values for the canvas
===============
*/
void C_Canvas::SetLayerDefaults()
{
	//C_Layer* pLayer = GetActiveLayer();

	//if (!pLayer)
	//	return;

	//switch (m_CanvasIndex)
	//{
	//case E_Canvas_Left:
	//case E_Canvas_Right:
	//	{
	//		pLayer->m_Position.x	= -0.1f;
	//		pLayer->m_Position.y	= -0.1f;
	//		pLayer->m_Size.x		= -0.3f;
	//		pLayer->m_Size.y		= 1.8f;
	//		pLayer->m_Rotation		= 180;
	//		pLayer->m_TextureIndex	= 37;
	//	}
	//	break;
	//case E_Canvas_Top:
	//	{
	//		pLayer->m_Position.x	= 0.0f;
	//		pLayer->m_Position.y	= -0.7f;
	//		pLayer->m_Size.x		= 1.9f;
	//		pLayer->m_Size.y		= 0.5f;
	//		pLayer->m_Rotation		= 180;
	//		pLayer->m_TextureIndex	= 37;
	//	}
	//	break;
	//case E_Canvas_Front:
	//	{
	//		pLayer->m_Position.x	= 0.0f;
	//		pLayer->m_Position.y	= 0.5f;
	//		pLayer->m_Size.x		= 0.46f;
	//		pLayer->m_Size.y		= 1.0f;
	//		pLayer->m_Rotation		= 180;
	//		pLayer->m_TextureIndex	= 37;
	//	}
	//	break;
	//case E_Canvas_Rear:
	//	{
	//		pLayer->m_Position.x	= 0.0f;
	//		pLayer->m_Position.y	= 0.24f;
	//		pLayer->m_Size.x		= -0.46f;
	//		pLayer->m_Size.y		= 1.0f;
	//		pLayer->m_Rotation		= 180;
	//		pLayer->m_TextureIndex	= 37;
	//	}
	//	break;
	//}
}

/*
===============
NAME: PromoteLayer

NOTE: Promote layer
===============
*/
void C_Canvas::PromoteLayer()
{
	if (!GetActiveLayer())
		return;

	if (m_CurrentActiveLayer == m_Layers.Size() - 1)
		return;

	C_Layer cLayer = m_Layers[m_CurrentActiveLayer];
	C_Layer nLayer = m_Layers[m_CurrentActiveLayer+1];

	m_Layers[m_CurrentActiveLayer+1] = cLayer;
	m_Layers[m_CurrentActiveLayer] = nLayer;

	NextLayer();
}

/*
===============
NAME: DemoteLayer

NOTE: Demote layer
===============
*/
void C_Canvas::DemoteLayer()
{
	if (!GetActiveLayer())
		return;

	if (m_CurrentActiveLayer == 0 || m_Layers.Size() <= 1)
		return;

	C_Layer cLayer = m_Layers[m_CurrentActiveLayer];
	C_Layer pLayer = m_Layers[m_CurrentActiveLayer-1];

	m_Layers[m_CurrentActiveLayer-1] = cLayer;
	m_Layers[m_CurrentActiveLayer] = pLayer;

	PreviousLayer();
}

/*
===============
NAME: GetActiveLayer

NOTE: Returns currently active layer
===============
*/
C_Layer* C_Canvas::GetActiveLayer()
{
	if (m_CurrentActiveLayer < 0 || m_CurrentActiveLayer >= m_Layers.Size())
		return NULL;

	return &m_Layers[m_CurrentActiveLayer];
}

/*
===============
NAME: Render

NOTE: Renders canvas to texture
===============
*/
void C_Canvas::Render(float flashIntensity)
{
	MathsStorage::Float4 activeColour;

	C_Layer *pLayer = GetActiveLayer();

	if (pLayer)
		activeColour = pLayer->m_Colour;

	for (int iLayer = 0; iLayer < m_Layers.Size(); iLayer++)
	{
		MathsStorage::Float4 colour = m_Layers[iLayer].m_Colour;

		if (flashIntensity > 0.0f && iLayer == m_CurrentActiveLayer)
		{
			m_Layers[iLayer].m_Colour.x = 255.0f - activeColour.x;
			m_Layers[iLayer].m_Colour.y = 255.0f - activeColour.y;
			m_Layers[iLayer].m_Colour.z = 255.0f - activeColour.z;
		}

		m_Layers[iLayer].Render(m_Width, m_Height);

		m_Layers[iLayer].m_Colour = colour;
	}

	if (pLayer)
		pLayer->m_Colour = activeColour;
}

} //namespace Livery

} //namespace Vehicle
