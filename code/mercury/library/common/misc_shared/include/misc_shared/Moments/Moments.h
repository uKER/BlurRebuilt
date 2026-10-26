#ifndef MOMENTS_H
#define MOMENTS_H

#include <core/UtMaths.h>
#include <core/Maths.h>

namespace Moments {
// -----------------------------------------------------------------


/*   -- GENERAL PURPOSE MASS, CENTRE OF MASS AND MOMENT OF INERTIA CALCULATOR --


The following class may be used to calculate, of a rigid body, its:

 - mass,
 - centre of mass position vector, and
 - inverse moment of inertia tensor taken about the centre of mass

given:

 - its density, and
 - a triangle mesh describing its shape.

If the mesh is to be treated as the boundary of a solid object, then it must be closed and non-intersecting. If it is
to be treated simply as a surface, then there are no such restrictions on its form.


Instructions for use:

(1)	Instantiate a C_MomentCalculator object.

(2)	Call AddTriangle repeatedly, once for each triangle of the mesh.

(2.5) If desired, check that IsMeshClosed returns true as a safety measure.

(3) Call CalculateResults, passing the desired density, to calculate the moments of the solid object delineated by the
mesh. Alternatively, call CalculateResults_Surface, passing the desired mass, to calculate the moments of the mesh
surface.

(4)	The calculation is now complete, so you may use the result accessors to obtain the results.

(5) If you wish to use the moment calculator again, call Reset then go back to step (2).


Notes:

PLEASE BE AWARE that the results produced by the moment calculator can be significantly affected by rounding
error, even with double precision arithmetic. The relationship between the nature of the input mesh and the
errors in the results is not trivial, and I have not had time to study it in detail. However, here are some
things to note:

The sensitivity of error in MOI results is of a higher order than for mass or COM, so MOI results are to be trusted
the least.

Generally, a mesh of 10,000 polygons or less seems to produce good MOI results, correct to within 0.1%. However,
I have only tested this in a few cases. A highly skewed mesh, with lots of very small triangles on one side and
a few very large triangles on the other, may produce bad results. Also, a mesh with a very large surface area to
volume ratio (e.g. a tree) may produce bad results. A mesh whose distance from the origin is large compared with
its size may produce bad results too. By bad I mean BAD: off by a more than 100%. So don't take these
results as gospel.


*/


class C_MomentCalculator
{
public:

	// Uses automatic default constructor.


	void AddTriangle( const Maths::C_Vertex & vertex0, const Maths::C_Vertex & vertex1,
		const Maths::C_Vertex & vertex2 )
	{
		Triangle temp( vertex0, vertex1, vertex2 );
		m_TriangleList.PushBack( temp );
	}


	bool IsMeshClosed() const;
	// Checks if the mesh has an Euler characteristic of 2
	//	(see http://en.wikipedia.org/wiki/Euler_characteristic). This is not guaranteed to catch out every
	//	invalid mesh, but it will pick up on the most likely errors such as missing polygons.
	//
	// Note - this isn't really a test for closure: a closed mesh that is not homeomorphic to a sphere, e.g.
	//	a doughnut, will not have an Euler characteristic of 2 and will fail this test. However, a doughnut mesh
	//	is still valid for moment calculation purposes!


	void CalculateResults( const Maths::HF32 & density );

	void CalculateResults_Surface( const Maths::HF32 & mass );


	// Result accessors.
	const Maths::HF32 & GetMass() const { return m_Mass; }
	const Maths::C_Vertex & GetCentreOfMass() const { return m_CentreOfMass; }
	const Maths::HMat33 & GetInverseInertiaTensor() const { return m_InverseInertiaTensor; }


	void Reset()
	{
		m_TriangleList.Clear();
	}


private:

	typedef double														_Scalar;
	typedef UtMaths::MVector< UtMaths::VectorXYZ< double > >			_Vector;
	typedef UtMaths::MMatrix< UtMaths::BasicMatrix< _Vector, 3 > >		_Matrix;


	static const _Vector ZeroVector()
	{
		_Vector ret;
		Clear(&ret);
		return ret;
	}


	static const _Matrix ZeroMatrix()
	{
		_Matrix ret;
		Clear(&ret);
		return ret;
	}


