#ifndef EDITORLIB_UPDATE_SINGNALLER_H
#define EDITORLIB_UPDATE_SINGNALLER_H

#ifndef _MSC_VER
#pragma once
#endif

#include <EditorLib/Dg/Hints/HintCollection.h>

namespace EditorLib
{
	class C_UpdateSignaller
	{
	public:
		C_UpdateSignaller(I_DgNode* pNode, E_ObjType invokingType, C_UpdateHintCollection* pHc);

		~C_UpdateSignaller();

		C_UpdateHintCollection& GetCollection() { return *m_pHc; }

		const C_UpdateHintCollection& GetCollection() const { return *m_pHc; } 

	private:
		I_DgNode*			m_pNode;
		E_ObjType			m_invokingType;
		//Only used by invoking instance.
		bool				m_emitSignal;
		C_UpdateHintCollection*	m_pHc;
	};
}

#endif
