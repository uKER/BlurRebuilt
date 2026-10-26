#include "stdafx.h"

#include "Lux/Common.h"
#include "CServ/ClusterServHeaders.h"
#include "Lux/SHLighting.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/SamplePointIterator.h"

#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/LightingModel.h"
#include "Lux/GlobalIllumination/GlobalIlluminationModel.h"
#include "Lux/VtxLighting.h"
#include "Lux/VertexIterator2.h"
#include "Lux/DataTypes.h"
#include "Lux/OutputTypeSelect.h"
#include "lux_common/LightingPass.h"
#include "lux/Lux.h"

using namespace LUX;

IMPLEMENT_PERSISTENCE(VtxLightingResult, "VtxResult");
IMPLEMENT_PERSISTENCE(VtxLightingJob, "VtxJob");

// initialize with some global variables
VtxLighting::VtxLighting(Aftk::C_Scene *pScene, float fBrightness, float fGamma, Lux *pLux)
{
	m_fBrightness = fBrightness;
	m_fInvGamma = 1.0f / fGamma;
	m_pLux = pLux;
	m_pScene = pScene;
	m_bBouncePass = false;
	m_iRenderMode = 0;
}

// add a node to the list of nodes that need Vtx lighting
void VtxLighting::AddNodeToLight(Aftk::C_SceneNode *pNode, bool bSmoothSpeedTreeLeaves)
{
	// only accept mesh or remappednode
	if (!pNode->HasType(Aftk::E_ObjType_ISceneMesh))
		return;

	// skip things that don't need lighting
	// find a lightingpass that lights this node
	const Ut::Vector<LightingPass*> *pLightingPasses = m_pLux->GetLightingPassVector();
	int lp;
	for (lp=0; lp<pLightingPasses->Size() && !(*pLightingPasses)[lp]->DoesNodeNeedLighting(pNode); lp++) { ; }
	if (lp == pLightingPasses->Size())
		return; // don't add a job for things that aren't in any passes to get lit

	Aftk::I_SceneMeshNode *pMesh = (Aftk::I_SceneMeshNode*)pNode;
	m_ProcessList.PushBack();
	m_ProcessList.Back().pNode = pMesh;
	m_ProcessList.Back().m_bSmoothSpeedTreeLeaves = bSmoothSpeedTreeLeaves;
}

// use the list of Vtx nodes to generate jobs on the scheduler
void VtxLighting::AddJobs(JobScheduler *pScheduler)
{
	// add jobs of batches of items
	int nItems = m_ProcessList.Size();
	for (int i=0; i<nItems; i++)
		pScheduler->AddJob(i | VTXLIGHTING_JOB, m_ProcessList[i].pNode->GetNumPolygons(0)*3); // cost equals the number of vertices
}

// turn on bounce mode (ignore brightness/gamma when on)
void VtxLighting::SetBouncePass(bool bOn)
{
	m_bBouncePass = bOn;
}

// set SH lighting mode
void VtxLighting::SetRenderMode(int iMode)
{
	m_iRenderMode = iMode;
}

