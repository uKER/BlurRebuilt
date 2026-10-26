#include "stdafx.h"
#include "Render/Primitive/PrimitiveRender.h"

#include "Render/Shaders/Constants.h"

EXTERN_SHADER(vsPrimitive2D);
EXTERN_SHADER(vsPrimitive3D);
EXTERN_SHADER(vsPrimitive3DLit);
EXTERN_SHADER(psPrimitive);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Debug/PS3/vsPrimitive2D.shader"
#include "Render/Shaders/Debug/PS3/vsPrimitive3D.shader"
#include "Render/Shaders/Debug/PS3/psPrimitive.shader"
#endif
#endif

namespace Render
{
namespace Primitive
{

C_PrimitiveDrawCall::C_PrimitiveDrawCall( const Display::E_PrimitiveType primitiveType, const bool lit, const Maths::HVec3 PCREF lightDirection, const bool is3D, const bool depthEnable, const Display::E_BlendMode blendMode, U32 primitiveVertexBufferIndex )
{
	m_PrimitiveType = primitiveType;
	m_Lit = lit;
	m_LightDirection = lightDirection;
	m_DepthEnable = depthEnable;
	m_BlendMode = blendMode;
	m_3D = is3D;

	m_PrimitiveVertexBufferIndex = primitiveVertexBufferIndex;

	m_NextFreeIndex = 0;

	m_pIndexBuffer = NULL;
}

C_PrimitiveDrawCall::C_PrimitiveDrawCall()
{
	m_pIndexBuffer = NULL;
}

C_PrimitiveDrawCall::~C_PrimitiveDrawCall()
{
	
}

void C_PrimitiveDrawCall::Flush()
{
	//Unlock index buffer for usage
	m_pIndexBuffer->Unlock();

	m_pIndexBufferMemoryZero = NULL;
}

void C_PrimitiveDrawCall::GenerateIndexes( const U16 vertexIndex, const U32 vertexCount)
{
	assert(m_NextFreeIndex + vertexCount < MaxIndexBufferSize);

	U16* pLockedIndexMemory = m_pIndexBufferMemoryZero + m_NextFreeIndex;

	for (U32 i = 0; i < vertexCount; i++)
	{
		pLockedIndexMemory[i] =  vertexIndex + i;
	}

	//Reserve memory
	m_NextFreeIndex += vertexCount;
}

void C_PrimitiveDrawCall::Create()
{
	RESOURCE_MEMORY_ID("PrimitiveRender: IndexBuffer");

	m_pIndexBuffer = new Display::C_IndexBuffer;

	//Create IndexBuffer
	if (!Display::Device()->CreateIndexBuffer(MaxIndexBufferSize * sizeof(U16), 0, Display::E_Format_Index16, m_pIndexBuffer))
	{
		LOGFATAL(g_Log, "PrimitiveRender : Failed to create index buffer with size %i\n", MaxVertexBufferSize);
	}

	//Lock Index Buffer
	m_pIndexBufferMemoryZero = (U16*)m_pIndexBuffer->Lock();
}

void C_PrimitiveDrawCall::Destroy()
{
	if (m_pIndexBufferMemoryZero)
		m_pIndexBuffer->Unlock();


	Display::Device()->DestroyIndexBuffer(m_pIndexBuffer);
	
	delete m_pIndexBuffer;
}

C_PrimitiveVertexBuffer::C_PrimitiveVertexBuffer( const bool lit, const U32 vertexStride)
{
	m_NextFreeVertex = 0;
	m_Lit = lit;
	m_VertexStride = vertexStride;

	m_pVertexBuffer = NULL;
}

C_PrimitiveVertexBuffer::C_PrimitiveVertexBuffer()
{
	m_pVertexBuffer = NULL;
}

C_PrimitiveVertexBuffer::~C_PrimitiveVertexBuffer()
{
}

void* C_PrimitiveVertexBuffer::GetVertexMemoryChecked( const U32 vertexCount, U16 &vertexIndex)
{
	if (m_NextFreeVertex + vertexCount >=  MaxVertexBufferSize)
	{
		//Full vertex buffer
		return NULL;
	}
	//Return memory and allocate
	void* pLockedMemory = (U8*)m_pVertexBufferMemoryZero + m_NextFreeVertex * m_VertexStride;
	vertexIndex = m_NextFreeVertex;
	m_NextFreeVertex += vertexCount;

	return pLockedMemory;
}

void C_PrimitiveVertexBuffer::Flush()
{
	//Unlock vertex buffer for usage
	m_pVertexBuffer->Unlock();

	m_pVertexBufferMemoryZero = NULL;
}

void C_PrimitiveVertexBuffer::Create()
{
	RESOURCE_MEMORY_ID("PrimitiveRender: VertexBuffer");

	m_pVertexBuffer = new Display::C_VertexBuffer;

	//Create VertexBuffer
	if (!Display::Device()->CreateVertexBuffer(MaxVertexBufferSize * m_VertexStride, Display::E_Usage_Video | Display::E_Usage_Dynamic, m_pVertexBuffer))
	{
		LOGFATAL(g_Log, "PrimitiveRender : Failed to create vertex buffer with size %i\n", MaxVertexBufferSize * m_VertexStride);
	}

	//Lock Vertex Buffer
	m_pVertexBufferMemoryZero = m_pVertexBuffer->Lock();
	
}

void C_PrimitiveVertexBuffer::Destroy()
{
	if (m_pVertexBufferMemoryZero)
		m_pVertexBuffer->Unlock();


	Display::Device()->DestroyVertexBuffer(m_pVertexBuffer);
	delete m_pVertexBuffer;
}

C_PrimitiveRender::C_PrimitiveRender(const U32 maxMemory) : Display::I_Renderer("PrimitiveRender")
{
	//Init states
	m_LightDirection = Maths::HVec3(0.f,0.f,1.f);
	m_DepthEnable = true;
	m_BlendMode = Display::E_BlendMode_Solid;
	m_3D = false;

	m_MaxMemory = maxMemory;
	m_CurrentMemory = 0;

	m_FirstAvaliableDrawCall = 0;

	m_CurrentCounter = 0;
	m_Index = 0;
	m_Full = false;
	m_FullWarning = false;
}

C_PrimitiveRender::~C_PrimitiveRender()
{

}

void* C_PrimitiveRender::BeginVertices( const Display::E_PrimitiveType primitiveType, const U32 vertexCount, const bool lit /*= false*/)
{
	if (m_Full)
		return NULL;


	assert(vertexCount < MaxVertexBufferSize);
	assert(vertexCount < MaxIndexBufferSize);

	assert(primitiveType == Display::E_PrimitiveType_QuadList || primitiveType == Display::E_PrimitiveType_LineList || primitiveType == Display::E_PrimitiveType_PointList || primitiveType == Display::E_PrimitiveType_TriangleList);
	

	//bool needVertexBuffer = false;
	//Find PrimitiveDrawCall
	for (int i = m_FirstAvaliableDrawCall; i < GetPrimitiveDrawCall().Size(); i++)
	{
		C_PrimitiveDrawCall& primitiveDrawCall = GetPrimitiveDrawCall()[i];

		C_PrimitiveVertexBuffer& primitiveVertexBuffer = GetPrimitiveVertexBuffer()[primitiveDrawCall.m_PrimitiveVertexBufferIndex];

		if ( primitiveVertexBuffer.CheckVertexMemory(vertexCount))
		{
			//Match
			if (primitiveDrawCall.Match(primitiveType, lit, m_LightDirection, m_3D, m_DepthEnable, m_BlendMode))
			{
				//space in the vertex buffer
				if (primitiveDrawCall.CheckIndexMemory(vertexCount))
				{
					//space in the index buffer
					U16 indexStart = 0;

					//Lock Vertex memory
					void* pLockedMemory = primitiveVertexBuffer.GetVertexMemory(vertexCount, indexStart);
					
					//Generate Indexes for this memory with the vertex start
					primitiveDrawCall.GenerateIndexes(indexStart, vertexCount);

					return pLockedMemory;

				}
				//It doesn't fit in the IndexBuffer
			}			
		}
	}

	U32 vertexBufferIndex = 0xFFFFFFFF;
	
	//space in the index buffer
	U16 indexStart = 0;

	void* pLockedMemory = NULL;
	//Create another PrimitiveVertexBuffer if needed

	{
		//Find the vertex buffer first free
		int i = 0;
		while (i < GetPrimitiveVertexBuffer().Size())
		{
			if (GetPrimitiveVertexBuffer()[i].IsLit() == lit)
			{
				//try if it has space in the VertexBuffer
				pLockedMemory = GetPrimitiveVertexBuffer()[i].GetVertexMemoryChecked(vertexCount, indexStart);

				if (pLockedMemory != NULL)
				{
					//Found
					vertexBufferIndex = i;
					break;
				}
			}
			i++;
		}

		//Any found
		if (vertexBufferIndex == 0xFFFFFFFF)
		{
			//Check memory
			if (!CheckFreeMemory(GetVertexStride(lit) * MaxVertexBufferSize))
			{
				if (!m_FullWarning)
				{
					LOGWARN	(g_Log, "PrimitiveRender : Primitive Render is full\n");
					m_FullWarning = true;
				}
				return NULL;
			}

			//Create a new one
			vertexBufferIndex = GetPrimitiveVertexBuffer().Size(); 
			GetPrimitiveVertexBuffer().PushBack( C_PrimitiveVertexBuffer(lit, GetVertexStride(lit)));
			C_PrimitiveVertexBuffer& primitiveVertexBuffer = GetPrimitiveVertexBuffer()[GetPrimitiveVertexBuffer().Size() - 1];

			primitiveVertexBuffer.Create();

			//try if it has space in the VertexBuffer
			pLockedMemory = primitiveVertexBuffer.GetVertexMemoryChecked(vertexCount, indexStart);
			if (pLockedMemory == NULL)
			{
				LOGFATAL(g_Log, "PrimitiveRender : Primitive doesn't fit in any PrimitiveVertexBuffer, Max vertex count %i\n", MaxVertexBufferSize);
			}
		}
	}

	//Check memory
	if (!CheckFreeMemory(sizeof(U16) * MaxIndexBufferSize))
	{
		if (!m_FullWarning)
		{
			LOGWARN	(g_Log, "PrimitiveRender : Primitive Render is full\n");
			m_FullWarning = true;
		}
		return NULL;
	}

	GetPrimitiveDrawCall().PushBack(C_PrimitiveDrawCall(primitiveType, lit, m_LightDirection, m_3D, m_DepthEnable, m_BlendMode, vertexBufferIndex));
	C_PrimitiveDrawCall& primitiveDrawCall = GetPrimitiveDrawCall()[GetPrimitiveDrawCall().Size() - 1];

	//Create Resources
	primitiveDrawCall.Create();

	//Generate Indexes for the LockedVertices
	primitiveDrawCall.GenerateIndexes(indexStart, vertexCount);


	return pLockedMemory;
}

void C_PrimitiveRender::EndVertices(void* /*pLockedMemory*/)
{

}

void C_PrimitiveRender::Flush()
{

}

void C_PrimitiveRender::AddFence()
{
	m_FirstAvaliableDrawCall = GetPrimitiveDrawCall().Size();	
}

bool C_PrimitiveRender::OnCreate()
{
	RESOURCE_MEMORY_ID("PrimitiveRender: Shaders");

	//Vertex Declaration
	Display::C_VertexFormat vertexFormat2D;
	vertexFormat2D.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_Position, 0);
	vertexFormat2D.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
	m_pVertexDeclaration2D = Display::Device()->CreateVertexDeclaration(vertexFormat2D);

