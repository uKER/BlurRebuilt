#include "stdafx.h"
#include "Display/Buffer/IndexBuffer.h"
#include "Display/Device/Device.h"

namespace Display
{

bool CopyIndexBuffer(C_IndexBuffer* pSrc, C_IndexBuffer* pDst)
{
	assert(pSrc);
	assert(pDst);

	if(!pSrc->IsCreated())
		return false;

	if(pSrc->GetSize() != pDst->GetSize())
	{
		Device()->Destroy(pDst);
		Device()->CreateIndexBuffer(pSrc->GetSize(), pSrc->GetFlags(), (E_Format)pSrc->GetFormat(), pDst);
	}

	if(!pDst->IsCreated())
		return false;
	
	bool result = false;
	
	void* pSrcIndex = pSrc->Lock();
	if(pSrcIndex)
	{
		void* pDstIndex = pDst->Lock();
		if(pDstIndex)
		{
			#ifdef PLATFORM_PS3
			if(memcpy(pDstIndex, pSrcIndex, pSrc->GetSize()) == 0)
			#else
			if(memcpy_s(pDstIndex, pDst->GetSize(), pSrcIndex, pSrc->GetSize()) == 0)
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
