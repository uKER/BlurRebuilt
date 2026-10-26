#ifndef RENDER_DYNAMICEFFECT_CLOTH_H
#define RENDER_DYNAMICEFFECT_CLOTH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Renderer.h"

#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Buffer/DynamicVB.h"

#include "Dynamic/Block.h"
#include "Dynamic/Cloth/Cloth.h"
#include "Dynamic/InstancedEffectInterface.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Shaders/Constants.h"

#include "Misc_Shared/Noise/Perlin.h"

namespace DynamicEffect
{

extern Debug::Logger& g_Log;
extern Debug::Bool g_debugClothEnable;

struct ClothSettings : Cloth::SheetParameters
{
	ClothSettings() : Cloth::SheetParameters(), fExpiryTime(0.1f), fCullDistance(1000.0f), bLocalWind(false) {}
	~ClothSettings() {}

	F32								fExpiryTime;				// additional time offscreen before particles are removed
	F32								fCullDistance;
	bool							bLocalWind;
};

class C_Cloth : public Cloth::C_Sheet, public C_RenderNode, public Ut::ListElementDouble<C_Cloth>
{
public:
	C_Cloth(): m_iNoiseX(0),
			   m_iNoiseY(1),
			   m_iNoiseZ(2),
			   m_fWindRandomAnimSpeed(0.1f),
			   m_fWindRandomScale(1.f),
			   m_bUseLocalWind(false)
	{}

	S32				m_iIndexBuffer;
	mutable F32		m_fTime;

	void UseLocalWind(bool bUseLocalWind);
	void SetLocalWind(const MathsStorage::Float3& vWindLocal, F32 fRandomScale, F32 fRandomAnimSpeed);
		
	template<class T, class Instance> friend class C_ClothRenderer;

private:
	void UpdateWind(F32 fFrameTime, const Maths::HVec3& vGlobalWind);

	MathsStorage::Float3			m_vWindLocal;
	FP32							m_iNoiseX;
	FP32							m_iNoiseY;
	FP32							m_iNoiseZ;
	F32								m_fWindRandomAnimSpeed;
	F32								m_fWindRandomScale;
	
	bool							m_bUseLocalWind;
};

INLINE void C_Cloth::UseLocalWind(bool bUseLocalWind)
{
	m_bUseLocalWind = bUseLocalWind;
}

INLINE void C_Cloth::SetLocalWind(const MathsStorage::Float3& vWindLocal, F32 fWindRandomScale, F32 fWindRandomAnimSpeed)
{
	m_vWindLocal = vWindLocal;
	m_fWindRandomScale = fWindRandomScale;
	m_fWindRandomAnimSpeed = fWindRandomAnimSpeed;
}

INLINE void C_Cloth::UpdateWind(F32 fFrameTime, const Maths::HVec3& vGlobalWind)
{
	if(m_bUseLocalWind)
	{
		Maths::HF32 fWindRandomScale(m_fWindRandomScale);
		Maths::HVec3 vWindLocal(Maths::Load<Maths::HVec3>(&m_vWindLocal));

		// update noise FP32s
		m_iNoiseX += fFrameTime * m_fWindRandomAnimSpeed;
		m_iNoiseY += fFrameTime * m_fWindRandomAnimSpeed;
		m_iNoiseZ += fFrameTime * m_fWindRandomAnimSpeed;

		C_PerlinNoiseDefault noise;
		Maths::HVec3 vWindRandom(noise.SoftNoise1D(m_iNoiseX), noise.SoftNoise1D(m_iNoiseY), noise.SoftNoise1D(m_iNoiseZ));
		Maths::HVec3 vWind = vWindLocal + vWindRandom * fWindRandomScale * Maths::Constants<Maths::HF32>::Half();
		SetWind(vWind);
	}
	else
		SetWind(vGlobalWind);
}

typedef Instance<C_Cloth>		C_ClothInstance;

struct ClothIndexBuffer
{
	MathsStorage::Int2			size;
	Display::C_IndexBuffer	indexBuffer;
};

template<class T, class Instance>
class C_ClothRenderer : public I_InstancedEffect<C_Cloth, Instance>
{
public:
	typedef I_InstancedEffect<C_Cloth, Instance>	Base;

