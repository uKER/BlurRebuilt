#include "stdafx.h"
#include "renderingdata/archive.h"
#include "renderingdata/noninterface/archive.inl"

namespace RenderingData
{

void ActivateHandlers()
{
	SERIALISATION_HANDLER(RenderingData::Node)
	SERIALISATION_HANDLER(RenderingData::CullNode)
	SERIALISATION_HANDLER(RenderingData::GroupNode)
	SERIALISATION_HANDLER(RenderingData::LodNode)
	SERIALISATION_HANDLER(RenderingData::RenderNode)
	SERIALISATION_HANDLER(RenderingData::MaterialNode)
}

SERIALISATION_KEY(Node)
SERIALISATION_KEY(CullNode)
SERIALISATION_KEY(GroupNode)
SERIALISATION_KEY(LodNode)
SERIALISATION_KEY(RenderObject)
SERIALISATION_KEY(RenderNode)
SERIALISATION_KEY(RenderListNode)
SERIALISATION_KEY(MaterialNode)

}
