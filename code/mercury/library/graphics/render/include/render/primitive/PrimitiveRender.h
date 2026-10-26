#ifndef PRIMITIVE_RENDER_H
#define PRIMITIVE_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/Device.h"
#include "Display/Buffer/VertexDeclaration.h"
#include "Display/Buffer/VertexBuffer.h"
#include "Display/Buffer/IndexBuffer.h"
#include "Display/utils/Renderer.h"
#include <btl/containers/Vector.h>

namespace Render
{
namespace Primitive
{
//Max vertices in one PrimitiveVertexBuffer
const U32 MaxVertexBufferSize = 10000;
const U32 MaxIndexBufferSize = 5000;
const U32 RingBufferSize = 8;

struct Vertex2D
{
	F32 px, py;
	U32 c;

	void Pos(const F32 x, const F32 y) { px = x; py = y; }
	void Pos(const Maths::HVec2& p) {  Maths::Store(p, (MathsStorage::Float2*)&px); }
	void Col(const Maths::RGBA col) { c = Display::GPU_RGBA(col); }
};

struct Vertex3D
{
	F32 px, py, pz;
	U32 c;

	void Pos(const F32 x, const F32 y, const F32 z) { px = x; py = y; pz = z; }
	void Pos(const Maths::HVec3& p) { Maths::Store(p, (MathsStorage::Float3*)&px); }
	void Col(const Maths::RGBA col) { c = Display::GPU_RGBA(col); }
};

struct Vertex3DLit : Vertex3D
{
	F32 nx, ny, nz;

	void Norm(const Maths::HVec3& n) { Maths::Store(n, (MathsStorage::Float3*)&nx); }
};

enum E_DebugRenderVertexShader
{
	E_PrimitiveRenderVertexShader_2D = 0,
	E_PrimitiveRenderVertexShader_3D,
	E_PrimitiveRenderVertexShader_3DLit,
	E_PrimitiveRenderVertexShader_Count
};

enum E_DebugRenderPixelShader
{
	E_PrimitiveRenderPixelShader_Default = 0,
	E_PrimitiveRenderPixelShader_Count
};


class C_PrimitiveVertexBuffer;

class C_PrimitiveDrawCall
{
public:
	friend class C_PrimitiveRender;
	friend class C_PrimitiveVertexBuffer;
	
	C_PrimitiveDrawCall();
	~C_PrimitiveDrawCall();

private:

	C_PrimitiveDrawCall(const Display::E_PrimitiveType	primitiveType, const bool lit, const Maths::HVec3 PCREF lightDirection, const bool is3D, const bool depthEnable, const Display::E_BlendMode blendMode, U32 primitiveVertexBufferIndex);

	//States
	Maths::HVec3 m_LightDirection;
	bool m_DepthEnable;
	Display::E_BlendMode m_BlendMode;
	bool m_3D;

	//Type
	Display::E_PrimitiveType m_PrimitiveType;
	bool m_Lit;

	//VertexBuffer
	U32 m_PrimitiveVertexBufferIndex;

	//Vertex Buffer
	Display::C_IndexBuffer* m_pIndexBuffer;

	//Next free Index
	U32 m_NextFreeIndex;

	//Vertex Buffer Memory Zero
	U16* m_pIndexBufferMemoryZero;
	
	bool CheckIndexMemory(const U32 indexCount) const
	{
		return m_NextFreeIndex + indexCount < MaxIndexBufferSize;
	}
	void GenerateIndexes(const U16 vertexStartIndex, const U32 vertexCount);

	bool Match(const Display::E_PrimitiveType primitiveType, const bool lit, const Maths::HVec3 PCREF lightDirection, const bool is3D, const bool depthEnable, const Display::E_BlendMode blendMode) const
	{
		return	(primitiveType == m_PrimitiveType) &&
				(lit == m_Lit) &&
				(blendMode == m_BlendMode) &&
				(depthEnable == m_DepthEnable) &&
				(is3D == m_3D) &&
				((lit)?((m_LightDirection[0] == lightDirection[0])):true);
	}

	//Resource allocation
	void Create();
	void Destroy();

	//Prepare to Render
	void Flush();

};

class C_PrimitiveVertexBuffer
{
public:
	friend class C_PrimitiveRender;
	C_PrimitiveVertexBuffer();
	~C_PrimitiveVertexBuffer();

private:
	C_PrimitiveVertexBuffer(const bool lit, const U32 vertexStride);

