
#include "stdafx.h"

#include <world_render/ModelInstance.h>
#include <world_render/WorldRender.h>
#include <world_render/RenderModel.h>
#include <world_render/RenderModelData.h>
#include <world_render/RenderModelResourceUsage.h>
#include <SceneGraph/renderingdata/ModelInstanceCullNode.h>
#include <SceneGraph/renderingdata/PortalCullNode.h>
#include <system/Memory.h>
#include <renderingdata/RenderingData.h>
#include <SceneGraph/SceneGraph.h>

namespace WorldRender
{

#if PLATFORM_PC
// [Blur2PC] Render interpolation for decoupled update/render (see C_Application::IdleLoop, PC).
// The game updates at 60 Hz; the renderer may run faster. Every world matrix the game stores into
// material data during an update tick (C_ModelInstance::SetElementMatrix, and vehicle parts that
// store through their own world-matrix pointer, see Blur2PCInterpRecordStore) is recorded by its
// destination address with the value it had on the previous tick. Before a render, each destination
// written on the latest tick receives a blend of the two; the true value is written back after the
// render, so game code never reads a blended matrix.
}	// namespace WorldRender
#include <map>
#include <vector>
#include <windows.h>
namespace WorldRender
{
namespace
{
	struct S_InterpEntry
	{
		const void*		owner;			// model instance whose material data holds the destination
		Maths::HMat43	prev;
		Maths::HMat43	cur;
		U32				tick;
		bool			hasPrev;		// prev is the previous tick's final value (see CanBlend)
		bool			written;		// blended value written for the current render
		bool			overridden;		// stored again during the render by other code: do not restore
		bool			row43;			// destination is a MathsStorage::MatrixRow43 (skinning palette), else MatrixRow44
		// spinning wheel parts (Blur2PCInterpRecordWheelStore): the part's matrix is spinLocal * wheel
		bool			spin;			// the latest store of this tick was a wheel store
		bool			wheelHasPrev;	// prevWheel is the wheel matrix of the previous tick
		U32				wheelTick;
		F32				spinAngle;		// expected turn about the wheel's local X since the previous tick (radians)
		Maths::HMat43	prevWheel;
		Maths::HMat43	curWheel;
		Maths::HMat43	spinLocal;
	};
	typedef std::map<void*, S_InterpEntry> T_InterpMap;

	bool				g_InterpEnabled = false;
	bool				g_InterpInRender = false;
	U32					g_InterpTick = 1;
	F32					g_InterpRenderAlpha = 1.0f;
	F32					g_InterpTickSeconds = 1.0f / 60.0f;	// game time the latest update covered
	T_InterpMap*		g_pInterpMap = NULL;
	struct S_RenderHook { T_Blur2PCRenderHook fn; void* ctx; };
	std::vector<S_RenderHook>*	g_pRenderHooks = NULL;
	// [Blur2PC] Hooks are added from C_VehicleActor::DeferredUpdate, which runs on scheduler
	// worker threads, one job per car, while the render thread calls them: without this lock
	// two cars' first updates reallocated the vector at once (crash in memmove at the start of
	// a 20-car race, 2026-09-25). Static init: no first-use race.
	SRWLOCK						g_RenderHooksLock = SRWLOCK_INIT;

	void CallRenderHooks(bool begin)
	{
		std::vector<S_RenderHook> hooks;
		AcquireSRWLockShared(&g_RenderHooksLock);
		if (g_pRenderHooks)
			hooks = *g_pRenderHooks;
		ReleaseSRWLockShared(&g_RenderHooksLock);
		for (size_t i = 0; i < hooks.size(); ++i)
			hooks[i].fn(hooks[i].ctx, begin);
	}
	CRITICAL_SECTION	g_InterpLock;

	T_InterpMap& InterpMap()
	{
		if (!g_pInterpMap)
		{
			InitializeCriticalSection(&g_InterpLock);
			g_pInterpMap = new T_InterpMap;
		}
		return *g_pInterpMap;
	}

