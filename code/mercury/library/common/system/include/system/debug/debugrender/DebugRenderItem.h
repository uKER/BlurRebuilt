#ifndef DEBUGRENDERITEM_H
#define DEBUGRENDERITEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system\debug\I_DebugRender.h>

namespace DbUtil
{

enum E_DebugRenderFileVersion
{
	E_DebugRenderFileVersion_Initial = 0,

	E_DebugRenderFileVersion_Current = E_DebugRenderFileVersion_Initial
};

enum E_DebugRenderItemType
{
	E_DebugRenderItemType_None = 0,

	E_DebugRenderItemType_3dTriangle,
	E_DebugRenderItemType_3dTriangleLit,
	E_DebugRenderItemType_3dTriangleOutline,

	E_DebugRenderItemType_3dPoint,
	E_DebugRenderItemType_3dCross,
	E_DebugRenderItemType_3dStar,

	E_DebugRenderItemType_3dSphere,
	E_DebugRenderItemType_3dSphereOutline,

	E_DebugRenderItemType_3dPlanePoint,
	E_DebugRenderItemType_3dPlaneCross,
	E_DebugRenderItemType_3dPlaneStar,
	E_DebugRenderItemType_3dPlane,

	E_DebugRenderItemType_3dLine,
	E_DebugRenderItemType_3dArrow,
	E_DebugRenderItemType_3dCircle,

	E_DebugRenderItemType_3dMatrix,
	E_DebugRenderItemType_3dBox,
	E_DebugRenderItemType_3dBoxLit,
	E_DebugRenderItemType_3dGrid,

	E_DebugRenderItemType_2dLine,
	E_DebugRenderItemType_2dArrow,
	E_DebugRenderItemType_2dBar,
	E_DebugRenderItemType_2dCircle,
	E_DebugRenderItemType_2dTriangle,

	E_DebugRenderItemType_2dText,
	E_DebugRenderItemType_3dText,

	E_DebugRenderItemType_DepthEnable,
	E_DebugRenderItemType_LightDirection,
	E_DebugRenderItemType_BlendMode
};

namespace Pod
{
	struct F32_2 	
	{ 
		F32 x;	
		F32 y; 
	};

	struct F32_3 	
	{ 
		F32 x;	
		F32 y;		
		F32 z; 
	};
	
	struct F32_4 	
	{ 
		F32 x;	
		F32 y;		
		F32 z;		
		F32 w; 
	};
	
	struct U8_4  	
	{ 
		union
		{
			U32 v;
			struct 
			{
				U8 x;		
				U8 y;		
				U8 z;		
				U8 w; 
			};
		};
	};
	
	struct F32_4x3	
	{ 
		F32_3 ax;
		F32_3 ay;	
		F32_3 az;	
		F32_3 aw;
	};
};

class C_DebugRenderItem
{

friend class C_ItemDebugRenderer;
public:
	C_DebugRenderItem();
	C_DebugRenderItem& operator = ( const C_DebugRenderItem& other );

	C_DebugRenderItem( E_DebugRenderItemType type );
	~C_DebugRenderItem();
	void DebugRender( I_DebugRender* pDebugRender, Maths::HMat43& mWorld )  const;

	template <typename Archive>
	void Serialise(Archive& ar) const;

	template <typename Archive>
	void Deserialise(Archive& ar);

private:

	union
	{
		// 3D Debug Structures
		struct 
		{ 			
			Pod::F32_3		m_Point3dPos;	
			F32				m_Point3dSize;		
			Pod::U8_4		m_Point3dColour; 
		};

		struct 
		{ 
			Pod::F32_3		m_PlanePoint3dPos;	
			Pod::F32_3		m_PlanePoint3dNormal;	
			F32				m_PlanePoint3dSize;		
			Pod::U8_4		m_PlanePoint3dColour; 
		};

		struct 
		{ 
			Pod::F32_4  	m_Plane3dPlane;	
			Pod::F32_3 		m_Plane3dRefPoint;	
			F32				m_Plane3dSize;		
			Pod::U8_4	    m_Plane3dColour; 
		};

		struct 
		{ 
			Pod::F32_3		m_Line3dStart;		
			Pod::F32_3		m_Line3dEnd;		
			Pod::U8_4	    m_Line3dColourA;
			Pod::U8_4	    m_Line3dColourB;
		};

		struct 
		{ 
			Pod::F32_3		m_Triangle3dPosA;
			Pod::F32_3		m_Triangle3dPosB;
			Pod::F32_3		m_Triangle3dPosC;
			Pod::U8_4		m_Triangle3dColourA;
			Pod::U8_4		m_Triangle3dColourB;
			Pod::U8_4		m_Triangle3dColourC;
		};

		struct 
		{ 
			Pod::F32_3		m_Arrow3dPos;	
			Pod::F32_3		m_Arrow3dDir;		
			F32		    	m_Arrow3dLengthScale;		
			F32		    	m_Arrow3dArrowheadSize;	
			Pod::U8_4		m_Arrow3dColour; 
		};

		struct
		{
			Pod::F32_3		m_Circle3dCentre;
			Pod::F32_3		m_Circle3dNormal;
			F32				m_Circle3dRadius;
			Pod::U8_4		m_Circle3dColour;
		};