	//Vertex Buffer
	Display::C_VertexBuffer* m_pVertexBuffer;

	//Vertex Buffer Memory Zero
	void* m_pVertexBufferMemoryZero;
	
	//lit?
	bool m_Lit;

	//Vertex stride
	U32 m_VertexStride;
	//Current allocation
	U32 m_NextFreeVertex;

	bool IsLit() const
	{
		return m_Lit;
	}

	bool CheckVertexMemory(const U32 vertexCount) const
	{
		return m_NextFreeVertex + vertexCount <  MaxVertexBufferSize;
	}

	void* GetVertexMemory(const U32 vertexCount, U16 &vertexIndex)
	{
		void* pLockedMemory = (U8*)m_pVertexBufferMemoryZero + m_NextFreeVertex * m_VertexStride;
		vertexIndex = m_NextFreeVertex;
		m_NextFreeVertex += vertexCount;
		return pLockedMemory;
	}

	void* GetVertexMemoryChecked(const U32 vertexCount, U16 &vertexIndex);

	//Resource allocation
	void Create();
	void Destroy();

	//Prepare to render
	void Flush();
};


class C_PrimitiveRender: public Display::I_Renderer
{
public:

	C_PrimitiveRender(const U32 maxMemory = 2000000); 
	~C_PrimitiveRender();
	
	//States
	void SetLightDirection(const Maths::HVec3& lightDirection)
	{
		m_LightDirection = lightDirection;
	}
	void SetDepthEnable(const bool depthEnable)
	{
		m_DepthEnable = depthEnable;
	}
	void SetBlendMode( const Display::E_BlendMode blendMode )
	{
		m_BlendMode = blendMode;
	}
	void Begin3D()
	{
		m_3D = true;
	}
	void Begin2D()
	{
		m_3D = false;
	}

	//Render
	void* BeginVertices(const Display::E_PrimitiveType primitiveType, const U32 vertexCount, const bool lit = false);
	void EndVertices(void* pLockedMemory);

	//Flush
	void Flush();

	//Fence
	void AddFence();

	//Render
	void Render();

	//EndFrame
	void EndFrame();

private:
	//Max memory
	U32 m_MaxMemory;
	//Current memory
	U32 m_CurrentMemory;


	//Double buffering
	U32 m_CurrentCounter;
	U32 m_Index;
	bool m_Full;
	bool m_FullWarning;

	//States
	Maths::HVec3 m_LightDirection;
	bool m_DepthEnable;
	Display::E_BlendMode m_BlendMode;
	bool m_3D;
	
	//First PrimitiveDrawCall avalaible
	U32 m_FirstAvaliableDrawCall;

	//Vertex buffers
	Btl::Vector<C_PrimitiveVertexBuffer> m_PrimitiveVertexBuffer[RingBufferSize];

	//PrimitiveDrawCalls
	Btl::Vector<C_PrimitiveDrawCall> m_PrimitiveDrawCalls[RingBufferSize];
	
	//Vertex Declaration
	Display::C_VertexDeclaration*	m_pVertexDeclaration2D;
	Display::C_VertexDeclaration*	m_pVertexDeclaration3D;
	Display::C_VertexDeclaration*	m_pVertexDeclaration3DLit;

	Display::C_VertexShader			m_VertexShader[E_PrimitiveRenderVertexShader_Count];
	Display::C_PixelShader			m_PixelShader[E_PrimitiveRenderPixelShader_Count];

	bool OnCreateVolatile();
	void OnDestroyVolatile();

	bool OnCreate();
	void OnDestroy();

	void Reset();

	Btl::Vector<C_PrimitiveVertexBuffer>& GetPrimitiveVertexBuffer()
	{
		return m_PrimitiveVertexBuffer[m_Index];
	}

	Btl::Vector<C_PrimitiveDrawCall>& GetPrimitiveDrawCall()
	{
		return m_PrimitiveDrawCalls[m_Index];
	}

	U32 GetVertexStride(const bool lit) const
	{
		if (lit)
		{
			return sizeof(Vertex3DLit);
		}
		else
		{
			if (m_3D)
				return sizeof(Vertex3D);
			else
				return sizeof(Vertex2D);
		}
	}

	bool CheckFreeMemory(U32 memory)
	{
		if (m_CurrentMemory + memory > m_MaxMemory)
		{
			return false;
		}
		else
		{
			m_CurrentMemory += memory;
			return true;
		}

	}
};

} //namespace Primitive
} //namespace Render


#endif // PRIMITIVE_RENDER_H
