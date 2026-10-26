/////////////////////////////////////////////////////////////
// C_Layer
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Livery/Layer.h"
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
					class C_Layer

NOTES: 
===============================================================
*/

/*
===============
NAME: C_Layer

NOTE: Constructor
===============
*/
C_Layer::C_Layer()
{
	m_Position		= MathsStorage::Float2(0.5f, 0.5f);
	m_Size			= MathsStorage::Float2(0.5f, 0.5f);

	m_Rotation		= 0;

	m_TextureIndex	= 0;

	m_Colour		= MathsStorage::Float4(0.0f, 0.0f, 0.0f, 255.0f);
}

/*
===============
NAME: ~C_Layer

NOTE: Destructor
===============
*/
C_Layer::~C_Layer()
{

}

/*
===============
NAME: UpdateDebugValues

NOTE: Updates debug values
===============
*/
void C_Layer::UpdateDebugValues()
{
	g_debugAmaxLiveryLiveryPositionX		= m_Position.x;
	g_debugAmaxLiveryLiveryPositionY		= m_Position.y;
	g_debugAmaxLiveryLiverySizeX			= m_Size.x;
	g_debugAmaxLiveryLiverySizeY			= m_Size.y;
	g_debugAmaxLiveryLiveryRotationZ		= m_Rotation;
	g_debugAmaxLiveryLiveryTextureIndex	= m_TextureIndex;	
	g_debugAmaxLiveryLiveryColour.SetValue(m_Colour.x, m_Colour.y, m_Colour.z, m_Colour.w);
}

/*
===============
NAME: Render

NOTE: Renders layer
===============
*/
void C_Layer::Render(U32 p_Width, U32 p_Height)
{
	//Maths::RGBA colour(m_Colour.x, m_Colour.y, m_Colour.z, m_Colour.w);
	//Maths::RGBA colour(m_Colour.x * 255.0f, m_Colour.y * 255.0f, m_Colour.z * 255.0f, m_Colour.w * 255.0f);
	Maths::RGBA colour((S32)m_Colour.x, (S32)m_Colour.y, (S32)m_Colour.z, (S32)m_Colour.w);
	//float ax = 1024.0f / (float)p_Width;
	//float ay = 768.0f / (float)p_Height;
	p_Width = 1024;
	p_Height = 768;
	float x			= m_Position.x * (float)p_Width;
	float y			= m_Position.y * (float)p_Height;
	float width		= m_Size.x * (float)p_Width * 0.5f;
	float height	= m_Size.y * (float)p_Height * 0.5f;

	float xmin = x - width;
	float xmax = x + width;
	float ymin = y - height;
	float ymax = y + height;
	Display::C_Float2 uvs[3];
	uvs[0].x = 0.0f; uvs[0].y = 1.0f;
	uvs[1].x = 0.0f; uvs[1].y = 0.0f;
	uvs[2].x = 1.0f; uvs[2].y = 0.0f;
	Display::BasicRender()->DrawTextured2DTri(xmin, ymax, xmin, ymin, xmax, ymin, uvs, colour, m_pLiveryTexture, NULL);
	uvs[0].x = 0.0f; uvs[0].y = 1.0f;
	uvs[1].x = 1.0f; uvs[1].y = 0.0f;
	uvs[2].x = 1.0f; uvs[2].y = 1.0f;
	Display::BasicRender()->DrawTextured2DTri(xmin, ymax, xmax, ymin, xmax, ymax, uvs, colour, m_pLiveryTexture, NULL);
	//Display::BasicRender()->DrawTextured2DRect(x, y, width, height, colour, &m_LiveryTexture);
	//Display::BasicRender()->DrawTextured2DRect(0, 0, 384, 256, colour, &m_LiveryTexture);
		
	//D3DXMATRIX rotMat;
	//D3DXMatrixIdentity(&rotMat);

	//
	//float rads = Maths::DegToRad((float)m_Rotation);
	//D3DXMatrixRotationY(&rotMat, rads);
	//D3DXVECTOR4 p0(x - (width/2.0f), y - (height/2.0f), 0.0f, 1.0f);
	//D3DXVECTOR4 p1(x + (width/2.0f), y - (height/2.0f), 0.0f, 1.0f);
	//D3DXVECTOR4 p2(x + (width/2.0f), y + (height/2.0f), 0.0f, 1.0f);
	//D3DXVECTOR4 p3(x - (width/2.0f), y + (height/2.0f), 0.0f, 1.0f);

	//D3D::SetVertexShaderConstantF(CV_CAR_DAMAGE_MAP, (float*)&rotMat, 4);

	//DynamicVB::Begin(D3DPT_TRIANGLEFAN);

	//DynamicVB::SetVertexData2f(D3DVSDE_TEXCOORD0, 0.0f, 0.0f);
	//DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p0.x, p0.y, 1.f, 1.0f);

	//DynamicVB::SetVertexData2f(D3DVSDE_TEXCOORD0, 1.0f, 0.0f);
	//DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p1.x, p1.y, 1.f, 1.0f);

	//DynamicVB::SetVertexData2f(D3DVSDE_TEXCOORD0, 1.0f, 1.0f);
	//DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p2.x, p2.y, 1.f, 1.0f);

	//DynamicVB::SetVertexData2f(D3DVSDE_TEXCOORD0, 0.0f, 1.0f);
	//DynamicVB::SetVertexData4f(D3DVSDE_VERTEX, p3.x, p3.y, 1.f, 1.0f);

	//DynamicVB::End();
}

} //namespace Livery

} //namespace Vehicle
