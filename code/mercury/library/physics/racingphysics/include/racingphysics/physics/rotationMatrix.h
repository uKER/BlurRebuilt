////////////////////////////////////////////////////////////////////////////////
// RotationMatrix
////////////////////////////////////////////////////////////////////////////////

#ifndef RP_ROTATION_MATRIX_H
#define RP_ROTATION_MATRIX_H
#ifdef _MSC_VER
#pragma once
#endif

#include "geometry.h"

namespace RacingPhysics 
{

class Quaternion;

/// Rotation matrix in 3D.
/// This is a special case of a 3x3 matrix. It uses Matrix33R internally.
class RotationMatrix
{
public:
	//typedef Matrix33R::DataType::VectorType VectorType;

	// private data to keep it normalised
private:
	Matrix33R matrix;

	// general constructor private
	RotationMatrix(const Vector3R& ax, const Vector3R& ay, const Vector3R& az)
	{
		matrix.AX(ax);
		matrix.AY(ay);
		matrix.AZ(az);
	}

public:
	// constructor
	RotationMatrix() {} ///<
	RotationMatrix(float heading,	float pitch, float roll); ///<
	RotationMatrix(const Normal3R& v, FloatVR a); ///<
	explicit RotationMatrix(const Matrix33R& m) : matrix(m) {}

	// initialisation
	void SetIdentityMatrix() {SetIdentity(&matrix);} ///< Set to the identity matrix.
	void SetRotation(const Quaternion& q); ///< Set rotation from quaternion.
	void SetRotation(const Normal3R& v, FloatVR a); ///< Set rotation from normal and angle.
	void SetRotation(const Matrix33R& m) {matrix = m;} ///< Set rotation from another matrix, which must be orthonormal.
	void SetRotation(const RotationMatrix& m) {matrix = m.matrix;} 

	// access
	__forceinline const Matrix33R& Matrix3() const {return matrix;} ///< Return 3x3 matrix.
	__forceinline const Normal3R AxisX() const {return MakeNormal(matrix.AX());} ///< 1st row of matrix.
	__forceinline const Normal3R AxisY() const {return MakeNormal(matrix.AY());} ///< 2nd row of matrix.
	__forceinline const Normal3R AxisZ() const {return MakeNormal(matrix.AZ());} ///< 3rd row of matrix.

	// information
	RotationMatrix Inverse() const; ///< Return the inverse rotation.
	RotationMatrix Rotated(const RotationMatrix& ir) const; ///< Return combined rotation. 
	RotationMatrix Rotated(const Vector3R& iv) const; ///< Return combined rotation. 
	RotationMatrix Rotated(const Normal3R& v, FloatVR a)	const; ///< Return combined rotation.
	RotationMatrix RotatedLocalY180() const; ///<
	
	// modification
	RotationMatrix&	operator = (const RotationMatrix& ir) {matrix = ir.matrix; return *this;}
	void				Set(const RotationMatrix& ir) {matrix = ir.matrix;} ///< Set equal to input matrix.
	void				Invert(); ///< Transpose the rotation matrix.
	void				Rotate(const RotationMatrix& r); ///< Rotate by input matrix.

	void Rotate(const Vector3R& v);  ///< Rotate by input vector.

	void				Rotate(const Normal3R& v, FloatVR a); ///< Rotate by angle around normal.
	void				LookAlong(const Normal3R& dir, const Normal3R& up); ///<
	void				RotateLocalY180() {matrix.AX(-matrix.AX()); matrix.AZ(-matrix.AZ());} ///<

	static RotationMatrix RotationMatrixZ(float a); ///< Return a matrix that rotates around the z axis.

	bool DebugTest() const;

private:

	void RotateDelta(const Vector3R& v);
};

///////////////////////////////////////////////////////////////////

// Rotate a small amount by addition and subtraction.
__forceinline void RotationMatrix::RotateDelta(const Vector3R& v)
{
	matrix.AX(matrix.AX() + (v ^ MakeVector(matrix.AX())));
	matrix.AY(matrix.AY() + (v ^ MakeVector(matrix.AY())));

	Normalise(&matrix);
}

inline void RotationMatrix::Rotate(const Vector3R& v) 
{
	// this method requires rotations ~ 0.01 rad
	FloatVR a = Magnitude2(v);
	
	if (a > 0.02f * 0.02f)
	{
		FloatVR angle = Magnitude(v);
		Normal3R n = MakeNormal(v / angle);
		Rotate(n, angle);
		Normalise(&matrix);
	}
	else
	{
		RotateDelta(v);
	}
}

}

#endif

