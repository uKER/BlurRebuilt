#include "stdafx.h"
#include "Display/Buffer/VertexBuffer.h"
#include "Display/Device/Device.h"

namespace Display
{

bool CopyVertexBuffer(C_VertexBuffer* pSrc, C_VertexBuffer* pDst)
{
	assert(pSrc);
	assert(pDst);

	if(!pSrc->IsCreated())
		return false;

	if(pSrc->GetSize() != pDst->GetSize())
	{
		Device()->Destroy(pDst);
		Device()->CreateVertexBuffer(pSrc->GetSize(), pSrc->GetFlags(), pDst);
	}

	if(!pDst->IsCreated())
		return false;
	
	bool result = false;
	
	void* pSrcVertex = pSrc->Lock();
	if(pSrcVertex)
	{
		void* pDstVertex = pDst->Lock();
		if(pDstVertex)
		{
			#ifdef PLATFORM_PS3
			if(memcpy(pDstVertex, pSrcVertex, pSrc->GetSize()) == 0)
			#else
			if(memcpy_s(pDstVertex, pDst->GetSize(), pSrcVertex, pSrc->GetSize()) == 0)
			#endif
			{
				result = true;
			}
		}
		pDst->Unlock();
	}
	pSrc->Unlock();

	return result;
}

}; // namespace Display
