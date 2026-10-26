#include "stdafx.h"
#include "renderingdata/effect.h"

namespace RenderingData
{

// MaterialElement
bool operator == (const MaterialElement& p1, const MaterialElement& p2)
{
	return (p1.name == p2.name)
			&& (p1.type == p2.type)
			&& (p1.count == p2.count);
}

bool operator != (const MaterialElement& p1, const MaterialElement& p2)
{
	return (p1.name != p2.name)
			|| (p1.type != p2.type)
			|| (p1.count != p2.count);
}

// MaterialType
MaterialType::MaterialType()
{
	m_effect = -1;
	m_listIndex = 0;

	m_dataSize = 0;
	m_maxAlign = 1;
}

U32 MaterialType::GetDataSize() const
{
	return Btl::AlignUp(m_dataSize, m_maxAlign);
}

U32 MaterialType::GetElementDataSize(S32 element) const
{
	assert(element >= 0 && element < m_offsets.Size());

	if(element == m_offsets.Size() - 1)
		return GetDataSize() - m_offsets[element];
	else
		return m_offsets[element+1] - m_offsets[element];
}

S32 MaterialType::GetElementIndex(const Btl::String& name) const
{
	for (int i = 0; i < m_elements.Size(); i++)
	{
		if (m_elements[i].name == name)
			return i;
	}

	return -1;
}

S32 MaterialType::AddElement(const MaterialElement& e, S32 align)
{
	assert(e.type > 0 && e.type < MATERIAL_ELEMENT_TYPE_MAX);

	S32 i = m_elements.Size();
	m_elements.PushBack(e);

	// align
	align = Maths::Max(materialDataTypeAlignment[e.type], align);
	m_dataSize = Btl::AlignUp(m_dataSize, align);

	m_offsets.PushBack(m_dataSize);
	m_dataSize += materialDataTypeSizes[e.type] * e.count;

	// align total size to max
	m_maxAlign = Maths::Max(m_maxAlign, (U32)align);

	return i;
}

bool MaterialType::operator == (const MaterialType& other) const
{
	if (m_name != other.m_name) return false;
	if (m_effect != other.m_effect) return false;
	if (m_listIndex != other.m_listIndex) return false;
	if (m_elements.Size() != other.m_elements.Size()) return false;

	for (int i = 0; i < m_elements.Size();i++)
		if (m_elements[i] != other.m_elements[i])
			return false;
	
	return true;
}

int Compare(const U8* first, const U8* second, S32 size)
{
	assert((first && second && size > 0) || (!first && !second && size == 0));

	for (int i = 0; i < size; i++)
	{
		if (first[i] < second[i]) return -1;
		else if (first[i] > second[i]) return 1;
	}

	return 0;
}

// Material
Material::Material(const Material& material)
{
	Copy(material);
}

Material& Material::operator = (const Material& material)
{
	Copy(material);
	return *this;
}

void Material::Copy(const Material& material)
{
	m_typeIndex = material.m_typeIndex;
	m_dataSize = material.m_dataSize;
	m_alignment = material.m_alignment;

	if (Pointer(material.m_pData))
	{
		Allocate(m_pData, m_dataSize / 4, m_alignment);
		memcpy(Pointer(m_pData), Pointer(material.m_pData), m_dataSize);
	}
}

bool operator < (const Material& m1, const Material& m2)
{
	if (m1.GetTypeIndex() < m2.GetTypeIndex()) return true;
	if (m2.GetTypeIndex() < m1.GetTypeIndex()) return false;

	int c = Compare(m1.GetData(), m2.GetData(), m1.GetDataSize());
	return (c < 0); 
}

bool operator == (const Material& m1, const Material& m2)
{
	if (m1.GetTypeIndex() != m2.GetTypeIndex()) return false;
	if (m2.GetTypeIndex() != m1.GetTypeIndex()) return false;

	int c = Compare(m1.GetData(), m2.GetData(), m1.GetDataSize());
	return (c == 0); 
}

// MaterialList
void MaterialList::CreateMaterial(Material& m, S32 typeIndex) const
{
	const MaterialType& type = m_materialTypes[typeIndex];
	m.m_typeIndex = typeIndex;
	m.m_dataSize = type.GetDataSize();
	m.m_alignment = type.GetMaxAlign();

	if (m.m_dataSize > 0)
	{
		Allocate(m.m_pData, m.m_dataSize / 4, m.m_alignment);
		memset(Pointer(m.m_pData), 0, m.m_dataSize);
	}
}

S32 MaterialListData::GetDataSize() const
{
	S32 size = 0;

	for (int i = 0; i < m_materials.Size(); i++)
		size += m_materials[i].GetDataSize();

	return size;
}

// MaterialListData
void MaterialListData::CopyData(const MaterialListData& materialList, bool internalStorage)
{
#if !CFG_CONVERTER && !PLATFORM_SPU
	if (internalStorage)
	{
		m_materials.Destroy();
		m_autoStackAllocator.Free();

		if (materialList.m_materials.Size())
		{
			S32 size = materialList.GetDataSize();
			Begin(size);
			m_autoStackAllocator.GetAllocatorPtr()->SetGrowSize(0);
			m_materials = materialList.m_materials;
			End();
		}
	}
	else
#endif
	{
		m_materials = materialList.m_materials;
	}
}

#if !CFG_CONVERTER && !PLATFORM_SPU
void MaterialListData::Begin(S32 reservedBytes)
{
	Btl::DynamicStackAllocator* pAllocator = Btl::StackAllocatorSystem::Instance().Begin();
	m_autoStackAllocator.Set(pAllocator);

	if (reservedBytes > 0)
	{
		pAllocator->SetGrowSize(reservedBytes);
		pAllocator->Reserve(reservedBytes);
		pAllocator->SetGrowSize(1024);
	}
}

void MaterialListData::End()
{
	m_autoStackAllocator.End();
}

#endif

}
