#include "SystemPch.h"

#include <system/debug/debugrender/DebugRenderItem.h>
#include <system/debug/debugrender/DebugRenderItem.inl>

namespace DbUtil
{

// C_DebugRenderItem

C_DebugRenderItem::C_DebugRenderItem()
	: m_Type(E_DebugRenderItemType_None)	
{
}

C_DebugRenderItem::C_DebugRenderItem( E_DebugRenderItemType type )
	: m_Type( type )
{
	switch(type)
	{
		case E_DebugRenderItemType_2dText: m_Data.m_Text2dText = NULL; break;
		case E_DebugRenderItemType_3dText: m_Data.m_Text3dText = NULL; break;
	}
}

C_DebugRenderItem& C_DebugRenderItem::operator = ( const C_DebugRenderItem& other )
{
	m_Type = other.m_Type;
	m_Data = other.m_Data;
	switch(m_Type)
	{
		case E_DebugRenderItemType_2dText:
		{
			if(other.m_Data.m_Text2dText != NULL)
			{				
				m_Data.m_Text2dText = new char[ strlen(other.m_Data.m_Text2dText) + 1 ]; 
				strcpy( m_Data.m_Text2dText, other.m_Data.m_Text2dText );
			}
		}
		break;
		case E_DebugRenderItemType_3dText:
		{
			if(other.m_Data.m_Text3dText != NULL)
			{
				m_Data.m_Text3dText = new char[ strlen(other.m_Data.m_Text3dText) + 1 ]; 
				strcpy( m_Data.m_Text3dText, other.m_Data.m_Text3dText );
			}
		}
		break;
	}

	return *this;
}

C_DebugRenderItem::~C_DebugRenderItem()
{
	switch(m_Type)
	{
		case E_DebugRenderItemType_2dText:
		{
			SAFE_DELETE_ARY( m_Data.m_Text2dText );
		}
		break;
		case E_DebugRenderItemType_3dText: 
		{
			SAFE_DELETE_ARY( m_Data.m_Text3dText );
		}
		break;
	}
}

void C_DebugRenderItem::DebugRender( I_DebugRender* pDebugRender, Maths::HMat43& mWorld ) const
{
	switch( m_Type )
	{
		default:
		case E_DebugRenderItemType_None:
			break;

		case E_DebugRenderItemType_3dLine:
			pDebugRender->Draw3dLine( Load(m_Data.m_Line3dStart), Load(m_Data.m_Line3dEnd), Load(m_Data.m_Line3dColourA), Load(m_Data.m_Line3dColourB) ); 
			break;

		case E_DebugRenderItemType_3dTriangle:
			pDebugRender->Draw3dTriangle( Load(m_Data.m_Triangle3dPosA), Load(m_Data.m_Triangle3dPosB), Load(m_Data.m_Triangle3dPosC), Load(m_Data.m_Triangle3dColourA), Load(m_Data.m_Triangle3dColourB), Load(m_Data.m_Triangle3dColourC) ); 
			break;

		case E_DebugRenderItemType_3dTriangleLit:
			pDebugRender->Draw3dTriangleLit( Load(m_Data.m_Triangle3dPosA), Load(m_Data.m_Triangle3dPosB), Load(m_Data.m_Triangle3dPosC), Load(m_Data.m_Triangle3dColourA), Load(m_Data.m_Triangle3dColourB), Load(m_Data.m_Triangle3dColourC) ); 
			break;

		case E_DebugRenderItemType_3dTriangleOutline:
			pDebugRender->Draw3dTriangleOutline( Load(m_Data.m_Triangle3dPosA), Load(m_Data.m_Triangle3dPosB), Load(m_Data.m_Triangle3dPosC), Load(m_Data.m_Triangle3dColourA) ); 
			break;

		case E_DebugRenderItemType_3dPoint:
			pDebugRender->Draw3dPoint( Load(m_Data.m_Point3dPos), m_Data.m_Point3dSize, Load(m_Data.m_Point3dColour) );                    
			break;

		case E_DebugRenderItemType_3dCross:
			pDebugRender->Draw3dCross( Load(m_Data.m_Point3dPos), m_Data.m_Point3dSize, Load(m_Data.m_Point3dColour) );                    
			break;

		case E_DebugRenderItemType_3dStar:
			pDebugRender->Draw3dStar( Load(m_Data.m_Point3dPos), m_Data.m_Point3dSize, Load(m_Data.m_Point3dColour) );                     
			break;

		case E_DebugRenderItemType_3dPlanePoint:
			pDebugRender->Draw3dPlanePoint( Load(m_Data.m_PlanePoint3dPos), Load(m_Data.m_PlanePoint3dNormal), m_Data.m_PlanePoint3dSize, Load(m_Data.m_PlanePoint3dColour) ); 
			break;

		case E_DebugRenderItemType_3dPlaneCross:
			pDebugRender->Draw3dPlaneCross( Load(m_Data.m_PlanePoint3dPos), Load(m_Data.m_PlanePoint3dNormal), m_Data.m_PlanePoint3dSize, Load(m_Data.m_PlanePoint3dColour) ); 
			break;

		case E_DebugRenderItemType_3dPlaneStar:
			pDebugRender->Draw3dPlaneStar( Load(m_Data.m_PlanePoint3dPos), Load(m_Data.m_PlanePoint3dNormal), m_Data.m_PlanePoint3dSize, Load(m_Data.m_PlanePoint3dColour) ); 
			break;

		case E_DebugRenderItemType_3dPlane:
			pDebugRender->Draw3dPlane( Maths::C_Plane(Load(m_Data.m_Plane3dPlane)), Load(m_Data.m_Plane3dRefPoint), m_Data.m_Plane3dSize, Load(m_Data.m_Plane3dColour) ); 
			break;

		case E_DebugRenderItemType_3dArrow:
			pDebugRender->Draw3dArrow( Load(m_Data.m_Arrow3dPos), Load(m_Data.m_Arrow3dDir), m_Data.m_Arrow3dLengthScale, m_Data.m_Arrow3dArrowheadSize, Load(m_Data.m_Arrow3dColour) );   
			break;

		case E_DebugRenderItemType_3dCircle:
			pDebugRender->Draw3dCircle( Load(m_Data.m_Circle3dCentre), Load(m_Data.m_Circle3dNormal), m_Data.m_Circle3dRadius, Load(m_Data.m_Circle3dColour) ); 
			break;

		case E_DebugRenderItemType_3dSphere:
			pDebugRender->Draw3dSphere( Load(m_Data.m_Sphere3dCentre), m_Data.m_Sphere3dRadius, Load(m_Data.m_Sphere3dColour) );           
			break;

		case E_DebugRenderItemType_3dSphereOutline:
			pDebugRender->Draw3dSphereOutline( Load(m_Data.m_Sphere3dCentre), m_Data.m_Sphere3dRadius, Load(m_Data.m_Sphere3dColour) );    
			break;

		case E_DebugRenderItemType_3dMatrix:
			pDebugRender->Draw3dMatrix( Load(m_Data.m_Matrix3d), m_Data.m_Matrix3dSize ); 
			break;

		case E_DebugRenderItemType_3dBox:
			pDebugRender->Draw3dBox( Load(m_Data.m_Box3dMin), Load(m_Data.m_Box3dMax), Load(m_Data.m_Box3dMatrix), Load(m_Data.m_Box3dColour) );  
			break;

		case E_DebugRenderItemType_3dBoxLit:
			pDebugRender->Draw3dBoxLit( Load(m_Data.m_Box3dMin), Load(m_Data.m_Box3dMax), Load(m_Data.m_Box3dMatrix), Load(m_Data.m_Box3dColour) );  
			break;

		case E_DebugRenderItemType_3dGrid:
			pDebugRender->Draw3dGrid( Load(m_Data.m_Matrix3d) ); 
			break;

		case E_DebugRenderItemType_2dLine:
			pDebugRender->Draw2dLine( Load(m_Data.m_Line2dStart), Load(m_Data.m_Line2dEnd), Load(m_Data.m_Line2dColour) ); 
			break;

		case E_DebugRenderItemType_2dCircle:
			pDebugRender->Draw2dCircle( Load(m_Data.m_Circle2dCentre), Load(m_Data.m_Circle2dSize), Load(m_Data.m_Circle2dColour) );   
			break;

		case E_DebugRenderItemType_2dArrow:
			pDebugRender->Draw2dArrow( Load(m_Data.m_Arrow2dPos), Load(m_Data.m_Arrow2dDir), m_Data.m_Arrow2dArrowheadSize, Load(m_Data.m_Arrow2dColour) ); 
			break;

		case E_DebugRenderItemType_2dBar:
			pDebugRender->Draw2dBar( Load(m_Data.m_Bar2dPos), Load(m_Data.m_Bar2dSize), Load(m_Data.m_Bar2dColour) ); 
			break;

		case E_DebugRenderItemType_2dTriangle:
			pDebugRender->Draw2dTriangle( Load(m_Data.m_Triangle2dPosA), Load(m_Data.m_Triangle2dPosB), Load(m_Data.m_Triangle2dPosC), Load(m_Data.m_Triangle2dColour) ); 
			break;

		case E_DebugRenderItemType_2dText:
			pDebugRender->Draw2dText( Load(m_Data.m_Text2dPos), m_Data.m_Text2dText, Load(m_Data.m_Text2dColour), m_Data.m_Text2dHeight ); 
			break;

		case E_DebugRenderItemType_LightDirection:
			pDebugRender->SetLightDirection( Load(m_Data.m_LightDirection) ); 
			break;

		case E_DebugRenderItemType_DepthEnable:
			pDebugRender->SetDepthEnable( m_Data.m_DepthEnable!=0 ); 
			break;

		case E_DebugRenderItemType_BlendMode:
			pDebugRender->SetBlendMode( (DbUtil::E_BlendMode)m_Data.m_BlendMode ); 
			break;

		case E_DebugRenderItemType_3dText:              
			if(m_Data.m_Text3dSize > 0.0f)
			{
				pDebugRender->Draw3dText( Load(m_Data.m_Text3dPos), m_Data.m_Text3dSize, m_Data.m_Text3dText ); 
			}
			else
			{
				pDebugRender->Draw3dText( Load(m_Data.m_Text3dPos), m_Data.m_Text3dText ); 
			}
			break;
	}
}

// C_ItemDebugRenderer

C_ItemDebugRenderer::C_ItemDebugRenderer()
{

}

C_ItemDebugRenderer::~C_ItemDebugRenderer()
{

}

void C_ItemDebugRenderer::Draw2dLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_2dLine);

	item.m_Data.m_Line2dStart			= Store(start);
	item.m_Data.m_Line2dEnd				= Store(end);
	item.m_Data.m_Line2dColour			= Store(colour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw2dCircle(const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_2dCircle);

	item.m_Data.m_Circle2dCentre		= Store(centre);
	item.m_Data.m_Circle2dSize			= Store(size);
	item.m_Data.m_Circle2dColour		= Store(colour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw2dArrow(const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_2dArrow);

	item.m_Data.m_Arrow2dPos			= Store(start);
	item.m_Data.m_Arrow2dDir			= Store(dir);
	item.m_Data.m_Arrow2dArrowheadSize	= Store(arrowheadSize);
	item.m_Data.m_Arrow2dColour			= Store(colour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw2dBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_2dBar);

	item.m_Data.m_Bar2dPos				= Store(pos);
	item.m_Data.m_Bar2dSize				= Store(size);
	item.m_Data.m_Bar2dColour			= Store(colour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw2dTriangle(const Maths::HVec2& A, const Maths::HVec2& B, const Maths::HVec2& C, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_2dTriangle);

	item.m_Data.m_Triangle2dPosA		= Store(A);
	item.m_Data.m_Triangle2dPosB		= Store(B);
	item.m_Data.m_Triangle2dPosC		= Store(C);
	item.m_Data.m_Triangle2dColour		= Store(colour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dLine(const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dLine);

	item.m_Data.m_Line3dStart			= Store(start);
	item.m_Data.m_Line3dEnd				= Store(end);
	item.m_Data.m_Line3dColourA			= Store(colour);
	item.m_Data.m_Line3dColourB			= Store(colour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dLine(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::RGBA Acolour, const Maths::RGBA Bcolour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dLine);

	item.m_Data.m_Line3dStart			= Store(A);
	item.m_Data.m_Line3dEnd				= Store(B);
	item.m_Data.m_Line3dColourA			= Store(Acolour);
	item.m_Data.m_Line3dColourB			= Store(Bcolour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dTriangleOutline(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dTriangleOutline);

	item.m_Data.m_Triangle3dPosA		= Store(A);
	item.m_Data.m_Triangle3dPosB		= Store(B);
	item.m_Data.m_Triangle3dPosC		= Store(C);
	item.m_Data.m_Triangle3dColourA		= Store(colour);
	item.m_Data.m_Triangle3dColourB		= Store(colour);
	item.m_Data.m_Triangle3dColourC		= Store(colour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dTriangle);

	item.m_Data.m_Triangle3dPosA		= Store(A);
	item.m_Data.m_Triangle3dPosB		= Store(B);
	item.m_Data.m_Triangle3dPosC		= Store(C);
	item.m_Data.m_Triangle3dColourA		= Store(colour);
	item.m_Data.m_Triangle3dColourB		= Store(colour);
	item.m_Data.m_Triangle3dColourC		= Store(colour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dTriangle);

	item.m_Data.m_Triangle3dPosA		= Store(A);
	item.m_Data.m_Triangle3dPosB		= Store(B);
	item.m_Data.m_Triangle3dPosC		= Store(C);
	item.m_Data.m_Triangle3dColourA		= Store(Acolour);
	item.m_Data.m_Triangle3dColourB		= Store(Bcolour);
	item.m_Data.m_Triangle3dColourC		= Store(Ccolour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dTriangleLit(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dTriangleLit);

	item.m_Data.m_Triangle3dPosA		= Store(A);
	item.m_Data.m_Triangle3dPosB		= Store(B);
	item.m_Data.m_Triangle3dPosC		= Store(C);
	item.m_Data.m_Triangle3dColourA		= Store(Acolour);
	item.m_Data.m_Triangle3dColourB		= Store(Bcolour);
	item.m_Data.m_Triangle3dColourC		= Store(Ccolour);

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dPoint(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dPoint);

	item.m_Data.m_Point3dPos			= Store(pos);
	item.m_Data.m_Point3dSize			= Store(size);
	item.m_Data.m_Point3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dCross(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dCross);

	item.m_Data.m_Point3dPos			= Store(pos);
	item.m_Data.m_Point3dSize			= Store(size);
	item.m_Data.m_Point3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dStar(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dStar);

	item.m_Data.m_Point3dPos			= Store(pos);
	item.m_Data.m_Point3dSize			= Store(size);
	item.m_Data.m_Point3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dPlanePoint(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dPlanePoint);

	item.m_Data.m_PlanePoint3dPos		= Store(pos);
	item.m_Data.m_PlanePoint3dNormal	= Store(normal);
	item.m_Data.m_PlanePoint3dSize		= Store(size);
	item.m_Data.m_PlanePoint3dColour	= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dPlaneCross(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dPlaneCross);

	item.m_Data.m_PlanePoint3dPos		= Store(pos);
	item.m_Data.m_PlanePoint3dNormal	= Store(normal);
	item.m_Data.m_PlanePoint3dSize		= Store(size);
	item.m_Data.m_PlanePoint3dColour	= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dPlaneStar(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dPlaneStar);

	item.m_Data.m_PlanePoint3dPos		= Store(pos);
	item.m_Data.m_PlanePoint3dNormal	= Store(normal);
	item.m_Data.m_PlanePoint3dSize		= Store(size);
	item.m_Data.m_PlanePoint3dColour	= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dPlane(const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dPlane);

	item.m_Data.m_Plane3dPlane			= Store((Maths::HVec4)plane);
	item.m_Data.m_Plane3dRefPoint		= Store((Maths::HVec3)referencePoint);
	item.m_Data.m_Plane3dSize			= Store(size);
	item.m_Data.m_Plane3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dArrow(const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 lengthScale, const F32 arrowheadSize, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dArrow);

	item.m_Data.m_Arrow3dPos			= Store(pos);
	item.m_Data.m_Arrow3dDir			= Store(dir);
	item.m_Data.m_Arrow3dLengthScale	= Store(lengthScale);
	item.m_Data.m_Arrow3dArrowheadSize	= Store(arrowheadSize);
	item.m_Data.m_Arrow3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dCircle);

	item.m_Data.m_Circle3dCentre		= Store(centre);
	item.m_Data.m_Circle3dNormal		= Store(normal);
	item.m_Data.m_Circle3dRadius		= Store(radius);
	item.m_Data.m_Circle3dColour		= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dSphere);

	item.m_Data.m_Sphere3dCentre		= Store(centre);
	item.m_Data.m_Sphere3dRadius		= Store(radius);
	item.m_Data.m_Sphere3dColour		= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dSphere);

	item.m_Data.m_Sphere3dCentre		= Store((Maths::HVec3)sphere.Centre());
	item.m_Data.m_Sphere3dRadius		= Store(sphere.Radius());
	item.m_Data.m_Sphere3dColour		= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dSphereOutline(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dSphereOutline);
	
	item.m_Data.m_Sphere3dCentre		= Store(centre);
	item.m_Data.m_Sphere3dRadius		= Store(radius);
	item.m_Data.m_Sphere3dColour		= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dSphereOutline(const Maths::C_Sphere& sphere, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dSphereOutline);

	item.m_Data.m_Sphere3dCentre		= Store((Maths::HVec3)sphere.Centre());
	item.m_Data.m_Sphere3dRadius		= Store(sphere.Radius());
	item.m_Data.m_Sphere3dColour		= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dMatrix(const Maths::HMat43& m, const Maths::HF32& size )
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dMatrix);
	
	item.m_Data.m_Matrix3d				= Store(m);
	item.m_Data.m_Matrix3dSize			= Store(size);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dBox(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dBox);

	item.m_Data.m_Box3dMin				= Store(min);
	item.m_Data.m_Box3dMax				= Store(max);
	item.m_Data.m_Box3dMatrix			= Store(m);
	item.m_Data.m_Box3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dBox(const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dBox);

	item.m_Data.m_Box3dMin				= Store(box.Min());
	item.m_Data.m_Box3dMax				= Store(box.Max());
	item.m_Data.m_Box3dMatrix			= Store(m);
	item.m_Data.m_Box3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dBoxLit(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dBoxLit);

	item.m_Data.m_Box3dMin				= Store(min);
	item.m_Data.m_Box3dMax				= Store(max);
	item.m_Data.m_Box3dMatrix			= Store(m);
	item.m_Data.m_Box3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dBoxLit(const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dBoxLit);

	item.m_Data.m_Box3dMin				= Store(box.Min());
	item.m_Data.m_Box3dMax				= Store(box.Max());
	item.m_Data.m_Box3dMatrix			= Store(m);
	item.m_Data.m_Box3dColour			= Store(colour);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dGrid(const Maths::HMat43& m)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dGrid);
	
	item.m_Data.m_Matrix3d		= Store(m);

	OnRender(&item);

}

void C_ItemDebugRenderer::Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA colour)
{
	Draw3dSphere( sphere * transform, colour );
}

