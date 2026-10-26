#ifndef __CLIENT_SIDE_H__
#define __CLIENT_SIDE_H__

#include "Vis_Core\Scene.h"

class CD3DRenderer;
class CD3DMesh;
class Frustum;
class PVSResult;

// this is the client side 'initial update' object which does the actual PVS rendering
class ClientSide : public ClientInterface
{
public:
	DECLARE_PERSISTENCE(ClientSide);

	// to be called by the server code
	void	SetScene(Scene *pScene);
	void	SetVS2(Ut::Vector<BYTE> *pData);
	
	// to be called by the client network code
			ClientSide();
			~ClientSide();
	void		Initialize();
	BaseObject *Process(BaseObject *pJob);
	void		CleanUp();

	void write( ost::Engine &ar );
	void read ( ost::Engine &ar );

protected:
    Scene			*m_pScene;		// the serializable scene mesh
	CD3DRenderer	*m_pD3DR;
	Ut::Vector<BYTE>	*m_pVS2Data;
	DWORD			m_Timer;

	void	Render(Frustum &frustum);
	void	ProcessInstances(PVSResult *pResult, Frustum *pFrustum, float3 &vCameraPos);
	int		TestInstanceBatch(int *piBatch, int n);
	void	TestAndSaveVisibilityOfInstanceBatch(PVSResult *pResult, Ut::Vector<int> &list);
	void	RenderLayer(int iLayer, Frustum &frustum);
	bool	IsInCurrentPass(int iLayer);
	void	IncVisCount(int iID, int iAmount);
	void	FlushResults();

	Ut::Vector<CD3DMesh*>	m_D3DMeshList;
	Ut::Vector<CD3DMesh*>	m_D3DBaseInstanceMeshList;
	CD3DMesh				*m_pBBoxMesh;
	IDirect3DQuery9			*m_pD3DQuery;

	bool	m_bInitialized;
	int		m_iCurrentPass;
	U32*	m_pVisCount;
	U32*	m_pClusterVisCount;
	PVSResult* m_pResult;

	static const Vertex m_BBoxVertexList[12];
};


#endif
