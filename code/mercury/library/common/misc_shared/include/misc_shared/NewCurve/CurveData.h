#ifndef CURVE_CURVE_DATA_H
#define CURVE_CURVE_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename KNOT>
	class C_CurveData
	{
	public:
		///Required
		typedef KNOT KnotType;
		typedef Btl::Vector<KNOT> KnotVector;

		C_CurveData();

		~C_CurveData();

		///Required
		KNOT& AcquireKnot(U32 index);

		///Required
		const KNOT& AcquireKnot(U32 index) const;

		///Required
		void ReleaseKnot() const;

		///Required
		void CommitKnot();

		///Required
		void CommitKnots();

		///Required
		S32 GetNumKnots() const;

		Btl::Vector<KNOT>& GetKnots();

		const Btl::Vector<KNOT>& GetKnots() const;

	private:
		KnotVector			m_knots;
		mutable S32			m_stackSize;
	};




	template<typename KNOT, typename DATA_POLICY>
	class C_CurveDataProxy : public DATA_POLICY
	{
	public:
		///Required
		typedef KNOT KnotType;

		C_CurveDataProxy();

		~C_CurveDataProxy();

		///Required
		KNOT& AcquireKnot(U32 index);

		///Required
		const KNOT& AcquireKnot(U32 index) const;

		///Required
		void ReleaseKnot() const;

		///Required
		void CommitKnot();

		///Required
		void CommitKnots();

		///Required
		S32 GetNumKnots() const;

	private:
		struct C_Acquired
		{
			U32	 index;
			KNOT knot;
		};

		mutable C_Acquired		m_acquired[CURVE_ACQUIRE_MAX];
		mutable S32				m_stackSize;
	};

	template<typename KNOT>
	class C_SourceProxyExample
	{
	protected:
		void LoadSourceKnot(KNOT& knot, U32 index)
		{
			//You would fill out a knot from your own data storage here.
		}

		void StoreSourceKnot(const KNOT& knot, U32 index)
		{
			//Knot has changed. Here you should write back to your own storage.
		}

		U32 GetNumSourceKnots() const
		{
			return 0;
		}
	};

	#include "CurveData.inl"
}

#endif
