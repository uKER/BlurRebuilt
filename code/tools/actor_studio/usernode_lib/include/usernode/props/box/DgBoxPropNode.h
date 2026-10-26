#ifndef EDITOR_PROPS_DG_BOX_PROP_NODE_H
#define EDITOR_PROPS_DG_BOX_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace EditorLib
{
	class C_DgBoxPropNode : public C_DgXtPropNodeBase
	{
	//From C_XtObject
	public:
		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

	//From I_DgObject
	public:
		virtual E_ObjType		GetType() const;

		virtual const char*		GetTypeName() const;

		virtual bool			IsA(E_ObjType type) const;

	//From I_DgNode
	public:
		virtual bool			Update(C_UpdateHintCollection* pHc = 0);

		virtual void			GetPlugs(Btl::Vector<I_Plug*>& plugs);

	//From C_DgXtPropNodeBase
	public:
		virtual bool			IsVisible() const;

	//From C_DgBoxPropNode
	public:
		enum E_StaticPlug
		{
			E_StaticPlug_Rotate,
			E_StaticPlug_Size,

			E_StaticPlug_Count
		};

		static const char*		GetStaticTypeName();

		static const char*		GetStaticDisplayName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		const Matrix44&			GetLocalMatrix() const;

		C_NullPlug&				GetRotatePlug();

		C_NullPlug&				GetSizePlug();
		
		const C_NullPlug&		GetRotatePlug() const;
		
		const C_NullPlug&		GetSizePlug() const;

	//From C_XtObject
	protected:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	//From C_DgBoxPropNode
	protected:
		C_DgBoxPropNode();
		virtual ~C_DgBoxPropNode();

		C_DgBoxPropNode(const C_DgBoxPropNode& rhs);
		const C_DgBoxPropNode& operator=(const C_DgBoxPropNode& rhs);

		I_Plug*		m_staticPlugs[E_StaticPlug_Count];
		Matrix44	m_localMatrix;

	//From C_DgBoxPropNode
	private:
		static void OnPlugChanged(I_Plug* pPlug);
	};
}

#endif
