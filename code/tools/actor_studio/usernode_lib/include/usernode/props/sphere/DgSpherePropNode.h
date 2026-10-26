#ifndef EDITOR_PROPS_DG_SPHERE_PROP_NODE_H
#define EDITOR_PROPS_DG_SPHERE_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace EditorLib
{
	class C_XtDirty;

	class C_DgSpherePropNode : public C_DgXtPropNodeBase
	{
	// From C_XtObject
	public:
		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

	// From I_DgObject
	public:
		virtual E_ObjType GetType() const;

		virtual const char*		GetTypeName() const;

		virtual bool IsA(E_ObjType type) const;

	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual void GetPlugs(Btl::Vector<I_Plug*>& plugs);

	// From C_DgXtPropNodeBase
	public:
		virtual bool IsVisible() const;

	//From C_DgSpherePropNode
	public:
		enum E_StaticPlug
		{
			E_StaticPlug_Position,
			E_StaticPlug_Radius,

			E_StaticPlug_Count
		};

		static const char*		GetStaticTypeName();

		static const char*		GetStaticDisplayName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		C_NullPlug& GetPositionPlug();

		const C_NullPlug& GetPositionPlug() const;

		C_StaticPlug<F32>& GetRadiusPlug();

		const C_StaticPlug<F32>& GetRadiusPlug() const;

	// From C_XtObject
	protected:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	// From C_DgSpherePropNode
	protected:
		C_DgSpherePropNode();
		virtual ~C_DgSpherePropNode();

		C_DgSpherePropNode(const C_DgSpherePropNode& rhs);
		const C_DgSpherePropNode& operator=(const C_DgSpherePropNode& rhs);

		I_Plug* m_staticPlugs[E_StaticPlug_Count];

	// From C_DgSpherePropNode
	private:
		static void OnPlugChanged(I_Plug* pPlug);
	};
}

#endif
