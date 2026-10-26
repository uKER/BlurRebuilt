#include "stdafx.h"
#include "renderingdata/target.h"

//#pragma optimize("", off)

namespace RenderingData {
namespace Target
{

MaterialList::MaterialList(const MaterialList& other)
{
	m_materialTypes = other.m_materialTypes;

	if (!other.m_materials.Empty())
	{
		Begin(other.m_autoStackAllocator.BytesUsed());
		m_materials = other.m_materials;
		End();
	}
}

MaterialList& MaterialList::operator = (const MaterialList& other)
{
	if (this != &other)
	{
		m_materials.Destroy();
		m_autoStackAllocator.Free();
		m_materialTypes = other.m_materialTypes;

		if (!other.m_materials.Empty())
		{
			Begin(other.m_autoStackAllocator.BytesUsed());
			m_materials = other.m_materials;
			End();
		}
	}

	return *this;
}

const RenderingData::MaterialList::VectorType& MaterialList::GetMaterials() const
{
	return m_materials;
}

S32 MaterialList::AddMaterialType(const MaterialType& t)
{
	for (int i=0; i<m_materialTypes.Size(); i++)
	{
		if (m_materialTypes[i] == t)
			return i;
	}
	m_materialTypes.PushBack(t);
	return m_materialTypes.Size() - 1;
}

S32 MaterialList::AddMaterial(S32 typeIndex)
{
	m_materials.PushBack();
	CreateMaterial(m_materials.Back(), typeIndex);
	return m_materials.Size() - 1;
}

void MaterialList::ClearAll()
{
	ClearMaterials();
	m_materialTypes.Clear();
}

void MaterialList::ClearMaterials()
{
	m_materials.Clear();
	m_autoStackAllocator.Free();
}

}}
