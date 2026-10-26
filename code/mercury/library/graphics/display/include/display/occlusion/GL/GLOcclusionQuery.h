#ifndef DISPLAY_GLOCCLUSIONQUERY_H
#define DISPLAY_GLOCCLUSIONQUERY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Occlusion/OcclusionQueryTypes.h"

namespace NewDisplay
{
namespace GL
{
    
class C_OcclusionQuery
{
public:

	C_OcclusionQuery();
	~C_OcclusionQuery();

	const bool IsCreated() const;

	bool Create();
	void Destroy();

	void Begin();
	void End();

	const E_OcclusionQueryResult Visible() const;
	const E_OcclusionQueryResult Visible(U32& visible) const;
	const E_OcclusionQueryResult Visible(F32& visible) const;
};

#include "GLOcclusionQuery.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLOCCLUSIONQUERY_H
