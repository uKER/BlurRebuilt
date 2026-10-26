#include "GameResourcePch.h"
#include <system/debug/Blur2PCAutomation.h>
//#include <system/time/scopedtimer.h>

#include <system/serialise/C_FileBlock.h>
#include <system/debug/metadebug/MetaDebug.h>
#include <system/debug/metadebug/DefaultMetaDebugNotify.h>

#include <gravity/importer/Importer.h>
#include <world_render/RenderModel.h>

#include <game_resource/GameResource.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <game_resource/ModelTypeManager.h>
#include <game_resource/EffectVariations.h>
#include <system/Memory.h>

#include <renderingdata/DataTypes.h>
#include <renderingdata/EffectVariationCallback.h>
#include <renderingdata/reloadabletextures.h>

#include <fx/fx.h>
#include <fx/RenderMutex.h>
#include <gravity/sdk/PhysicsSDK.h>

#define RENDERINGDATA_ALLOW_INCLUDE // only for ModelResource.cpp

#include <renderingdata/renderingdata.h>
//#pragma optimize("", off)

namespace GameResource
{

extern Debug::Bool g_StoreAllMetadebug;

Debug::Int g_MaxModelWarningMessages( "Gamesys.GameResource.MaxModelWarningMessages", 0 );

enum E_ModelFileVersion
{
	E_ModelFileVersion_Initial = 0,

