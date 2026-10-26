#ifndef __SHLIGHTING_H__
#define __SHLIGHTING_H__

class JobScheduler;

#define SPEED_TREE_SAMPLE_POINTS 8

namespace LUX
{

class SurfaceIntegrator;
class SHLightingResult;

// class that contains the result of an SH Lighting Job
class SHLightingResult : public BaseObject
{
	DECLARE_PERSISTENCE(SHLightingResult);

	Ut::Vector<float>	m_ResultList;
	int					m_iResultStride;
	int					m_iItemIdx;

	void write(ost::Engine &ar)
	{
		ar << m_ResultList << m_iItemIdx << m_iResultStride;
	}
	void read(ost::Engine &ar)
	{
		ar >> m_ResultList >> m_iItemIdx >> m_iResultStride;
	}
};

// class that contains information for a SH lighting rendering job
class SHLightingJob : public BaseObject
{
public:
	DECLARE_PERSISTENCE(SHLightingJob);

	// information for one SH item to render
	struct Item
	{
		box3f			m_localbbox;
		box3f			m_worldspacebbox;
		Aftk::Matrix44	m_matrix;		// the object's world-space matrix
		int				m_iUniqueID;	// Unique ID of the aftk node (to turn it on and off in the collision tree)

		// generate SH lighting info from an aftk node to send to the server
		void GenerateFromNode(Aftk::C_SceneNode *pNode);
		// client-side processing function that lights this item
		void Process(SurfaceIntegrator *pIntegrator, Ut::Vector<float> &results, SHLightingJob *pParentJob);
		// client-side processing function that lights this item using the given LUX::DataType
		template<typename T>
		void ProcessTemplated(SurfaceIntegrator *pIntegrator, Ut::Vector<float> &results, SHLightingJob *pParentJob);

		// serialize / deserialize
		friend ost::Engine& operator << (ost::Engine& ar, Item &t)
		{
			ar << t.m_matrix;
			ar << t.m_iUniqueID;
			ar << t.m_localbbox;
			ar << t.m_worldspacebbox;
			return ar;
		}
		friend ost::Engine& operator >> (ost::Engine& ar, Item &t)
		{
			ar >> t.m_matrix >> t.m_iUniqueID >> t.m_localbbox >> t.m_worldspacebbox;
			return ar;
		}
	};	

	// job contains a list of a bunch of SH items to render in one go
	Ut::Vector<Item> m_ItemList;
	int				m_iItemIdx;
	float			m_fBrightness;
	float			m_fInvGamma;
	int				m_iRenderMode;
	Ut::BitArray128		m_iLightingPassBits;

	// process an SH lighting job - this does the actual client-side rendering
	SHLightingResult *Process(SurfaceIntegrator *pIntegrator);

	void write(ost::Engine &ar)
	{
		ar << m_ItemList << m_iItemIdx << m_fBrightness << m_fInvGamma << m_iLightingPassBits << m_iRenderMode;
	}
	void read(ost::Engine &ar)
	{
		ar >> m_ItemList >> m_iItemIdx >> m_fBrightness >> m_fInvGamma >> m_iLightingPassBits >> m_iRenderMode;
	}
};

// class that keeps track of which nodes to light, generating jobs, saving the results etc.
class SHLighting
{
public:
	// initialize with some global variables
	SHLighting(int iRenderMode, float fBrightness, float fGamma, const Ut::BitArray128 &iLightingPassBits);

	// add a node to the list of nodes that need SH lighting
	void	AddNodeToLight(Aftk::C_SceneNode *pNode);

	// use the list of SH nodes to generate jobs on the scheduler
	void	AddJobs(JobScheduler *pScheduler);

	// given a scheduler job index, generate the data to be sent to the client
	BaseObject *GenerateJob(int iID);

	// receive data back from the client and store it in the SH
	void	StoreJobResult(BaseObject *pInResult);

private:
	Ut::Vector<Aftk::C_SceneNode*>	m_ProcessList;	// list of nodes that need SH lighting

	static const int			m_iBatchSize;	// amount of nodes that will be batched in one job
	float						m_fBrightness;	// copied from GIProperties
	float						m_fInvGamma;
	Ut::BitArray128				m_iLightingPassBits;
	int							m_iRenderMode;
};

}

#endif
