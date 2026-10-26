#ifndef EDITOR_LIB_XT_SCENE_NODE_TYPE_H
#define EDITOR_LIB_XT_SCENE_NODE_TYPE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtSceneNodeType : public C_XtNodeType
	{
	public:
		C_XtSceneNodeType(LtStruct* pSchema) : C_XtNodeType(pSchema) {}

		virtual ~C_XtSceneNodeType() {}

		virtual E_XtType GetType() const {
			return E_XtType_SceneXtNode;
		}

		virtual bool IsA(E_XtType type) const {
			if (__super::IsA(type))
				return true;
			if (type == C_XtSceneNodeType::GetType()) 
				return true;

			return false;
		}

	protected:
		C_XtSceneNodeType(const C_XtSceneNodeType& rhs);
		const C_XtSceneNodeType& operator=(const C_XtSceneNodeType& rhs);
	};
}

#endif
