#ifndef EDITOR_MANIPULATOR_CONTEXT_MAPPING_H
#define EDITOR_MANIPULATOR_CONTEXT_MAPPING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Misc/Status.h>

namespace Editor
{
	class C_ManipulatorContextMapping : public Ut::Singleton<
		C_ManipulatorContextMapping, Ut::CreateUsingNew>
	{
	public:
		C_ManipulatorContextMapping();
		~C_ManipulatorContextMapping();

		E_ManipulatorContextType FindContext(
			EditorLib::E_ObjType input1, E_ManipulatorType input2) const;

		EditorLib::C_Status Register(
			EditorLib::E_ObjType input1, 
			E_ManipulatorType input2, 
			E_ManipulatorContextType output);

	private:
		//Use inputs to index into 2d array. 
		//[EditorLib::E_ObjType][E_ManipulatorType]=E_ManipulatorContextType
		Ut::Vector<
			Ut::Vector<E_ManipulatorContextType> > m_dispatchTable;
	};
}

#endif
