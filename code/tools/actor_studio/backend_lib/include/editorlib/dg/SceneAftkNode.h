#ifndef EDITOR_LIB_SCENE_AFTK_NODE_H
#define EDITOR_LIB_SCENE_AFTK_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/SceneNode.h>

namespace EditorLib
{
	class C_SceneAftkNode : public C_SceneNode
	{
	public:
		static const char*		GetStaticTypeName();

		virtual const char*		GetTypeName() const;

		static E_ObjType		GetStaticType();

		static void*			Create();

		virtual E_ObjType		GetType() const;

		virtual bool			IsA(E_ObjType type) const;

		void					GetAftkPath(Btl::String& path) const;

		const C_AftkRef&		GetRef() const;

		void					SetRef(const C_AftkRef& ref);

	protected:
		C_SceneAftkNode();
		virtual ~C_SceneAftkNode();

		const C_AftkRef* m_pRef;
	};
}

#endif
