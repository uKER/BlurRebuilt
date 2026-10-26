#ifndef __VTXLIGHTING_H__
#define __VTXLIGHTING_H__

class JobScheduler;

#define SPEED_TREE_SAMPLE_POINTS 8

namespace LUX
{
	
class Lux;
class SurfaceIntegrator;
class VtxLightingResult;

// class that contains the result of a vtx Lighting Job
class VtxLightingResult : public BaseObject
{
public:
	DECLARE_PERSISTENCE(VtxLightingResult);

	Ut::Vector<float>	m_Data;
	int					m_iStride;
	int					m_nVertices;
	bool				m_bSmoothSpeedTreeLeaves;
	int					m_iItem;
	Ut::Vector<U8>		m_CustomData;
	bool				m_bFoundQNAN;

	void write(ost::Engine &ar)
	{
		ar << m_Data << m_iStride << m_bSmoothSpeedTreeLeaves << m_iItem << m_CustomData << m_nVertices;
	}
	void read(ost::Engine &ar)
	{
		ar >> m_Data >> m_iStride >> m_bSmoothSpeedTreeLeaves >> m_iItem >> m_CustomData >> m_nVertices;
	}
};

// class that contains information for a SH lighting rendering job
class VtxLightingJob : public BaseObject
{
public:
	DECLARE_PERSISTENCE(VtxLightingJob);

	// information for one vertex to render
	struct Vertex
	{
		// serialize / deserialize
		friend ost::Engine& operator << (ost::Engine& ar, Vertex &t)
		{
			ar << t.vPos << t.vNormal;
			return ar;
		}
		friend ost::Engine& operator >> (ost::Engine& ar, Vertex &t)
		{
			ar >> t.vPos >> t.vNormal;
			return ar;
		}

		float3 vPos;
		float3 vNormal;
	};	

	// process a lighting job - this does the actual client-side rendering
	VtxLightingResult *Process(SurfaceIntegrator *pIntegrator);

	// job contains a list of vertices to render for this node
	Ut::Vector<Vertex>	m_Vertices;
	float				m_fBrightness;
	float				m_fInvGamma;
	Ut::BitArray128		m_iLightingPassBits;
	box3f				m_bbox;
	int					m_iRenderMode;
	bool				m_bBouncePass;
	bool				m_bSmoothSpeedTreeLeaves;
	int					m_iItem;

	void write(ost::Engine &ar)
	{
		ar << m_Vertices << m_iItem << m_fBrightness << m_fInvGamma << m_iLightingPassBits;
		ar << m_iRenderMode << m_bBouncePass << m_bSmoothSpeedTreeLeaves << m_bbox;
	}
	void read(ost::Engine &ar)
	{
		ar >> m_Vertices >> m_iItem >> m_fBrightness >> m_fInvGamma >> m_iLightingPassBits;
		ar >> m_iRenderMode >> m_bBouncePass >> m_bSmoothSpeedTreeLeaves >> m_bbox;
	}
};

// class that keeps track of which nodes to light, generating jobs, saving the results etc.
class VtxLighting
{
public:
	struct Item
	{
		Aftk::I_SceneMeshNode	*pNode;
		bool					m_bSmoothSpeedTreeLeaves;
	};

	// initialize with some global variables
	VtxLighting(Aftk::C_Scene *pScene, float fBrightness, float fGamma, Lux *pLux);

	// add a node to the list of nodes that needs lighting
	void	AddNodeToLight(Aftk::C_SceneNode *pNode, bool bSmoothSpeedTreeLeaves);

	// use the list of nodes to generate jobs on the scheduler
	void	AddJobs(JobScheduler *pScheduler);

	// create a vertex data channel on this object for the vertex data to go.
	// also handles putting vertex data for instances
	void	CreateVertexDataChannel(Aftk::I_SceneMeshNode *_pMesh, Aftk::C_SceneMeshNode **ppMeshNode, int *piVertexColourChannel, int iStride);

	// given a scheduler job index, generate the data to be sent to the client
	// node can be either an instance or a meshnode
	BaseObject *GenerateJob(int iID);

	// receive data back from the client and store it
	void	StoreJobResult(BaseObject *pInResult);

	// return number of items in the process list
	int		GetNumItems();

	// return a particular item in the process list
	Item	*GetItem(int i);

	// turn on bounce mode (ignore brightness/gamma when on)
	void	SetBouncePass(bool bOn);

	// set rendering mode
	void	SetRenderMode(int iMode);

private:
	Ut::Vector<Item>	m_ProcessList;	// list of nodes that need lighting

	float						m_fBrightness;	// copied from GIProperties
	float						m_fInvGamma;
	Lux *						m_pLux;
	bool						m_bBouncePass;
	Aftk::C_Scene				*m_pScene;
	int							m_iRenderMode;
};

}

#endif
