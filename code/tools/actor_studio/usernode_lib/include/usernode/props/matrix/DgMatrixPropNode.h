#ifndef EDITOR_LIB_MATRIX_PROP_H
#define EDITOR_LIB_MATRIX_PROP_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace EditorLib
{
	class C_XtDirty;

	class C_DgMatrixPropNode : public C_DgXtPropNodeBase
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

	// From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual void GetPlugs(Btl::Vector<I_Plug*>& plugs);

	// From C_DgXtPropNodeBase
	public:
		virtual bool IsVisible() const;

	//From C_DgMatrixPropNode
	public:
		enum E_StaticPlug
		{
			E_StaticPlug_Translate,
			E_StaticPlug_Rotate,
			E_StaticPlug_Scale,
			E_StaticPlug_Shear,

			E_StaticPlug_Count
		};

		static const char*		GetStaticTypeName();

		static const char*		GetStaticDisplayName();

		static E_ObjType		GetStaticType();

		static void*			Create();
	
		void SetLocalMatrix(const Matrix44& matrix);

		const Matrix44& GetLocalMatrix() const;

		C_NullPlug& GetTranslatePlug();

		C_NullPlug& GetRotatePlug();
		
		C_NullPlug& GetScalePlug();
		
		C_NullPlug& GetShearPlug();

		const C_NullPlug& GetTranslatePlug() const;

		const C_NullPlug& GetRotatePlug() const;
		
		const C_NullPlug& GetScalePlug() const;
		
		const C_NullPlug& GetShearPlug() const;

	// From C_XtObject
	protected:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	// From C_DgMatrixPropNode
	protected:
		C_DgMatrixPropNode();
		virtual ~C_DgMatrixPropNode();

		I_Plug* m_staticPlugs[E_StaticPlug_Count];

		Matrix44 m_localMatrix;

	// From C_DgMatrixPropNode
	private:
		static void OnPlugChanged(I_Plug* pPlug);
	};
}

#endif
