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
#include "Lux/DataTypes.h"
#include "Lux/OutputTypeSelect.h"

using namespace LUX;

IMPLEMENT_PERSISTENCE(SHLightingResult, "SHResult");
IMPLEMENT_PERSISTENCE(SHLightingJob, "SHJob");

// amount of nodes that will be batched in one job
const int SHLighting::m_iBatchSize = 250;

// initialize with some global variables
SHLighting::SHLighting(int iRenderMode, float fBrightness, float fGamma, const Ut::BitArray128 &iLightingPassBits)
{
	m_fBrightness = fBrightness;
	m_fInvGamma = 1.0f / fGamma;
	m_iLightingPassBits = iLightingPassBits;
	m_iRenderMode = iRenderMode;
}

// add a node to the list of nodes that need SH lighting
void SHLighting::AddNodeToLight(Aftk::C_SceneNode *pNode)
{
	m_ProcessList.PushBack(pNode);
}

// use the list of SH nodes to generate jobs on the scheduler
void SHLighting::AddJobs(JobScheduler *pScheduler)
{
	// add jobs of batches of items
	int nTotalBatches = (m_ProcessList.Size()+m_iBatchSize-1)/m_iBatchSize;
	for (int i=0; i<nTotalBatches; i++)
		pScheduler->AddJob(i | SHLIGHTING_JOB, m_iBatchSize); // cost equals the number of sample points in the batch
}

// given a scheduler job index, generate the data to be sent to the client
BaseObject *SHLighting::GenerateJob(int iID)
{
	SHLightingJob *pJob = new SHLightingJob();
	iID = iID & JOB_INDEX_MASK;

	int iFirstItem = iID * m_iBatchSize;
	int nTotalItems = m_ProcessList.Size();

	// add the samplepoints to the job
	pJob->m_ItemList.Reserve(m_iBatchSize);
	for (int i=iFirstItem; i<nTotalItems && i<iFirstItem+m_iBatchSize; i++)
	{
		int iItem = pJob->m_ItemList.Size();
		pJob->m_ItemList.PushBack();
		pJob->m_ItemList[iItem].GenerateFromNode( m_ProcessList[i] );
	}
	assert(pJob->m_ItemList.Size()>0);

	pJob->m_iItemIdx = iFirstItem;
	pJob->m_fBrightness = m_fBrightness;
	pJob->m_fInvGamma = m_fInvGamma;
	pJob->m_iLightingPassBits = m_iLightingPassBits;
	pJob->m_iRenderMode = m_iRenderMode;

	return pJob;
}

// receive data back from the client and store it in the SH
void SHLighting::StoreJobResult(BaseObject *pInResult)
{
	if (strcmp(pInResult->getPersistenceID(), "SHResult"))
		return;

	SHLightingResult *pResult = (SHLightingResult*)pInResult;

	// get index of the first item that this job result is for
	int iLast = pResult->m_iItemIdx + m_iBatchSize;
	if (m_ProcessList.Size() < iLast)
		iLast = m_ProcessList.Size();

	// store the results in our sample array
	for (int iResult=0, i=pResult->m_iItemIdx; i<iLast; i++, iResult++)
	{
		// store the average SH vector as blind data on the tree node
		SetLuxCustomDataToNode(m_ProcessList[i], (U8*)&pResult->m_ResultList[iResult * pResult->m_iResultStride], pResult->m_iResultStride * sizeof(float));
	}
}

// generate SH lighting info from an aftk node to send to the server
void SHLightingJob::Item::GenerateFromNode(Aftk::C_SceneNode *pNode)
{
	m_iUniqueID = pNode->GetUniqueId();
	m_matrix = pNode->GetWorldMatrix();

	if (pNode->HasType( Aftk::E_ObjType_ISceneMesh ))
	{
		// for a normal mesh object / instance, get the bounding boxes
		CalculateAftkNodeBoundingBox(pNode, m_localbbox, false);
		CalculateAftkNodeBoundingBox(pNode, m_worldspacebbox, true);
	}
	else
	{
		// for locators, etc. use the pivot
		Position3 vPos = LPVS::ApplyMatrix(m_matrix, Position3(0,0,0));
		m_localbbox.UpdateBox( float3(0,0,0) );
		m_worldspacebbox.UpdateBox( vPos );
	}
}

