#ifndef LIGHTSAMPLEPOINTSCENE_H
#define LIGHTSAMPLEPOINTSCENE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <btl/Containers/List.h>

#include <world_render/LightSamplePoints.h>

namespace DbUtil{ class I_DebugRender;};

namespace RenderingData
{
namespace Target
{
struct SharedTextureData;
}
}

namespace WorldRender
{

class C_Scene;
class C_LightSamplePointScene;

//! C_LightSampler
class C_LightSampler : public Btl::ListElementDouble<C_LightSampler>
{
public:
	C_LightSampler();
	~C_LightSampler();

	void Initialise( C_LightSamplePointScene* pLightSamplePointScene );

	const Maths::HVec3& GetPosition() const;
	void SetPosition(const Maths::HVec3& position);
	void Update();
	const C_LightSampleData& GetLightSampleData() const;
	void Reset();

private:
	Maths::HVec3				m_Position;
	bool						m_FirstUpdate;
	C_LightSampleData			m_NearestLightSampleData;
	C_LightSamplePointScene*	m_pLightSamplePointScene;
	C_LightSampleData			m_CurrentLightSampleData;
};

typedef Btl::Set<const C_LightSamplePoints*> C_LightSamplePointList;
typedef Btl::List<C_LightSampler> C_LightSamplerList;

//! C_LightSamplePointScene
class C_LightSamplePointScene
{
friend class C_LightSampler;
public:
	C_LightSamplePointScene(C_Scene *);
	~C_LightSamplePointScene();

	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HVec3 PCREF vCamera );
	bool GetClosestSample( C_LightSampleData& result, const Maths::HVec3 PCREF vPosition );
	bool GetClosestSamplePosition( Maths::HVec3& vResult, const Maths::HVec3 PCREF vPosition );
	RenderingData::Target::SharedTextureData *GetClosestCubemap( const Maths::HVec3 PCREF vPosition );

	void AddLightSamplePoints( C_LightSamplePoints* pLightSamplePoints );
	void RemoveLightSamplePoints( C_LightSamplePoints* pLightSamplePoints );

	const C_LightSamplePointList& GetLightSamplePointsList() const;

	const C_LightSampleData& GetDefaultLightSampleData() const { return m_DefaultLightSampleData; }
	void SetDefaultLightSampleData( const C_LightSampleData& lightSampleData ) { m_DefaultLightSampleData = lightSampleData; }

private:
	void AddLightSampler( C_LightSampler* pLightSampler );
	void RemoveLightSampler( C_LightSampler* pLightSampler );

private:
	C_LightSamplePointList m_LightSamplePointsList;
	C_LightSamplerList m_LightSamplerList;
	C_Scene *m_pScene;

	C_LightSampleData m_DefaultLightSampleData;
};



} // namespace WorldRender

#endif // WORLD_RENDER_LEVEL_H

