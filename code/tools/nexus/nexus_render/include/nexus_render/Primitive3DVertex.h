#pragma once

#define D3DFVF_PRIMITIVE3DVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

class Primitive3DVertex
{
public:
    U32	GetFVF() 
	{
		return D3DFVF_PRIMITIVE3DVERTEX;
	}

	void SetPosition(const Maths::HVec3& pos) 
	{ 
		m_x = pos.X();
		m_y = pos.Y();
		m_z = pos.Z(); 
	}
	
	void SetPosition(F32 vx, F32 vy, F32 vz) 
	{
		m_x = vx; 
		m_y = vy;
		m_z = vz; 
	}

    void SetColour(const Maths::HVec4& rgba) 
	{ 
		m_b = (U8)(rgba.X()*255.0f);
		m_g = (U8)(rgba.Y()*255.0f);
		m_r = (U8)(rgba.Z()*255.0f);
		m_a = (U8)(rgba.W()*255.0f);
	}

    void SetColour(U8 r, U8 g, U8 b, U8 a) 
	{ 
		m_b = r;
		m_g = g;
		m_r = b;
		m_a = a;
	}

	void Scale(F32 scale)
	{
		m_x *= scale;
		m_y *= scale;
		m_z *= scale;
	}
/*
    void SetUV(F32 u, F32 v) 
	{
		m_u = u;
		m_v = v;
	}
*/
public:
    F32	m_x, m_y, m_z;      // The untransformed, 3D position for the vertex
    U8	m_r, m_g, m_b, m_a;	// The vertex color
//  F32 m_u, m_v;			// 2D texture coordinates
};
