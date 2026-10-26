
#pragma once

template <class VERTEXTYPE> class DebugVertexBuffer 
{
public:
                                        DebugVertexBuffer();
                                        ~DebugVertexBuffer();

	void                                Init(SRD::D3DWindow* pDisplayWindow, const U32 count, const U32 dynamic);
    void                                Close();

	U32				                    LockVertices( const U32 noToLock, VERTEXTYPE*& vertexPtr ); // returns index of first locked vertex
	void				                UnlockVertices();

public:
	SRD::D3DWindow*						m_pD3DWindow;
    U32                                 m_VertexCount;
    U32                                 m_Dynamic;
    U32                                 m_CurrVertexIndex;
    U32                                 m_IsLocked;

    //D3DVertexBuffer*                    m_pVertexBuffer;
    VertexBufferResource                m_VertexBuffer;
};


/////////////////////////////////////////////////////
// DebugVertexBuffer implementation
/////////////////////////////////////////////////////

template <class VERTEXTYPE> DebugVertexBuffer<VERTEXTYPE>::DebugVertexBuffer()
{
    m_IsLocked = 0;
}


template <class VERTEXTYPE> void DebugVertexBuffer<VERTEXTYPE>::Init(SRD::D3DWindow* pD3DWindow, const U32 count, const U32 dynamic)
{
    m_pD3DWindow = pD3DWindow;
    m_Dynamic = dynamic;
    m_VertexCount = count;
    m_CurrVertexIndex = 0;

    m_VertexBuffer.m_CreateInfo.m_Length = m_VertexCount*sizeof(VERTEXTYPE);
    m_VertexBuffer.m_CreateInfo.m_FVF = ((VERTEXTYPE*)NULL)->GetFVF();
/*
    if(m_Dynamic)
	{
		m_VertexBuffer.m_CreateInfo.m_Usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
        m_VertexBuffer.m_CreateInfo.m_Pool = D3DPOOL_DEFAULT;
	}
	else
*/	{
        m_VertexBuffer.m_CreateInfo.m_Usage = D3DUSAGE_WRITEONLY;
        m_VertexBuffer.m_CreateInfo.m_Pool = D3DPOOL_MANAGED;
    }

    m_VertexBuffer.Create(m_pD3DWindow);
}

template <class VERTEXTYPE> void DebugVertexBuffer<VERTEXTYPE>::Close()
{
	m_VertexBuffer.Release();

    m_pD3DWindow = NULL;
}

template <class VERTEXTYPE> U32 DebugVertexBuffer<VERTEXTYPE>::LockVertices( const U32 noToLock, VERTEXTYPE*& vertexPtr ) 
{
    if(m_IsLocked)
    {
        //dbError(DBSTR "Vertex buffer already locked");
    }

    // returns index of first locked vertex
	U32 lockType = D3DLOCK_NOOVERWRITE;
	//[Ken]
	lockType = 0;

	if((m_CurrVertexIndex + noToLock) > m_VertexCount)
	{
		//if(!m_Dynamic)
			//dbError("DebugVertexBuffer::LockVertices() - Overrun vertex buffer");

		m_CurrVertexIndex = 0;
        lockType = D3DLOCK_DISCARD;
		//[Ken]
		lockType = 0;
	}
	else
	{
		if(!m_Dynamic)
		{
            lockType = 0;
		}
		else
		{
            lockType = D3DLOCK_NOOVERWRITE;
			//[Ken]
			lockType = 0;
		}
	}

	m_VertexBuffer.m_pVertexBuffer->Lock( 
        m_CurrVertexIndex * sizeof(VERTEXTYPE), 
        noToLock * sizeof(VERTEXTYPE), 
        (void**)&vertexPtr, 
        lockType );

	U32 baseVertex = m_CurrVertexIndex;
	m_CurrVertexIndex += noToLock;

    m_IsLocked = 1;

	return(baseVertex);
}

template <class VERTEXTYPE> void DebugVertexBuffer<VERTEXTYPE>::UnlockVertices()
{
    if(!m_IsLocked)
    {
        //dbError(DBSTR "Vertex buffer not locked");
    }

    m_IsLocked = 0;
	m_VertexBuffer.m_pVertexBuffer->Unlock();
}

template <class VERTEXTYPE> DebugVertexBuffer<VERTEXTYPE>::~DebugVertexBuffer()
{
    if(m_VertexBuffer.m_pVertexBuffer)
    {
        //dbWarn(DBSTR "Debug Vertex Buffer Not Closed");
        Close();
    }
}