	C_ClothRenderer(C_Renderer* pRenderer, const U32 iIndex) : I_InstancedEffect<C_Cloth, Instance>(pRenderer, NULL, iIndex, E_EffectTypeIndex_Cloth), m_bIndexBuffersCreated(false) {}
	virtual ~C_ClothRenderer() { Base::m_ObjectList.Free(); }

	typedef Display::Vertex_XYZ_C_UV_N	ClothVertex;
	
	void						Update();
	void						Reset() {};
	void						OnBuild(C_CompositeRenderNode& batch, RenderNodes& blocks, const C_View& view) const;
	C_VertexBufferInfo			OnRender(const C_CompositeRenderNode& batch, const U32 states) const;
	MathsStorage::Int2			OnDisplayDebug(MathsStorage::Int2 position) const;

	virtual bool				Create() = 0;
	bool						CreateVolatile();
	virtual void				Destroy() = 0;
	void						DestroyVolatile();

	C_Cloth*					AddCloth(const ClothSettings& settings);
	void						RemoveCloth(C_Cloth* pCloth);
	
	void						AddInstanceManaged(const Instance* pInstance, const E_InstancedObjectType eObjectType, const S32 iIndex, const ClothSettings& settings);
	void						AddInstanceManaged(const Instance& instance, const E_InstancedObjectType eObjectType, const S32 iIndex, const ClothSettings& settings);
	
	void						ClearManaged();

	bool						AddIndexBuffer(const MathsStorage::Int2 size);
	bool						ClearIndexBuffers();
protected:
	void						UpdateCloth(C_Cloth* pCloth, S32 iMinorTimeSteps, F32 fMinorTimeStep);
	C_Cloth*					GetCloth(const S32 iIndex, const ClothSettings& settings);

	const T*					Derived() const { return static_cast<const T*>(this); }

	Display::C_PixelShader			m_PixelShader;
	Display::C_VertexShader			m_VertexShader;

