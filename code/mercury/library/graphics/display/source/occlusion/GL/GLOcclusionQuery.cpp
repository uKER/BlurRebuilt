#include "stdafx.h"
#include "Display/Occlusion/GL/GLOcclusionQuery.h"
#include "Display/Device/Device.h"

namespace NewDisplay
{
namespace GL
{

C_OcclusionQuery::C_OcclusionQuery()
{
}

C_OcclusionQuery::~C_OcclusionQuery()
{
	Destroy();
}

bool C_OcclusionQuery::Create()
{
	return false;
}

void C_OcclusionQuery::Destroy()
{
}

void C_OcclusionQuery::Begin()
{
}

void C_OcclusionQuery::End()
{
}

}; // namespace GL
}; // namespace NewDisplay