void C_ItemDebugRenderer::Draw3dLines( const Maths::HVec3* startArray, const Maths::HVec3* endArray, const U32 count, const Maths::RGBA colour)
{
	for ( U32 i=0; i<count; i++ )
	{
		Draw3dLine( startArray[i], endArray[i], colour, colour );
	}
}

void C_ItemDebugRenderer::Draw3dTriangles( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)
{
	for(U32 i=0; i<count; i++)
	{
		Draw3dTriangle( AArray[i], BArray[i], CArray[i], colourArray[i]);
	}
}

void C_ItemDebugRenderer::Draw3dTrianglesLit( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)
{
	for(U32 i=0; i<count; i++)
	{
		Draw3dTriangleLit( AArray[i], BArray[i], CArray[i], colourArray[i], colourArray[i], colourArray[i]);
	}
}

void C_ItemDebugRenderer::Draw2dText(const Maths::HVec2& pos, const Util::C_String& text)
{
	C_DebugRenderItem item(E_DebugRenderItemType_2dText);

	item.m_Data.m_Text2dPos = Store( pos );

	item.m_Data.m_Text2dColour = Store( Maths::RGBA(255, 255, 255, 255) );
	item.m_Data.m_Text2dHeight = 32.0f;

	item.m_Data.m_Text2dText = new char[text.Length()+1];
	strcpy(item.m_Data.m_Text2dText, text.Chars());

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw2dText(const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags)
{
	C_DebugRenderItem item(E_DebugRenderItemType_2dText);

	item.m_Data.m_Text2dPos = Store( pos );

	item.m_Data.m_Text2dColour = Store( colour );
	item.m_Data.m_Text2dHeight = height;

	item.m_Data.m_Text2dText = new char[text.Length()+1];
	strcpy(item.m_Data.m_Text2dText, text.Chars());

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dText(const Maths::HVec3& pos, const Util::C_String text)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dText);

	item.m_Data.m_Text3dPos = Store(pos);
	item.m_Data.m_Text3dSize = Store(0.0f);
	
	item.m_Data.m_Text3dText = new char[text.Length()+1];
	strcpy(item.m_Data.m_Text3dText, text.Chars());

	OnRender(&item);
}

