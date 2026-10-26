// aftkut.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "crtdbg.h"
#include <system/time/time.h>

#include "core/maths/conversion.h"

#define USING_TEST_SCENE

void OutputFunc(const std::string& text)
{
	std::cout << text;
}

class C_TestProgress : public Aftk::I_Progress
{
public:
	bool OnProgress(F32 percent)
	{
		std::cout << percent << std::endl;
		return false;
	}
	
	void OnPrintProgress(const std::string& text)
	{
		std::cout << text << std::endl;
	}
};

using namespace Time;

class BarrierShadowChopper
{
public:
	/// constructor
	BarrierShadowChopper(Aftk::C_Scene *pScene);

	// set the direction of the shadow
	void SetShadowDirection(Maths::C_Normal &vNormal);

	/// do the processing
	void Process();

	/// set output scene
	void SetOutScene(Aftk::C_Scene *pOutScene);

private:
	/// find all the barriers in the scene and generate clip-planes for them
	void GenerateBarrierList();

	/// get the bone positions for the two bones in this barrier. returns false in case of error
	bool GetBones(Aftk::C_SceneNode *pNode, Maths::HVec3 vPos[2]);

	/// generate barrier shadows by clipping the barriers into the mesh
	void GenerateShadows();

	/// generate a list of objects to be shadowed
	void GenerateObjectList();

	typedef Ut::Vector<AftkUt::RadialEdgeMesh::C_Plane> PlaneVector;

	struct Barrier
	{
		Aftk::C_SceneNode	*pNode;
		PlaneVector			planes;
	};
	Ut::Vector<Barrier>		m_BarrierList;
	Ut::Vector<Aftk::C_SceneNode*> m_ObjectList;

	Aftk::C_Scene			*m_pScene;
	Aftk::C_Scene			*m_pOutScene;

	Maths::C_Normal			m_vDirection;
};

using namespace Maths;

Debug::Logger &g_BSCLog = Debug::GetLogger("tools.aftkut.barriershadows");

/// constructor
BarrierShadowChopper::BarrierShadowChopper(Aftk::C_Scene *pScene) :
	m_pScene(pScene)
{
}

/// get the bone positions for the two bones in this barrier. returns false in case of error
bool BarrierShadowChopper::GetBones(Aftk::C_SceneNode *pAftkSceneNode, Maths::HVec3 vPos[2])
{
	if (pAftkSceneNode->GetType() != Aftk::E_ObjType_SceneMeshBonedInstance)
		return false;

	Aftk::C_SceneMeshBonedInstanceNode *pNode = (Aftk::C_SceneMeshBonedInstanceNode*)pAftkSceneNode;

	for (int i=0; i<2; i++)
	{
		Aftk::C_SceneNode *pBone = pNode->GetBonePtr(i);
		if (!pBone)
			return false;

		Aftk::Matrix44 mtx = pBone->GetWorldMatrix();
		Aftk::Vector4F32 pos = mtx[3];
		HVec4 _pos;
		Convert(_pos, pos);
		vPos[i] = HVec3(_pos);
	}

	return true;
}

/// set output scene
void BarrierShadowChopper::SetOutScene(Aftk::C_Scene *pOutScene)
{
	m_pOutScene = pOutScene;
}

/// set the direction of the shadows
void BarrierShadowChopper::SetShadowDirection(Maths::C_Normal &vNormal)
{
	m_vDirection = vNormal;
}

