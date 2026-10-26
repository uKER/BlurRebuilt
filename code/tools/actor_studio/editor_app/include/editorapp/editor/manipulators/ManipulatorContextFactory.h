#ifndef EDITOR_MANIPULATOR_CONTEXT_FACTORY_H
#define EDITOR_MANIPULATOR_CONTEXT_FACTORY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class I_ManipulatorContext;

	struct C_ManipulatorRegPolicy
	{
		template<typename DERIVED>
		static bool Register(C_RegInfo* pRegInfo) 
		{ 
			DERIVED::ManipulatorContextRegister();
			return true; 
		}
	};

	class C_ManipulatorContextFactory : 
		public C_IdFactory<
			I_ManipulatorContext, 
			C_RegInfo, 
			C_ManipulatorRegPolicy>, 
		public Ut::Singleton<
			C_ManipulatorContextFactory, 
			Ut::CreateUsingNew>
	{
	};
}

#endif
