#ifndef WORLD_RENDER_LEVEL_H
#define WORLD_RENDER_LEVEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <world_render/RenderModeldata.h>
#include <btl/Containers/vector.h>
#include <misc_shared/kdtree/kdtree.h>
#include <world_render/LightList.h>
#include <world_render/LightSamplePointScene.h>
#include <world_render/pvs.h>
#include <world_render/CubemapPoints.h>
#include <Portal/System.h>
#include <Game_Resource/CrowdData.h>

namespace DbUtil{ class I_DebugRender;};

namespace SceneGraph { class C_View; }

namespace RenderingData 
{
	namespace Target 
	{
		class RequestInfo;
	}
}

namespace WorldRender
{

//! C_RenderLevel
class C_RenderLevel : public C_RenderModelData
{
public:
	C_RenderLevel();
	~C_RenderLevel();
	
	void OnLoad();
	void RenderingDataStreamerLoadAllRequested();
	bool Read( Sys::I_File* pFile );
	bool InitStreaming( const Util::C_String& streamFileName );

	void UpdateView(const SceneGraph::C_View& view);
	bool UpdateStreaming(RenderingData::Target::RequestInfo* pRequests, S32 nRequests);
	bool NodesChanged() const;

	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m, const Maths::HVec3 PCREF vCamera );

	const C_LightList& GetLightList() const
	{
		return m_LightList;
	};

	C_LightSamplePoints& GetLightSamplePoints()
	{
		return m_LightSamplePoints;
	};

	const C_LocalisedCubemapData& GetCubemapData() const
	{
		return m_CubemapData;
	}

	const Util::C_String& GetEffectVariation() const
	{
		return m_EffectVariation;
	};

	const Portal::C_System&		GetPortalSystem() const
	{
		return m_PortalSystem;
	}

	const GameResource::C_CrowdData& GetCrowdData() const
	{
		return m_CrowdData;
	}

	const Pvs::Target::PvsData& GetPVSData() const
	{
		return m_PVSData;
	}
private:
	//Light Sample Points
	C_LightSamplePoints m_LightSamplePoints;
	//LightList information
	C_LightList m_LightList;
	//PVS Data
	Pvs::Target::PvsData m_PVSData;
	// Localised cubemap data
	C_LocalisedCubemapData m_CubemapData;
	S32 m_pvsCell;
	S32 m_futurePvsCell;
	Portal::C_System m_PortalSystem;
	Util::C_String m_EffectVariation;
	Btl::BitVector m_DebugPortalBitField;
	GameResource::C_CrowdData m_CrowdData;
};


} // namespace WorldRender

#endif // WORLD_RENDER_LEVEL_H