/// find all the barriers in the scene and generate clip-planes for them
void BarrierShadowChopper::GenerateBarrierList()
{
	for (Aftk::C_SceneNodeIterator it(m_pScene); it; ++it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;

		if (pAftkSceneNode->GetNumInstancesOfThisNode())
			continue;

		// get the bones for this barrier
		HVec3 vPos[2];
		if (!GetBones(it, vPos))
			continue;

		m_BarrierList.PushBack();
		m_BarrierList.Back().pNode = pAftkSceneNode;

		// generate polygon verts from the bone positions
		float fBarrierHeight = 1.5f;
		HVec3 vHeight = fBarrierHeight * HVec3(0,1.f,0);

		HVec3 vVert[4];
		vVert[0] = vPos[0], vVert[1] = vPos[1];
		vVert[3] = vPos[0] + vHeight, vVert[2] = vPos[1] + vHeight;

		HVec3 vEdge1 = vVert[0] - vVert[1];
		HVec3 vEdge2 = vVert[0] - vVert[2];
		C_Normal vTriNormal =C_Normal(vEdge1 ^ vEdge2);
		bool bFlip = ((vTriNormal | m_vDirection) > 0.f)?true:false;

		// generate clip-planes from the vertices
		AftkUt::RadialEdgeMesh::C_Plane plane;
		int nVerts = 4;
		HVec3 vPrev = vVert[nVerts-1];
		for (int i=0; i<nVerts; i++)
		{
			HVec3 vCur = vVert[i];
			C_Normal vEdge = C_Normal(vCur - vPrev);

			C_Normal planeNormal = C_Normal(vEdge ^ m_vDirection);
			if (bFlip)
				planeNormal = -planeNormal;
			Convert(plane.m_pos, vCur);
			Convert(plane.m_norm, planeNormal);
			m_BarrierList.Back().planes.PushBack(plane);
			vPrev = vCur;
		}
		// add a clippling plane for the polygon itself
		Convert(plane.m_pos, vVert[1]);
		if (bFlip)
			Convert(plane.m_norm, vTriNormal);
		else
			Convert(plane.m_norm, -vTriNormal);
		m_BarrierList.Back().planes.PushBack(plane);
	}
}