	Display::C_VertexFormat vertexFormat3D;
	vertexFormat3D.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	vertexFormat3D.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
	m_pVertexDeclaration3D = Display::Device()->CreateVertexDeclaration(vertexFormat3D);

	Display::C_VertexFormat vertexFormat3DLit;
	vertexFormat3DLit.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	vertexFormat3DLit.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
	vertexFormat3DLit.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_TexCoord, 0);
	m_pVertexDeclaration3DLit = Display::Device()->CreateVertexDeclaration(vertexFormat3DLit);	

	//Shaders
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsPrimitive2D", 0, &m_VertexShader[E_PrimitiveRenderVertexShader_2D], REF_SHADER(vsPrimitive2D)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsPrimitive3D", 0, &m_VertexShader[E_PrimitiveRenderVertexShader_3D], REF_SHADER(vsPrimitive3D)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsPrimitive3D", 0, &m_VertexShader[E_PrimitiveRenderVertexShader_3DLit], REF_SHADER(vsPrimitive3DLit)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psPrimitive", 0, &m_PixelShader[E_PrimitiveRenderPixelShader_Default], REF_SHADER(psPrimitive)))
		return false;

	return true;
}

void C_PrimitiveRender::OnDestroy()
{

	//Destroy resources
	for (int j = 0; j < RingBufferSize; j++)
	{
		for (int i = 0; i < m_PrimitiveVertexBuffer[j].Size(); i++)
			m_PrimitiveVertexBuffer[j][i].Destroy();

		for (int i = 0; i < m_PrimitiveDrawCalls[j].Size(); i++)
			m_PrimitiveDrawCalls[j][i].Destroy();
	}
	
	//Shaders
	Display::Device()->Destroy(&m_VertexShader[E_PrimitiveRenderVertexShader_2D]);
	Display::Device()->Destroy(&m_VertexShader[E_PrimitiveRenderVertexShader_3D]);
	Display::Device()->Destroy(&m_VertexShader[E_PrimitiveRenderVertexShader_3DLit]);
	Display::Device()->Destroy(&m_PixelShader[E_PrimitiveRenderPixelShader_Default]);

}

