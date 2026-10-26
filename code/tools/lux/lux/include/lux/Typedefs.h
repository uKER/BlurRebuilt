#ifndef LUX_TYPEDEFS_H
#define LUX_TYPEDEFS_H

namespace LUX
{
#ifdef LUX_USE_32_BIT_POSITIONS
	typedef UtMaths::MPoint<UtMaths::VectorXYZW<F32> >				Position4;
	typedef UtMaths::MPoint<UtMaths::VectorXYZ<F32> >				Position3;
	typedef UtMaths::MPoint<UtMaths::VectorXY<F32> >				Position2;

	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				Vector3;
#elif  LUX_USE_64_BIT_POSITIONS
	typedef UtMaths::MPoint<UtMaths::VectorXYZW<F64> >				Position4;
	typedef UtMaths::MPoint<UtMaths::VectorXYZ<F64> >				Position3;
	typedef UtMaths::MPoint<UtMaths::VectorXY<F64> >				Position2;

	typedef UtMaths::MVector<UtMaths::VectorXYZ<F64> >				Vector3;
#else
	// default to 32-bit positions
	typedef UtMaths::MPoint<UtMaths::VectorXYZW<F32> >				Position4;
	typedef UtMaths::MPoint<UtMaths::VectorXYZ<F32> >				Position3;
	typedef UtMaths::MPoint<UtMaths::VectorXY<F32> >				Position2;

	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				Vector3;
#endif

typedef UtMaths::MPoint<UtMaths::VectorXY<S32> >					ImagePosition2;

#ifdef LUX_USE_32_BIT_NORMALS
	typedef UtMaths::MNormal<UtMaths::VectorXYZ<F32> >				Normal3;
#elif  LUX_USE_64_BIT_NORMALS
	typedef UtMaths::MNormal<UtMaths::VectorXYZ<F64> >				Normal3;
#else
	// default to 32-bit normals
	typedef UtMaths::MNormal<UtMaths::VectorXYZ<F32> >				Normal3;
#endif

#ifdef LUX_USE_32_BIT_3_ELEMENT_UVS
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				TextureCoordinate3;
#elif  LUX_USE_64_BIT_3_ELEMENTUVS
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F64> >				TextureCoordinate3;
#else
	// default to 32-bit uvs
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				TextureCoordinate3;
#endif
	typedef UtMaths::MVector<UtMaths::VectorXY<F32> >				TextureCoordinate2;

#ifdef LUX_USE_32_BIT_COLOURS
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Colour4;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Colour3;
#elif LUX_USE_64_BIT_COLOURS
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F64> >				Colour4;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F64> >				Colour3;
#else
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Colour4;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Colour3;
#endif

typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >					Vector4F32;
typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >					Vector3F32;
typedef UtMaths::MVector<UtMaths::VectorXY<F32> >					Vector2F32;
typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4F32, 4> >		Matrix44F32;

typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4F32, 4> >		Matrix44;

enum TextureFormat
{
	TEXFMT_A8L8,
	TEXFMT_DXT4,
	TEXFMT_A8R8G8B8,
	TEXFMT_DXT1,
	TEXFMT_R5G6B5,
	TEXFMT_UYVY,
};
}

#endif
