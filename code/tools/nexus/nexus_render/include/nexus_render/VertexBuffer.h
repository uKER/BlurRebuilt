
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

                                    
	void						    Create(SRD::D3DWindow* pD3DWindow);
    void						    Release(); 

    S32								IsDynamic();

public:
	SRD::D3DWindow*					m_pD3DWindow;

    VertexBufferCreateInfo          m_CreateInfo;
    D3DVertexBuffer*                m_pVertexBuffer;
};