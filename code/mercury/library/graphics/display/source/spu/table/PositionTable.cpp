#include <cell/dma.h>

#include <Core/Core.h>
#include <Core/Maths.h>

#include "Display/Spu/Table/PositionTable.h"



namespace Display
{
namespace Spu
{



C_PositionTable::C_PositionTable()
{
	m_pBuffer = 0;
}

bool C_PositionTable::Create(const U32 count)
{
	m_Count = count;
	m_pBuffer = (MathsStorage::Float4A16 *) memalign(0x80, m_Count * sizeof(MathsStorage::Float4A16));

	return true;
}

void C_PositionTable::Destroy()
{
	if (m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = 0;
	}
}

void C_PositionTable::Build(const void * pArray, const U32 stride)
{
	MathsStorage::Float4A16 * pPosition = m_pBuffer;

	F32 * pVertex = (F32 *) pArray;
	for (U32 i = 0; i < m_Count; ++i)
	{
		pPosition->x = pVertex[0];
		pPosition->y = pVertex[1];
		pPosition->z = pVertex[2];
		pPosition->w = 1.0f;

		pPosition += 1;
		pVertex += stride >> 2;
	}
}



}; // namespace Spu
}; // namespace Display