	E_ModelFileVersion_Current = E_ModelFileVersion_Initial
};

// C_ModelData

C_ModelResource::C_ModelResource()
	: m_pPhysicsImport(NULL)
	, m_pModelData(NULL)
	, m_pRenderModel(NULL)
	, m_EvictDelay(0)
{
}

C_ModelResource::~C_ModelResource()
{
}

bool C_ModelResource::LoadResourceData()
{
	// Strip any characters after the pipe symbol in the filename - NOTE: I still dont like this and think this should be baked into the model in the converter - PaulM
	const Sys::C_FilePath& resourceFilename = GetResourceFilename();
	const Sys::C_FilePath* pFilename;
	Sys::C_FilePath strippedFilename;
	Util::C_String modelParamType;

	S32 lodLevel = INT_MAX;
	const S32 pipePos = resourceFilename.String().Search('|');
	if(pipePos == -1)
	{
		pFilename = &resourceFilename;
	}
	else
	{		
		const S32 pipePos2 = resourceFilename.String().RevSearch('|');
		if (pipePos != pipePos2)
		{
			Util::C_String lodLevelStr = resourceFilename.String()( pipePos2+1, resourceFilename.Length() ).Chars();
			lodLevel = atoi(lodLevelStr.Chars());								
			modelParamType = resourceFilename.String()( pipePos+1, pipePos2 ).Chars();
		}
		else
		{			
			modelParamType = resourceFilename.String()( pipePos+1, resourceFilename.Length() ).Chars();			
		}

		strippedFilename = resourceFilename.String()(0, pipePos);
		pFilename = &strippedFilename;
	}

	Sys::I_File* pFile = OpenResourceFile( *pFilename );

	if( !pFile )
		return false;

	bool success = LoadModelData( pFile, modelParamType, lodLevel );

#if !defined(CFG_GOLD) && !defined(CFG_PROFILE)
	if(success)
	{
		if(pFile->GetPosition() < pFile->Length())
		{
			Sys::C_FileBlock metaDebugBlock( pFile );
			if( metaDebugBlock.HeaderId() != "MtaDbg" )
			{
				LOGWARN( g_Log, "Expected a 'MtaDbg' Block" );
			}
			else
			{
				Util::C_String logHeader = Util::C_String("Begin Model Export Log (section '%n') from file '") + pFile->Name().Chars() + "':";
				Util::C_String logFooter = Util::C_String("End Model Export Log (section '%n') from file '") + pFile->Name().Chars() + "':";
				Util::C_String logMessage = Util::C_String("   > %c");

				DebugLevel::Level filter=DebugLevel::Info;
				bool storeLogs = false;

				if(g_StoreAllMetadebug)
				{
					filter = DebugLevel::Debug;
					storeLogs = true;
				}

				MetaDebug::C_DefaultMetaDebugNotify notify(filter, g_MaxModelWarningMessages, logMessage.Chars(), logHeader.Chars(), logFooter.Chars(), storeLogs);
				MetaDebug::C_MetaNode* pRootMetaNode = new MetaDebug::C_MetaNode("");
				LOGDEBUG(MetaDebug::g_Log, "MetaDebug for %s", GetResourceFilename().Chars() );
				pRootMetaNode->Read( &metaDebugBlock, (MetaDebug::I_MetaDebugNotify*)&notify );
				pRootMetaNode->Dump();
				m_pModelData->SetRootMetaNode(pRootMetaNode);
			}
		}
	}
#endif

	SAFE_DELETE_PTR( pFile );

	return success;
}

static Btl::String ApplyVariation(const char *psz)
{
	return EffectVariations::Instance().ApplyVariation(psz);
}

bool C_ModelResource::PostLoadResourceData()
{
	bool status = false;
	if (FX::TrySerialiseMutex())
	{
		{
			//SCOPED_TIMER("		 C_ModelResource::PostLoadResourceData 1");
			m_pRenderModel->OnLoad(&ApplyVariation);
		}
		{
			//SCOPED_TIMER("		 C_ModelResource::PostLoadResourceData 2");
			m_pRenderModel->BuildModelsEnd();
		}
		FX::ReleaseSerialiseMutex();
		status = true;
	}

	return status;
}

bool C_ModelResource::PreFreeResourceData()
{
	//m_pRenderModel->GetScene().DoSomething();
	return true;
}

bool C_ModelResource::FreeResourceData()
{
	SAFE_DELETE_PTR(m_pModelData);
	SAFE_DELETE_PTR(m_pRenderModel);
	//SAFE_DELETE_PTR(m_pPhysicsImport);
	Gravity::C_PhysicsSDK::PhysicsSDK()->ReleaseImportScene(m_pPhysicsImport);
	return true;
}

S32 C_ModelResource::GetEvictDelay()
{
	return m_EvictDelay;
}

void C_ModelResource::SetEvictDelay(const S32 evictDelay)
{
	m_EvictDelay = evictDelay;
}

Util::C_String C_ModelResource::StaticGetResourceTypeName()
{
	return "Model";
}

bool C_ModelResource::LoadModelData( Sys::I_File* pFile, const Util::C_String& paramType, S32 lodLevel )
{
	const U32 prevFileVersion = pFile->GetVersionNumber();
	U32 fileVersion = E_ModelFileVersion_Initial;
	
	if( !ValidatePlatformId( pFile ) )
	{
		LOGERROR( g_Log, "Wrong Platform %s", pFile->Name().Chars());
		return false;
	}

	Sys::C_FileBlock rootBlock( pFile );
	if( rootBlock.HeaderId() != "Model" )
	{
		LOGWARN( g_Log, "Expected a 'Model' Block" );
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
		}
		else			
		if( block.HeaderId() == "MdlDat" )
		{
			MEMORY_TRACKER_PUSH_SUB("modeldata");
			m_pModelData = new C_ModelData();
			bool success = m_pModelData->Read( &block );
			MEMORY_TRACKER_POP_SUB("modeldata");
			if(!success)
			{
				FreeResourceData();
				return false;
			}
		}
		else			
		if( block.HeaderId() == "Coll" )
		{
			MEMORY_TRACKER_PUSH_SUB("gravity");
			m_pPhysicsImport = Gravity::C_PhysicsSDK::PhysicsSDK()->CreateImportScene();;
			bool success = m_pPhysicsImport->Import( &block );
			MEMORY_TRACKER_POP_SUB("gravity");
			if(!success)
			{
				FreeResourceData();
				return false;
			}
		}
		else
		if( block.HeaderId() == "Render" )
		{
			MEMORY_TRACKER_PUSH_SUB("render");
			m_pRenderModel = new WorldRender::C_RenderModel;
			m_pRenderModel->GetScene().SetMaxLodToLoad(lodLevel);
			bool success = m_pRenderModel->Read( &block, paramType );
			MEMORY_TRACKER_POP_SUB("render");
			if(!success)
			{
				FreeResourceData();
				return false;
			}
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
		}
	}

	if(m_pModelData)
	{
		m_pModelData->m_pPhysicsImport = m_pPhysicsImport;
		m_pModelData->Dump();
	}