// process an SH lighting job - this does the actual client-side rendering
SHLightingResult *SHLightingJob::Process(SurfaceIntegrator *pIntegrator)
{
	SHLightingResult *pResult = new SHLightingResult;

	for (int i=0; i<m_ItemList.Size(); i++)
	{
		m_ItemList[i].Process(pIntegrator, pResult->m_ResultList, this);
	}

	pResult->m_iResultStride = pResult->m_ResultList.Size() / m_ItemList.Size();

	pResult->m_iItemIdx = m_iItemIdx;

	return pResult;
}

// calculate the lighting for a single SH node
void SHLightingJob::Item::Process(SurfaceIntegrator *pIntegrator, Ut::Vector<float> &results, SHLightingJob *pParentJob)
{
	// switch samplepoint datatype in depending on settings in OutputTypeSelect.cpp
	// TODO: ideally we want to tidy this up so we don't need an extra switch-case statement (use 'CreateJobRenderer' instead)
	switch (DataTypes::g_ModeDataTypes[(LUX::DataTypes::RenderMode)pParentJob->m_iRenderMode][DataTypes::E_SamplePointIterator][1])
	{
	case DataTypes::E_DataType_GothamSamplePoint:
		ProcessTemplated<DataTypes::GothamSamplePoint>(pIntegrator, results, pParentJob);
		break;
	case DataTypes::E_DataType_SamplePoint:
		ProcessTemplated<DataTypes::SamplePoint>(pIntegrator, results, pParentJob);
		break;
	default:
		LOGFATAL( g_Log, "Unknown output format for SHLightingJob\n");
		break;
	}
}

template<typename T>
void RemoveBrightestSamplePoint(SamplePointIterator<T> &points)
{
	int iBrightest = 0;
	float fMaxAbsValue = 0;
	int n = T::Scalars();
	float *pData = new float[n];
	for (int i=0; i<points.m_Samples.Size(); i++)
	{
		float *p = pData;
		T::ContextDataType context;
		points.m_Samples[i].GetData(p, context);
		for (int j=0; j<n; j++)
		{
			float f = fabs(pData[j]);
			if (f > fMaxAbsValue)
			{
				fMaxAbsValue = f;
				iBrightest = i;
			}
		}
	}

	points.m_Samples.EraseByIndex(iBrightest);

	delete pData;
}

// calculate the lighting for a single SH node
template<typename T>
void SHLightingJob::Item::ProcessTemplated(SurfaceIntegrator *pIntegrator, Ut::Vector<float> &results, SHLightingJob *pParentJob)
{
static int iLast = 0;

	// spatial hierarchy
	SD::SpatialHierarchy *pTree = (SD::SpatialHierarchy*)pIntegrator->m_pHierarchy;

	// turn off collision
	int iInstance = pTree->GetInstanceFromAftkID(m_iUniqueID, iLast);
	iLast = iInstance;
	pTree->EnableInstance(iInstance, false);

	// sample points array
	SamplePointIterator<T> samplePoints;
	float3 vDummy;
	samplePoints.m_Samples.Resize( SPEED_TREE_SAMPLE_POINTS );

	// generate sample points inside the bounding box
	samplePoints.Clear();

	float2 *pfSamples = pIntegrator->m_Sample;

	// for instances of mesh, calculate the centre of the bounding box in local coordinate space and then sample that
	for (int i=0; i<SPEED_TREE_SAMPLE_POINTS; i++)
	{
		// sample bounding box in local-coordinate space
		Position3 v = Position3(	m_localbbox.vmin.x + (m_localbbox.vmax.x - m_localbbox.vmin.x) * pfSamples[i].x, 
									m_localbbox.vmin.y + (m_localbbox.vmax.y - m_localbbox.vmin.y) * pfSamples[i].y,
									m_localbbox.vmin.z + (m_localbbox.vmax.z - m_localbbox.vmin.z) * pfSamples[i+32].x );


		// transform into world-space
		samplePoints.m_Samples[i].m_vPos = LPVS::ApplyMatrix(m_matrix, v);
	}

	pIntegrator->GetLights(m_worldspacebbox, NULL);

	// render the sample points as SH vectors
	float fBrightness = pParentJob->m_fBrightness;
	float fInvGamma = pParentJob->m_fInvGamma;

	pIntegrator->LightData( &samplePoints );

	// average samples
	T sh;
	sh = 0;

	RemoveBrightestSamplePoint(samplePoints);
	RemoveBrightestSamplePoint(samplePoints);

	// average the resulting samples
	float fSamples = 0;
	for (int i=0; i<samplePoints.m_Samples.Size(); i++)
	{
		if (!(samplePoints.m_Samples[i] == 0))
		{
			sh = sh + samplePoints.m_Samples[i];
			fSamples++;
		}
	}
	if (fSamples == 0)
		fSamples = 1.0f;

	float fScale = 1.0f * (1.f/fSamples);
	sh = sh * fScale;

	// apply brightness
	sh = sh * fBrightness;

	// get resulting data
	int n = sh.Scalars();
	int iOrg = results.Size();
	results.Resize(iOrg + n);
	float *pf = &results[iOrg]; 
	sh.GetData(pf, samplePoints.m_context);

	// turn collision back on
	pTree->EnableInstance(iInstance, true);
}

