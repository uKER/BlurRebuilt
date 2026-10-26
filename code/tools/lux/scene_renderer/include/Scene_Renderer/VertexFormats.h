#ifndef VERTEX_FORMATS_H
#define VERTEX_FORMATS_H

namespace SRD
{

struct D3DVertexBasic
{
	void SetPosition(const Position3& position)
	{
		m_x = position[0];
		m_y = position[1];
		m_z = position[2];
	}

	void SetPosition(F32 vx, F32 vy, F32 vz) 
	{ 
		m_x = vx; 
		m_y = vy; 
		m_z = vz; 
	}

	void SetNormal(F32 nx, F32 ny, F32 nz)
	{
		m_nx = nx;
		m_ny = ny;
		m_nz = nz;
	}

	void SetNormal(const Normal3& normal) 
	{ 
		m_nx = normal[0]; 
		m_ny = normal[1];
		m_nz = normal[2];
	}

	void SetColour(const Colour4& colour)
	{ 
		m_colourR = (U8)colour[0]; 
		m_colourG = (U8)colour[1]; 
		m_colourB = (U8)colour[2]; 
		m_colourA = (U8)colour[3];
	}

	void SetColour(U8 r, U8 g, U8 b, U8 a)
	{ 
		m_colourR = r; 
		m_colourG = g; 
		m_colourB = b; 
		m_colourA = a;
	}

	// The untransformed, 3D position for the vertex
    F32 m_x, m_y, m_z;
	F32 m_nx, m_ny, m_nz;
	
    U8 m_colourR;
	U8 m_colourG;
	U8 m_colourB;
	U8 m_colourA;
	
};

struct D3DVertexBasicTex
{
	void SetPosition(const Position3& position)
	{
		m_x = position[0];
		m_y = position[1];
		m_z = position[2];
	}

	void SetPosition(F32 vx, F32 vy, F32 vz) 
	{ 
		m_x = vx; 
		m_y = vy; 
		m_z = vz; 
	}

	void SetNormal(F32 nx, F32 ny, F32 nz)
	{
		m_nx = nx;
		m_ny = ny;
		m_nz = nz;
	}

	void SetNormal(const Normal3& normal) 
	{ 
		m_nx = normal[0]; 
		m_ny = normal[1];
		m_nz = normal[2];
	}

	void SetColour(const Colour4& colour)
	{ 
		m_colourR = (U8)colour[0]; 
		m_colourG = (U8)colour[1]; 
		m_colourB = (U8)colour[2]; 
		m_colourA = (U8)colour[3];
	}

	void SetColour(U8 r, U8 g, U8 b, U8 a)
	{ 
		m_colourR = r; 
		m_colourG = g; 
		m_colourB = b; 
		m_colourA = a;
	}

	void SetTextureCoordinate(U32 set, const Aftk::TextureCoordinate4& uv)
	{ 
		if (set == 0)
		{
			m_u0 = uv[0];
			m_v0 = uv[1];
		}
		if (set == 1)
		{
			m_u1 = uv[0];
			m_v1 = uv[1];
		}
	}

	void SetTextureCoordinate(U32 set, F32 u, F32 v)
	{ 
		if (set == 0)
		{
			m_u0 = u;
			m_v0 = v;
		}
		if (set == 1)
		{
			m_u1 = u;
			m_v1 = v;
		}
	}

	// The untransformed, 3D position for the vertex
    F32 m_x, m_y, m_z;
	F32 m_nx, m_ny, m_nz;
	
	U8 m_colourB;
	U8 m_colourG;
    U8 m_colourR;
	U8 m_colourA;
	
	F32 m_u0, m_v0;
	F32 m_u1, m_v1;
};
/*
DWORD dwFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE |
              D3DFVF_SPECULAR | D3DFVF_TEX2 |
              D3DFVF_TEXCOORDSIZE1(0) |  // Uses 1-D texture coordinates for
                                         // texture coordinate set 1 (index 0).
              D3DFVF_TEXCOORDSIZE2(1);   // And 2-D texture coordinates for 
                                         // texture coordinate set 2 (index 1).
										 */
// Our custom FVF, which describes our custom vertex structure
//#define D3DFVF_SIMPLE (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE)
#define D3DFVF_BASIC (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE)
#define D3DFVF_BASIC_TEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2)
//|D3DFVF_TEXCOORDSIZE1(1)|D3DFVF_TEXCOORDSIZE2(1))
//#define D3DFVF_SIMPLE_TEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

}

#endif
