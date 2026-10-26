#include "MiscSharedPch.h"
#include <misc_shared/Moments/Moments.h>

#include <misc_shared/geometry/RegionBSP.h>

using namespace UtMaths;

namespace Moments {
// -----------------------------------------------------------------


bool C_MomentCalculator::IsMeshClosed() const
{
	using namespace RegionBSP;

	C_Polyhedron polyhedron;

	for ( TriangleList::ConstIterator it = m_TriangleList.Begin(); it != m_TriangleList.End(); ++it )
	{
		C_Polyhedron::C_PolyhedronPolygon & polygon = polyhedron.AddPolygon();

		polyhedron.AddVertexToPolygon( polygon, polyhedron.AddVertex( it->m_Vertex0 ) );
		polyhedron.AddVertexToPolygon( polygon, polyhedron.AddVertex( it->m_Vertex1 ) );
		polyhedron.AddVertexToPolygon( polygon, polyhedron.AddVertex( it->m_Vertex2 ) );
	}

	return polyhedron.IsClosed();
}


void C_MomentCalculator::CalculateResults( const Maths::HF32 & density )
{
	_Scalar moment0 = 0.0;
	_Vector moment1 = ZeroVector();
	_Matrix moment2 = ZeroMatrix();


	for ( TriangleList::Iterator it = m_TriangleList.Begin(); it != m_TriangleList.End(); ++it )
	{
		_Matrix R = it->GetMatrix();
		_Scalar detR = GetDeterminant3x3( R );

		moment0 += Moment0_Tetrahedron( R, detR );
		moment1 += Moment1_Tetrahedron( R, detR );
	}

	_Vector centreOfMass = moment1 / moment0;


	for ( TriangleList::Iterator it = m_TriangleList.Begin(); it != m_TriangleList.End(); ++it )
	{
		_Matrix R = it->GetMatrix(centreOfMass);
		_Scalar detR = GetDeterminant3x3( R );

		moment2 = moment2 + Moment2_Tetrahedron( R, detR );
	}

	Maths::HMat33 inertiaTensor = GetHMat33( ConvertMoment2ToMoi(moment2) ) * density;


	m_Mass = density * Maths::HF32(moment0);
	m_CentreOfMass = GetHVec3(centreOfMass);
	m_InverseInertiaTensor = inertiaTensor.Inverse();
}


void C_MomentCalculator::CalculateResults_Surface( const Maths::HF32 & mass )
{
	_Scalar moment0 = 0.0;
	_Vector moment1 = ZeroVector();
	_Matrix moment2 = ZeroMatrix();


	for ( TriangleList::Iterator it = m_TriangleList.Begin(); it != m_TriangleList.End(); ++it )
	{
		_Matrix R = it->GetMatrix();
		_Scalar area = GetTriangleArea( R );

		moment0 += Moment0_Triangle( R, area );
		moment1 += Moment1_Triangle( R, area );
	}

	_Vector centreOfMass = moment1 / moment0;


	for ( TriangleList::Iterator it = m_TriangleList.Begin(); it != m_TriangleList.End(); ++it )
	{
		_Matrix R = it->GetMatrix(centreOfMass);
		_Scalar area = GetTriangleArea( R );

		moment2 = moment2 + Moment2_Triangle( R, area );
	}

	Maths::HMat33 inertiaTensor = GetHMat33( ConvertMoment2ToMoi(moment2) ) * ( mass / Maths::HF32(moment0) );


	m_Mass = mass;
	m_CentreOfMass = GetHVec3(centreOfMass);
	m_InverseInertiaTensor = inertiaTensor.Inverse();
}


// -----------------------------------------------------------------
};
