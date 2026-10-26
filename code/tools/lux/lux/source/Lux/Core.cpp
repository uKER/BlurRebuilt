#include "stdafx.h"

#include "Lux/lux.h"
#include "Lux/Core.h"
#include "lux_common/MultiProgressNotification.h"
#include "tools_common/StringProc.h"
#include "Lux/LightingModelProperties.h"
#include "cc++/missing.h"
#include <tools_common/StringProc.h>
#include <spatial_hierarchy/AlphaTextureInfo.h>
#include <spatial_hierarchy/AlphaTextureManager.h>
#include "ftm_xml_api\Headers.h"
#include "Lux/AlphaTextureManager.h"
#include <system\misc\progress.h>

using namespace LUX;
using namespace LPVS;

void MyAssert(char *text, int iLine)
{
	static Debug::Logger &assertLog = Debug::GetLogger("tools.lux");
	LOGFATAL(assertLog, text, iLine);
}

Core::Core()
{
	Reset();
}


U32 Core::GetScenePolyCount() const
{
	U32 polyCount = 0;
	for (Aftk::C_SceneNodeConstIterator it(&m_aftk); it; ++it)
	{
		const Aftk::C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			const Aftk::C_SceneMeshNode* pMesh = (const Aftk::C_SceneMeshNode*)pSceneNode;
			for (U32 i = 0; i < pMesh->GetNumLodLevels(); ++i)
				polyCount += pMesh->GetNumPolygons(i);
		}
	}

	return polyCount;
}

// read a scene, triangulate it, convert instances, calculate spatial hierarchy
bool Core::ReadScene(MultiProgressNotification* pProgress, Progress::I_ProgressBar *p_Progress)
{
	std::string fileName = m_lux.GetInputSceneFileName();

	Progress::UpdateSimple prog(p_Progress, "Loading");

	CloseScene();

	m_pSpatialHierarchy = new LUX::SpatialHierarchy;

	int dwStart = ost::getTicks();

	if (pProgress)
	{
		pProgress->SetSubProcess(0);
		pProgress->OnProgress(0);
		pProgress->OnPrintProgress("Reading scene...");
	}
	Aftk::E_FileType filetype;
	Aftk::C_Global::GetFileType(fileName, filetype);

	S_INFO( g_Log, "Loading scene '" << fileName << "'.\n");
	
	MEMORY_TRACKER_PUSH("tools.aftk");

	Aftk::C_Status status = m_aftk.Read(fileName,
		Aftk::E_FileType_BestGuess, 
		0,
		0);

	LOGINFO( g_Log, "Scene '%s' loaded in %3.2f seconds.\n", fileName.c_str(), time);
	LOGINFO( g_Log, "Initial scene poly-count : %i\n", GetScenePolyCount());

	prog.SetProgress(60.f);

	m_aftk.Optimise();

	prog.SetProgress(70.f);

	if (!status)
	{
		S_ERROR( g_Log, status.GetErrorString() << "\n");
		return false;
	}
	
	U32 errorCount = m_aftk.Validate();
	if (errorCount > 0)
	{
		S_ERROR( g_Log, "Scene loaded, but was found to contain errors - aborting\n");
		return false;
	}

	if (pProgress)
	{
		pProgress->SetSubProcess(1);
		pProgress->OnProgress(0);
		pProgress->OnPrintProgress("Triangulating scene...");
	}

	Aftk::C_SceneNodeVisit::Traverse(
		&m_aftk, 
		Aftk::C_MeshTriangulate::C_TriangulateVisitor());
	LOGINFO( g_Log, "Poly-count after triangulation : %i\n", GetScenePolyCount());

	if (pProgress)
	{
		pProgress->OnProgress(100);
	}

	prog.SetProgress(100.f);

	int dwEnd = ost::getTicks();
	int elapsedTime = dwEnd - dwStart;
	float time = (float) elapsedTime;
	time *= 0.001f;
	{
		LOGINFO( g_Log, "Scene '%s' loaded in %3.2f seconds.\n", fileName.c_str(), time);
	}

	if (pProgress)
	{
		pProgress->SetSubProcess(2);
		pProgress->OnProgress(0);
		pProgress->OnPrintProgress("Building spatial hierarchy...");
	}
	dwStart = ost::getTicks();

	MEMORY_TRACKER_POP("tools.aftk");

	prog.SetProgress(80.f);

	Memory::Tracker::Instance().DumpNodeStats(false, true);
	Memory::Tracker::Instance().DumpNodeStats(true, true);

	m_lux.SetSpatialHierarchy(m_pSpatialHierarchy);
	m_lux.SetAftkScene(&m_aftk);

	// set alpha texture manager to spatial hierarchy
	AlphaTextureManager atl;
	m_pSpatialHierarchy->SetAlphaTextureManager(&atl);

	// get layer usage
	SD::SpatialHierarchy::LayerMode *piLayerMode = new SD::SpatialHierarchy::LayerMode[m_aftk.GetNumLayers()];

	m_lux.GetLayerUsage(piLayerMode);
	if (m_lux.GetLightingModelPropertiesPtr())
		m_pSpatialHierarchy->SetNoCollisionPolyflagMask( m_lux.GetLightingModelPropertiesPtr()->GetDoNotCastShadowsPolyflag() );

	m_pSpatialHierarchy->SetCallbackFunction(&LUX::DoesNodeNeedProcessing);
	m_pSpatialHierarchy->SetBaseInstanceCallbackFunction(&LUX::DoesBaseInstanceNeedExporting);

	MEMORY_TRACKER_PUSH("tools.aftk");

	if (m_lux.GetLightingModelPropertiesPtr() && m_lux.GetLightingModelPropertiesPtr()->m_iInstanceMode == INSTANCE_MODE_INSTANTIATE)
	{
		ConvertNormalInstancesToRemappedInstances(&m_aftk, m_pSpatialHierarchy);
		ConvertRemappedInstancesToMesh(&m_aftk);
	}

	MEMORY_TRACKER_POP("tools.aftk");

	m_pSpatialHierarchy->Build(&m_aftk, piLayerMode);
	delete piLayerMode;

	MEMORY_TRACKER_PUSH("tools.aftk");

	ConvertNormalInstancesToRemappedInstances(&m_aftk, m_pSpatialHierarchy);

	MEMORY_TRACKER_POP("tools.aftk");

	dwEnd = ost::getTicks();
	elapsedTime = dwEnd - dwStart;
	time = (float) elapsedTime;
	time *= 0.001f;
	{
		LOGINFO( g_Log, "Scene hierarchy built in %3.2f seconds.\n", time);
	}
	if (pProgress)
	{
		pProgress->OnProgress(100);
	}

	if (pProgress)
	{
		pProgress->SetSubProcess(3);
		pProgress->OnProgress(0);
		pProgress->OnPrintProgress("Creating renderers...");
	}

	m_lux.SetSpatialHierarchy(m_pSpatialHierarchy);

	if (pProgress)
	{
		pProgress->OnProgress(100);
		pProgress->OnPrintProgress("Finished reading scene.");
	}

	LOGINFO( g_Log, "Final scene poly-count : %i\n", GetScenePolyCount());
	return true;
}