	if (m_pRenderModel)
	{
		{
			RenderingData::Target::Scene& scene = m_pRenderModel->GetScene();
			RenderingData::Node& root = scene.GetRootNode();
			for (S32 child = 0; child < root.GetNumSubNodes(); ++child)
			{
				const RenderingData::ResourceIndex index = root.GetSubNode(child);
				if (!scene.GetNodePtr(index))
					Blur2PCAutomation::LogF("DX11 model %s lod=%d missing root child %d block=%d index=%d",
						m_Filename.Chars(), lodLevel, child, index.GetBlock(), index.GetIndex());
			}
		}
		m_pRenderModel->BuildModelsStart();

		g_ModelTypeManager.OnLoad(paramType, this);

		//m_pRenderModel->BuildModelsEnd();
	}

	SetDebugNames();

	pFile->SetVersionNumber( prevFileVersion );
	return true;
}

void C_ModelResource::SetDebugNames()
{
	m_pRenderModel->GetScene().SetDebugName( m_Filename.Chars() );

	// set friendly names for each model inside the resource. this is so we can print out the names of model instances inside the game
	for (int i=0; i<m_pRenderModel->GetNumModels(); i++)
	{
		const char *psz = m_pModelData->GetModel(i)->GetName();
		if (stricmp(psz, "DefaultModel"))
			m_pRenderModel->GetModel(i).SetFriendlyName(psz);
		else
		{
			const char *psz = m_Filename.Chars();
			const char *psz2 = strrchr(psz, '\\');
			if (!psz2)
				psz2 = strrchr(psz, '/');
			if (!psz2)
				psz2 = psz;
			else
				psz2++;
			m_pRenderModel->GetModel(i).SetFriendlyName(psz2);
		}
	}
}

WorldRender::C_RenderModelData* C_ModelResource::GetRenderModelData()
{
	return (WorldRender::C_RenderModelData*)m_pRenderModel;
}

const WorldRender::C_RenderModelData* C_ModelResource::GetRenderModelData() const
{
	return (const WorldRender::C_RenderModelData*)m_pRenderModel;
}

GameResource::C_ModelData* C_ModelResource::GetModelData()
{
	return m_pModelData;
}

const GameResource::C_ModelData* C_ModelResource::GetModelData() const
{
	return m_pModelData;
}

MetaDebug::C_MetaNode* C_ModelResource::GetRootMetaNode() const
{
#ifdef CFG_GOLD
	return NULL;
#else
	return	m_pModelData->GetRootMetaNode();
#endif
}

void C_ModelType::OnLoad( GameResource::C_ModelResource* p_pModelResource )
{
	GameResource::C_ModelData *pModelDesc = p_pModelResource->GetModelData();
	WorldRender::C_RenderModelData *pGRModel = p_pModelResource->GetRenderModelData();

	// for each model inside the file
	for (int iModel = 0; iModel < pModelDesc->GetModelCount(); iModel++)
	{
		GameResource::C_Model *pModel = pModelDesc->GetModel(iModel);
		WorldRender::C_Model *pWRModel = &p_pModelResource->GetRenderModelData()->GetModel(iModel);

		pWRModel->ExposeParameter("g_SHNode", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
		pWRModel->ExposeParameter("g_tEnvironment", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
	}
}

void RegisterDefaultModelType()
{
	GameResource::g_ModelTypeManager.RegisterModelType("", new GameResource::C_ModelType);
}

void RefreshModelTextures()
{
	static S32 prevReloadCount = 0;
	static U32 prevFXReloadCount = 0;
	S32 currReloadCount = RenderingData::Target::ReloadableTextureManager::Instance().GetReloadCount();
	U32 currFXReloadCount = FX::GetReloadCount();

	if(prevReloadCount == currReloadCount && currFXReloadCount == prevFXReloadCount)
		return;

	prevReloadCount = currReloadCount;
	prevFXReloadCount = currFXReloadCount;

	// get list of all model resources
	Resource::C_HandleList resHandleList;
	Resource::C_Manager::GetInstance()->GetHandleList( resHandleList, C_ModelResource::StaticGetResourceTypeId(), Resource::E_State_Ready );

	for(S32 i=0; i<resHandleList.Size(); i++)
	{
		C_ModelResource* pModelResource = (C_ModelHandle)resHandleList[i];

		if (pModelResource->GetRenderModelData())
			pModelResource->GetRenderModelData()->GetScene().UpdateResourcePointers();
	}
}

} // namespace GameResource

