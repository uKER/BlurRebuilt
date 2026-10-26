
#pragma once

struct VertexBufferCreateInfo
{
    UINT                m_Length;
    DWORD               m_Usage;
    DWORD               m_FVF;
    D3DPOOL             m_Pool;
};

class VertexBufferResource
{
public:
                                    VertexBufferResource();
    virtual                         ~VertexBufferResource();

    void                            Init(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool);

                                    
	void						    Create(SRD::D3dWindow* pD3dWindow);
    void						    Release(); 

    S32								IsDynamic();

public:
	SRD::D3dWindow*					m_pD3dWindow;

    VertexBufferCreateInfo          m_CreateInfo;
    D3DVertexBuffer*                m_pVertexBuffer;
};