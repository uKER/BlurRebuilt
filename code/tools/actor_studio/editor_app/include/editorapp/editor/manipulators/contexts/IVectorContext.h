#ifndef EDITOR_I_VECTOR_CONTEXT_H
#define EDITOR_I_VECTOR_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Misc/Status.h>

namespace Editor
{
	class I_VectorContext : public I_ManipulatorContext
	{
	public:
		I_VectorContext() : I_ManipulatorContext() {}
		virtual ~I_VectorContext() {}

		virtual EditorLib::C_Status Update(
			const Maths::HVec3& v) = 0;
		virtual EditorLib::C_Status Apply(
			const Maths::HVec3& v) = 0;
	};
}

#endif
