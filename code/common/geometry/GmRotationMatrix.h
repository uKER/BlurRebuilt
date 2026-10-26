////////////////////////////////////////////////////////////////////////////////
// GmRotationMatrix
////////////////////////////////////////////////////////////////////////////////

namespace Gm {

class GmQuaternion;

/// Rotation matrix in 3D.
/// This is a special case of a 3x3 matrix. It uses GmMatrix internally.
class GmRotationMatrix
{
public:
	typedef GmMatrix::DataType VectorType;

	// private data to keep it normalised
private:
	GmMatrix matrix;

	// general constructor private
	GmRotationMatrix(const UtMaths::MVector<VectorType>& ax, const UtMaths::MVector<VectorType>& ay, const UtMaths::MVector<VectorType>& az)
	{
		matrix.At<0>() = ax;
		matrix.At<1>() = ay;
		matrix.At<2>() = az;
	}

public:
	// constructor
	GmRotationMatrix() {UtMaths::SetIdentity(&matrix);} ///<
	GmRotationMatrix(float heading,	float pitch, float roll); ///<
	GmRotationMatrix(const GmNormal& v, float a); ///<

	// initialisation
	void SetIdentity() {UtMaths::SetIdentity(&matrix);} ///< Set to the identity matrix.
	void SetRotation(const GmQuaternion& q); ///< Set rotation from quaternion.
	void SetRotation(const GmNormal& v, float a); ///< Set rotation from normal and angle.
	void SetRotation(const GmMatrix& m) {matrix = m;} ///< Set rotation from another matrix, which must be orthonormal.
	void SetRotation(const GmRotationMatrix& m) {matrix = m.matrix;} 

	// access
	__forceinline const GmMatrix&	Matrix3() const {return matrix;} ///< Return 3x3 matrix.
	__forceinline const UtMaths::MNormal<typename VectorType::VectorType>& AxisX() const {return MakeNormal(matrix.At<0>());} ///< 1st row of matrix.
	__forceinline const UtMaths::MNormal<typename VectorType::VectorType>& AxisY() const {return MakeNormal(matrix.At<1>());} ///< 2nd row of matrix.
	__forceinline const UtMaths::MNormal<typename VectorType::VectorType>& AxisZ() const {return MakeNormal(matrix.At<2>());} ///< 3rd row of matrix.

	// information
	GmRotationMatrix Inverse() const; ///< Return the inverse rotation.
	GmRotationMatrix Rotated(const GmRotationMatrix& ir) const; ///< Return combined rotation. 
	GmRotationMatrix Rotated(const UtMaths::MVector<VectorType>& iv) const; ///< Return combined rotation. 
	GmRotationMatrix Rotated(const GmNormal& v, float a)	const; ///< Return combined rotation.
	GmRotationMatrix RotatedLocalY180() const; ///<
	
	// modification
	GmRotationMatrix&	operator = (const GmRotationMatrix& ir) {matrix = ir.matrix; return *this;}
	void				Set(const GmRotationMatrix& ir) {matrix = ir.matrix;} ///< Set equal to input matrix.
	void				Invert(); ///< Transpose the rotation matrix.
	void				Rotate(const GmRotationMatrix& r); ///< Rotate by input matrix.

	template <typename VectorType2>
	void Rotate(const UtMaths::MConstVector<VectorType2>& v);  ///< Rotate by input vector.

	void				Rotate(const GmNormal& v, float a); ///< Rotate by angle around normal.
	void				LookAlong(const GmNormal& dir, const GmNormal& up); ///<
	void				RotateLocalY180() {Negate(&matrix.At<0>()); Negate(&matrix.At<2>());} ///<

	static GmRotationMatrix RotationMatrixZ(float a); ///< Return a matrix that rotates around the z axis.

private:

	template <typename VectorType2>
	void RotateDelta(const UtMaths::MConstVector<VectorType2>& v);
};

template <typename VectorType2>
void GmRotationMatrix::Rotate(const UtMaths::MConstVector<VectorType2>& v) 
{
	// this method requires rotations ~ 0.01 rad
	float a = Magnitude(v);
	
	if (a > 0.02f)
	{
		Rotate(GmNormal(v, a), a);
	}
	else
	{
		int n = 1 + (int) (a / 0.01f);
		MVector<VectorType> v1 = v / DmValue(n);

		for (int i = 0; i < n; i++)
			RotateDelta(v1);
	}
}

// Rotate a small amount by addition and subtraction.
template <typename VectorType2>
void GmRotationMatrix::RotateDelta(const UtMaths::MConstVector<VectorType2>& v)
{
	matrix.At<0>() += v ^ matrix.At<0>();
	matrix.At<1>() += v ^ matrix.At<1>();

	Normalise(&matrix);
}

// normal * rotation matrix
inline GmNormal MatrixMultiply(const GmNormal& v, const GmRotationMatrix& m)
{
	return GmNormal(v.X() * m.Matrix3().At<0>() + v.Y() * m.Matrix3().At<1>() + v.Z() * m.Matrix3().At<2>());
}

inline GmNormal MatrixMultiply(const GmRotationMatrix& m, const GmNormal& v)
{
	GmNormal normal;
	normal.x = v | m.Matrix3().At<0>();
	normal.y = v | m.Matrix3().At<1>();
	normal.z = v | m.Matrix3().At<2>();
	return normal;
}

} // End Gm