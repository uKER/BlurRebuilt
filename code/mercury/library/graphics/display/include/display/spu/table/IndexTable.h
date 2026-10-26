#ifndef DISPLAY_SPU_INDEX_TABLE_H
#define DISPLAY_SPU_INDEX_TABLE_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



class C_IndexTable
{
public:

	C_IndexTable();

	bool	Create(const U32 count);
	void	Destroy();

	void	Load(const U32 address, const U32 memoryId = 0);
	void	LoadSync(const U32 address, const U32 memoryId = 0);

public:

	U16 *	m_pBuffer;
	U32		m_Count;

};



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_INDEX_TABLE_H
