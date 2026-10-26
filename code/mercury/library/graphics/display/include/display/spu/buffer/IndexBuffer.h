#ifndef DISPLAY_SPU_INDEX_BUFFER_H
#define DISPLAY_SPU_INDEX_BUFFER_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



class C_IndexBuffer
{

public:

	void Store(const void * pBuffer, const U32 size, const U32 memoryId = 0);
	void StoreSync(const void * pBuffer, const U32 size, const U32 memoryId = 0);

public:

	U32	m_Address;
	U32	m_Offset;
	U32	m_Location;
	U32	m_Size;

};



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_INDEX_BUFFER_H
