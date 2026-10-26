#ifndef EDITOR_DELETE_CONTEXT_MAPPING_H
#define EDITOR_DELETE_CONTEXT_MAPPING_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{
	enum E_DeleteContextType
	{
		E_DeleteContextType_Unknown,

		E_DeleteContextType_SplineDelete,
		E_DeleteContextType_PolyDelete,
		E_DeleteContextType_AnimationDelete
	};

	class C_DeleteContextMapping : public Ut::Singleton<
		C_DeleteContextMapping, Ut::CreateUsingNew>
	{
	public:
		C_DeleteContextMapping();
		~C_DeleteContextMapping();

		E_DeleteContextType FindContext(
			EditorLib::E_ObjType input1) const;

		EditorLib::C_Status Register(
			EditorLib::E_ObjType input1, 
			E_DeleteContextType output);

	private:
		Ut::Vector<E_DeleteContextType> m_dispatchTable;
	};
}

#endif