// Lights the current scene, with the results being stored within the aftk scene in memory.
bool Core::LightScene(Progress::I_ProgressBar *pProgress)
{
	m_lux.GetNumPasses();
	m_lux.SetCurrentPass(0);

	int dwStart = ost::getTicks();

	if (m_lux.Light(pProgress) == false)
		return false;

	int dwEnd = ost::getTicks();
	int elapsedTime = dwEnd - dwStart;
	float time = (float) elapsedTime;
	time *= 0.001f;
	{
		LOGINFO( g_Log, "Scene lit in %3.2f seconds.\n", time);
	}
	
	return true;
}

void printcallback(const std::string &s)
{
	std::cout << s;
}

// save the lit scene
bool Core::WriteScene(Progress::I_ProgressBar *pProgress)
{
	std::string fileName = m_lux.GetOutputSceneFileName();
	int dwStart = ost::getTicks();

#ifdef BUILD_CONFIG_DEBUG
	m_aftk.Write("c:/lighting_tool_test.a3d", Aftk::E_FileType_A3d);
#endif

	if (m_aftk.Validate() > 0)
		LOGWARN( g_Log, "Failed to validate scene for writing\n");

	m_aftk.Optimise();

	if (m_aftk.Validate() > 0)
		LOGWARN( g_Log, "Failed to validate scene for writing (after calling optimise)\n");

	// generate filename for temporary file
#ifdef WIN32
	char buffer[512];
	GetTempPath(512,buffer); // WIN32
	std::string temporaryPath = std::string(buffer) + "\\" + Tu::FileNameFromString( fileName );

	S_INFO( g_Log, "Saving scene '" << fileName << "'.\n");

	// save aftk to temporary folder
	if (m_aftk.Write(temporaryPath, Aftk::E_FileType_BestGuess) == false)
	{
		LOGERROR( g_Log, "Failed to write scene '%s'.\n", temporaryPath.c_str() );
		return false;
	}
	// copy temporary file to destination
	if (CopyFile(temporaryPath.c_str(), fileName.c_str(), false) == false)
	{
		LOGERROR( g_Log, "Failed to copy lit scene to '%s'.\n", fileName.c_str() );
		return false;
	}
#else

	// save aftk
	if (m_aftk.Write(fileName, Aftk::E_FileType_BestGuess) == false)
	{
		LOGERROR( g_Log, "Failed to write scene '%s'\n", fileName.c_str() );
		return false;
	}
#endif

	int dwEnd = ost::getTicks();
	int elapsedTime = dwEnd - dwStart;
	float time = (float) elapsedTime;
	time *= 0.001f;
	{
		LOGINFO( g_Log, "Scene '%s' saved in %3.2f seconds.\n", fileName.c_str(), time);
	}

	return true;
}

// free stuff
void Core::Destroy()
{
	CloseScene();
	m_lux.Clear();
}

void Core::CloseScene()
{
	// Remove the source scene from the aftk
	m_aftk.Clear();

	// delete the spatial hierachy
	if (m_pSpatialHierarchy != 0)
	{
		delete m_pSpatialHierarchy;
		m_pSpatialHierarchy = 0;
	}
}

void Core::Reset()
{
	m_pSpatialHierarchy = 0;
	m_pLogCallbackFunc = 0;
}

Lux* Core::GetLightingSystemPtr()
{
	return &m_lux;
}

// get lux pointer
const Lux* Core::GetLightingSystemPtr() const
{
	return &m_lux;
}

