#pragma once

namespace NexusRender
{
	struct D3DVertexPos
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		// The untransformed, 3D position for the vertex
		F32 m_x, m_y, m_z;
	};

	struct D3DVertexNorm
	{
		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		// The untransformed, 3D position for the vertex
		F32 m_nx, m_ny, m_nz;
	};

	struct D3DVertexCol
	{
		void SetColour(const Maths::HVec4& colour)
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

		void Colour(U8& r, U8& g, U8& b, U8 a)
		{
			r = m_colourR;
			g = m_colourG;
			b = m_colourB;
			a = m_colourA;
		}

		U8 m_colourB;
		U8 m_colourG;
		U8 m_colourR;
		U8 m_colourA;
	};

	struct D3DVertexPosCol
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetColour(const Maths::HVec4& colour)
		{ 
			m_colourR = (U8)(colour[0]*255.f);
			m_colourG = (U8)(colour[1]*255.f);
			m_colourB = (U8)(colour[2]*255.f); 
			m_colourA = (U8)(colour[3]*255.f);
		}

		void SetColour(U8 r, U8 g, U8 b, U8 a)
		{ 
			m_colourR = r; 
			m_colourG = g; 
			m_colourB = b; 
			m_colourA = a;
		}

		void Colour(U8& r, U8& g, U8& b, U8 a)
		{
			r = m_colourR;
			g = m_colourG;
			b = m_colourB;
			a = m_colourA;
		}

		// The untransformed, 3D position for the vertex
		F32 m_x, m_y, m_z;
		
		U8 m_colourB;
		U8 m_colourG;
		U8 m_colourR;
		U8 m_colourA;
	};

	struct D3DVertexTex1
	{
		void SetTextureCoordinate(const Aftk::TextureCoordinate4& uv)
		{ 
			m_u0 = uv[0];
			m_v0 = uv[1];
		}

		void SetTextureCoordinate(F32 u, F32 v)
		{ 
			m_u0 = u;
			m_v0 = v;
		}

		F32 m_u0, m_v0;
	};

	struct D3DVertexTex2
	{
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
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;
			case 1:
				m_u1 = u;
				m_v1 = v;
				break;
			default:
				break;
			}
		}

		F32 m_u0, m_v0;
		F32 m_u1, m_v1;
	};

	struct D3DVertexPosNormTex1
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		void SetColour(const Maths::HVec4& colour)
		{ 
		}

		void SetColour(U8 r, U8 g, U8 b, U8 a)
		{ 
		}

		void SetTextureCoordinate(U32 set, const Aftk::TextureCoordinate4& uv)
		{ 
			if (set == 0)
			{
				m_u0 = uv[0];
				m_v0 = uv[1];
			}
		}

		void SetTextureCoordinate(U32 set, F32 u, F32 v)
		{ 
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;
			default:
				break;
			}
		}

		void SetWeight(U32 index, F32 weight) {}

		void SetPaletteIndex(U32 instance, U8 index) {}

		F32 m_x, m_y, m_z;
		F32 m_nx, m_ny, m_nz;
		F32 m_u0, m_v0;
	};

	struct D3DVertexPosNormColTex1
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		void SetColour(const Maths::HVec4& colour)
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

		void Colour(U8& r, U8& g, U8& b, U8 a)
		{
			r = m_colourR;
			g = m_colourG;
			b = m_colourB;
			a = m_colourA;
		}

		void SetTextureCoordinate(U32 set, const Aftk::TextureCoordinate4& uv)
		{ 
			if (set == 0)
			{
				m_u0 = uv[0];
				m_v0 = uv[1];
			}
		}

		void SetTextureCoordinate(U32 set, F32 u, F32 v)
		{ 
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;
			default:
				break;
			}
		}

		void SetWeight(U32 index, F32 weight) {}

		void SetPaletteIndex(U32 instance, U8 index) {}


			// The untransformed, 3D position for the vertex
		F32 m_x, m_y, m_z;
		F32 m_nx, m_ny, m_nz;
		
		U8 m_colourB;
		U8 m_colourG;
		U8 m_colourR;
		U8 m_colourA;
		
		F32 m_u0, m_v0;
	};


	struct D3DVertexPosNormColTex2
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		void SetColour(const Maths::HVec4& colour)
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

		void Colour(U8& r, U8& g, U8& b, U8 a)
		{
			r = m_colourR;
			g = m_colourG;
			b = m_colourB;
			a = m_colourA;
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
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;
			case 1:
				m_u1 = u;
				m_v1 = v;
				break;
			default:
				break;
			}
		}

		void SetWeight(U32 index, F32 weight) {}

		void SetPaletteIndex(U32 instance, U8 index) {}


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

	struct D3DVertexPosNormTex2
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		void SetColour(const Maths::HVec4& colour)
		{ 
		}

		void SetColour(U8 r, U8 g, U8 b, U8 a)
		{ 
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
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;
			case 1:
				m_u1 = u;
				m_v1 = v;
				break;
			default:
				break;
			}
		}

		void SetWeight(U32 index, F32 weight) {}

		void SetPaletteIndex(U32 instance, U8 index) {}


			// The untransformed, 3D position for the vertex
		F32 m_x, m_y, m_z;
		F32 m_nx, m_ny, m_nz;
		
		F32 m_u0, m_v0;
		F32 m_u1, m_v1;
	};


	struct D3DVertexPosNormTex1Weight4Idx4
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		void SetColour(const Maths::HVec4& colour)
		{ 
		}

		void SetColour(U8 r, U8 g, U8 b, U8 a)
		{ 
		}

		void SetTextureCoordinate(U32 set, const Aftk::TextureCoordinate4& uv)
		{ 
			if (set == 0)
			{
				m_u0 = uv[0];
				m_v0 = uv[1];
			}
		}

		void SetTextureCoordinate(U32 set, F32 u, F32 v)
		{ 
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;

			default:
				break;
			}
		}

		void SetWeight(U32 index, F32 weight)
		{
			m_weights[index] = weight;
		}

		void SetPaletteIndex(U32 instance, U8 index)
		{
			m_paletteIndices[instance] = index;
		}

		U8 PaletteIndex(U32 index)
		{
			return m_paletteIndices[index];
		}

		F32 Weight(U32 index)
		{
			return m_weights[index];
		}

		// The untransformed, 3D position for the vertex
		F32 m_x, m_y, m_z;
		F32 m_nx, m_ny, m_nz;
		
		F32 m_u0, m_v0;

		F32 m_weights[4];
		U8 m_paletteIndices[4];
	};


	struct D3DVertexPosNormColTex1Weight4Idx4
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		void SetColour(const Maths::HVec4& colour)
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

		void Colour(U8& r, U8& g, U8& b, U8 a)
		{
			r = m_colourR;
			g = m_colourG;
			b = m_colourB;
			a = m_colourA;
		}

		void SetTextureCoordinate(U32 set, const Aftk::TextureCoordinate4& uv)
		{ 
			if (set == 0)
			{
				m_u0 = uv[0];
				m_v0 = uv[1];
			}
		}

		void SetTextureCoordinate(U32 set, F32 u, F32 v)
		{ 
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;

			default:
				break;
			}
		}

		void SetWeight(U32 index, F32 weight)
		{
			m_weights[index] = weight;
		}

		void SetPaletteIndex(U32 instance, U8 index)
		{
			m_paletteIndices[instance] = index;
		}

		U8 PaletteIndex(U32 index)
		{
			return m_paletteIndices[index];
		}

		F32 Weight(U32 index)
		{
			return m_weights[index];
		}

		// The untransformed, 3D position for the vertex
		F32 m_x, m_y, m_z;
		F32 m_nx, m_ny, m_nz;
		
		U8 m_colourB;
		U8 m_colourG;
		U8 m_colourR;
		U8 m_colourA;
		
		F32 m_u0, m_v0;

		F32 m_weights[4];
		U8 m_paletteIndices[4];
	};

	struct D3DVertexPosNormTex2Weight4Idx4
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		void SetColour(const Maths::HVec4& colour)
		{ 
		}

		void SetColour(U8 r, U8 g, U8 b, U8 a)
		{ 
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
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;
			case 1:
				m_u1 = u;
				m_v1 = v;
				break;
			default:
				break;
			}
		}

		void SetWeight(U32 index, F32 weight)
		{
			m_weights[index] = weight;
		}

		void SetPaletteIndex(U32 instance, U8 index)
		{
			m_paletteIndices[instance] = index;
		}

		U8 PaletteIndex(U32 index)
		{
			return m_paletteIndices[index];
		}

		F32 Weight(U32 index)
		{
			return m_weights[index];
		}

		// The untransformed, 3D position for the vertex
		F32 m_x, m_y, m_z;
		F32 m_nx, m_ny, m_nz;
		
		F32 m_u0, m_v0;
		F32 m_u1, m_v1;

		F32 m_weights[4];
		U8 m_paletteIndices[4];
	};

	struct D3DVertexPosNormColTex2Weight4Idx4
	{
		void SetPosition(const Maths::HVec3& position)
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

		void Position(F32& vx, F32& vy, F32& vz)
		{
			vx = m_x;
			vy = m_y;
			vz = m_z;
		}

		void SetNormal(F32 nx, F32 ny, F32 nz)
		{
			m_nx = nx;
			m_ny = ny;
			m_nz = nz;
		}

		void SetNormal(const Maths::HVec3& normal) 
		{ 
			m_nx = normal[0]; 
			m_ny = normal[1];
			m_nz = normal[2];
		}

		void Normal(F32& vx, F32& vy, F32& vz)
		{
			vx = m_nx;
			vy = m_ny;
			vz = m_nz;
		}

		void SetColour(const Maths::HVec4& colour)
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

		void Colour(U8& r, U8& g, U8& b, U8 a)
		{
			r = m_colourR;
			g = m_colourG;
			b = m_colourB;
			a = m_colourA;
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
			switch(set)
			{
			case 0:
				m_u0 = u;
				m_v0 = v;
				break;
			case 1:
				m_u1 = u;
				m_v1 = v;
				break;
			default:
				break;
			}
		}

		void SetWeight(U32 index, F32 weight)
		{
			m_weights[index] = weight;
		}

		void SetPaletteIndex(U32 instance, U8 index)
		{
			m_paletteIndices[instance] = index;
		}

		U8 PaletteIndex(U32 index)
		{
			return m_paletteIndices[index];
		}

		F32 Weight(U32 index)
		{
			return m_weights[index];
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

		F32 m_weights[4];
		U8 m_paletteIndices[4];
	};
	
}