	Maths::HMat43 BlendMatrixImpl(const Maths::HMat43& a, const Maths::HMat43& b, F32 t)
	{
		const F32 sax = (F32)a.AX().Length(), say = (F32)a.AY().Length(), saz = (F32)a.AZ().Length();
		const F32 sbx = (F32)b.AX().Length(), sby = (F32)b.AY().Length(), sbz = (F32)b.AZ().Length();
		if (sax < 1e-6f || say < 1e-6f || saz < 1e-6f || sbx < 1e-6f || sby < 1e-6f || sbz < 1e-6f)
			return b;
		Maths::HMat43 na = a, nb = b;
		na.AX(a.AX() / sax); na.AY(a.AY() / say); na.AZ(a.AZ() / saz);
		nb.AX(b.AX() / sbx); nb.AY(b.AY() / sby); nb.AZ(b.AZ() / sbz);
		const Maths::C_Quaternion qa(na.Rotation()), qb(nb.Rotation());
		const Maths::HVec3 pos = a.AW() + (b.AW() - a.AW()) * t;
		Maths::HMat43 r(qa.ShortestSlerp(qb, t), pos);
		r.AX(r.AX() * (sax + (sbx - sax) * t));
		r.AY(r.AY() * (say + (sby - say) * t));
		r.AZ(r.AZ() * (saz + (sbz - saz) * t));
		return r;
	}

	// rotation about the local X axis (row vectors, v_world = v_local * m: Rx * m turns m about its own X)
	Maths::HMat43 RotationX(F32 a)
	{
		const F32 c = cosf(a), s = sinf(a);
		Maths::HMat43 r;
		r.AX(Maths::HVec3(1.0f, 0.0f, 0.0f));
		r.AY(Maths::HVec3(0.0f, c, s));
		r.AZ(Maths::HVec3(0.0f, -s, c));
		r.AW(Maths::HVec3(0.0f, 0.0f, 0.0f));
		return r;
	}

	// A wheel turns up to about 200 degrees per 60 Hz tick at race speed; the shortest-arc slerp of
	// BlendMatrixImpl turned it backwards whenever the turn passed 180 degrees, so rims jittered back and
	// forth. The turn about the axle is taken whole: the measured angle between the two ticks, with the
	// number of full turns that brings it nearest the rotor's spin over the tick; the rest (steering,
	// suspension, car motion) is blended as before.
	Maths::HMat43 BlendWheelImpl(const S_InterpEntry& e, F32 t)
	{
		const Maths::HMat43& a = e.prevWheel;
		const Maths::HMat43& b = e.curWheel;
		const Maths::HMat43 rel = b * a.InverseRotTrans();	// b = rel * a
		const F32 measured = atan2f((F32)rel.AY().Z(), (F32)rel.AY().Y());
		const F32 twoPi = 6.28318531f;
		const F32 angle = measured + twoPi * floorf((e.spinAngle - measured) / twoPi + 0.5f);
		const Maths::HMat43 unspun = RotationX(-angle) * b;
		return e.spinLocal * (RotationX(angle * t) * BlendMatrixImpl(a, unspun, t));
	}

	// A jump of more than 20 m in one tick is a teleport (reset, respawn): no blend. Judged on the tick's
	// final value: some destinations take a non-world value first in a tick (the blurred alloy element gets
	// its part-local matrix from C_PartList::UpdateMatrices before SynchData stores the wheel), which judged
	// on the first store made those parts never blend.
	bool CanBlend(const S_InterpEntry& e)
	{
		return e.hasPrev && (F32)(e.cur.AW() - e.prev.AW()).Length() < 20.0f;
	}

	Maths::HMat43 BlendEntry(const S_InterpEntry& e, F32 t)
	{
		if (e.spin && e.wheelHasPrev && e.wheelTick == g_InterpTick)
			return BlendWheelImpl(e, t);
		return BlendMatrixImpl(e.prev, e.cur, t);
	}

	void StoreMatrix(void* pDst, bool row43, const Maths::HMat43& m)
	{
		if (row43)
		{
			// same layout as AnimatedModel's StoreMat43(MatrixRow43*): three float4 columns (x, y, z of rows AX..AW)
			MathsStorage::MatrixRow43* dest = (MathsStorage::MatrixRow43*)pDst;
			dest->v[0] = (F32)m.AX().X(); dest->v[1] = (F32)m.AY().X(); dest->v[2]  = (F32)m.AZ().X(); dest->v[3]  = (F32)m.AW().X();
			dest->v[4] = (F32)m.AX().Y(); dest->v[5] = (F32)m.AY().Y(); dest->v[6]  = (F32)m.AZ().Y(); dest->v[7]  = (F32)m.AW().Y();
			dest->v[8] = (F32)m.AX().Z(); dest->v[9] = (F32)m.AY().Z(); dest->v[10] = (F32)m.AZ().Z(); dest->v[11] = (F32)m.AW().Z();
		}
		else
			Maths::Store(Maths::HMat44(m), (MathsStorage::MatrixRow44*)pDst);
	}