/// generate a list of objects to be shadowed
void BarrierShadowChopper::GenerateObjectList()
{
	int i = 0;

	for (Aftk::C_SceneNodeIterator it(m_pScene); it; ++it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;
		// object has to be normal mesh
		if (pAftkSceneNode->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		if (pAftkSceneNode->GetNumInstancesOfThisNode())
			continue;
#ifndef USING_TEST_SCENE
		// temporary hack to get just the track surface for St Petes
		Aftk::C_SceneNode *pParent = pAftkSceneNode->GetParentPtr();
		if (!pParent)
			continue;

		std::string parentname = pParent->GetName();
		if (parentname.substr(0,6) != "Tarmac")
			continue;
		// -- end hack
#endif


/*
		// object has to be in the root layer
		int n = pAftkSceneNode->GetNumLayers();
		for (int i=0; i<n; i++)
		{
			const Aftk::C_LayerNode *pLayer = pAftkSceneNode->GetLayerPtr(i);
			std::string name = pLayer->GetName();
			// TODO: some logic here
		}

/*
		// object has to be in the root layer
		int n = pAftkSceneNode->GetNumLayers();
		for (int i=0; i<n; i++)
		{
			const Aftk::C_LayerNode *pLayer = pAftkSceneNode->GetLayerPtr(i);
			std::string name = pLayer->GetName();
			// TODO: some logic here
		}
*/
#ifndef USING_TEST_SCENE
		if (i == 1)
#endif 
			m_ObjectList.PushBack(pAftkSceneNode);
		i++;
	}
}

S32 CreateVertexColourChannel(Aftk::C_SceneMeshNode *pMesh)
{
	std::string channelName = AFTK_DATA_CHANNEL_USAGE_VERTEX_COLOUR;

	// find channel for putting vertex colour info in
	bool found = false;
	S32 vertexColourChannel;
	for (U32 i = 0; i < pMesh->GetNumVertexDataChannels(); i++)
	{
		if (pMesh->GetVertexDataChannelInfo(i).GetName() == channelName)
		{
			if (pMesh->GetVertexDataChannelInfo(i).GetDataType() == Aftk::E_GenericAttributeType_U32)
			{
				// change channel info type - destroys previous contents
				Aftk::C_VertexDataChannelInfo info;

				info.SetName(channelName);
				info.SetDataType(Aftk::E_GenericAttributeType_U32);

				pMesh->SetVertexDataChannelInfo(i, info);
			}

			found = true;
			vertexColourChannel = i;
			break;
		}
	}

	// set vertex channel info
	if (!found)
	{
		Aftk::C_VertexDataChannelInfo info;

		info.SetName(channelName);
		info.SetDataType(Aftk::E_GenericAttributeType_U32);

		vertexColourChannel = pMesh->AddVertexDataChannelInfo(info);
	}

	return vertexColourChannel;
}


/// generate barrier shadows by clipping the barriers into the mesh
void BarrierShadowChopper::GenerateShadows()
{
	int nNodes = 0;
	Aftk::C_Status status;
	
	// TODO: make a list of objects to process and then iterate over that list.
	// this is to prevent us trying to process objects we've just created
	int n = m_ObjectList.Size();
	for (int iNode=0; iNode<n; iNode++)
	{
		Aftk::C_SceneNode* pAftkSceneNode = m_ObjectList[iNode];
		if (pAftkSceneNode->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		printf("Processing node %d/%d - BuildRadial             \r", iNode, n);

		// build a radial mesh from the current aftk scene node
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;
		CreateVertexColourChannel(pAftkSceneMeshNode);
		AftkUt::RadialEdgeMesh::C_Mesh* pRadialMesh = new AftkUt::RadialEdgeMesh::C_Mesh(pAftkSceneMeshNode, true);
		pRadialMesh->Build(status, true);
		if (!status)
		{
			LOGWARN(g_BSCLog, "Failed to build radial mesh. Some barriers won't have shadows.\n");
			continue;
		}

		// clip all the barriers into the mesh
		int iBarrier = 0;
		int nBarriers = m_BarrierList.Size();
		for (Ut::Vector<Barrier>::Iterator i=m_BarrierList.Begin(); i!=m_BarrierList.End(); ++i)
		{
			printf("Processing node %d/%d - Clip (%d/%d)      \r", iNode, n, iBarrier, nBarriers);
			pRadialMesh->ClipByPlaneList(&i->planes);
			iBarrier++;
		}

		// retrieve the clipped polygons
		Ut::Vector<AftkUt::RadialEdgeMesh::ListPoly*> list;
		iBarrier = 0;
		for (Ut::Vector<Barrier>::Iterator i=m_BarrierList.Begin(); i!=m_BarrierList.End(); ++i)
		{
			printf("Processing node %d/%d - List (%d/%d)      \r", iNode, n, iBarrier, nBarriers);
			list.Clear();
			pRadialMesh->GetPolysInsidePlaneList(&i->planes, &list, false);
			// set the vertex colours on these polygons to black

			for (int i=0; i<list.Size(); i++)
			{
				Ut::UtVector<Ut::DynamicVector<AftkUt::RadialEdgeMesh::ListVert*> > verts;
				list[i]->m_data.GetOrderedEdgeVerts(verts);

				for (int iVtx=0; iVtx<verts.Size(); iVtx++)
				{
					AftkUt::RadialEdgeMesh::ListPolyVert *pPolyVert = verts[iVtx]->m_data.GetPolygonVertex(&list[i]->m_data);
					pPolyVert->m_data.SetVertexColour(-1);
				}
			}

			iBarrier++;
		}


		printf("Processing node %d/%d (%d polys)- RemoveCoplanar          \r", iNode, n, pRadialMesh->GetNumPolys());

		//pRadialMesh->RemoveCoPlanarPolys(status);
		
		printf("Processing node %d/%d (%d polys) - BuildAftk              \r", iNode, n, pRadialMesh->GetNumPolys());

		// generate a new aftk mesh using the list of polygons we got
		Aftk::C_SceneMeshNode* pNewAftkNode = pRadialMesh->BuildAftk(status);//, &list);
		if (!status)
		{
			LOGWARN(g_BSCLog, "Failed to build new aftknode. Some barriers won't have shadows.\n");
			continue;
		}

		printf("Processing node %d/%d - done                \r", iNode, n);

		Aftk::Matrix44 worldmtx = m_pScene->GetRootSceneNodePtr()->GetWorldMatrix();
		UtMaths::Invert4x4(&worldmtx, worldmtx);
		SetIdentity(&worldmtx);
		pNewAftkNode->SetWorldMatrix(worldmtx);

		// add the new mesh to the aftk
//		m_pScene->GetRootSceneNodePtr()->AddChildPtr(pNewAftkNode);
		if (m_pScene != m_pOutScene)
		{
			Aftk::C_SceneNode *pClonedNode = (Aftk::C_SceneNode*)pNewAftkNode->Clone(m_pOutScene);
			m_pOutScene->GetRootSceneNodePtr()->AddChildPtr(pClonedNode);
		}

		nNodes++;
	}
}

/// do the processing
void BarrierShadowChopper::Process()
{
	printf("GenerateBarrierList\n");
	GenerateBarrierList();

	printf("GenerateObjectList\n");
	GenerateObjectList();

	printf("GenerateShadows\n");
	GenerateShadows();
}

void TestIW()
{
	AftkUt::g_Log.SetLevel( DebugLevel::Debug );

	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetBuildPc().c_str());
	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetBuildStamp().c_str());
	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetSvnRevision().c_str());

	Aftk::C_Global::Create();

	Aftk::C_Status status;

	Aftk::C_Scene* pAftkScene = new Aftk::C_Scene();

	LOGINFO(AftkUt::g_Log, "Reading AFTK scene...\n\n");
	pAftkScene->Read("test_scenes\\test_sphere.a3d");

	Aftk::Position3 pos0(0.3f, 0.0f, 0.3f);
	Aftk::Position3 pos1(0.3f, 0.0f, -0.3f);
	Aftk::Position3 pos2(-0.3f, 0.0f, -0.3f);
	Aftk::Position3 pos3(-0.3f, 0.0f, 0.3f);
	Aftk::Vector3F32 normVec0(-1.0f, 0.f, -1.0f);
	Aftk::Vector3F32 normVec1(-1.0f, 0.f, 1.0f);
	Aftk::Vector3F32 normVec2(1.0f, 0.f, 1.0f);
	Aftk::Vector3F32 normVec3(1.0f, 0.f, -1.0f);
	Aftk::Normal3 norm0 = (Aftk::Normal3&)normVec0;
	Aftk::Normal3 norm1 = (Aftk::Normal3&)normVec1;
	Aftk::Normal3 norm2 = (Aftk::Normal3&)normVec2;
	Aftk::Normal3 norm3 = (Aftk::Normal3&)normVec3;

	Ut::Vector<AftkUt::RadialEdgeMesh::C_Plane> planes;
	planes.PushBack(AftkUt::RadialEdgeMesh::C_Plane(pos0, norm0));
	planes.PushBack(AftkUt::RadialEdgeMesh::C_Plane(pos1, norm1));
	planes.PushBack(AftkUt::RadialEdgeMesh::C_Plane(pos2, norm2));
	planes.PushBack(AftkUt::RadialEdgeMesh::C_Plane(pos3, norm3));


	for (Aftk::C_SceneNodeIterator it(pAftkScene); it; ++it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;
		// object has to be normal mesh
		if (pAftkSceneNode->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;
		AftkUt::RadialEdgeMesh::C_Mesh* pRadialMesh = new AftkUt::RadialEdgeMesh::C_Mesh(pAftkSceneMeshNode, false);
		pRadialMesh->Build(status);

		Ut::Vector<AftkUt::RadialEdgeMesh::ListPoly*> polys;
		pRadialMesh->ClipByPlaneList(&planes, &polys);

		Aftk::C_SceneMeshNode* pNewAftkNode = pRadialMesh->BuildAftk(status, &polys);
		F32 yTrans;
		pNewAftkNode->GetTranslatePlug().GetChildPtr(1)->GetValue(yTrans);
		yTrans += 2.0f;
		((Aftk::C_StaticPlug<F32>*)pNewAftkNode->GetTranslatePlug().GetChildPtr(1))->SetValue(yTrans);

		pAftkSceneMeshNode->AddChildPtr(pNewAftkNode);
		it++;
		delete pRadialMesh;		

	}


	pAftkScene->Write("test_scenes\\test_sphere_out.a3d");

	delete pAftkScene;
	Aftk::C_Global::Destroy();
}

