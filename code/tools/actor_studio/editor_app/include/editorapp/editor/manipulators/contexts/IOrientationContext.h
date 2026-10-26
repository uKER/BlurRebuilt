#ifndef EDITOR_I_ORIENTATION_CONTEXT_H
#define EDITOR_I_ORIENTATION_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Misc/Status.h>

namespace Editor
{
	class I_OrientationContext : public I_ManipulatorContext
	{
	public:
		I_OrientationContext() : I_ManipulatorContext() {}
		virtual ~I_OrientationContext() {}

		virtual EditorLib::C_Status Update(
			const Maths::HVec3& axis, F32 angle) = 0;
		virtual EditorLib::C_Status Apply(
			const Maths::HVec3& axis, F32 angle) = 0;
	};
}

#endif
