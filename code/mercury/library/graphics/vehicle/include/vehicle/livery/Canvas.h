/////////////////////////////////////////////////////////////
// C_Canvas
/////////////////////////////////////////////////////////////

#ifndef __LIVERY_CANVAS_H__
#define __LIVERY_CANVAS_H__

#include "Layer.h"

namespace Vehicle
{

namespace Livery
{

enum E_Canvas
{
	E_Canvas_None = -1,

	E_Canvas_Front,
	E_Canvas_Rear,
	E_Canvas_Left,
	E_Canvas_Right,
	E_Canvas_Top,

	E_Canvas_Max,
};

/* 
===============================================================
					class C_Canvas

NOTES: 
===============================================================
*/
class C_Canvas
{
public:

	C_Canvas();

	~C_Canvas();

	//! Creates texture
	bool Create(E_Canvas p_CanvasIndex, U32 p_Width, U32 p_Height);

	//! Clears out canvas
	void Destroy();

	//! Clears the layers
	void Clear();

	//! Adds a new layer
	void AddLayer();

	//! Removes active layer
	void RemoveLayer();

	//! Sets previous layer as active
	void PreviousLayer();

	//! Sets next layer as active
	void NextLayer();

	//! Updates the currently active layer
	void UpdateActiveLayer();

	//! Sets default layer values for the canvas
	void SetLayerDefaults();

	//! Promote layer
	void PromoteLayer();

	//! Demote layer
	void DemoteLayer();

	//! Returns currently active layer
	C_Layer* GetActiveLayer();

	//! Renders canvas to texture
	void Render(float flashIntensity = 0.0f);

public:
	
	E_Canvas					m_CanvasIndex;

	Display::C_RenderTexture2D	m_RenderTexture;

	U32							m_Width;
	U32							m_Height;

	int							m_CurrentActiveLayer;

	Ut::Vector<C_Layer>			m_Layers;
};

} //namespace Livery

} //namespace Vehicle

#endif



