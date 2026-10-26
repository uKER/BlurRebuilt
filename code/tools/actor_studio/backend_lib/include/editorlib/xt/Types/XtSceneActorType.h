#ifndef EDITOR_LIB_SCENE_XT_ACTOR_TYPE_H
#define EDITOR_LIB_SCENE_XT_ACTOR_TYPE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtSceneActorType : public C_XtNodeType
	{
	public:
		C_XtSceneActorType(LtStruct* pSchema) : C_XtNodeType(pSchema) {}

		virtual ~C_XtSceneActorType() {}

		virtual E_XtType GetType() const {
			return E_XtType_SceneXtActorNode;
		}

		virtual bool IsA(E_XtType type) const {
			if (__super::IsA(type))
				return true;
			if (type == C_XtSceneActorType::GetType()) 
				return true;

			return false;
		}

	protected:
		C_XtSceneActorType(const C_XtSceneActorType& rhs);
		const C_XtSceneActorType& operator=(const C_XtSceneActorType& rhs);
	};
}

#endif