	Ut::Vector<ClothIndexBuffer>		m_IndexBuffers;
	bool								m_bIndexBuffersCreated;
};

template<class T, class Instance>
bool C_ClothRenderer<T, Instance>::CreateVolatile()
{
	RESOURCE_MEMORY_ID("DynamicEffect");

	m_bIndexBuffersCreated = true;

	for(S32 i = 0; i < m_IndexBuffers.Size(); ++i)
	{
		ClothIndexBuffer& clothIndexBuffer = m_IndexBuffers[i];

		U32 x = clothIndexBuffer.size.x - 1;
		U32 y = clothIndexBuffer.size.y - 1;
		U32 iIndicies = x * y * 6;	// 2 triangles per grid square
		if (!Display::Device()->CreateIndexBuffer(iIndicies * sizeof(U16), 0, Display::E_Format_Index16, &clothIndexBuffer.indexBuffer))
		{
			m_IndexBuffers.PopBack();
			return false;
		}

		U16* pIndex = (U16*)clothIndexBuffer.indexBuffer.Lock();

		for(U32 j = 0; j < y; ++j)
		{
			for(U32 i = 0; i < x; ++i)
			{
				U32 offset = j * (x + 1) + i;
				// tri 1
				*pIndex = offset; ++pIndex;
				*pIndex = offset + (x + 1); ++pIndex;
				*pIndex = offset + 1; ++pIndex;
				// tri 2
				*pIndex = offset + (x + 1); ++pIndex;
				*pIndex = offset + (x + 1) + 1; ++pIndex;
				*pIndex = offset + 1; ++pIndex;
			}
		}

		clothIndexBuffer.indexBuffer.Unlock();
	}

	return true;
}

template<class T, class Instance>
void C_ClothRenderer<T, Instance>::DestroyVolatile()
{
	ClearManaged();

	m_bIndexBuffersCreated = false;

	for(S32 i = 0; i < m_IndexBuffers.Size(); ++i)
	{
		Display::Device()->Destroy(&m_IndexBuffers[i].indexBuffer);
	}
}

template<class T, class Instance>
void C_ClothRenderer<T, Instance>::Update()
{
	F32 fMaxMinorTimeStep = Cloth::g_DebugMaxMinorTimeStep;
	S32 iMinorTimeSteps = S32( ceil( Base::m_pRenderer->GetFrameTime() / fMaxMinorTimeStep ) );
	F32 fMinorTimeStep = Base::m_pRenderer->GetFrameTime() / iMinorTimeSteps;

	PERF_EVENT("ClothRenderer");

	for(typename Base::ObjectList::Iterator pIterator = Base::m_ObjectList.m_Active.Begin(); pIterator != Base::m_ObjectList.m_Active.End(); ++pIterator)
	{
		C_Cloth* pCloth(&(*pIterator));

		/*Maths::HVec3 vPosition0 = pCloth->Particle(0, 0).position;
		Maths::HVec3 vPosition1 = pCloth->Particle(pCloth->GetSizeX() - 1, 0).position;
		Maths::HVec3 vPosition2 = pCloth->Particle(pCloth->GetSizeX() - 1, pCloth->GetSizeY() - 1).position;
		Maths::HVec3 vPosition3 = pCloth->Particle(0, pCloth->GetSizeY() - 1).position;
		assert(Maths::Length2(vPosition0) < 400.0f);
		assert(Maths::Length2(vPosition1) < 400.0f);
		assert(Maths::Length2(vPosition2) < 400.0f);
		assert(Maths::Length2(vPosition3) < 400.0f);*/

		UpdateCloth(pCloth, iMinorTimeSteps, fMinorTimeStep);
	}

	for(S32 i = 0; i < Base::m_ManagedObjects.Size(); ++i)
	{
		if(C_Cloth* pCloth = Base::m_ManagedObjects[i])
		{
			if(pCloth->m_fTime < Base::m_pRenderer->GetTime() - 0.5f)
			{
				--Base::m_nBlocks;
				Base::m_ObjectList.Free(pCloth);
				Base::m_ManagedObjects[i] = NULL;
			}
		}
	}
}

template<class T, class Instance>
void C_ClothRenderer<T, Instance>::ClearManaged()
{
	for(S32 i = 0; i < Base::m_ManagedObjects.Size(); ++i)
	{
		if(C_Cloth* pCloth = Base::m_ManagedObjects[i])
		{
			--Base::m_nBlocks;
			Base::m_ObjectList.Free(pCloth);
			Base::m_ManagedObjects[i] = NULL;
		}
	}
}

template<class T, class Instance>
void C_ClothRenderer<T, Instance>::UpdateCloth(C_Cloth* pCloth, S32 iMinorTimeSteps, F32 fMinorTimeStep)
{
	Maths::HVec3 vMin(10000.0f, 10000.0f, 10000.0f);
	Maths::HVec3 vMax(-10000.0f, -10000.0f, -10000.0f);
//	Maths::HVec3 vWind(Maths::Load<Maths::HVec3>(&Base::m_pRenderer->GetWind()));
	Maths::HVec3 vWind(-10.0f, 0.0f, 0.0f);

	pCloth->UpdateWind(Base::m_pRenderer->GetFrameTime(), vWind);
	
	for(S32 y = 0; y < pCloth->GetSizeY(); ++y)
	{
		for(S32 x = 0; x < pCloth->GetSizeX(); ++x)
		{
			Maths::HVec3 vPosition = pCloth->Particle(x, y).position;
			vMin = Maths::Min(vMin, vPosition);
			vMax = Maths::Max(vMax, vPosition);
		}
	}

	for(S32 i = 0; i < iMinorTimeSteps; ++i)
		pCloth->Update(fMinorTimeStep);

	Maths::HVec3 vCentre = (vMax + vMin) * Maths::Constants<Maths::HF32>::Half();
	Maths::HVec3 vSize = vMax - vCentre;
	
	Maths::Store(vCentre, (MathsStorage::Float3A16*)&pCloth->m_vCentre);
	Maths::Store(vSize, (MathsStorage::Float3A16*)&pCloth->m_vSize);
}

template<class T, class Instance>
void C_ClothRenderer<T, Instance>::OnBuild(C_CompositeRenderNode& batch, RenderNodes& blocks, const C_View& view) const
{
	PERF_EVENT("ClothRenderer::Build");
	
	batch.blocks = blocks;
	batch.iVertexBuffer = 0xFFFFFFFF;
	batch.pEffect = this;
	batch.pMatrix = NULL;

#ifndef CFG_SPEED
	if(!g_debugClothEnable)
		return;
#endif

	assert(blocks.Size() > 0);
	
	const C_Cloth* pCloth = (C_Cloth*)blocks[0].pBlock;

	assert(pCloth->m_iVertexBuffer == 0xFFFF);

	U32 iNumVertices = pCloth->GetSizeX() * pCloth->GetSizeY();
	C_DynamicVertexBufferBlock block = Base::GetRenderer()->GetDynamicVertexBuffer()->Begin("Cloth", sizeof(ClothVertex), iNumVertices);
	
	volatile ClothVertex* pVertices = (volatile ClothVertex*)block.pVertices;

	if(pVertices)
	{
		F32 du = 1.f / (F32)(pCloth->GetSizeX() - 1);
		F32 dv = 1.f / (F32)(pCloth->GetSizeY() - 1);
		
		F32 v = 1.0f;
		for(S32 j = 0; j < pCloth->GetSizeY(); ++j)
		{
			F32 u = 0.0f;
			for(S32 i = 0; i < pCloth->GetSizeX(); ++i)
			{
				const Cloth::C_Particle& particle = pCloth->Particle(i, j);
				Maths::Store(particle.position, (MathsStorage::Float3*)&pVertices->x);
				pVertices->c = 0xFFFFFFFF;
				pVertices->u = u;
				pVertices->v = v;
				Maths::Store(particle.normal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;

				u += du;
			}
			v -= dv;
		}

		Base::GetRenderer()->GetDynamicVertexBuffer()->End(block);
	}

	pCloth->m_iVertexBuffer = block.blockIndex;
	batch.iVertexBuffer = block.blockIndex;
}

template<class T, class Instance>
C_VertexBufferInfo C_ClothRenderer<T, Instance>::OnRender(const C_CompositeRenderNode& batch, const U32 states) const
{
	/*
#ifndef CFG_SPEED
	if(!g_debugClothEnable)
		return C_VertexBufferInfo();
#endif

	if(states & E_States_BindEffect)
	{
		Display::Device()->SetPixelShader(&m_PixelShader);
		Display::Device()->SetVertexShader(&m_VertexShader);
		Display::Device()->SetVertexDeclaration(ClothVertex::m_pDeclaration);
		Display::Device()->SetCullMode(Display::E_CullMode_None);
	}

	C_VertexBufferInfo vertexInfo;
	
	for(S32 i = 0; i < batch.blocks.Size(); ++i)
	{
		const C_Cloth* pCloth = (C_Cloth*)batch.blocks[i];
		const Instance* pInstance = (Instance*)batch.blocks[i]->pInstance;

		Base::GetRenderer()->SetLightConstants4(pInstance->lights);

		Derived()->SetMaterial(pInstance);

		if(pCloth->m_iIndexBuffer > -1)
		{
			U32 iIndexCount = (pCloth->GetSizeX() - 1) * (pCloth->GetSizeY() - 1) * 6;
			Base::GetRenderer()->GetDynamicVertexBuffer()->DrawIndexedPrimitiveBlock(Display::E_PrimitiveType_TriangleList, batch.iVertexBuffer, &m_IndexBuffers[pCloth->m_iIndexBuffer].indexBuffer, iIndexCount);
		#ifndef CFG_SPEED
			vertexInfo.m_iUsageSuccess += sizeof(ClothVertex) * iIndexCount;
		#endif
		}
	}

	if(states & E_States_UnbindEffect)
	{
		Display::Device()->SetCullMode(Display::E_CullMode_Default);
	}

	return vertexInfo;*/
}

template<class T, class Instance>
MathsStorage::Int2 C_ClothRenderer<T, Instance>::OnDisplayDebug(MathsStorage::Int2 position) const
{
	Util::C_String str;
	str.Printf("Cloth Manager", Base::m_ObjectList.m_Active.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.x += 25;
	position.y += 25;
	
	str.Printf("Active Cloth: %i", Base::m_ObjectList.m_Active.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Persistent Instances: %i", Base::m_InstancePtrsPersistent.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Dynamic Frame Instances: %i", Base::m_InstancePtrsDynamicFrame.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Dynamic View Instances: %i", Base::m_InstancePtrsDynamicView.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;
	position.x -= 25;

	return position;
}

template<class T, class Instance>
C_Cloth* C_ClothRenderer<T, Instance>::AddCloth(const ClothSettings& settings)
{
	if(C_Cloth* pCloth = Base::m_ObjectList.Allocate())
	{
		++Base::m_nBlocks;
		pCloth->m_pEffect = this;
		//pCloth->m_nCount = settings.size.x * settings.size.y;
		pCloth->m_fTime = 0.0f;
		pCloth->m_vCentre = MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f);
		pCloth->m_vSize = MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, settings.fCullDistance);
		pCloth->m_iVertexBuffer = 0xFFFF;
		// make flags unique per cloth dimension so renderer doesn't try and batch cloth requiring different index buffers
		pCloth->m_iFlags = (settings.size.x << 16) + settings.size.y;
		pCloth->Initialise(settings);
		pCloth->m_bUseLocalWind = settings.bLocalWind;
		
		UpdateCloth(pCloth, 100, Cloth::g_DebugMaxMinorTimeStep);

		pCloth->m_iIndexBuffer = -1;
		for(S32 j = 0; j < m_IndexBuffers.Size(); ++j)
		{
			if(pCloth->GetSizeX() == m_IndexBuffers[j].size.x && pCloth->GetSizeY() == m_IndexBuffers[j].size.y)
				pCloth->m_iIndexBuffer = j;
		}
		
		return pCloth;
	}
	else
		return NULL;
}

template<class T, class Instance>
void C_ClothRenderer<T, Instance>::RemoveCloth(C_Cloth* pCloth)
{
	--Base::m_nBlocks;
	pCloth->Shutdown();
	Base::m_ObjectList.Free(pCloth);
}

template<class T, class Instance>
C_Cloth* C_ClothRenderer<T, Instance>::GetCloth(const S32 iIndex, const ClothSettings& settings)
{
	C_Cloth* pCloth = Base::GetManagedObject(iIndex);
	
	if(!pCloth)
	{
		pCloth = AddCloth(settings);
		Base::SetManagedObject(iIndex, pCloth);
	}
	
	return pCloth;
}

template<class T, class Instance>
void C_ClothRenderer<T, Instance>::AddInstanceManaged(const Instance* pInstance, const E_InstancedObjectType eObjectType, const S32 iIndex, const ClothSettings& settings)
{
	C_Cloth* pCloth = GetCloth(iIndex, settings);
	pCloth->m_fTime = Base::m_pRenderer->GetTime();

	pInstance->pObject = pCloth;
	
	Base::AddInstance(pInstance, eObjectType);
}

template<class T, class Instance>
void C_ClothRenderer<T, Instance>::AddInstanceManaged(const Instance& instance, const E_InstancedObjectType eObjectType, const S32 iIndex, const ClothSettings& settings)
{
	C_Cloth* pCloth = GetCloth(iIndex, settings);
	pCloth->m_fTime = Base::m_pRenderer->GetTime();
	
	Base::AddInstance(instance, eObjectType);

	switch(eObjectType)
	{
	case E_InstancedObjectType_Persistent:
		Base::m_InstancesPersistent.Back().pObject = pCloth;
		break;
	case E_InstancedObjectType_DynamicFrame:
		Base::m_InstancesDynamicFrame.Back().pObject = pCloth;
		break;
	case E_InstancedObjectType_DynamicView:
		Base::m_InstancesDynamicView[Base::m_iView].Back().pObject = pCloth;
		break;
	}
}

template<class T, class Instance>
bool C_ClothRenderer<T, Instance>::AddIndexBuffer(const MathsStorage::Int2 size)
{
	if(!m_bIndexBuffersCreated)
	{
		bool bAlreadyCreated = false;
		for(S32 i = 0; i < m_IndexBuffers.Size(); ++i)
		{
			if(m_IndexBuffers[i].size.x == size.x && m_IndexBuffers[i].size.y == size.y)
				bAlreadyCreated = true;
		}

		if(!bAlreadyCreated)
		{
			m_IndexBuffers.PushBack();
			ClothIndexBuffer& clothIndexBuffer = m_IndexBuffers.Back();
			clothIndexBuffer.size = size;
		}
		return true;
	}
	else
		return false;
	
}

template<class T, class Instance>
bool C_ClothRenderer<T, Instance>::ClearIndexBuffers()
{
	if(!m_bIndexBuffersCreated)
	{
		m_IndexBuffers.Clear();
		return true;
	}
	else
		return false;
}

} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_CLOTH_H