	static const _Vector UniformVector(const _Scalar & elem)
	{
		return _Vector(elem, elem, elem);
	}


	static const _Matrix SemiUniformMatrix(const _Scalar & onDiag, const _Scalar & offDiag)
	{
		_Matrix val;
		val.At<0>() = _Vector(onDiag, offDiag, offDiag);
		val.At<1>() = _Vector(offDiag, onDiag, offDiag);
		val.At<2>() = _Vector(offDiag, offDiag, onDiag);
		return val;
	}


	static const Maths::HVec3 GetHVec3(const _Vector & vector)
	{
		return Maths::HVec3( vector.X(), vector.Y(), vector.Z() );
	}


	static const _Vector GetVector(const Maths::HVec3 & hvec3)
	{
		return _Vector( hvec3.X(), hvec3.Y(), hvec3.Z() );
	}


	static const Maths::HMat33 GetHMat33(const _Matrix & matrix)
	{
		return Maths::HMat33(
			GetHVec3( matrix.At<0>() ),
			GetHVec3( matrix.At<1>() ),
			GetHVec3( matrix.At<2>() )
			);
	}


	static const _Matrix ConvertMoment2ToMoi(const _Matrix & moment2)
	{
		_Matrix identity;
		SetIdentity( &identity );

		_Scalar trace = Trace( moment2 );
		_Matrix temp1 = trace * identity;
		_Matrix temp2 = moment2 * ( -1.0 );
		return temp1 + temp2;
	}


// -------------------

	// The following refer to a tetrahedron of unit density defined by matrix R.

	static const _Scalar Moment0_Tetrahedron( const _Matrix & R, const _Scalar & detR )
	{
		return (1.0 / 6.0) * detR;
	}

	static const _Vector Moment1_Tetrahedron( const _Matrix & R, const _Scalar & detR )
	{
		return UniformVector( 1.0 / 24.0 ) * R * detR;
	}

	static const _Matrix Moment2_Tetrahedron( const _Matrix & R, const _Scalar & detR )
	{
		return ( ~R ) * SemiUniformMatrix( 1.0 / 60.0, 1.0 / 120.0 ) * R * detR;
	}


// -------------------

	// The following refer to a triangle defined by matrix R whose mass is equal to its area.

	static const _Scalar GetTriangleArea( const _Matrix & R )
	{
		return 0.5 * Magnitude( (R.At<2>() - R.At<0>()) ^ (R.At<1>() - R.At<0>()) );
	}

	static const _Scalar Moment0_Triangle( const _Matrix & R, const _Scalar & area )
	{
		return area;
	}

	static const _Vector Moment1_Triangle( const _Matrix & R, const _Scalar & area )
	{
		return UniformVector(1.0 / 3.0) * R * area;
	}

	static const _Matrix Moment2_Triangle( const _Matrix & R, const _Scalar & area )
	{
		return ( ~R ) * SemiUniformMatrix(1.0 / 6.0, 1.0 / 12.0) * R * area;
	}


// -------------------

	Maths::HF32 m_Mass;
	Maths::C_Vertex m_CentreOfMass;
	Maths::HMat33 m_InverseInertiaTensor;


	struct Triangle
	{
		Triangle() {}
		Triangle( const Maths::C_Vertex & vertex0, const Maths::C_Vertex & vertex1,
			const Maths::C_Vertex & vertex2 ): m_Vertex0( vertex0 ), m_Vertex1( vertex1 ), m_Vertex2( vertex2 ) {}

		// Returns the R matrix of the triangle, relative to the specified origin.
		const _Matrix GetMatrix(const _Vector & origin = ZeroVector()) const
		{
			_Matrix R;
			R.At<0>() = GetVector(m_Vertex0) - origin;
			R.At<1>() = GetVector(m_Vertex1) - origin;
			R.At<2>() = GetVector(m_Vertex2) - origin;
			return R;
		}

		Maths::C_Vertex m_Vertex0;
		Maths::C_Vertex m_Vertex1;
		Maths::C_Vertex m_Vertex2;
	};

	typedef Ut::UtList< Triangle > TriangleList;
	TriangleList m_TriangleList;
};


// -----------------------------------------------------------------
};

#endif
