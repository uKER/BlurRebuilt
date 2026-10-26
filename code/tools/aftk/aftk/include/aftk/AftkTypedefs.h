#ifndef AFTK_TYPEDEFS_H
#define AFTK_TYPEDEFS_H

#ifdef _MSC_VER_
#pragma once
#endif


#define	AFTK_EPSILON 1e-6f

namespace Aftk
{
	typedef Ut::UtVector<Ut::DynamicVector<F32> >							F32Array;

	typedef UtMaths::MPoint<UtMaths::VectorXYZ<F32> >						Position3;
	typedef Ut::UtVector<Ut::DynamicVector<Position3> >						Position3Array;
	typedef UtMaths::MNormal<UtMaths::VectorXYZ<F32> >						Normal3;
	typedef Ut::UtVector<Ut::DynamicVector<Normal3> >						Normal3Array;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >						TextureCoordinate4;
	typedef Ut::UtVector<Ut::DynamicVector<TextureCoordinate4> >			TextureCoordinate4Array;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >						Vector4F32;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >						Vector3F32;
	typedef Ut::UtVector<Ut::DynamicVector<Vector3F32> >					Vector3F32Array;
	typedef UtMaths::MVector<UtMaths::VectorXY<F32> >						Vector2F32;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4F32, 4> >			Matrix44F32;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<F64> >						Vector4F64;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F64> >						Vector3F64;
	typedef UtMaths::MVector<UtMaths::VectorXY<F64> >						Vector2F64;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<S64> >						Vector4S64;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<S64> >						Vector3S64;
	typedef UtMaths::MVector<UtMaths::VectorXY<S64> >						Vector2S64;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<S32> >						Vector4S32;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<S32> >						Vector3S32;
	typedef UtMaths::MVector<UtMaths::VectorXY<S32> >						Vector2S32;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<S16> >						Vector4S16;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<S16> >						Vector3S16;
	typedef UtMaths::MVector<UtMaths::VectorXY<S16> >						Vector2S16;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<S8> >						Vector4S8;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<S8> >						Vector3S8;
	typedef UtMaths::MVector<UtMaths::VectorXY<S8> >						Vector2S8;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<U64> >						Vector4U64;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<U64> >						Vector3U64;
	typedef UtMaths::MVector<UtMaths::VectorXY<U64> >						Vector2U64;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<U32> >						Vector4U32;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<U32> >						Vector3U32;
	typedef UtMaths::MVector<UtMaths::VectorXY<U32> >						Vector2U32;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<U16> >						Vector4U16;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<U16> >						Vector3U16;
	typedef UtMaths::MVector<UtMaths::VectorXY<U16> >						Vector2U16;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<U8> >						Vector4U8;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<U8> >						Vector3U8;
	typedef UtMaths::MVector<UtMaths::VectorXY<U8> >						Vector2U8;

	typedef UtMaths::MVector<UtMaths::VectorXYZW<bool> >					Vector4Bool;
	typedef UtMaths::MVector<UtMaths::VectorXYZ<bool> >						Vector3Bool;
	typedef UtMaths::MVector<UtMaths::VectorXY<bool> >						Vector2Bool;


	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4F32, 4> >			Matrix44;

	typedef Ut::UtVector< Ut::DynamicVector<std::string> >					StringArray;


	#define AFTK_RGBA_TO_U32(r, g, b, a) (r)|(g<<8)|(b<<16)|(a<<24);
	#define AFTK_R_FROM_U32(integer) (U8)(integer);
	#define AFTK_G_FROM_U32(integer) (U8)(integer >> 8);
	#define AFTK_B_FROM_U32(integer) (U8)(integer >> 16);
	#define AFTK_A_FROM_U32(integer) (U8)(integer >> 24);

	#define AFTK_INFLUENCE_CLUSTER_USAGE_SKIN "skin"
	#define AFTK_DATA_CHANNEL_USAGE_VERTEX_COLOUR "vertexColour"

	typedef void(*TextCallback)(const std::string& text);

	//typedef std::basic_string<char, std::char_traits<char>, StringAllocator<char> > String;
}

#endif
