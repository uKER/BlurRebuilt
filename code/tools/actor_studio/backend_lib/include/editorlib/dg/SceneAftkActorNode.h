#ifndef EDITOR_LIB_SCENE_AFTK_ACTOR_NODE_H
#define EDITOR_LIB_SCENE_AFTK_ACTOR_NODE_H

#pragma once

namespace EditorLib
{
	class C_SceneAftkActorNode : public C_SceneAftkNode
	{
	public:
		static const char*		GetStaticTypeName();

		virtual const char*		GetTypeName() const;

		static E_ObjType		GetStaticType();

		static void*			Create();

		virtual E_ObjType		GetType() const;

		virtual bool			IsA(E_ObjType type) const;

		Util::ConstKeyValueArray&
								GetAftkProperties() const;

		Util::KeyValueArray&
								GetAftkProperties();

		U32						GetAftkUniqueId() const;

		void					SetAftkUniqueId(U32 uid);

	protected:
		C_SceneAftkActorNode();
		virtual ~C_SceneAftkActorNode();

		Util::KeyValueArray		m_attributes;
		U32						m_aftkUniqueId;
	};
}

#endif
