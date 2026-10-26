#ifndef __SCENE_H__
#define __SCENE_H__

#include "Vis_Core\Polyref.h"

namespace RenderingData
{
class C_Converter;
}

class MeshNode;

typedef enum
{
	SplitMode_Monolith,
	SplitMode_Object,
	SplitMode_Tree,
} SplitMode;

class Instance
{
public:
	box3f	m_bbox;
	float	m_fDrawDistance;
	int		m_iBaseInstance;
	int		m_iAftkLayer;
	int		m_iRefIdx;

	friend ost::Engine& operator << (ost::Engine& ar, const Instance &t) { ar << t.m_bbox << t.m_fDrawDistance << t.m_iBaseInstance << t.m_iAftkLayer << t.m_iRefIdx; return ar; }
	friend ost::Engine& operator >> (ost::Engine& ar, Instance &t)		 { ar >> t.m_bbox >> t.m_fDrawDistance >> t.m_iBaseInstance >> t.m_iAftkLayer >> t.m_iRefIdx; return ar; }
};

class PVSPass
{
public:
	Ut::Vector<int>	m_PVSLayers;
	Ut::Vector<int> m_BothLayers;
	Ut::Vector<int>	m_OcclusionLayers;

	friend ost::Engine& operator << (ost::Engine& ar, const PVSPass &t) { ar << t.m_PVSLayers << t.m_BothLayers << t.m_OcclusionLayers; return ar; }
	friend ost::Engine& operator >> (ost::Engine& ar, PVSPass &t)		{ ar >> t.m_PVSLayers >> t.m_BothLayers >> t.m_OcclusionLayers; return ar; }
};

class Scene : public BaseObject
{
public:
	DECLARE_PERSISTENCE(Scene);

			Scene();
			~Scene();

	void write( ost::Engine &ar );
	void read ( ost::Engine &ar );

	Ut::Vector<PolygonRef>	*GetPolyRefs()		{ return m_pPolyRefs; }
	int						GetNumIds()			{ return m_iCurId; }
	int						GetNumTriangles()	{ return m_iFirstInstance; }

	int						GetNumClusterIds()	{ return m_iCurClusterId; }
	int						GetClusterRef(int i) { return m_ClusterRefs[i]; }

	int						GetNumNodes()		{ return m_MeshNodeList.Size(); }
	MeshNode				*GetNode(int i)		{ return m_MeshNodeList[i]; }

	int						GetNumBaseInstances()	{ return m_BaseInstanceList.Size(); }
	MeshNode				*GetBaseInstance(int i)	{ return m_BaseInstanceList[i]; }

	int						GetFirstInstanceIdx()			{ return m_iFirstInstance; }
	int						GetNumInstances()				{ return m_InstanceList.Size(); }
	const Instance			&GetInstance(int i)				{ return m_InstanceList[i]; }
	box3f					&GetInstanceBBox(int i)			{ return m_InstanceList[i].m_bbox; }
	float					&GetInstanceDrawDistance(int i)	{ return m_InstanceList[i].m_fDrawDistance; }

	void					SetClusterThreshold(int i)		{ m_iClusterThreshold = i; }
	int						GetClusterThreshold()			{ return m_iClusterThreshold ; }

	void					SetPolygonThreshold(int i)		{ m_iPolygonThreshold = i; }
	int						GetPolygonThreshold()			{ return m_iPolygonThreshold; }

	void					TakeOwnerShipOfPolyRefs();
	void					LoadAftkScene(Aftk::C_Scene *pAftk);
	void					SetPaths(const char *pszTexCachePath, const char *pszFXBPath);

	void					AddPVSPass(PVSPass &pass);

	Ut::Vector<PVSPass>		m_Passes;
	Ut::Vector<MeshNode*>	m_MeshNodeList;

protected:
	void	Initialize();
	void	AddMesh(Aftk::C_SceneMeshNode* pAftkMesh, MeshNode *pExternalMeshNode, bool *pbHasOcclusionQuery);
	void	AddPolygon(const Aftk::C_SceneMeshNode& aftkMesh, 
					   const Aftk::C_Polygon& poly, 
					   const Aftk::Matrix44& mtx, 
					   U32 iFace, U32 cluster,
					   MeshNode* pExternalMeshNode,
					   FaceFlags& faceFlags);
	/// add a base-instance mesh. returns the index of the mesh
	int		AddBaseInstance(Aftk::C_SceneMeshNode *pBaseInstance);

	void	Build_Monolith(Aftk::C_Scene *pAftk);
	void	Build_Tree(Aftk::C_Scene *pAftk);
	void	AddInstances(Aftk::C_Scene *pAftk);
	void	AddInstance(Aftk::C_SceneNode *pInstance, Aftk::I_SceneMeshNode *pRefNode, int iBaseInstance);
	bool	NodeInCurrentLayer(Aftk::C_SceneNode *pNode);

	// return true if the polygon has a "do not export" polyflag on
	bool	DoesPolygonHaveNoExportPolyFlag(Aftk::C_SceneMeshNode *pAftkMesh, Aftk::C_Polygon *pPoly);
	// check whether this node has "pvsBBox" properties and if so add it to the instance list with the given bbox dimensions
	bool	NodeHasPVSBlindData(Aftk::C_SceneNode *pNode);

	int						m_iCurId;
	int						m_iFirstInstance;
	int						m_iCurNode;
	int						m_nPolyRefs;
	Ut::Vector<PolygonRef>	*m_pPolyRefs;
	Ut::Vector<Instance>	m_InstanceList;
	Ut::Vector<MeshNode*>	m_BaseInstanceList;
	Ut::Vector<Aftk::C_SceneMeshNode*>	m_PVSOcclusionQueryNodes;
	int						m_iTransparentForPVSPolyFlagMask;
	int						m_iCurrentLayer;
	int						m_iClusterThreshold;
	int						m_iPolygonThreshold;

	Ut::Vector<U32>			m_ClusterRefs;	//same length as m_pPolyRefs
	int						m_iCurClusterId;

	RenderingData::C_Converter *m_pConverter;

	mutable const Aftk::C_MaterialNode*		m_pPreviousNode;
	mutable const Aftk::C_BlindDataNode*	m_pPreviousBdNode;
	mutable FaceFlags						m_previousFaceFlags;
};



#endif