// given a scheduler job index, generate the data to be sent to the client
BaseObject *VtxLighting::GenerateJob(int iID)
{
	VtxLightingJob *pJob = new VtxLightingJob();
	iID = iID & JOB_INDEX_MASK;

	pJob->m_iRenderMode = m_iRenderMode;
	pJob->m_bBouncePass = m_bBouncePass;

	Aftk::I_SceneMeshNode *pMesh = m_ProcessList[iID].pNode;

	// skip things that don't need lighting
	// find a lightingpass that lights this node
	const Ut::Vector<LightingPass*> *pLightingPasses = m_pLux->GetLightingPassVector();
	int lp;
	for (lp=0; lp<pLightingPasses->Size() && !(*pLightingPasses)[lp]->DoesNodeNeedLighting(pMesh); lp++) { ; }
	assert(lp != pLightingPasses->Size()); // does not get lit by any pass
	pJob->m_iLightingPassBits = m_pLux->GetLightingPassBits(lp);

	// grab vertices from the node and add them into the job
	int nPolygons = pMesh->GetNumPolygons(0);
	pJob->m_Vertices.Resize(nPolygons * 3);
	CalculateAftkNodeBoundingBox( pMesh, pJob->m_bbox, true );
	pJob->m_bSmoothSpeedTreeLeaves = m_ProcessList[iID].m_bSmoothSpeedTreeLeaves;

	pJob->m_iItem = iID;
	if (m_bBouncePass)
	{
		// disable the application of brightness/gamma when rendering bounce lighting
		pJob->m_fBrightness = 1.0f;
		pJob->m_fInvGamma = 1.0f;
	}
	else
	{
		pJob->m_fBrightness = m_fBrightness;
		pJob->m_fInvGamma = m_fInvGamma;
	}

	int iVtxList = 0;

	Aftk::Matrix44 mWorldMatrix = pMesh->GetWorldMatrix();

	// for each poly
	for (int iPoly=0; iPoly<nPolygons; iPoly++)
	{
		// for each vertex
		for (int iVtx=0; iVtx<3; iVtx++)
		{
			int iPV = pMesh->GetPolygon(iPoly).GetPolygonVertexIndex(iVtx);

			// get position
			LUX::Position3 v = pMesh->GetEvaluatedPolygonVertexPosition(iPV);
			pJob->m_Vertices[iVtxList].vPos = LPVS::ApplyMatrix(mWorldMatrix, v );

			// get normal
			LUX::Normal3 norm = pMesh->GetEvaluatedPolygonVertexNormal(iPV);
			pJob->m_Vertices[iVtxList].vNormal = LPVS::ApplyMatrix(mWorldMatrix, norm);			

			// check that the normal and position are valid numbers
			if (CheckBuggeredFloat(pJob->m_Vertices[iVtxList].vPos.x) ||
				CheckBuggeredFloat(pJob->m_Vertices[iVtxList].vPos.y) ||
				CheckBuggeredFloat(pJob->m_Vertices[iVtxList].vPos.z))
			{
				LOGWARN( g_Log, "Invalid position for polyvert %d on object '%s'\n", iPV, pMesh->GetPath().c_str());
				pJob->m_Vertices[iVtxList].vPos = float3(0,0,0);
			}

			if (CheckBuggeredFloat(pJob->m_Vertices[iVtxList].vNormal.x) ||
				CheckBuggeredFloat(pJob->m_Vertices[iVtxList].vNormal.y) ||
				CheckBuggeredFloat(pJob->m_Vertices[iVtxList].vNormal.z))
			{
				LOGWARN( g_Log, "Invalid normal for polyvert %d on object '%s'\n", iPV, pMesh->GetPath().c_str());
				pJob->m_Vertices[iVtxList].vNormal = float3(0,1,0);
			}

			iVtxList++;
		}
	}

	return pJob;
}

// TODO: remove this hack when Ken adds support for arbitrary size data on vertices
template<typename T>
void StoreCustomDataHack(Aftk::C_PolygonVertex *pVtx, int iChannel, float *pf, int c)
{
	float vSample[16];

	for (int i=0; i<c; i++)
		vSample[i] = max(min(pf[i], 1.f), 0.f);
	for (int i=c; i<(sizeof(T)/sizeof(float)); i++)
		vSample[i] = 1.f;

	pVtx->SetData(iChannel,*(T*)&vSample);
}

// receive data back from the client and store it in the SH
void VtxLighting::StoreJobResult(BaseObject *pInResult)
{
	if (strcmp(pInResult->getPersistenceID(), "VtxResult"))
		return;

	VtxLightingResult *pResult = (VtxLightingResult*)pInResult;

	// get the vertex data channel for the vertex data to go into
	Aftk::C_SceneMeshNode *pDataMesh;
	int iChannel;
	CreateVertexDataChannel(m_ProcessList[pResult->m_iItem].pNode, &pDataMesh, &iChannel, pResult->m_iStride);

	int nTriangles = pDataMesh->GetNumPolygons(0);

	// write the vertex data
	// read colour data from buffer and store in vertex colour channel
	int iVert = 0;
	int iData = 0;
	for (int iTriangle=0; iTriangle<nTriangles; iTriangle++)
	{
		for (int iVtx=0; iVtx<3; iVtx++, iVert++, iData+=pResult->m_iStride)
		{
			Aftk::C_PolygonVertex *pV = &pDataMesh->GetPolygon(iTriangle).GetPolygonVertex(iVtx);

			// set colour to vertex
			if (pResult->m_iStride <= 4)
				StoreCustomDataHack<Aftk::Vector4F32>(pV, iChannel, &pResult->m_Data[iData], pResult->m_iStride);
			else if (pResult->m_iStride <= 8)
				StoreCustomDataHack<Aftk::Vector4F64>(pV, iChannel, &pResult->m_Data[iData], pResult->m_iStride);
			else if (pResult->m_iStride <= 16)
				StoreCustomDataHack<Aftk::Matrix44F32>(pV, iChannel, &pResult->m_Data[iData], pResult->m_iStride);
			else
			{
				assert(0); // need support in Aftk for arbitrary size data on vertices
			}
		}
	}

	// store custom data on the node as a plug
	if (pResult->m_CustomData.Size() > 0)
		SetLuxCustomDataToNode(m_ProcessList[pResult->m_iItem].pNode, &pResult->m_CustomData[0], pResult->m_CustomData.Size());

	//We need to call this so that empty memory pools resulting from our vertex channel manipulation
	//get freed. This makes a huge difference to memory usage.
	Aftk::C_MemoryManager::CollectGarbage();
}