#define POLYFLAG_COLLISIONGROUND		"polySet_collisionGround"
#define POLYFLAG_PAVEMENT				"polySet_pavement"
#define MERGE_WITH_TRACK_ACTOR			"MergeWithTrack"

void BuildMeshListToMergeWithTrackSurface(
	Aftk::C_Scene* pScene,
	Ut::Vector<Aftk::C_SceneMeshNode*>& toMerge)
{
	Ut::Vector<std::string> flags;
	flags.PushBack(std::string(POLYFLAG_COLLISIONGROUND));
	flags.PushBack(std::string(POLYFLAG_PAVEMENT));

	for (Aftk::C_SceneNodeIterator it(pScene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;

		//We're only interested in mesehs.
		if (pSceneNode->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		Aftk::C_SceneMeshNode* pThisMesh = (Aftk::C_SceneMeshNode*)pSceneNode;

		//Look for meshes with 'merge with track specified'.
		if (AftkUt::C_NodeFlagParser::IsFlagged(*pSceneNode))
		{
			bool status;
			AftkUt::C_NodeFlagParser parser(*pSceneNode, &status);
			std::string tmp;
			if (parser.GetParameterValue(MERGE_WITH_TRACK_ACTOR, tmp) && tmp == "1")
			{
				toMerge.PushBack(pThisMesh);
				continue;
			}
			AftkUt::C_NodeFlagParser::DeleteFlagString(*pSceneNode);
		}

		//Look for polyflags indicating we should be merged.
		bool found = false;
		for (U32 i = 0; i < (U32)flags.Size(); i++)
		{
			const std::string& searchString = flags[i];

			for (U16 j = 0; j < (U16)pThisMesh->GetNumPolygonBlindDataMappings(); j++)
			{
				const Aftk::C_BlindDataResource* pRes = pThisMesh->GetPolygonBlindDataMappingPtr(j);
				if (!pRes)
					continue;

				const Aftk::C_BlindDataNode& bd = pRes->GetContent();

				for (U32 k = 0; k < bd.GetNumAttributes(); k++)
				{
					//Case sensitive compare.
					if (bd.GetAttribute(k).GetName() == searchString)
					{
						toMerge.PushBack(pThisMesh);
						goto foundFlag;
					}
				}
			}
		}
		foundFlag:	
		continue;
	}
}


void TestKM()
{
	AftkUt::g_Log.SetLevel( DebugLevel::Debug );

	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetBuildPc().c_str());
	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetBuildStamp().c_str());
	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetSvnRevision().c_str());

	Aftk::C_Global::Create();

	Aftk::C_Status status;

	Aftk::C_Scene* pAftkScene = new Aftk::C_Scene();

	LOGINFO(AftkUt::g_Log, "Reading AFTK scene...\n\n");

	pAftkScene->Read("amaxxsi.a3d");

	Aftk::C_SceneNode* p = pAftkScene->GetSceneNodePtr("cube");
	AftkUt::C_NodeFlagParser parser(*p);
	parser.StoreFlags(*p);

	std::string str = parser.GetActorName();

	pAftkScene->Write("actortest_out.a3d");

	delete pAftkScene;

	Aftk::C_Global::Destroy();
}

