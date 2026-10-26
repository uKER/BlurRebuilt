#ifndef EDITOR_LIB_GAME_DATA_NODE_H
#define EDITOR_LIB_GAME_DATA_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtNodeBase.h>

namespace EditorLib
{
	class C_DgXtGameDataNode : public C_DgXtNodeBase
	{
	//From I_DgObject
		virtual E_ObjType		GetType() const;

		virtual const char*		GetTypeName() const;

		virtual bool			IsA(E_ObjType type) const;

	//From I_DgNode
	public:
		virtual bool			Update(C_UpdateHintCollection* pHc = 0);

		virtual bool			DirtyDependents(bool setThisDirty = true);

	//From XtObject
		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType,
			Xt::DomElement* pData);

	//From C_DgXtGameDataNode
	public:
		static const char*		GetStaticTypeName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		virtual C_NullPlug&			GetReferencesPlug();

		virtual const C_NullPlug&	GetReferencesPlug() const;

	//From C_XtObject
	protected:
		virtual void			OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	protected:
		union DgXtGameDataNodeDirtyFlags
		{
			struct
			{
			};
			U32 data;
		};

		C_DgXtGameDataNode();
		virtual ~C_DgXtGameDataNode();

		DgXtGameDataNodeDirtyFlags	m_dgXtGameDataNodeDirtyFlags;

		C_NullPlug m_references;
	};
}

#endif