bool C_PrimitiveRender::OnCreateVolatile()
{
	return true;
}

void C_PrimitiveRender::OnDestroyVolatile()
{
	//Destroy resources
	for (int j = 0; j < RingBufferSize; j++)
	{
		for (int i = 0; i < m_PrimitiveVertexBuffer[j].Size(); i++)
			m_PrimitiveVertexBuffer[j][i].Destroy();

		for (int i = 0; i < m_PrimitiveDrawCalls[j].Size(); i++)
			m_PrimitiveDrawCalls[j][i].Destroy();

		//Empty all the lists
		m_PrimitiveVertexBuffer[j].Clear();
		m_PrimitiveDrawCalls[j].Clear();
	}
}

void C_PrimitiveRender::Reset()
{
	m_CurrentCounter++;
	if (m_CurrentCounter >= RingBufferSize/2)
	{
		m_Full = true;
		if (!m_FullWarning)
		{
			LOGINFO(g_Log, "Primitive Render full");
			m_FullWarning = true;
		}
		return;
	}
	m_Index = (m_Index + 1) % RingBufferSize;
	m_CurrentMemory = 0;
	m_FirstAvaliableDrawCall = 0;

	//Destroy resources
	for (int i = 0; i < GetPrimitiveVertexBuffer().Size(); i++)
		GetPrimitiveVertexBuffer()[i].Destroy();

	for (int i = 0; i < GetPrimitiveDrawCall().Size(); i++)
		GetPrimitiveDrawCall()[i].Destroy();

	//Empty all the lists
	GetPrimitiveDrawCall().Clear();
	GetPrimitiveVertexBuffer().Clear();
}

