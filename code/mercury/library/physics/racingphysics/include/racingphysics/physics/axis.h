////////////////////////////////////////////////////////////////////////////////
// Axis
////////////////////////////////////////////////////////////////////////////////

#ifndef RP_AXIS_H
#define RP_AXIS_H
#ifdef _MSC_VER
#pragma once
#endif

#include "rotationmatrix.h"

namespace RacingPhysics 
{

/// Represents a coordinate system and transforms other types.
class Axis : public RotationMatrix
{
public:
	typedef Point3R PointType;

	// constructor
	Axis() {}
	Axis(const Point3R& ip, const RotationMatrix& im) : position(ip), RotationMatrix(im) {} ///< 
	explicit Axis(const Matrix44R& m);

	void Translate(const Vector3R& v);
	void SetPosition(const Point3R& p) {position = p;}

	void LookAt(const Point3R& dest, const Normal3R& up = Maths::Constants<Normal3R>::Y())
	{
		LookAlong(Normalised(dest - position), up);
	}

	void Set(const Axis& a) {RotationMatrix::Set(a); position = a.position;} ///<
	void Blend(const Axis& a1, const Axis& a2, float f); ///< %Blend from a1 (at f == 1) to a2 (at f == 0).

	// information
#if !defined(CFG_SPEED)
	bool DebugTest() const;
#endif

	__forceinline const PointType& Position() const {return position;} ///<

	/// Return a rotated axis.
	Axis Rotated(const RotationMatrix& ir) const {return Axis(position, RotationMatrix::Rotated(ir));}
	
	/// Return a rotated axis.
	Axis Rotated(const Vector3R& iv) const {return Axis(position, RotationMatrix::Rotated(iv));}
	
	/// Return a rotated axis.
	Axis Rotated(const Normal3R& v, FloatVR a) const {return Axis(position, RotationMatrix::Rotated(v, a));}

	/// Return an axis rotated about the local Y axis.
	Axis RotatedLocalY180() const {return Axis(position, RotationMatrix::RotatedLocalY180());}

	// transformations

	// axis
	Axis Global(const Axis& r) const; ///< Transform an axis out of this coordinate system.
	Axis Local(const Axis& r) const; ///< Transform an axis into this coordinate system.

	// matrix
	Matrix33R Global(const Matrix33R& m) const; ///< Transform a matrix out of this coordinate system.
	Matrix33R Local(const Matrix33R& m) const; ///< Transform an matrix into this coordinate system.

	__forceinline PointType Global(const PointType& p) const
	{
		return PointType(MakeVector(p) * Matrix3() + position);
	}

	__forceinline PointType Local(const PointType& p) const
	{
		Vector3R v = p - position;
		return MakePoint(Matrix3() * v);
	}

	// vector
	__forceinline  Vector3R Global(const Vector3R& v) const {return v * Matrix3();}
	__forceinline  Vector3R Local(const Vector3R& v) const {return Matrix3() * v;}

	// normal
	__forceinline  Normal3R Global(const Normal3R& v) const {return MakeNormal(v * Matrix3());}
	__forceinline  Normal3R Local(const Normal3R& v) const {return MakeNormal(Matrix3() * v);}

	void GetRollPitchYaw_Trig(float& tanRoll, float& sinPitch, float& tanYaw) const;

	MathsStorage::Matrix44 GetMatrix() const
	{
		const Normal3R& axisX = AxisX();
		const Normal3R& axisY = AxisY();
		const Normal3R& axisZ = AxisZ();
		const Point3R& pos = Position();

		MathsStorage::Matrix44 matrix = {	axisX.X(), axisX.Y(), axisX.Z(), 0.f,
											axisY.X(), axisY.Y(), axisY.Z(), 0.f,
											axisZ.X(), axisZ.Y(), axisZ.Z(), 0.f,
											pos.X(), pos.Y(), pos.Z(), 1.f 
										};
		return matrix;
	}

