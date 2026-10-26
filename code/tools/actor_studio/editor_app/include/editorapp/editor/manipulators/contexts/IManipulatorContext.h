#ifndef EDITOR_I_MANIPULATOR_CONTEXT_H
#define EDITOR_I_MANIPULATOR_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class I_ManipulatorContext
	{
	public:
		I_ManipulatorContext() {}
		virtual ~I_ManipulatorContext() {}

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space) = 0;
	};
}

#endif