	// tickStore: the value belongs to the current update tick even when the store happens inside a
	// render (deferred update tasks flushed at the start of the frame, e.g. animated models); the blend
	// is written right away so the render already shows it. Other stores inside a render are render-
	// specific values and are left alone (overridden).
	void RecordImpl(const void* owner, void* pDst, bool row43, const Maths::HMat43& m, bool tickStore,
		const Maths::HMat43* pWheel = NULL, F32 spinRate = 0.0f)
	{
		if (!g_InterpEnabled || !pDst)
			return;
		T_InterpMap& map = InterpMap();
		EnterCriticalSection(&g_InterpLock);
		T_InterpMap::iterator it = map.find(pDst);
		if (g_InterpInRender && !tickStore)
		{
			if (it != map.end())
				it->second.overridden = true;
		}
		else if (it == map.end())
		{
			S_InterpEntry e;
			e.owner = owner; e.prev = m; e.cur = m; e.tick = g_InterpTick; e.hasPrev = false; e.written = false; e.overridden = false; e.row43 = row43;
			e.spin = pWheel != NULL; e.wheelHasPrev = false; e.wheelTick = g_InterpTick; e.spinAngle = spinRate * g_InterpTickSeconds;
			if (pWheel)
			{
				e.prevWheel = e.curWheel = *pWheel;
				e.spinLocal = m * pWheel->InverseRotTrans();
			}
			map.insert(std::make_pair(pDst, e));
		}
		else
		{
			S_InterpEntry& e = it->second;
			e.owner = owner;
			e.row43 = row43;
			if (e.tick == g_InterpTick)
				e.cur = m;
			else
			{
				const bool consecutive = (e.tick + 1 == g_InterpTick);
				e.prev = consecutive ? e.cur : m;
				e.cur = m;
				e.tick = g_InterpTick;
				e.hasPrev = consecutive;
			}
			e.spin = pWheel != NULL;	// the last store of the tick decides
			if (pWheel)
			{
				if (e.wheelTick != g_InterpTick)
				{
					e.wheelHasPrev = (e.wheelTick + 1 == g_InterpTick);
					e.prevWheel = e.wheelHasPrev ? e.curWheel : *pWheel;
					e.wheelTick = g_InterpTick;
				}
				e.curWheel = *pWheel;
				e.spinLocal = m * pWheel->InverseRotTrans();
				e.spinAngle = spinRate * g_InterpTickSeconds;
			}
			if (g_InterpInRender && CanBlend(e) && g_InterpRenderAlpha < 1.0f)
			{
				StoreMatrix(pDst, row43, BlendEntry(e, g_InterpRenderAlpha));
				e.written = true;
				e.overridden = false;
			}
		}
		LeaveCriticalSection(&g_InterpLock);
	}
}

void Blur2PCInterpEnable(bool enable)	{ g_InterpEnabled = enable; }
void Blur2PCInterpTick()				{ ++g_InterpTick; }
void Blur2PCInterpSetTickSeconds(F32 s)	{ g_InterpTickSeconds = s; }
Maths::HMat43 Blur2PCBlendMatrix(const Maths::HMat43& a, const Maths::HMat43& b, F32 t) { return BlendMatrixImpl(a, b, t); }
U32 Blur2PCInterpCurrentTick()			{ return g_InterpTick; }
F32 Blur2PCInterpRenderAlpha()			{ return g_InterpRenderAlpha; }

void Blur2PCInterpRecordStore(const void* owner, MathsStorage::MatrixRow44* pDst, const Maths::HMat43& m)
{
	RecordImpl(owner, pDst, false, m, false);
}

void Blur2PCInterpRecordWheelStore(const void* owner, MathsStorage::MatrixRow44* pDst, const Maths::HMat43& m, const Maths::HMat43& wheel, F32 spinRate)
{
	RecordImpl(owner, pDst, false, m, false, &wheel, spinRate);
}

void Blur2PCInterpRecordTickStore(const void* owner, MathsStorage::MatrixRow44* pDst, const Maths::HMat43& m)
{
	RecordImpl(owner, pDst, false, m, true);
}

void Blur2PCInterpRecordTickStore43(const void* owner, MathsStorage::MatrixRow43* pDst, const Maths::HMat43& m)
{
	RecordImpl(owner, pDst, true, m, true);
}

void Blur2PCInterpAddRenderHook(T_Blur2PCRenderHook fn, void* ctx)
{
	AcquireSRWLockExclusive(&g_RenderHooksLock);
	if (!g_pRenderHooks)
		g_pRenderHooks = new std::vector<S_RenderHook>;
	S_RenderHook h = { fn, ctx };
	g_pRenderHooks->push_back(h);
	ReleaseSRWLockExclusive(&g_RenderHooksLock);
}

void Blur2PCInterpRemoveRenderHook(T_Blur2PCRenderHook fn, void* ctx)
{
	AcquireSRWLockExclusive(&g_RenderHooksLock);
	if (g_pRenderHooks)
		for (size_t i = 0; i < g_pRenderHooks->size(); ++i)
			if ((*g_pRenderHooks)[i].fn == fn && (*g_pRenderHooks)[i].ctx == ctx)
			{
				g_pRenderHooks->erase(g_pRenderHooks->begin() + i);
				break;
			}
	ReleaseSRWLockExclusive(&g_RenderHooksLock);
}

void Blur2PCInterpForget(const void* owner)
{
	if (!g_pInterpMap)
		return;
	EnterCriticalSection(&g_InterpLock);
	for (T_InterpMap::iterator it = g_pInterpMap->begin(); it != g_pInterpMap->end(); )
	{
		if (it->second.owner == owner)
			it = g_pInterpMap->erase(it);
		else
			++it;
	}
	LeaveCriticalSection(&g_InterpLock);
}

void Blur2PCInterpBeginRender(F32 alpha)
{
	if (!g_InterpEnabled)
		return;
	g_InterpRenderAlpha = alpha;
	T_InterpMap& map = InterpMap();
	EnterCriticalSection(&g_InterpLock);
	for (T_InterpMap::iterator it = map.begin(); it != map.end(); ++it)
	{
		S_InterpEntry& e = it->second;
		e.written = false;
		e.overridden = false;
		if (!CanBlend(e) || e.tick != g_InterpTick || alpha >= 1.0f)
			continue;
		StoreMatrix(it->first, e.row43, BlendEntry(e, alpha));
		e.written = true;
	}
	g_InterpInRender = true;
	LeaveCriticalSection(&g_InterpLock);
	CallRenderHooks(true);
}

void Blur2PCInterpEndRender()
{
	if (!g_InterpEnabled)
		return;
	CallRenderHooks(false);
	g_InterpRenderAlpha = 1.0f;
	T_InterpMap& map = InterpMap();
	EnterCriticalSection(&g_InterpLock);
	g_InterpInRender = false;
	for (T_InterpMap::iterator it = map.begin(); it != map.end(); ++it)
	{
		S_InterpEntry& e = it->second;
		if (e.written && !e.overridden)
			StoreMatrix(it->first, e.row43, e.cur);
		e.written = false;
	}
	LeaveCriticalSection(&g_InterpLock);
}
#endif

// C_ModelInstance

C_ModelInstance::C_ModelInstance( const C_RenderModelData* pRenderModelData, const S32 modelIndex, const SceneGraph::C_SceneGraph* pSceneGraph ) :
	SceneGraph::C_ModelInstance(pSceneGraph, &pRenderModelData->GetScene(), pRenderModelData->GetModel(modelIndex).GetRenderListNode())
{
	if(pRenderModelData)
	{
		m_pRenderModel = &pRenderModelData->GetModel(modelIndex);
	
		MEMORY_TRACKER_PUSH("worldrender.modelinstance.materiallist");
		// copy material output data structure from render model
		if(m_pRenderListNode)
		{
			m_MaterialIndex = m_pRenderModel->GetMaterialIndex();
			m_pMaterialListData->CopyData(m_pRenderModel->GetMaterialList());
			m_pMaterialListTypes = &m_pRenderModel->GetMaterialList();
			m_pMaterialListDmaData = m_pMaterialListData->GetDmaData();
		}
		MEMORY_TRACKER_POP("worldrender.modelinstance.materiallist");

		BuildDefaultDynamicMaterialOutput();

		m_pszFriendlyName = m_pRenderModel->GetFriendlyName();
	}
	else
	{
		m_pRenderModel = NULL;
	}
}

C_ModelInstance::~C_ModelInstance()
{
#if PLATFORM_PC
	Blur2PCInterpForget(this);	// [Blur2PC]
#endif
	return;
}

void C_ModelInstance::BuildDefaultDynamicMaterialOutput()
{
	Maths::HMat43 matrix;

	matrix.Translation(Maths::HVec3(0.0f, 0.0f, 0.0f));

	for(S32 i = 0; i < m_pRenderModel->GetNumElements(); ++i)
	{
		SetElementMatrix(i, matrix);
	}

	m_pRenderModel->SetDefaultValues(m_pMaterialListData);
}

void C_ModelInstance::SetWorldSphere(const Maths::C_Sphere& sphere)
{
	if(GetCullNode())
	{
		GetCullNode()->SetSphere(sphere);
		/*if(GetCullNode()->GetType() == SceneGraph::I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_PORTAL)
		{
			SceneGraph::C_PortalCullNode* pCullNode = (SceneGraph::C_PortalCullNode*)GetCullNode();
			assert(pCullNode);	// Sanity check.

			// [Oscar] temporary workaround until we get an official interface for disabling portal culling on specific model instances...
			if (pCullNode->GetSpatialProbeCount() > 0)
				pCullNode->SetSpatialProbeAtIndex(0, sphere.Centre());
		}

		// Flag the movement of the node to the SG.
		FlagMovement();*/
	}
	else
	{
		// Stop spam & slowdown:
		static S32 count = 0;
		if (count++ < 10)
			LOGWARN(g_Log, "ModelInstance: SetWorldSphere called but object not in SceneGraph");
	}
}

void C_ModelInstance::SetWorldSpherePosition(const Maths::HVec3& position)
{
	if(GetCullNode())
	{
		GetCullNode()->SetSpherePosition(position);
		/*if(GetCullNode()->GetType() == SceneGraph::I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_PORTAL)
		{
			SceneGraph::C_PortalCullNode* pCullNode = (SceneGraph::C_PortalCullNode*)GetCullNode();
			assert(pCullNode);	// Sanity check.
			pCullNode->SetSpatialProbeAtIndex(0, position);
		}

		// Flag the movement of the node to the SG.
		FlagMovement();*/
	}
	else
	{
		// Stop spam & slowdown:
		static S32 count = 0;
		if (count++ < 10)
			LOGWARN(g_Log, "ModelInstance: SetWorldSphere called but object not in SceneGraph");
	}
}

Maths::C_Sphere C_ModelInstance::GetWorldSphere() const
{
	if(m_pCullNode)
		return m_pCullNode->GetSphere();
	else
		return Maths::C_Sphere();
}

void C_ModelInstance::SetNearFarDistance(const F32 nearDistance, const F32 farDistance)
{
	if(m_pCullNode)
		m_pCullNode->SetNearFarDistance(nearDistance, farDistance);
	else
		LOGWARN(g_Log, "ModelInstance: SetNearFarDistance called but object not in SceneGraph");
}


void C_ModelInstance::SetElementMatrix( const S32 elementIndex, const Maths::HMat43 PCREF m )
{
#if PLATFORM_PC
	if (g_InterpEnabled && m_pRenderModel && m_pMaterialListData)
	{
		// [Blur2PC] render interpolation, recorded per destination matrix (a write to all elements
		// records each element, so a later write to one element owns that element)
		const S32 first = elementIndex < 0 ? 0 : elementIndex;
		const S32 last = elementIndex < 0 ? m_pRenderModel->GetNumElements() - 1 : elementIndex;
		for (S32 j = first; j <= last && j < m_pRenderModel->GetNumElements(); ++j)
		{
			const MaterialParameter* element = m_pRenderModel->GetWorldMatrixParameter(j);
			if (element != NULL && element->index >= 0 && element->index < m_pMaterialListData->GetNumMaterials())
			{
				RenderingData::Material& material = m_pMaterialListData->GetMaterial(element->index);
				Blur2PCInterpRecordStore(this, (MathsStorage::MatrixRow44*)material.GetData(element->offset), m);
			}
		}
	}
#endif
	WriteElementMatrix(elementIndex, m);
}

#if PLATFORM_PC
// [Blur2PC] SetElementMatrix for a model that spins with a wheel (see Blur2PCInterpRecordWheelStore)
void C_ModelInstance::Blur2PCSetWheelElementMatrix( const S32 elementIndex, const Maths::HMat43 PCREF m, const Maths::HMat43 PCREF wheel, const F32 spinRate )
{
	if (g_InterpEnabled && m_pRenderModel && m_pMaterialListData)
	{
		const S32 first = elementIndex < 0 ? 0 : elementIndex;
		const S32 last = elementIndex < 0 ? m_pRenderModel->GetNumElements() - 1 : elementIndex;
		for (S32 j = first; j <= last && j < m_pRenderModel->GetNumElements(); ++j)
		{
			const MaterialParameter* element = m_pRenderModel->GetWorldMatrixParameter(j);
			if (element != NULL && element->index >= 0 && element->index < m_pMaterialListData->GetNumMaterials())
			{
				RenderingData::Material& material = m_pMaterialListData->GetMaterial(element->index);
				Blur2PCInterpRecordWheelStore(this, (MathsStorage::MatrixRow44*)material.GetData(element->offset), m, wheel, spinRate);
			}
		}
	}
	WriteElementMatrix(elementIndex, m);
}
#endif

// [Blur2PC] the original SetElementMatrix body: stores the matrix into the material data
void C_ModelInstance::WriteElementMatrix( const S32 elementIndex, const Maths::HMat43 PCREF m )
{
	if(!m_pRenderModel)
		return;

	Maths::HMat44 mat44(m);

	if(elementIndex < 0)
	{
		for(S32 j = 0, count = m_pRenderModel->GetNumElements(); j < count; ++j)
		{
			const MaterialParameter* element = m_pRenderModel->GetWorldMatrixParameter(j);
			
			if (element != NULL)
			{
				if (element->index >= 0)
				{
					RenderingData::Material& material = m_pMaterialListData->GetMaterial(element->index);
					Maths::Store(mat44, (MathsStorage::MatrixRow44A16*)material.GetData(element->offset));
				}
			}			
		}
	}
	else
	{
		if(elementIndex < m_pRenderModel->GetNumElements())
		{
			const MaterialParameter* element = m_pRenderModel->GetWorldMatrixParameter(elementIndex);

			if (element != NULL)
			{
				if(element->index >= 0 && element->index < m_pMaterialListData->GetNumMaterials())
				{
					RenderingData::Material& material = m_pMaterialListData->GetMaterial(element->index);
					Maths::Store(mat44, (MathsStorage::MatrixRow44A16*)material.GetData(element->offset));
				}
			}
		}
	}
}

Maths::HMat43 C_ModelInstance::GetElementMatrix( const S32 elementIndex ) const
{
	Maths::HMat43 mat;
	mat.SetIdentity();
	return mat;
}

void C_ModelInstance::SetParameter(const MaterialParameter& parameter, const void* pData)
{
	if (parameter.index < 0 || parameter.index >= m_pMaterialListData->GetNumMaterials())
		return;
	RenderingData::Material& material = m_pMaterialListData->GetMaterial(parameter.index);
	material.SetData(parameter.offset, pData, parameter.size);
}

U8* C_ModelInstance::GetParameter(const MaterialParameter& parameter)
{
	if (parameter.index < 0 || parameter.index >= m_pMaterialListData->GetNumMaterials())
		return 0;

	RenderingData::Material& material = m_pMaterialListData->GetMaterial(parameter.index);
	return material.GetData(parameter.offset);
}

const U8* C_ModelInstance::GetParameter(const MaterialParameter& parameter) const
{
	RenderingData::ConstMaterialReference material = m_pMaterialListData->RenderingData::MaterialListData::GetMaterial(parameter.index);
	return material.GetData(parameter.offset);
}

S32 C_ModelInstance::GetNumElements() const
{
	if(m_pRenderModel)
		return m_pRenderModel->GetNumElements();
	else
		return 0;
}	

void C_ModelInstance::AccumulateResourceUsage(RenderModelResourceUsage& ioResourceUsage) const
{
	RenderingData::Target::TextureInfoList renderingData;
	GetScene()->AccumulateTextureInfo(renderingData);
	ioResourceUsage.AccumulateRenderingDataTextures(renderingData);
}

} // namespace WorldRender
