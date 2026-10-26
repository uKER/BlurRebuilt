#include "stdafx.h"
#include "renderingdata/vertexbuffer.h"

//#pragma optimize ("", off)

namespace RenderingData
{

const S32 vertexDataTypeSizes[] = 
{
	4, // VERTEX_ELEMENT_TYPE_S16_2,
	8, // VERTEX_ELEMENT_TYPE_S16_4,
	4, // VERTEX_ELEMENT_TYPE_S16_2N,
	8, // VERTEX_ELEMENT_TYPE_S16_4N,
	4, // VERTEX_ELEMENT_TYPE_F32_1,
	8, // VERTEX_ELEMENT_TYPE_F32_2,
	12, // VERTEX_ELEMENT_TYPE_F32_3,
	16, // VERTEX_ELEMENT_TYPE_F32_4,
	4, // VERTEX_ELEMENT_TYPE_F16_2,
	8, // VERTEX_ELEMENT_TYPE_F16_4,
	4, // VERTEX_ELEMENT_TYPE_U8_4,
	4, // VERTEX_ELEMENT_TYPE_U8_4N,
	4, // VERTEX_ELEMENT_TYPE_SHHD_3N,
	4, // VERTEX_ELEMENT_TYPE_SDDD_3N,
};

const S32 vertexDataTypeComponents[] = 
{
	2, // VERTEX_ELEMENT_TYPE_S16_2,
	4, // VERTEX_ELEMENT_TYPE_S16_4,
	2, // VERTEX_ELEMENT_TYPE_S16_2N,
	4, // VERTEX_ELEMENT_TYPE_S16_4N,
	1, // VERTEX_ELEMENT_TYPE_F32_1,
	2, // VERTEX_ELEMENT_TYPE_F32_2,
	3, // VERTEX_ELEMENT_TYPE_F32_3,
	4, // VERTEX_ELEMENT_TYPE_F32_4,
	2, // VERTEX_ELEMENT_TYPE_F16_2,
	4, // VERTEX_ELEMENT_TYPE_F16_4,
	4, // VERTEX_ELEMENT_TYPE_U8_4,
	4, // VERTEX_ELEMENT_TYPE_U8_4N,
	3, // VERTEX_ELEMENT_TYPE_SHHD_3N,
	3, // VERTEX_ELEMENT_TYPE_SDDD_3N,
};

// VertexElement

S32 VertexFormat::VertexElement::GetNumF32s() const
{
	switch (type)
	{	
		case VERTEX_ELEMENT_TYPE_F32_1:
			return 1;
		case VERTEX_ELEMENT_TYPE_F32_2:
			return 2;
		case VERTEX_ELEMENT_TYPE_F32_3:
			return 3;
		case VERTEX_ELEMENT_TYPE_F32_4:
			return 4;
		default:
			return 0;
	}
}

S32 VertexFormat::VertexElement::GetF32Offset(S32 floatIndex) const
{
	return floatIndex * 4;
}

// VertexFormat
S32 VertexFormat::AddVertexElement(const VertexElement& e)
{
	assert(e.type > 0 && e.type < VERTEX_ELEMENT_TYPE_MAX);

	S32 i = m_elements.Size();
	m_elements.PushBack(e);

	m_offsets.PushBack(m_dataSize);
	m_dataSize += vertexDataTypeSizes[e.type];

	return i;
}

bool operator == (const VertexFormat::VertexElement& v1, const VertexFormat::VertexElement& v2)
{
	return  v1.type == v2.type
			&& v1.method == v2.method
			&& v1.usage == v2.usage
			&& v1.usageIndex == v2.usageIndex;
}

bool operator != (const VertexFormat::VertexElement& v1, const VertexFormat::VertexElement& v2)
{
	return  v1.type != v2.type
			|| v1.method != v2.method
			|| v1.usage != v2.usage
			|| v1.usageIndex != v2.usageIndex;
}

bool operator < (const VertexFormat::VertexElement& v1, const VertexFormat::VertexElement& v2)
{
	if (v1.type < v2.type) return true;
	if (v2.type < v1.type) return false;
	if (v1.method < v2.method) return true;
	if (v2.method < v1.method) return false;
	if (v1.usage < v2.usage) return true;
	if (v2.usage < v1.usage) return false;
	if (v1.usageIndex < v2.usageIndex) return true;
	if (v2.usageIndex < v1.usageIndex) return false;
	return false;
}

bool operator == (const VertexFormat& v1, const VertexFormat& v2)
{
	if (v1.GetNumVertexElements() != v2.GetNumVertexElements())
		return false;

	for (int i = 0; i < v1.GetNumVertexElements(); i++)
	{
		if (v1.GetVertexElement(i) != v2.GetVertexElement(i))
			return false;
	}

	return true;
}

bool operator != (const VertexFormat& v1, const VertexFormat& v2)
{
	return !(v1 == v2);
}

bool operator < (const VertexFormat& v1, const VertexFormat& v2)
{
	if (v1.GetNumVertexElements() < v2.GetNumVertexElements()) return true;
	if (v2.GetNumVertexElements() < v1.GetNumVertexElements()) return false;

	for (int i = 0; i < v1.GetNumVertexElements(); i++)
	{
		if (v1.GetVertexElement(i) < v2.GetVertexElement(i)) return true;
		if (v2.GetVertexElement(i) < v1.GetVertexElement(i)) return false;
	}

	return false;
}

// VertexDeclaration
bool operator == (const VertexDeclaration::VertexElement& v1, const VertexDeclaration::VertexElement& v2)
{
	return v1.stream == v2.stream
			&& v1.offset == v2.offset
			&& v1.type == v2.type
			&& v1.method == v2.method
			&& v1.usage == v2.usage
			&& v1.usageIndex == v2.usageIndex;
}

bool operator != (const VertexDeclaration::VertexElement& v1, const VertexDeclaration::VertexElement& v2)
{
	return v1.stream != v2.stream
			|| v1.offset != v2.offset
			|| v1.type != v2.type
			|| v1.method != v2.method
			|| v1.usage != v2.usage
			|| v1.usageIndex != v2.usageIndex;
}

bool operator == (const VertexDeclaration& v1, const VertexDeclaration& v2)
{
	if (v1.GetNumVertexElements() != v2.GetNumVertexElements())
		return false;

	for (int i = 0; i < v1.GetNumVertexElements(); i++)
	{
		if (v1.GetVertexElement(i) != v2.GetVertexElement(i))
			return false;
	}

	return true;
}

// VertexData
VertexData::VertexData(const VertexData& vertexData) : m_nElements(0), m_capacity(0), m_pData(0)
{
	m_vertexFormat = vertexData.m_vertexFormat;

	Reserve(vertexData.Size());

	for (int i = 0; i < vertexData.Size(); i++)
		PushBack(vertexData[i]);
}

VertexData& VertexData::operator = (const VertexData& vertexData)
{
	delete [] m_pData;
	m_nElements = 0;
	m_capacity = 0;
	m_pData = 0;

	m_vertexFormat = vertexData.m_vertexFormat;

	Reserve(vertexData.Size());

	for (int i = 0; i < vertexData.Size(); i++)
		PushBack(vertexData[i]);

	return *this;
}

void VertexData::Reserve(S32 n)
{
	if (m_capacity >= n) 
		return;
	
	if (n > 0)
	{
		U32* pNew = new U32[n * m_vertexFormat.GetDataSize() / 4];
		assert(pNew);
		memset(pNew, 0, n * m_vertexFormat.GetDataSize());

		if (m_nElements > 0)
			memcpy(pNew, m_pData, m_nElements * m_vertexFormat.GetDataSize());

		delete [] m_pData;
		m_pData = pNew;
	}

	m_capacity = n;
}

S32 VertexData::PushBack()
{
	if (Size() == Capacity())
		Reserve((Capacity() * 3) / 2 + 1);

	m_nElements++;
	return m_nElements - 1;
}

S32 VertexData::PushBack(const ConstVertexReference& vertex)
{
	assert(m_vertexFormat == vertex.GetVertexFormat());

	if (Size() == Capacity())
	{
		VertexData v;
		v.Create(vertex.GetVertexFormat());
		v.PushBack();
		v[0] = vertex; // copy it in case it is an item already in the vector
		Reserve((Capacity() * 3) / 2 + 1);
		m_nElements++;
		VertexReference dst = (*this)[m_nElements - 1];
		dst = v[0];
	}
	else
	{
		m_nElements++;
		VertexReference dst = (*this)[m_nElements - 1];
		dst = vertex;
	}
	
	return m_nElements - 1;
}

// VertexReference
void VertexData::VertexReference::SetData(S32 element, void* p, U32 size)
{
	U32 typeSize = vertexDataTypeSizes[m_vb.m_vertexFormat.GetVertexElement(element).type];
	assert(size == typeSize);
	memcpy((void*) &GetData<U8>(element), p, Maths::Min(size, typeSize));
}

S32 Compare(const VertexData::ConstVertexReference& p1, const VertexData::ConstVertexReference& p2)
{
	if (p1.GetVertexFormat().GetDataSize() < p2.GetVertexFormat().GetDataSize()) return -1;
	if (p1.GetVertexFormat().GetDataSize() > p2.GetVertexFormat().GetDataSize()) return 1;

	for (U32 i = 0; i < p1.GetVertexFormat().GetDataSize(); i++)
	{
		U8 b1 = *(&p1 + i);
		U8 b2 = *(&p2 + i);
		if (b1 < b2) return -1;
		if (b1 > b2) return 1;
	}

	return 0;
}

template<typename Dest, typename Source> 
void CopyData1(Dest* pDest, Source* pSource)
{
	pDest->X((typename Dest::DataType)pSource->X());
}

template<typename Dest, typename Source> 
void CopyData2(Dest* pDest, Source* pSource)
{
	pDest->X((typename Dest::DataType)pSource->X());
	pDest->Y((typename Dest::DataType)pSource->Y());
}

template<typename Dest, typename Source> 
void CopyData3(Dest* pDest, Source* pSource)
{
	pDest->X((typename Dest::DataType)pSource->X());
	pDest->Y((typename Dest::DataType)pSource->Y());
	pDest->Z((typename Dest::DataType)pSource->Z());
}

template<typename Dest, typename Source> 
void CopyData4(Dest* pDest, Source* pSource)
{
	pDest->X((typename Dest::DataType)pSource->X());
	pDest->Y((typename Dest::DataType)pSource->Y());
	pDest->Z((typename Dest::DataType)pSource->Z());
	pDest->W((typename Dest::DataType)pSource->W());
}

void ChangeDataType(U8* pSource, U8* pDest, VertexElementType originalType, VertexElementType finalType)
{
	assert(pSource != pDest);
	memset(pDest, 0, vertexDataTypeSizes[finalType]);

	if (originalType == finalType)
	{
		memcpy(pDest, pSource, vertexDataTypeSizes[originalType]);
	}
	else if (finalType == VERTEX_ELEMENT_TYPE_S16_4) // all integer types convert from S16_4
	{
		DataTypes::S16_4* p = (DataTypes::S16_4*) pDest;

		switch (originalType)
		{
			case VERTEX_ELEMENT_TYPE_S16_2: CopyData2(p, (DataTypes::S16_2*) pSource); break;
			case VERTEX_ELEMENT_TYPE_U8_4: CopyData4(p, (DataTypes::U8_4*) pSource); break;
			default:
				assert(!"Cannot convert vertex types.");
		}
	}
	else if (finalType == VERTEX_ELEMENT_TYPE_F32_4) // all non-integer types convert from F32_4
	{
		DataTypes::F32_4* p = (DataTypes::F32_4*) pDest;

		switch (originalType)
		{
			case VERTEX_ELEMENT_TYPE_S16_2N: CopyData2(p, (DataTypes::S16_2N*) pSource); break;
			case VERTEX_ELEMENT_TYPE_S16_4N: CopyData4(p, (DataTypes::S16_4N*) pSource); break;
			case VERTEX_ELEMENT_TYPE_F32_1: CopyData1(p, (DataTypes::F32_1*) pSource); break;
			case VERTEX_ELEMENT_TYPE_F32_2: CopyData2(p, (DataTypes::F32_2*) pSource); break;
			case VERTEX_ELEMENT_TYPE_F32_3: CopyData3(p, (DataTypes::F32_3*) pSource); break;
			case VERTEX_ELEMENT_TYPE_F16_2: CopyData2(p, (DataTypes::F16_2*) pSource); break;
			case VERTEX_ELEMENT_TYPE_F16_4: CopyData4(p, (DataTypes::F16_4*) pSource); break;
			case VERTEX_ELEMENT_TYPE_U8_4N: CopyData4(p, (DataTypes::U8_4N*) pSource); break;
			case VERTEX_ELEMENT_TYPE_SHHD_3N: CopyData3(p, (DataTypes::SHHD_3N*) pSource); break;
			case VERTEX_ELEMENT_TYPE_SDDD_3N: CopyData3(p, (DataTypes::SDDD_3N*) pSource); break;
			default:
				assert(!"Cannot convert vertex types.");
		}
	}
	else
	{
		switch (originalType)
		{
			case VERTEX_ELEMENT_TYPE_U8_4:
			case VERTEX_ELEMENT_TYPE_S16_2:
			{
				DataTypes::S16_4 temp;
				ChangeDataType(pSource, (U8*) &temp, originalType, VERTEX_ELEMENT_TYPE_S16_4);
				ChangeDataType((U8*) &temp, pDest, VERTEX_ELEMENT_TYPE_S16_4, finalType);
				break;
			}
			case VERTEX_ELEMENT_TYPE_S16_4: // convert to all other integer types
			{
				DataTypes::S16_4* p = (DataTypes::S16_4*) pSource;

				switch (finalType)
				{
					case VERTEX_ELEMENT_TYPE_S16_2: CopyData2((DataTypes::S16_2*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_U8_4: CopyData4((DataTypes::U8_4*)pDest, p); break;
					default:
						assert(!"Cannot convert vertex types.");
				}

				break;
			}
			case VERTEX_ELEMENT_TYPE_F32_1:
			case VERTEX_ELEMENT_TYPE_F32_2:
			case VERTEX_ELEMENT_TYPE_F32_3:
			case VERTEX_ELEMENT_TYPE_F16_2:
			case VERTEX_ELEMENT_TYPE_F16_4:
			case VERTEX_ELEMENT_TYPE_S16_2N:
			case VERTEX_ELEMENT_TYPE_S16_4N:
			case VERTEX_ELEMENT_TYPE_U8_4N:
			case VERTEX_ELEMENT_TYPE_SHHD_3N:
			case VERTEX_ELEMENT_TYPE_SDDD_3N:
			{
				DataTypes::F32_4 temp;
				ChangeDataType(pSource, (U8*) &temp, originalType, VERTEX_ELEMENT_TYPE_F32_4);
				ChangeDataType((U8*) &temp, pDest, VERTEX_ELEMENT_TYPE_F32_4, finalType);
				break;
			}
			case VERTEX_ELEMENT_TYPE_F32_4: // convert to all non-integer types
			{
				DataTypes::F32_4* p = (DataTypes::F32_4*) pSource;

				switch (finalType)
				{
					case VERTEX_ELEMENT_TYPE_S16_2N: CopyData2((DataTypes::S16_2N*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_S16_4N: CopyData2((DataTypes::S16_4N*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_F32_1: CopyData1((DataTypes::F32_1*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_F32_2: CopyData2((DataTypes::F32_2*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_F32_3: CopyData3((DataTypes::F32_3*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_F16_2: CopyData2((DataTypes::F16_2*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_F16_4: CopyData4((DataTypes::F16_4*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_SHHD_3N: CopyData3((DataTypes::SHHD_3N*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_SDDD_3N: CopyData3((DataTypes::SDDD_3N*)pDest, p); break;
					case VERTEX_ELEMENT_TYPE_U8_4N: 
					{
						// Input is signed, remap to 0-1.
						p->X(Maths::Clamped((p->X() + 1.f) * 0.5f, 0.f, 1.f));
						p->Y(Maths::Clamped((p->Y() + 1.f) * 0.5f, 0.f, 1.f));
						p->Z(Maths::Clamped((p->Z() + 1.f) * 0.5f, 0.f, 1.f));
						CopyData4((DataTypes::U8_4N*)pDest, p); 
						break;
					}
					default:
						assert(!"Cannot convert vertex types.");
				}

				break;
			}
			default:
				assert(!"Cannot convert vertex types.");
		}
	}
}

void VertexData::ReverseU8_4()
{
	for (int element = 0; element < GetVertexFormat().GetNumVertexElements(); element++)
	{
		if (GetVertexFormat().GetVertexElement(element).type == VERTEX_ELEMENT_TYPE_U8_4N
			|| GetVertexFormat().GetVertexElement(element).type == VERTEX_ELEMENT_TYPE_U8_4)
		{
			for (int vertex = 0; vertex < Size(); vertex++)
			{
				U8* pData = ((U8*) m_pData) + m_vertexFormat.GetDataSize() * vertex + m_vertexFormat.GetOffset(element);
				Btl::Swap(pData[0], pData[3]);
				Btl::Swap(pData[1], pData[2]);
			}
		}
	}
}

void VertexData::ChangeVertexElementType(VertexElementType originalType, VertexElementType finalType)
{
	assert(vertexDataTypeSizes[originalType] == vertexDataTypeSizes[finalType]);
	assert(vertexDataTypeComponents[originalType] <= vertexDataTypeComponents[finalType]);

	for (int element = 0; element < GetVertexFormat().GetNumVertexElements(); element++)
	{
		if (GetVertexFormat().GetVertexElement(element).type == originalType)
		{
			m_vertexFormat.m_elements[element].type = finalType;
			
			for (int vertex = 0; vertex < Size(); vertex++)
			{
				U8* pData = ((U8*) m_pData) + m_vertexFormat.GetDataSize() * vertex + m_vertexFormat.GetOffset(element);
				U8 temp[16];
				memcpy(temp, pData, vertexDataTypeSizes[originalType]);
				ChangeDataType(temp, pData, originalType, finalType);
			}
		}
	}
}

}
