
#include "stdafx.h"

#include <system/serialise/C_FileBlock.h>
#include <world_render/WorldRender.h>
#include <world_render/RenderLevel.h>
#include <system/debug/I_DebugRender.h>
#include <scenegraph/view.h>
#include <renderingdata/renderingdata.h>
#include <system/serialise/BinarySerialise.h>
#include <Portal/DeferredTracerFunctorBase.h>
#include <system/Memory.h>
#include <FX/RenderMutex.h>

Debug::Int g_debugGroupIndex("Graphics.WorldRender.GroupIndex", -1);
Debug::Bool g_SceneGraphFreezePvs("Graphics.WorldRender.Pvs.FreezePvs", false );

//#pragma optimize("", off)

namespace WorldRender
{

enum E_LevelRenderFileVersion
{
	E_LevelRenderFileVersion_Initial = 0,
	E_LevelRenderFileVersion_EffectVariation,

	E_LevelRenderFileVersion_Current = E_LevelRenderFileVersion_EffectVariation
};

// C_RenderLevel

C_RenderLevel::C_RenderLevel() : m_pvsCell(-1), m_futurePvsCell(-1)
{
}

C_RenderLevel::~C_RenderLevel()
{
}

void C_RenderLevel::OnLoad()
{
	m_pScene->OnLoad();
}

void C_RenderLevel::RenderingDataStreamerLoadAllRequested()
{
	m_pScene->GetStreamData().LoadAllRequested();
	m_pScene->UpdateResourcePointers();
}

bool C_RenderLevel::Read( Sys::I_File* pFile )
{
	bool success = true;

	RESOURCE_MEMORY_ID("LevelData");

	const U32 prevFileVersion = pFile->GetVersionNumber();
	U32 fileVersion = E_LevelRenderFileVersion_Initial;

	{
		Sys::C_FileBlock rootBlock( pFile );
		if( rootBlock.HeaderId() != "Render" )
		{
			LOGWARN( g_Log, "Expected a 'Render' Block" );
			return false;
		}

		// keep reading child blocks
		while ( rootBlock.GetPosition() < rootBlock.Length() )
		{
			Sys::C_FileBlock block( &rootBlock );

			if( block.HeaderId() == "Header" )
			{
				Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
				ar >> fileVersion;
				pFile->SetVersionNumber( fileVersion );

				if( fileVersion >= E_LevelRenderFileVersion_EffectVariation )
				{
					ar >> m_EffectVariation;
				}
			}
			else			
			if( block.HeaderId() == "Scene" )
			{	
				MEMORY_TRACKER_PUSH_SUB("scene");
 				ReadScene( &block, m_pScene );
				MEMORY_TRACKER_POP_SUB("scene");
			}
			else
			if( block.HeaderId() == "Lights" )
			{	
				MEMORY_TRACKER_PUSH_SUB("lights");
				m_LightList.Read( &block );
				MEMORY_TRACKER_POP_SUB("lights");
			}
			else
			if( block.HeaderId() == "LS" )
			{			
				MEMORY_TRACKER_PUSH_SUB("lightsamples");
				m_LightSamplePoints.Read( &block );
				MEMORY_TRACKER_POP_SUB("lightsamples");
			}
			else
			if( block.HeaderId() == "PVSData" )
			{		
				MEMORY_TRACKER_PUSH_SUB("pvs");
				Arch::BinaryInputArchive ar(block );
				ar >> m_PVSData;
				MEMORY_TRACKER_POP_SUB("pvs");
			}
			else
			if( block.HeaderId() == "CubePt2")
			{
				MEMORY_TRACKER_PUSH_SUB("cubepts");
				Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(block );
				ar >> m_CubemapData;
				MEMORY_TRACKER_POP_SUB("cubepts");
			}
			else 
			if( block.HeaderId() == "Portals" )
			{		
				MEMORY_TRACKER_PUSH_SUB("portals");
				m_PortalSystem.LoadFromFile(block);
				MEMORY_TRACKER_POP_SUB("portals");
			}
			else
			if( block.HeaderId() == "Crowd" )
			{
				MEMORY_TRACKER_PUSH_SUB("crowd");
				m_CrowdData.Read( &block );
				MEMORY_TRACKER_POP_SUB("crowd");
			}
			else
			{
				LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
			}
		}
	}

	pFile->SetVersionNumber( prevFileVersion );

	return true;
}

Debug::Int g_chunk("Graphics.WorldRender.Debug.Chunk", -1, Debug::Range(0, 100));

void C_RenderLevel::UpdateView(const SceneGraph::C_View& view)
{
	m_PortalSystem.Evaluate(view.GetVisiblePortalRoomSet(), view.GetVisibleRoomList(), view.GetViewSettings3D());
	m_DebugPortalBitField = view.GetVisiblePortalRoomSet();

	if (!g_SceneGraphFreezePvs)
	{
		m_pvsCell = m_PVSData.GetCell(view.GetViewSettings3D().GetCameraPosition());

		if (view.UseFutureCamera())
			m_futurePvsCell = m_PVSData.GetCell(view.GetFutureCamera().AW());
	}

	// todo: set cell per loaded level
	view.SetPvsCell(m_pvsCell, m_futurePvsCell > -1 ? m_futurePvsCell : m_pvsCell);
}

bool C_RenderLevel::UpdateStreaming(RenderingData::Target::RequestInfo* pRequests, S32 nRequests)
{
	PERF_EVENT("Update.Streamer");

#ifndef CFG_GOLD
	Time::C_Time startTime = Time::GetPerformanceCounter();
#endif

	RenderingData::Target::StreamData& streamData = m_pScene->GetStreamData();
	streamData.Request(pRequests, nRequests);

	//streamData.LoadAllRequested();
	bool bStreamDataStillLoading = streamData.UpdateMain();

	if (streamData.BlocksChanged() || streamData.CacheChanged())
		streamData.DebugInfo(); 

	if (streamData.NodesChanged())
		int breakhere = 0;

	m_pScene->UpdateResourcePointers();

#ifndef CFG_GOLD
	float totalTime = (1000 * (Time::GetPerformanceCounter() - startTime)) / (float)Time::GetPerformanceFrequency();
	Perf::AddNamedCounter((F32)totalTime,	"UpdateStreaming - TotalTimeInMs");

	RenderingData::Target::StreamData::DebugInfoStats stats;
	GetScene().GetStreamData().DebugInfo(stats);
	S32 sizeRequested	= stats.bytesRequested / (1024*1024);
	S32 sizeLoaded		= stats.bytesLoaded / (1024*1024);

	Perf::AddNamedCounter((S32)sizeRequested,	"UpdateStreaming - Requested");
	Perf::AddNamedCounter((S32)sizeLoaded,	"UpdateStreaming - Loaded");
#endif

	return bStreamDataStillLoading;
}

bool C_RenderLevel::NodesChanged() const
{
	return m_pScene->GetStreamData().NodesChanged();
}
	
bool C_RenderLevel::InitStreaming( const Util::C_String& streamFileName )
{
	bool streamFileExists = false;
	if (Sys::GetDefaultFileSystem()->FileExists( streamFileName ))
	{
		streamFileExists = true;
	}

	if (m_pScene->GetStreamData().GetNumChunks() > 0 && !streamFileExists)
		LOGERROR(g_Log, "Stream file missing: %s", streamFileName.Chars());

	if( streamFileExists )
	{
		// Open the stream file
		RenderingData::Target::StreamData& streamData = m_pScene->GetStreamData();
		streamData.OpenStreamFile( streamFileName.Chars() );

		// Load everything
		//streamData.RequestAll();
		//streamData.LoadAllRequested();
	}

	BuildModelsStart();
	BuildModelsEnd();

	if( streamFileExists )
	{
		// After streaming, update the scene graph and resource pointers
		//m_pStaticScene->Build();
		m_pScene->UpdateResourcePointers();
	}

	return true;
}

void C_RenderLevel::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m, const Maths::HVec3 PCREF vCamera )
{
	m_LightList.DebugRender(pDebugRender, m, vCamera);
	//m_LightSamplePoints.DebugRender(pDebugRender, m, mCamera);

	m_CubemapData.DebugRender(pDebugRender, vCamera);

	m_PVSData.DebugRender(vCamera);
	RenderingData::Target::StreamData& streamData = m_pScene->GetStreamData();
	streamData.DisplayDebug();
	
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	Btl::BitVector bitField;
	m_PortalSystem.DebugEvaluate(bitField, viewSettings3D, pDebugRender);
	m_PortalSystem.DebugRender(pDebugRender, m, m_DebugPortalBitField);
	return;
}

} // namespace WorldRender
