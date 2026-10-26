
#include <core\maths\utmaths\spline.h>
using namespace AttkUt;


	template <typename T>
	class LinearSpline
	{
	public:
		typedef T PointType;

		LinearSpline() : m_pCvs(0), m_n(0) {}

		LinearSpline(const PointType* pCvs, int nCvs) :
			m_pCvs(pCvs),
			m_n(nCvs - 1)
		{}

		void Initialise(const PointType* pCvs, int nCvs)
		{
			m_pCvs = pCvs;
			m_n = nCvs - 1;
		}

		PointType PositionFromX(float x) const
		{
			int i = 1;
			while (i < m_n && x > m_pCvs[i].x)
				++i;

			const PointType& p0 = m_pCvs[i-1];
			const PointType& p1 = m_pCvs[i];

			const PointType d = UtMaths::MakePoint(p1-p0);
			PointType result;

			F32 lerp = p1.x - p0.x;
			if (lerp < Maths::Constants<F32>::Epsilon())
				return p0;
			else
			{
				lerp = (x-p0.x)/lerp;
				lerp = Maths::Clamp01(lerp);
				result = p0 + lerp*MakeVector(d);
			}
			return result;
		}

	private:
		const PointType* m_pCvs; 
		int m_n;
	};








template <typename VertexType>
C_ImageAdjustmentCurve<VertexType>::C_ImageAdjustmentCurve()
{
}

template <typename VertexType>
C_ImageAdjustmentCurve<VertexType>::C_ImageAdjustmentCurve(
	const C_ImageAdjustmentCurve<VertexType>& rhs)
{
	*this = rhs;
}

template <typename VertexType>
C_ImageAdjustmentCurve<VertexType>::~C_ImageAdjustmentCurve()
{
}

template <typename VertexType>
const C_ImageAdjustmentCurve<VertexType>& C_ImageAdjustmentCurve<VertexType>::operator =(
	const AttkUt::C_ImageAdjustmentCurve<VertexType> &rhs)
{
	m_CurveDegree = rhs.m_CurveDegree;
	m_ControlVertices = rhs.m_ControlVertices;
	m_Knots = rhs.m_Knots;

	return *this;
}

template <typename VertexType>
U32 C_ImageAdjustmentCurve<VertexType>::AddControlVertex(
	const VertexType& ctrlVtx)
{
	//Ensure control points are increasing in x
	assert(m_ControlVertices.Size() == 0 || ctrlVtx.x >= m_ControlVertices.Back().x);

	m_ControlVertices.PushBack(ctrlVtx);
	return m_ControlVertices.Size()-1;
}

template <typename VertexType>
VertexType& C_ImageAdjustmentCurve<VertexType>::GetControlVertex(
	U32 index)
{
	assert(index < (U32)m_ControlVertices.Size());
	return m_ControlVertices[index];
}

template <typename VertexType>
const VertexType& C_ImageAdjustmentCurve<VertexType>::GetControlVertex(
	U32 index) const
{
	assert(index < (U32)m_ControlVertices.Size());
	return m_ControlVertices[index];
}

template <typename VertexType>
U32 C_ImageAdjustmentCurve<VertexType>::AddKnot(
	F32 knot)
{
	m_Knots.PushBack(knot);
	return m_Knots.Size()-1;
}

template <typename VertexType>
F32 C_ImageAdjustmentCurve<VertexType>::GetKnot(
	U32 index) const
{
	assert(index < (U32)m_Knots.Size());
	return m_Knots[index];
}

template <typename VertexType>
void C_ImageAdjustmentCurve<VertexType>::SetCurveDegree(
	U32 degree)
{
	m_CurveDegree = degree;
}

template <typename VertexType>
U32 C_ImageAdjustmentCurve<VertexType>::GetCurveDegree() const
{
	return m_CurveDegree;
}

template <typename VertexType>
typename C_ImageAdjustmentCurve<VertexType>::ControlVertexArray& C_ImageAdjustmentCurve<VertexType>::GetControlVertexArray()
{
	return m_ControlVertices;
}

template <typename VertexType>
typename const C_ImageAdjustmentCurve<VertexType>::ControlVertexArray& C_ImageAdjustmentCurve<VertexType>::GetControlVertexArray() const
{
	return m_ControlVertices;
}

template <typename VertexType>
typename C_ImageAdjustmentCurve<VertexType>::KnotArray& C_ImageAdjustmentCurve<VertexType>::GetKnotArray()
{
	return m_Knots;
}

template <typename VertexType>
typename const C_ImageAdjustmentCurve<VertexType>::KnotArray& C_ImageAdjustmentCurve<VertexType>::GetKnotArray() const
{
	return m_Knots;
}





template <typename VertexType>
C_EvaluateImageAdjustmentCurve<VertexType>::C_EvaluateImageAdjustmentCurve()
:	m_NumSubdivisions(0)
{
}

template <typename VertexType>
C_EvaluateImageAdjustmentCurve<VertexType>::~C_EvaluateImageAdjustmentCurve()
{
}

template <typename VertexType>
bool C_EvaluateImageAdjustmentCurve<VertexType>::Initialise(
	const C_ImageAdjustmentCurve<VertexType>& curve,
	U32 numSubdivisions)
{
	Destroy();
	m_NumSubdivisions = numSubdivisions;

	if (curve.GetCurveDegree() == 1)
	{
		m_ApproximationCurve = curve.GetControlVertexArray();
	}
	else
	{
		//Generate linear approximation 'curve'
		F32 minKnot = curve.GetKnotArray().Front();
		F32 maxKnot = curve.GetKnotArray().Back();
		F32 increment = (maxKnot - minKnot)/numSubdivisions;

		UtMaths::BSpline<VertexType> splineCurveEvaluater(
			&curve.GetControlVertexArray()[0], 
			&curve.GetKnotArray()[0], 
			curve.GetControlVertexArray().Size());

		m_ApproximationCurve.PushBack(curve.GetControlVertexArray().Front());
		for (F32 t=increment; t<maxKnot; t+=increment)
		{
			VertexType p = splineCurveEvaluater.Position(t);
			m_ApproximationCurve.PushBack(p);
		}
		m_ApproximationCurve.PushBack(curve.GetControlVertexArray().Back());
	}

	return true;
}

template <typename VertexType>
void C_EvaluateImageAdjustmentCurve<VertexType>::Destroy()
{
	m_NumSubdivisions = 0;
	m_ApproximationCurve.Clear();
}

template <typename VertexType>
VertexType C_EvaluateImageAdjustmentCurve<VertexType>::Evaluate(
	F32 x) const
{
	LinearSpline<VertexType> linearApproxCurveEvaluator(&m_ApproximationCurve[0], m_ApproximationCurve.Size());
	return linearApproxCurveEvaluator.PositionFromX(x);
}