void C_ItemDebugRenderer::Draw3dText(const Maths::HVec3& pos, const F32 size, const Util::C_String text)
{
	C_DebugRenderItem item(E_DebugRenderItemType_3dText);

	item.m_Data.m_Text3dPos = Store(pos);
	item.m_Data.m_Text3dSize = Store(size);

	item.m_Data.m_Text3dText = new char[text.Length()+1];
	strcpy(item.m_Data.m_Text3dText, text.Chars());

	OnRender(&item);
}

void C_ItemDebugRenderer::SetLightDirection(const Maths::HVec3& lightDir)
{
	C_DebugRenderItem item(E_DebugRenderItemType_LightDirection);

	item.m_Data.m_LightDirection = Store( lightDir );

	OnRender(&item);
}

void C_ItemDebugRenderer::SetBlendMode(const E_BlendMode blendMode)
{
	C_DebugRenderItem item(E_DebugRenderItemType_BlendMode);

	item.m_Data.m_BlendMode = blendMode;

	OnRender(&item);
}

void C_ItemDebugRenderer::SetDepthEnable(const bool depthEnable)
{
	C_DebugRenderItem item(E_DebugRenderItemType_DepthEnable);

	item.m_Data.m_DepthEnable = depthEnable;

	OnRender(&item);
}


} // namespace DbUtil