void TestOT()
{
	AftkUt::g_Log.SetLevel( DebugLevel::Debug );

	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetBuildPc().c_str());
	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetBuildStamp().c_str());
	LOGINFO(AftkUt::g_Log, "%s\n", AftkUt::C_Version::GetSvnRevision().c_str());

	Aftk::C_Global::Create();

	Aftk::C_Status status;

	Aftk::C_Scene* pAftkScene = new Aftk::C_Scene();

	LOGINFO(AftkUt::g_Log, "Reading AFTK scene...\n\n");

	pAftkScene->Read("steelmill_artlevel.a3d");

	AftkUt::C_RemoveBaseInstances::RemoveBaseInstances(pAftkScene);

	AftkUt::C_FindAmbiguousActorNames::ActorAttributeList actorAttrList;
	AftkUt::C_FindAmbiguousActorNames::ReadActorAttributeListFromFile("attrList.txt", actorAttrList);
	AftkUt::C_FindAmbiguousActorNames::C_FindClashingActorNames(pAftkScene);

	pAftkScene->Write("instancetestout.a3d");

	delete pAftkScene;

	Aftk::C_Global::Destroy();
}

int _tmain(int argc, _TCHAR* argv[])
{
/*
#ifdef USER_IANW
	TestIW();
	return 0;
#endif
*/

#ifdef USER_KENM
	TestKM();
	return 0;
#endif

#ifdef USER_OWENT
	TestOT();
	return 0;
#endif
}