void C_PrimitiveRender::Render()
{
	if (m_Full)
		return;

	if (GetPrimitiveDrawCall().Size() == 0) //Nothing to do
		return;

	//Flush all the VertexBuffer and IndexBuffers
	for (int i = 0; i < GetPrimitiveVertexBuffer().Size(); i++)
		GetPrimitiveVertexBuffer()[i].Flush();

	for (int i = 0; i < GetPrimitiveDrawCall().Size(); i++)
		GetPrimitiveDrawCall()[i].Flush();

	//Set Identity World Matrix 
	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);

	//Render PrimitiveDrawCalls
	for (int i = 0; i < GetPrimitiveDrawCall().Size(); i++)
	{
		const C_PrimitiveDrawCall& primitiveDrawCall = GetPrimitiveDrawCall()[i];

		//States
		if(primitiveDrawCall.m_DepthEnable)
			Display::Device()->SetDepthEnable(Display::E_Bool_True);
		else
			Display::Device()->SetDepthEnable(Display::E_Bool_False);
		
		if (primitiveDrawCall.m_Lit)
		{
			MathsStorage::Float4 light;
			Maths::Store(Maths::HVec4(primitiveDrawCall.m_LightDirection), &light);
			Display::Device()->SetVertexShaderConstantF(CV_RENDER_LIGHT_DIRECTION, (F32*)&light, 1);
		}

		Display::Device()->SetBlendMode(primitiveDrawCall.m_BlendMode);

		if (primitiveDrawCall.m_Lit)
		{
			Display::Device()->SetVertexShader(&m_VertexShader[E_PrimitiveRenderVertexShader_3DLit]);
			Display::Device()->SetPixelShader(&m_PixelShader[E_PrimitiveRenderPixelShader_Default]);

			Display::Device()->SetVertexDeclaration(m_pVertexDeclaration3DLit);
		}
		else
		{
			if (m_3D)
			{
				Display::Device()->SetVertexShader(&m_VertexShader[E_PrimitiveRenderVertexShader_3D]);
				Display::Device()->SetPixelShader(&m_PixelShader[E_PrimitiveRenderPixelShader_Default]);

				Display::Device()->SetVertexDeclaration(m_pVertexDeclaration3D);
			}
			else
			{
				Display::Device()->SetVertexShader(&m_VertexShader[E_PrimitiveRenderVertexShader_2D]);
				Display::Device()->SetPixelShader(&m_PixelShader[E_PrimitiveRenderPixelShader_Default]);
				
				Display::Device()->SetVertexDeclaration(m_pVertexDeclaration2D);
			}
		}

		Display::Device()->SetVertexBuffer(0, GetPrimitiveVertexBuffer()[primitiveDrawCall.m_PrimitiveVertexBufferIndex].m_pVertexBuffer, 0, GetPrimitiveVertexBuffer()[primitiveDrawCall.m_PrimitiveVertexBufferIndex].m_VertexStride);
		Display::Device()->SetIndexBuffer(primitiveDrawCall.m_pIndexBuffer);
	
		//Draw
		Display::Device()->DrawIndexedPrimitive(primitiveDrawCall.m_PrimitiveType, 0, 0, primitiveDrawCall.m_NextFreeIndex, 0, primitiveDrawCall.m_NextFreeIndex);

		//Unset
		Display::Device()->SetVertexBuffer(0, NULL , 0, 0);
		Display::Device()->SetIndexBuffer(NULL);
	}



	Reset();
}

void C_PrimitiveRender::EndFrame()
{
	m_CurrentCounter = 0;
	m_Full = false;
	m_FullWarning = false;
}
} //namespace Primitive
} //namespace Render



