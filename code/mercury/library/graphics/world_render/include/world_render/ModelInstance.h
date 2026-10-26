#ifndef WORLD_RENDER_MODEL_INSTANCE_H
#define WORLD_RENDER_MODEL_INSTANCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/RenderingData/ModelInstanceNode.h>

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace WorldRender
{

struct MaterialParameter;
struct RenderModelResourceUsage;
class C_RenderModelData;
class C_Model;

// [Blur2PC] render interpolation helper: blends two affine matrices (slerp rotation, lerp position and scale)
Maths::HMat43 Blur2PCBlendMatrix(const Maths::HMat43& a, const Maths::HMat43& b, F32 t);
U32 Blur2PCInterpCurrentTick();		// [Blur2PC] update tick counter (advances once per decoupled update)
F32 Blur2PCInterpRenderAlpha();		// [Blur2PC] render fraction during a render, 1 otherwise
// [Blur2PC] records a world matrix the game has just stored at pDst (inside owner's material data)
// so renders between updates can blend it; see ModelInstance.cpp
void Blur2PCInterpRecordStore(const void* owner, MathsStorage::MatrixRow44* pDst, const Maths::HMat43& m);
// [Blur2PC] same, for a store that belongs to the current update tick even when issued inside a render
// (deferred update tasks flushed at frame start, e.g. animated models); the blend is written immediately
void Blur2PCInterpRecordTickStore(const void* owner, MathsStorage::MatrixRow44* pDst, const Maths::HMat43& m);
// [Blur2PC] Blur2PCInterpRecordStore for a part that spins with a wheel: m is the part's matrix, wheel the
// wheel matrix it follows (orthonormal, the axle is its local X), spinRate the rotor's turn about that axle
// in radians per second; a render between ticks then turns the part the whole way it spun
void Blur2PCInterpRecordWheelStore(const void* owner, MathsStorage::MatrixRow44* pDst, const Maths::HMat43& m, const Maths::HMat43& wheel, F32 spinRate);
void Blur2PCInterpSetTickSeconds(F32 seconds);	// [Blur2PC] game time the update about to run covers
void Blur2PCInterpRecordTickStore43(const void* owner, MathsStorage::MatrixRow43* pDst, const Maths::HMat43& m);
void Blur2PCInterpForget(const void* owner);
// [Blur2PC] render hooks: called with begin=true after the blend pass that starts a render, and with
// begin=false before the true values are restored. For game objects that keep render positions outside
// material data (light flares, point lights) and move them to the render pose for the frame.
typedef void (*T_Blur2PCRenderHook)(void* ctx, bool begin);
void Blur2PCInterpAddRenderHook(T_Blur2PCRenderHook fn, void* ctx);
void Blur2PCInterpRemoveRenderHook(T_Blur2PCRenderHook fn, void* ctx);

class C_ModelInstance : public SceneGraph::C_ModelInstance
{
public:
	C_ModelInstance( const C_RenderModelData* pRenderModelData, const S32 modelIndex, const SceneGraph::C_SceneGraph* pSceneGraph );
	~C_ModelInstance();

	void				SetWorldSphere( const Maths::C_Sphere& sphere);
	void				SetWorldSpherePosition( const Maths::HVec3& position);
	Maths::C_Sphere		GetWorldSphere() const;
	void				SetNearFarDistance(const F32 nearDistance, const F32 farDistance);

	// setting and getting of 
	void				SetElementMatrix( const S32 elementIndex, const Maths::HMat43 PCREF m );
	void				WriteElementMatrix( const S32 elementIndex, const Maths::HMat43 PCREF m );	// [Blur2PC] raw store, no interpolation bookkeeping
#if PLATFORM_PC
	void				Blur2PCSetWheelElementMatrix( const S32 elementIndex, const Maths::HMat43 PCREF m, const Maths::HMat43 PCREF wheel, const F32 spinRate );	// [Blur2PC] see Blur2PCInterpRecordWheelStore
#endif
	Maths::HMat43		GetElementMatrix( const S32 elementIndex ) const;

	// set data to model parameter
	void				SetParameter(const MaterialParameter& parameter, const void* pData);
	U8*					GetParameter(const MaterialParameter& parameter);
	const U8*			GetParameter(const MaterialParameter& parameter) const;

	S32					GetNumElements() const;
	const C_Model*		GetRenderModel() const { return m_pRenderModel; }

	void				AccumulateResourceUsage(RenderModelResourceUsage& ioResourceUsage) const;

private:
	void				BuildDefaultDynamicMaterialOutput();
	
	const C_Model*		m_pRenderModel;

	// other data
};

} // namespace WorldRender


#endif // WORLD_RENDER_MODEL_INSTANCE_H

