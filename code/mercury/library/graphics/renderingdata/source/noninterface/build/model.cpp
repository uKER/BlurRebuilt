#include "stdafx.h"
#include "renderingdata/model.h"

namespace RenderingData {
namespace Build
{

S32 Model::InsertPrimitive(S32 batchID, PrimitiveType primitiveType, const PrimitiveInfo::BaseInfo& base)
{
	S32 index = FindPrimitive(batchID);

	if (index < m_primitives.Size())
	{
		// Make sure that all the other settings match (which they should if the material combination index is correct).
		assert(m_primitives[index].effect == base.effect);
		assert(m_primitives[index].sortCode == base.sortCode);
		assert(m_primitives[index].flags == base.flags);
		assert(m_primitives[index].sortType == base.sortType);
		assert(m_primitives[index].blendState == base.blendState);
		assert(m_primitives[index].primitiveType == primitiveType);
		return index;
	}

	m_primitives.PushBack();

	m_primitives.Back().batchID = batchID;
	m_primitives.Back().effect = base.effect;
	m_primitives.Back().sortCode = base.sortCode;
	m_primitives.Back().flags = base.flags;
	m_primitives.Back().sortType = base.sortType;
	m_primitives.Back().blendState = base.blendState;
	m_primitives.Back().primitiveType = primitiveType;
	m_primitives.Back().vertexDeclaration = -1;
	m_primitives.Back().indexBufferIndex = ResourceIndex::Null();
	m_primitives.Back().lods.PushBack();
	m_primitives.Back().lods.Back().nIndices = 0;
	m_primitives.Back().lods.Back().offset = 0;
	return index;
}

S32 Model::FindPrimitive(S32 batchID)
{
	for (int i = 0; i < m_primitives.Size(); i++)
		if (m_primitives[i].batchID == batchID)
			return i;
	
	return m_primitives.Size();
}

void ModelStream::InsertPrimitive(S32 primitiveIndex, S32 materialIndex, ResourceIndex vertexBufferIndex, S32 streamIndex)
{
	while (m_primitives.Size() < primitiveIndex + 1)
		m_primitives.PushBack();

	PrimitiveStream& primitiveStream = m_primitives[primitiveIndex];

	if (!primitiveStream.active)
	{
		primitiveStream.active = true;
		primitiveStream.materialIndex = materialIndex;
		primitiveStream.vertexBufferIndex = vertexBufferIndex;
		primitiveStream.streamIndex = streamIndex;
		primitiveStream.vertexOffset = 0;
		primitiveStream.frequency = 0;
	}
	else
	{
		assert(primitiveStream.materialIndex == materialIndex);
		assert(primitiveStream.vertexBufferIndex == vertexBufferIndex);
		assert(primitiveStream.streamIndex == streamIndex);
	}
}


}}