// render SH and/or vertex lighting for all the nodes that require it
void GlobalIlluminationModel::RenderVertexAndSHLighting()
{
	// do vertex lighting first
	for (Aftk::C_SceneNodeIterator it(m_pAftkScene); it; ++it)
	{
		Aftk::C_SceneNode* pAftkNode = it;

		// debugging code for only lighting a particular node
		extern int giLightOnlyNode;
		if (giLightOnlyNode != -1 && pAftkNode->GetUniqueId() != giLightOnlyNode)
			continue;

		// render speedtree instances
		if (LUX::IsNodeASpeedTreeInstance(pAftkNode))
		{
			// don't light trees that were turned into billboards
			AftkUt::C_NodeFlagParser parser(*pAftkNode);
			if (parser.GetFlagGroupCategory() == "Effect" && parser.GetFlagGroupName() == "TreeBillboard")
				continue;

			switch (GetNodeLightingMode(m_pProperties, pAftkNode))
			{
			case LMode_SH:
			default:
				if (pAftkNode->GetType() == Aftk::E_ObjType_SceneInstance ||
					pAftkNode->GetType() == Aftk::E_ObjType_SceneMesh ||
					pAftkNode->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance)
				{
					m_pSHLighting->AddNodeToLight(pAftkNode);
				}
				break;
			case LMode_VtxColour:
				// is it a mesh or an instance of a mesh?
				if (pAftkNode->HasType( Aftk::E_ObjType_ISceneMesh ))
				{
					m_pVtxLighting->AddNodeToLight(pAftkNode, true);
				}
				break;
			case LMode_DoNotLight:
				break;
			}
			continue;
		}

		if (!DoesNodeNeedProcessing(pAftkNode))
		{
			// render SH vector for locators with SH data
			if (NodeIsLocatorWithSHData(pAftkNode))
			{
				m_pSHLighting->AddNodeToLight(pAftkNode);
			}

			// this node does not need lighting
			continue;
		}

		// is it a mesh or an instance of a mesh?
		if (!pAftkNode->HasType( Aftk::E_ObjType_ISceneMesh ))
			continue;

		Aftk::I_SceneMeshNode *pMesh = (Aftk::I_SceneMeshNode*)pAftkNode;

		// this meshnode/instance needs lighting - figure out the lighting method
		switch (GetNodeLightingMode(m_pProperties, pAftkNode))
		{
		case LMode_VtxColour:
			m_pVtxLighting->AddNodeToLight(pAftkNode, false);
			break;
		case LMode_SH:
			// calculate a SH blind data vector to light this node with in the game
			// (lighting might not be calculated correctly for anything other than normal maya instances,
			//  unless the object is flagged with 'donotcastshadows')
			m_pSHLighting->AddNodeToLight(pAftkNode);
			break;
		case LMode_Lightmap:
		case LMode_DoNotLight:
			// do nothing
			break;
		}
	}
}