// process a vertex lighting job - this does the actual client-side rendering
VtxLightingResult *VtxLightingJob::Process(SurfaceIntegrator *pIntegrator)
{
	I_JobRenderer *pIter = LUX::DataTypes::CreateJobRenderer(
			(LUX::DataTypes::RenderMode)m_iRenderMode,
			LUX::DataTypes::E_VertexIterator, m_bBouncePass, 
			this );

	// retrieve the results object
	VtxLightingResult *pResult = (VtxLightingResult*)pIter->Process(pIntegrator);
	pResult->m_iItem = m_iItem;
	pResult->m_bSmoothSpeedTreeLeaves = m_bSmoothSpeedTreeLeaves;

	delete pIter;

	return pResult;
}

// figure out where to put the vertex colour data
void VtxLighting::CreateVertexDataChannel(Aftk::I_SceneMeshNode *_pMesh, Aftk::C_SceneMeshNode **ppMeshNode, int *piVertexColourChannel, int iStride)
{
	Aftk::C_SceneMeshNode *pMesh;
	Aftk::C_SceneMeshRemappedInstanceNode *pInstance = NULL;
	std::string channelName = "luxVertexColour";

	if (_pMesh->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		// this is a normal meshnode
		pMesh = (Aftk::C_SceneMeshNode*)_pMesh;
	}
	else
	// if this node is an instance, store the vertex data on the source mesh
	if (_pMesh->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance ))
	{
		pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)_pMesh;

		pMesh = pInstance->GetSourceNodePtr();

		// create a unique channel name using the instance's unique ID number
		std::ostringstream o;
		o << "luxVertexColour" << pInstance->GetUniqueId();
		channelName = o.str();

		// set blind data to associate this instance with the vertex colours set on it's source node
		Aftk::C_GenericAttribute vtxColAttr;
		vtxColAttr.SetName("luxVertexColour");
		vtxColAttr.SetValue(channelName);
		AddBlindData(pInstance, vtxColAttr);
	}
	else
	{
		// not sure what this is - can't write out vertex data
		assert(0);
		return;
	}

	// attribute type depends on stride
	Aftk::E_GenericAttributeType datatype;
	if (iStride <= 4)
		datatype = Aftk::E_GenericAttributeType_Vector4F32;
	else if (iStride <= 8)
		datatype = Aftk::E_GenericAttributeType_Vector4F64;
	else if (iStride <= 16)
		datatype = Aftk::E_GenericAttributeType_Matrix44F32;
	else
	{
		assert(0);
	}

	// find channel for putting vertex colour info in
	bool found = false;
	S32 vertexColourChannel;
	for (U32 i = 0; i < pMesh->GetNumVertexDataChannels(); i++)
	{
		if (pMesh->GetVertexDataChannelInfo(i).GetName() == channelName)
		{
			found = true;
			vertexColourChannel = i;

			if (pMesh->GetVertexDataChannelInfo(i).GetDataType() == Aftk::E_GenericAttributeType_U32)
			{
				// change channel info type - destroys previous contents
				Aftk::C_VertexDataChannelInfo info;

				info.SetName(channelName);
				info.SetDataType(datatype);

				vertexColourChannel = pMesh->SetVertexDataChannelInfo(i, info);
			}

			break;
		}
	}

	// set vertex channel info
	if (!found)
	{
		Aftk::C_VertexDataChannelInfo info;

		info.SetName(channelName);
		info.SetDataType(datatype);

		vertexColourChannel = pMesh->AddVertexDataChannelInfo(info);
	}

	// set output variables
	*ppMeshNode = pMesh;
	*piVertexColourChannel = vertexColourChannel;
}

// return number of items in the process list
int VtxLighting::GetNumItems()
{
	return m_ProcessList.Size();
}

// return a particular item in the process list
VtxLighting::Item *VtxLighting::GetItem(int i)
{
	return &m_ProcessList[i];
}
