/////////////////////////////////////////////////////////////
// C_Layer
/////////////////////////////////////////////////////////////

#ifndef __LIVERY_LAYER_H__
#define __LIVERY_LAYER_H__

namespace Vehicle
{

namespace Livery
{

/* 
===============================================================
					class C_Layer

NOTES: 
===============================================================
*/
class C_Layer
{
public:

	C_Layer();

	~C_Layer();

	//! Updates debug values
	void UpdateDebugValues();

	//! Renders layer
	void Render(U32 p_Width, U32 p_Height);

public:

	Display::C_Texture2D*		m_pLiveryTexture;

	MathsStorage::Float2		m_Position;
	MathsStorage::Float2		m_Size;

	int							m_Rotation;
	int							m_TextureIndex;

	MathsStorage::Float4		m_Colour;
};

} //namespace Livery

} //namespace Vehicle

#endif
