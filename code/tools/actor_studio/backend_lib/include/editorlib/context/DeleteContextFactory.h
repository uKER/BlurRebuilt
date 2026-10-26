#ifndef EDITOR_DELETE_CONTEXT_FACTORY_H
#define EDITOR_DELETE_CONTEXT_FACTORY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{
	class I_DeleteContext;

	struct C_DeleteRegPolicy
	{
		template<typename DERIVED>
		static bool Register(C_RegInfo* pRegInfo) 
		{ 
			DERIVED::DeleteContextRegister();
			return true; 
		}
	};

	class C_DeleteContextFactory : 
		public C_IdFactory<
			I_DeleteContext, 
			C_RegInfo, 
			C_DeleteRegPolicy>, 
		public Ut::Singleton<
			C_DeleteContextFactory, 
			Ut::CreateUsingNew>
	{
	};
}

#endif
