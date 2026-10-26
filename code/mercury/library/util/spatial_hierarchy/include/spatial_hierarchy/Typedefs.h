#ifndef SD_TYPEDEFS_H
#define SD_TYPEDEFS_H

extern int gnRaysCast;

namespace SD
{
#ifdef SD_USE_32_BIT_POSITIONS
	typedef UtMaths::MPoint<UtMaths::VectorXYZW<F32> >				Position4;
	typedef UtMaths::MPoint<UtMaths::VectorXYZ<F32> >				Position3;
	typedef UtMaths::MPoint<UtMaths::VectorXY<F32> >				Position2;
#elif  SD_USE_64_BIT_POSITIONS
	typedef UtMaths::MPoint<UtMaths::VectorXYZW<F64> >				Position4;
	typedef UtMaths::MPoint<UtMaths::VectorXYZ<F64> >				Position3;
	typedef UtMaths::MPoint<UtMaths::VectorXY<F64> >				Position2;
#else
	// default to 32-bit positions
	typedef UtMaths::MPoint<UtMaths::VectorXYZW<F32> >				Position4;
	typedef UtMaths::MPoint<UtMaths::VectorXYZ<F32> >				Position3;
	typedef UtMaths::MPoint<UtMaths::VectorXY<F32> >				Position2;
#endif

#ifdef SD_USE_32_BIT_NORMALS
	typedef UtMaths::MNormal<UtMaths::VectorXYZ<F32> >				Normal3;
#elif  SD_USE_64_BIT_NORMALS
	typedef UtMaths::MNormal<UtMaths::VectorXYZ<F64> >				Normal3;
#else
	// default to 32-bit normals
	typedef UtMaths::MNormal<UtMaths::VectorXYZ<F32> >				Normal3;
#endif

#ifdef SD_USE_32_BIT_3_ELEMENT_UVS
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				TextureCoordinate3;
#elif  SD_USE_64_BIT_3_ELEMENT_UVS
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F64> >				TextureCoordinate3;
#else
	// default to 32-bit uvs
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >				TextureCoordinate3;
#endif

#ifdef SD_USE_32_BIT_COLOURS
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Colour4;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >				Colour3;
#elif SD_USE_64_BIT_COLOURS
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

typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4F32, 4> >		Float44;
}

#endif