	MathsStorage::Matrix44 GetMatrixTranspose() const
	{
		const Normal3R& axisX = AxisX();
		const Normal3R& axisY = AxisY();
		const Normal3R& axisZ = AxisZ();
		const Point3R& pos = Position();

		MathsStorage::Matrix44 matrix = {	axisX.X(), axisY.X(), axisZ.X(), pos.X(),
											axisX.Y(), axisY.Y(), axisZ.Y(), pos.Y(),
											axisX.Z(), axisY.Z(), axisZ.Z(), pos.Z(),
											0.f, 0.f, 0.f, 1.f 
										};
		return matrix;
	}

private:
	class Point3_D
	{
	public:
		Point3_D() {}
		Point3_D(const Point3R& p) : m_p(p), m_x(p.X()), m_y(p.Y()), m_z(p.Z()) {}

		operator const Point3R& () const {return m_p;}

		void operator += (const Vector3R& v) 
		{
			m_x += v.X();
			m_y += v.Y();
			m_z += v.Z();
			Mirror();
		}

		double X() const {return m_x;}
		double Y() const {return m_y;}
		double Z() const {return m_z;}

	private:
		void Mirror()
		{
			m_p.X(m_x); 
			m_p.Y(m_y); 
			m_p.Z(m_z); 
		}

		Point3R m_p;
		double m_x, m_y, m_z;
	};

	Point3_D position;
};

///////////////////////////////////////////////////////////////////

inline Axis Lerp(const Axis& a1, const Axis& a2, float f)
{
	Axis a;
	a.Blend(a2, a1, f);
	return a;
}

// matrix
inline Matrix33R Axis::Global(const Matrix33R& m) const
{
	return Matrix3() * m * ~Matrix3();
}

inline Matrix33R Axis::Local(const Matrix33R& m) const
{
	return ~Matrix3() * m * Matrix3();
}

// axis
inline Axis Axis::Global(const Axis& r) const 
{
	return Axis(Global(r.position), RotationMatrix(r.Matrix3() * Matrix3()));
}

inline Axis Axis::Local(const Axis& r) const 
{
	return Axis(Local(r.position), RotationMatrix(r.Matrix3() * ~Matrix3()));
}

inline Axis ConvertHVec(const Maths::HMat43& src)
{
	// can't just cast this because hmat is stored transposed
	Axis dst;

	// HMat isn't necessarily normalised - just have to assume that it is
	const_cast<Matrix33R&>(dst.Matrix3()).AX(MakeNormal(ConvertHVec(src.AX())));
	const_cast<Matrix33R&>(dst.Matrix3()).AY(MakeNormal(ConvertHVec(src.AY())));
	const_cast<Matrix33R&>(dst.Matrix3()).AZ(MakeNormal(ConvertHVec(src.AZ())));
	dst.SetPosition(MakePoint(ConvertHVec(src.AW())));

	return dst;
}

inline Maths::HMat43 ConvertHVec(const Axis& src)
{
	// can't just cast this because hmat is stored transposed
	Maths::HMat43 dst;

	dst.AX(ConvertHVec(src.AxisX()));
	dst.AY(ConvertHVec(src.AxisY()));
	dst.AZ(ConvertHVec(src.AxisZ()));
	dst.AW(ConvertHVec(src.Position()));

	return dst;
}

inline Axis::Axis(const Matrix44R& m)
{
	Point3R point((FloatVR)(m[3][0]), (FloatVR)(m[3][1]), (FloatVR)(m[3][2]));

	Matrix33R mat;
	mat.AX(Vector3R((FloatVR)m[0][0], (FloatVR)m[0][1], (FloatVR)m[0][2]));
	mat.AY(Vector3R((FloatVR)m[1][0], (FloatVR)m[1][1], (FloatVR)m[1][2]));
	mat.AZ(Vector3R((FloatVR)m[2][0], (FloatVR)m[2][1], (FloatVR)m[2][2]));

	RotationMatrix rotMat;
	rotMat.SetRotation(mat);

	*this = Axis(point, rotMat);
}

inline void Axis::Translate(const Vector3R& v) 
{
	position += v;
}

}

#endif

