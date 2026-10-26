#pragma once

typedef UtMaths::MPoint<UtMaths::VectorXYZ<F32> >						Position3;
typedef UtMaths::MNormal<UtMaths::VectorXYZ<F32> >						Normal3;

typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >						Vector4F32;
typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >						Vector3F32;
typedef UtMaths::MVector<UtMaths::VectorXY<F32> >						Vector2F32;
typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4F32, 4> >			Matrix44F32;

typedef UtMaths::MVector<UtMaths::VectorRGBA<F32> >						AftkColour4;

typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4F32, 4> >		Matrix44F32;

typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4F32, 4> >		Matrix44;

namespace ost
{
	class Engine;
}

template <typename T>
ost::Engine& operator << (ost::Engine& ar, const Ut::Vector<T>&vec)
{
	ar << vec.Size();
	for (int i=0; i<vec.Size(); i++)
		ar << vec[i];

	return ar;
}

template <typename T>
ost::Engine& operator >> (ost::Engine& ar, Ut::Vector<T>&vec)
{
	int c;
	ar >> c;
	vec.Resize(c);
	for (int i=0; i<c; i++)
		ar >> vec[i];

	return ar;
}