		struct 
		{ 
			Pod::F32_3		m_Sphere3dCentre;	
			F32				m_Sphere3dRadius;	
			Pod::U8_4		m_Sphere3dColour; 
		};

		struct
		{
			Pod::F32_4x3	m_Matrix3d;
			F32				m_Matrix3dSize;
		};

		struct
		{
			Pod::F32_3		m_Box3dMin;
			Pod::F32_3		m_Box3dMax;
			Pod::F32_4x3	m_Box3dMatrix;
			Pod::U8_4		m_Box3dColour;
		};
		struct
		{
			Pod::F32_3		m_Text3dPos;
			F32				m_Text3dSize;
			char*			m_Text3dText;
		};

		// 2D Debug Structures
		struct
		{
			Pod::F32_2		m_Line2dStart;
			Pod::F32_2		m_Line2dEnd;
			Pod::U8_4		m_Line2dColour;
		};

		struct
		{
			Pod::F32_2		m_Arrow2dPos;
			Pod::F32_2		m_Arrow2dDir;
			F32				m_Arrow2dArrowheadSize;
			Pod::U8_4		m_Arrow2dColour;
		};

		struct
		{
			Pod::F32_2		m_Bar2dPos;
			Pod::F32_2		m_Bar2dSize;
			Pod::U8_4		m_Bar2dColour;
		};

		struct
		{
			Pod::F32_2		m_Circle2dCentre;
			Pod::F32_2		m_Circle2dSize;
			Pod::U8_4		m_Circle2dColour;
		};

		struct
		{
			Pod::F32_2		m_Triangle2dPosA;
			Pod::F32_2		m_Triangle2dPosB;
			Pod::F32_2		m_Triangle2dPosC;
			Pod::U8_4		m_Triangle2dColour;
		};

		struct
		{
			Pod::F32_2		m_Text2dPos;
			Pod::U8_4		m_Text2dColour;
			F32				m_Text2dHeight;
			char*			m_Text2dText;
		};

		struct
		{
			U8				m_DepthEnable;
		};

		struct
		{
			Pod::F32_3		m_LightDirection;
		};

		struct
		{
			U32				m_BlendMode;
		};
	} m_Data;

	E_DebugRenderItemType	m_Type;
};

class C_ItemDebugRenderer : public I_DebugRender
{
public:
	C_ItemDebugRenderer();
	~C_ItemDebugRenderer(); 

	// the pDebugRenderItem object only lives for the scope of the callback
	virtual void OnRender( C_DebugRenderItem* pDebugRenderItem ) = 0;

	// 2d rendering calls...

	void    Draw2dLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA colour) OVERRIDE;
	void    Draw2dCircle(const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA colour) OVERRIDE;
	void    Draw2dArrow(const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA colour) OVERRIDE;
	void    Draw2dBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA colour) OVERRIDE;
	void    Draw2dTriangle(const Maths::HVec2& A, const Maths::HVec2& B, const Maths::HVec2& C, const Maths::RGBA colour) OVERRIDE;

	void    Draw2dText(const Maths::HVec2& pos, const Util::C_String& text) OVERRIDE;
	void    Draw2dText(const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags=0) OVERRIDE;

	// 3d rendering calls...

	void    Draw3dLine(const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dLine(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::RGBA Acolour, const Maths::RGBA Bcolour) OVERRIDE;
	void    Draw3dLines( const Maths::HVec3* startArray, const Maths::HVec3* endArray, const U32 count, const Maths::RGBA colour) OVERRIDE;

	void    Draw3dTriangleOutline(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour)  OVERRIDE;
	void    Draw3dTriangles( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)  OVERRIDE;
	void    Draw3dTriangleLit(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour) OVERRIDE;
	void    Draw3dTrianglesLit( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count) OVERRIDE;

	void    Draw3dPoint(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour)  OVERRIDE;
	void    Draw3dCross(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dStar(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour) OVERRIDE;

	void    Draw3dPlanePoint(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dPlaneCross(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dPlaneStar(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dPlane(const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA colour) OVERRIDE;

	void    Draw3dArrow(const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 lengthScale, const F32 arrowheadSize, const Maths::RGBA colour) OVERRIDE;

	void    Draw3dCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA colour) OVERRIDE;

	void    Draw3dSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA colour) OVERRIDE;

	void    Draw3dSphereOutline(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dSphereOutline(const Maths::C_Sphere& sphere, const Maths::RGBA colour) OVERRIDE;

	void    Draw3dMatrix(const Maths::HMat43& m, const Maths::HF32& size = 1.0f ) OVERRIDE;

	void    Draw3dBox(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dBox(const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dBoxLit(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour) OVERRIDE;
	void    Draw3dBoxLit(const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour) OVERRIDE;

	void    Draw3dGrid(const Maths::HMat43& m) OVERRIDE;

	void	Draw3dText(const Maths::HVec3& pos, const Util::C_String text) OVERRIDE;
	void	Draw3dText(const Maths::HVec3& pos, const F32 size, const Util::C_String text) OVERRIDE;

	void	SetLightDirection(const Maths::HVec3&) OVERRIDE;
	void	SetBlendMode(const E_BlendMode) OVERRIDE;
	void	SetDepthEnable(const bool depthEnable) OVERRIDE;
};


}; // namespace DbUtil

#endif // DbUtil
