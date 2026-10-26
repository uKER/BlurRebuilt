#ifndef EDITOR_PROPS_DG_RECTANGLE_PROP_NODE_H
#define EDITOR_PROPS_DG_RECTANGLE_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace EditorLib
{
	class C_XtDirty;

	class C_DgRectanglePropNode : public C_DgXtPropNodeBase
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

	//From C_DgRectanglePropNode
	public:
		enum E_StaticPlug
		{
			E_StaticPlug_Width,
			E_StaticPlug_Height,

			E_StaticPlug_Count
		};

		static const char*		GetStaticTypeName();

		static const char*		GetStaticDisplayName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		const Matrix44& GetLocalMatrix() const;
		
		C_StaticPlug<F32>& GetWidthPlug();

		const C_StaticPlug<F32>& GetWidthPlug() const;

		C_StaticPlug<F32>& GetHeightPlug();

		const C_StaticPlug<F32>& GetHeightPlug() const;

	// From C_XtObject
	protected:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	// From C_DgRectanglePropNode
	protected:
		C_DgRectanglePropNode();
		virtual ~C_DgRectanglePropNode();

		C_DgRectanglePropNode(const C_DgRectanglePropNode& rhs);
		const C_DgRectanglePropNode& operator=(const C_DgRectanglePropNode& rhs);

		I_Plug* m_staticPlugs[E_StaticPlug_Count];

		Matrix44 m_localMatrix;

	// From C_DgRectanglePropNode
	private:
		static void OnPlugChanged(I_Plug* pPlug);
	};
}

#endif
