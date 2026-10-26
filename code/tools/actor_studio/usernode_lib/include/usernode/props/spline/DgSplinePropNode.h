#ifndef EDITOR_PROPS_DG_SPLINE_PROP_NODE_H
#define EDITOR_PROPS_DG_SPLINE_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace Curve
{
	template<typename KNOT>
	struct C_HermiteSplineKnot;

	template<typename KNOT>
	class C_CurveData;

	template<typename VECTYPE, typename STORAGE, typename CTRLPOINT_POLICY>
	class C_HermiteSpline;

	template<typename VECTYPE>
	class C_Reparameterise;

	template<typename VECTYPE, typename STORAGE>
	class C_HermiteCtrlPtDefault;
}

namespace EditorLib
{
	struct C_CollisionSpline;
	struct C_InternalCollisionInstance;
	class C_XtDirty;

	class C_DgSplinePropNode : public C_DgXtPropNodeBase
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

	// From C_DgXtPropNodeBase
	public:
		virtual bool IsVisible() const;

	//From C_DgSplinePropNode
	public:
		enum E_SplineCompType
		{
			E_SplineCompType_InTangent,
			E_SplineCompType_Value,
			E_SplineCompType_OutTangent,
			E_SplineCompType_Count
		};

		typedef Curve::C_CurveData<Curve::C_HermiteSplineKnot<Maths::HVec3> > CurveDataType;
		typedef Curve::C_HermiteSpline<
			Maths::HVec3,
			CurveDataType,
			Curve::C_HermiteCtrlPtDefault<Maths::HVec3, CurveDataType>
		> CurveType;

		static const char*		GetStaticTypeName();

		static const char*		GetStaticDisplayName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		C_Status XtSave();
		C_Status Apply();
		C_Status AddPoint(const Maths::HVec3& position, F32 tension);
		C_Status DeletePoints(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds);
		Ut::Vector<U32> InsertPoints(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds);
		void Reverse();

		const CurveType* GetCurve() const;
		CurveType* GetCurve();

		const CurveDataType* GetCurveData() const;

		CurveDataType* GetCurveData();

		const Curve::C_Reparameterise<Maths::HVec3>* GetReparam() const;

		U32 GetNumComponents() const;

		U32 GetComponentKnotIndex(U32 componentIndex) const;

		U32 GetKnotComponentIndex(U32 knotIndex) const;

		Maths::HVec3 GetComponent(U32 index, E_SplineCompType* pType = 0) const;

		void SetComponent(U32 index, const Maths::HVec3& pos, bool lockTangents);

		F32 GetDefaultTangentScale() const;

		bool IsSplineClosed() const;

	// From C_XtObject
	protected:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	protected:
		C_DgSplinePropNode();
		virtual ~C_DgSplinePropNode();

		C_DgSplinePropNode(const C_DgSplinePropNode& rhs);
		const C_DgSplinePropNode& operator=(const C_DgSplinePropNode& rhs);

		void OnSplineChanged();

		void UpdateCollisionSource();

		void UpdateCollisionInstance();

		CurveDataType*					m_pCurveData;
		CurveType*						m_pCurve;
		Curve::C_Reparameterise<Maths::HVec3>* m_pReparam;
		C_InternalCollisionInstance*	m_pCollisionInstance;
		C_CollisionSpline*				m_pCollisionSpline;

		bool							m_isSplineClosed;
		U32								m_reparamRes;
	};
}

#endif
