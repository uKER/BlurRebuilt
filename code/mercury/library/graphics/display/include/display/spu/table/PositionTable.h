#ifndef DISPLAY_SPU_POSITION_TABLE_H
#define DISPLAY_SPU_POSITION_TABLE_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



class C_PositionTable
{
public:

	C_PositionTable();

	bool	Create(const U32 count);
	void	Destroy();

	void	Build(const void * pArray, const U32 stride);

public:

	MathsStorage::Float4A16 *	m_pBuffer;
	U32							m_Count;

};



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_POSITION_TABLE_H
