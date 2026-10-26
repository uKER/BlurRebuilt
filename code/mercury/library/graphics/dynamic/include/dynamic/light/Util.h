#ifndef RENDER_DYNAMICEFFECT_LIGHT_UTIL_H
#define RENDER_DYNAMICEFFECT_LIGHT_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace DynamicEffect
{

class C_Node;

namespace Light
{

bool GetNodeFromDirection(C_Node* pNode, const Maths::HVec3& vDirection);

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_UTIL_H
