#ifndef DEBUGRENDERITEM_INL
#define DEBUGRENDERITEM_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace DbUtil
{

namespace Pod
{

// F32_2
template <typename Archive>
inline void Serialise(Archive& ar, const F32_2& v)
{
	ar << v.x << v.y;
}

template <typename Archive>
inline void Deserialise(Archive& ar, F32_2& v)
{
	ar >> v.x >> v.y;
}


// F32_3
template <typename Archive>
inline void Serialise(Archive& ar, const F32_3& v)
{
	ar << v.x << v.y << v.z;
}

template <typename Archive>
inline void Deserialise(Archive& ar, F32_3& v)
{
	ar >> v.x >> v.y >> v.z;
}



// F32_4
template <typename Archive>
inline void Serialise(Archive& ar, const F32_4& v)
{
	ar << v.x << v.y << v.z << v.w;
}

template <typename Archive>
inline void Deserialise(Archive& ar, F32_4& v)
{
	ar >> v.x >> v.y >> v.z >> v.w;
}

// U8_4
template <typename Archive>
inline void Serialise(Archive& ar, const U8_4& v)
{
	ar << v.v;
}

template <typename Archive>
inline void Deserialise(Archive& ar, U8_4& v)
{
	ar >> v.v;
}

// F32_4x3
template <typename Archive>
inline void Serialise(Archive& ar, const F32_4x3& v)
{
	ar << v.ax << v.ay << v.az << v.aw;
}

template <typename Archive>
inline void Deserialise(Archive& ar, F32_4x3& v)
{
	ar >> v.ax >> v.ay >> v.az >> v.aw;
}
} // namespace Pod

inline Maths::HVec2 Load(const Pod::F32_2& v)
{
	return Maths::HVec2(v.x, v.y);
}

inline Pod::F32_2 Store(const Maths::HVec2 PCREF v)
{
	Pod::F32_2 result;
	result.x = v.X();
	result.y = v.Y();
	return result;
}

inline Maths::HVec3 Load(const Pod::F32_3& v)
{
	return Maths::HVec3(v.x, v.y, v.z);
}

inline Pod::F32_3 Store(const Maths::HVec3 PCREF v)
{
	Pod::F32_3 result;
	result.x = v.X();
	result.y = v.Y();
	result.z = v.Z();
	return result;
}

inline Maths::HVec4 Load(const Pod::F32_4& v)
{
	return Maths::HVec4(v.x, v.y, v.z, v.w);
}

inline Pod::F32_4 Store(const Maths::HVec4 PCREF v)
{
	Pod::F32_4 result;
	result.x = v.X();
	result.y = v.Y();
	result.z = v.Z();
	result.w = v.W();
	return result;
}

inline Maths::RGBA Load(const Pod::U8_4& v)
{
	return Maths::RGBA(v.x, v.y, v.z, v.w);
}

inline Pod::U8_4 Store(const Maths::RGBA PCREF v)
{
	Pod::U8_4 result;
	result.x = v.R();
	result.y = v.G();
	result.z = v.B();
	result.w = v.A();
	return result;
}


inline Maths::HMat43 Load(const Pod::F32_4x3& v)
{
	return Maths::HMat43( Load(v.ax), Load(v.ay), Load(v.az), Load(v.aw) );
}

inline Pod::F32_4x3 Store(const Maths::HMat43 PCREF v)
{
	Pod::F32_4x3 result;
	result.ax = Store(v.AX());
	result.ay = Store(v.AY());
	result.az = Store(v.AZ());
	result.aw = Store(v.AW());
	return result;
}

inline F32 Store( const F32 v )
{
	return v;
}

// C_DebugRenderItem

template <typename Archive>
void C_DebugRenderItem::Serialise(Archive& ar) const
{
	ar << (U8)m_Type;

	switch(m_Type)
	{
		default:
		case E_DebugRenderItemType_None:
			break;

		case E_DebugRenderItemType_2dLine:
			ar << m_Data.m_Line2dStart;
			ar << m_Data.m_Line2dEnd;
			ar << m_Data.m_Line2dColour.v;
			break;

		case E_DebugRenderItemType_2dCircle:
			ar << m_Data.m_Circle2dCentre;
			ar << m_Data.m_Circle2dSize;
			ar << m_Data.m_Circle2dColour.v;
			break;

		case E_DebugRenderItemType_2dArrow:
			ar << m_Data.m_Arrow2dPos;
			ar << m_Data.m_Arrow2dDir;
			ar << m_Data.m_Arrow2dArrowheadSize;
			ar << m_Data.m_Arrow2dColour.v;
			break;

		case E_DebugRenderItemType_2dBar:
			ar << m_Data.m_Bar2dPos;
			ar << m_Data.m_Bar2dSize;
			ar << m_Data.m_Bar2dColour.v;
			break;

		case E_DebugRenderItemType_2dTriangle:
			ar << m_Data.m_Triangle2dPosA;
			ar << m_Data.m_Triangle2dPosB;
			ar << m_Data.m_Triangle2dPosC;
			ar << m_Data.m_Triangle2dColour.v;
			break;


		case E_DebugRenderItemType_3dLine:
			ar << m_Data.m_Line3dStart;
			ar << m_Data.m_Line3dEnd;
			ar << m_Data.m_Line3dColourA.v;
			ar << m_Data.m_Line3dColourB.v;
			break;

		case E_DebugRenderItemType_3dTriangle:
		case E_DebugRenderItemType_3dTriangleOutline:
		case E_DebugRenderItemType_3dTriangleLit:
			ar << m_Data.m_Triangle3dPosA;
			ar << m_Data.m_Triangle3dPosB;
			ar << m_Data.m_Triangle3dPosC;
			ar << m_Data.m_Triangle3dColourA.v;
			ar << m_Data.m_Triangle3dColourB.v;
			ar << m_Data.m_Triangle3dColourC.v;
			break;

		case E_DebugRenderItemType_3dPoint:
		case E_DebugRenderItemType_3dCross:
		case E_DebugRenderItemType_3dStar:
			ar << m_Data.m_Point3dPos;
			ar << m_Data.m_Point3dSize;
			ar << m_Data.m_Point3dColour.v;
			break;

		case E_DebugRenderItemType_3dPlanePoint:
		case E_DebugRenderItemType_3dPlaneCross:
		case E_DebugRenderItemType_3dPlaneStar:
			ar << m_Data.m_PlanePoint3dPos;
			ar << m_Data.m_PlanePoint3dNormal;
			ar << m_Data.m_PlanePoint3dSize;
			ar << m_Data.m_PlanePoint3dColour.v;
			break;

		case E_DebugRenderItemType_3dPlane:
			ar << m_Data.m_Plane3dPlane;
			ar << m_Data.m_Plane3dRefPoint;
			ar << m_Data.m_Plane3dSize;
			ar << m_Data.m_Plane3dColour.v;
			break;

		case E_DebugRenderItemType_3dArrow:
			ar << m_Data.m_Arrow3dPos;
			ar << m_Data.m_Arrow3dDir;
			ar << m_Data.m_Arrow3dLengthScale;
			ar << m_Data.m_Arrow3dArrowheadSize;
			ar << m_Data.m_Arrow3dColour.v;
			break;

		case E_DebugRenderItemType_3dCircle:
			ar << m_Data.m_Circle3dCentre;
			ar << m_Data.m_Circle3dNormal;
			ar << m_Data.m_Circle3dRadius;
			ar << m_Data.m_Circle3dColour.v;
			break;

		case E_DebugRenderItemType_3dSphere:
		case E_DebugRenderItemType_3dSphereOutline:
			ar << m_Data.m_Sphere3dCentre;
			ar << m_Data.m_Sphere3dRadius;
			ar << m_Data.m_Sphere3dColour.v;
			break;

		case E_DebugRenderItemType_3dMatrix:
			ar << m_Data.m_Matrix3d;
			ar << m_Data.m_Matrix3dSize;
			break;

		case E_DebugRenderItemType_3dBox:
		case E_DebugRenderItemType_3dBoxLit:
			ar << m_Data.m_Box3dMin;
			ar << m_Data.m_Box3dMax;
			ar << m_Data.m_Box3dMatrix;
			ar << m_Data.m_Box3dColour.v;
			break;

		case E_DebugRenderItemType_3dGrid:
			ar << m_Data.m_Matrix3d;
			break;

		case E_DebugRenderItemType_DepthEnable:
			ar << m_Data.m_DepthEnable;
			break;

		case E_DebugRenderItemType_LightDirection:
			ar << m_Data.m_LightDirection;
			break;

		case E_DebugRenderItemType_BlendMode:
			ar << m_Data.m_BlendMode;
			break;

		case E_DebugRenderItemType_2dText:
			{
				Util::C_String text = m_Data.m_Text2dText;
				ar << text;
			}
			break;

		case E_DebugRenderItemType_3dText:
			{
				ar << m_Data.m_Text3dSize;
				Util::C_String text = m_Data.m_Text3dText;
				ar << text;
			}
			break;
	}
}

template <typename Archive>
void C_DebugRenderItem::Deserialise(Archive& ar)
{
	U8 type;
	ar >> type;
	m_Type = (E_DebugRenderItemType)type;

	switch(m_Type)
	{
		default:
		case E_DebugRenderItemType_None:
			break;

		case E_DebugRenderItemType_2dLine:
			ar >> m_Data.m_Line2dStart;
			ar >> m_Data.m_Line2dEnd;
			ar >> m_Data.m_Line2dColour.v;
			break;

		case E_DebugRenderItemType_2dCircle:
			ar >> m_Data.m_Circle2dCentre;
			ar >> m_Data.m_Circle2dSize;
			ar >> m_Data.m_Circle2dColour.v;
			break;

		case E_DebugRenderItemType_2dArrow:
			ar >> m_Data.m_Arrow2dPos;
			ar >> m_Data.m_Arrow2dDir;
			ar >> m_Data.m_Arrow2dArrowheadSize;
			ar >> m_Data.m_Arrow2dColour.v;
			break;

		case E_DebugRenderItemType_2dBar:
			ar >> m_Data.m_Bar2dPos;
			ar >> m_Data.m_Bar2dSize;
			ar >> m_Data.m_Bar2dColour.v;
			break;

		case E_DebugRenderItemType_2dTriangle:
			ar >> m_Data.m_Triangle2dPosA;
			ar >> m_Data.m_Triangle2dPosB;
			ar >> m_Data.m_Triangle2dPosC;
			ar >> m_Data.m_Triangle2dColour.v;
			break;


		case E_DebugRenderItemType_3dLine:
			ar >> m_Data.m_Line3dStart;
			ar >> m_Data.m_Line3dEnd;
			ar >> m_Data.m_Line3dColourA.v;
			ar >> m_Data.m_Line3dColourB.v;
			break;

		case E_DebugRenderItemType_3dTriangle:
		case E_DebugRenderItemType_3dTriangleOutline:
		case E_DebugRenderItemType_3dTriangleLit:
			ar >> m_Data.m_Triangle3dPosA;
			ar >> m_Data.m_Triangle3dPosB;
			ar >> m_Data.m_Triangle3dPosC;
			ar >> m_Data.m_Triangle3dColourA.v;
			ar >> m_Data.m_Triangle3dColourB.v;
			ar >> m_Data.m_Triangle3dColourC.v;
			break;

		case E_DebugRenderItemType_3dPoint:
		case E_DebugRenderItemType_3dCross:
		case E_DebugRenderItemType_3dStar:
			ar >> m_Data.m_Point3dPos;
			ar >> m_Data.m_Point3dSize;
			ar >> m_Data.m_Point3dColour.v;
			break;

		case E_DebugRenderItemType_3dPlanePoint:
		case E_DebugRenderItemType_3dPlaneCross:
		case E_DebugRenderItemType_3dPlaneStar:
			ar >> m_Data.m_PlanePoint3dPos;
			ar >> m_Data.m_PlanePoint3dNormal;
			ar >> m_Data.m_PlanePoint3dSize;
			ar >> m_Data.m_PlanePoint3dColour.v;
			break;

		case E_DebugRenderItemType_3dPlane:
			ar >> m_Data.m_Plane3dPlane;
			ar >> m_Data.m_Plane3dRefPoint;
			ar >> m_Data.m_Plane3dSize;
			ar >> m_Data.m_Plane3dColour.v;
			break;

		case E_DebugRenderItemType_3dArrow:
			ar >> m_Data.m_Arrow3dPos;
			ar >> m_Data.m_Arrow3dDir;
			ar >> m_Data.m_Arrow3dLengthScale;
			ar >> m_Data.m_Arrow3dArrowheadSize;
			ar >> m_Data.m_Arrow3dColour.v;
			break;

		case E_DebugRenderItemType_3dCircle:
			ar >> m_Data.m_Circle3dCentre;
			ar >> m_Data.m_Circle3dNormal;
			ar >> m_Data.m_Circle3dRadius;
			ar >> m_Data.m_Circle3dColour.v;
			break;

		case E_DebugRenderItemType_3dSphere:
		case E_DebugRenderItemType_3dSphereOutline:
			ar >> m_Data.m_Sphere3dCentre;
			ar >> m_Data.m_Sphere3dRadius;
			ar >> m_Data.m_Sphere3dColour.v;
			break;

		case E_DebugRenderItemType_3dMatrix:
			ar >> m_Data.m_Matrix3d;
			ar >> m_Data.m_Matrix3dSize;
			break;

		case E_DebugRenderItemType_3dBox:
		case E_DebugRenderItemType_3dBoxLit:
			ar >> m_Data.m_Box3dMin;
			ar >> m_Data.m_Box3dMax;
			ar >> m_Data.m_Box3dMatrix;
			ar >> m_Data.m_Box3dColour.v;
			break;

		case E_DebugRenderItemType_3dGrid:
			ar >> m_Data.m_Matrix3d;
			break;

		case E_DebugRenderItemType_DepthEnable:
			ar >> m_Data.m_DepthEnable;
			break;

		case E_DebugRenderItemType_LightDirection:
			ar >> m_Data.m_LightDirection;
			break;

		case E_DebugRenderItemType_BlendMode:
			ar >> m_Data.m_BlendMode;
			break;

		case E_DebugRenderItemType_2dText:
			{
				Util::C_String text;
				ar >> text;

				m_Data.m_Text2dText = new char[text.Length() + 1];
				strcpy(m_Data.m_Text2dText, text.Chars());
			}
			break;

		case E_DebugRenderItemType_3dText:
			{
				ar >> m_Data.m_Text3dSize;
				Util::C_String text;
				ar >> text;

				m_Data.m_Text3dText = new char[text.Length() + 1];
				strcpy(m_Data.m_Text3dText, text.Chars());
			}
			break;
	}
}

} // DbUtil

#endif // DEBUGRENDERITEM_INL
