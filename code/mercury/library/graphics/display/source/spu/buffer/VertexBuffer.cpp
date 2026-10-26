#include "Display/Spu/Display.h"
#include "Display/Spu/Buffer/VertexBuffer.h"



namespace Display
{
namespace Spu
{




void C_VertexBuffer::Store(const void * pBuffer, const U32 size, const U32 memoryId)
{
	cellDmaPut(pBuffer, m_Address, size, memoryId, 0, 0);
}

void C_VertexBuffer::StoreSync(const void * pBuffer, const U32 size, const U32 memoryId)
{
	cellDmaPut(pBuffer, m_Address, size, memoryId, 0, 0);
	cellDmaWaitTagStatusAll(1 << memoryId);
}



}; // namespace Spu
}; // namespace Display
