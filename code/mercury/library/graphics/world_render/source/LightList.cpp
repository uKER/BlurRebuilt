
#include "stdafx.h"

#include <system/serialise/C_FileBlock.h>
#include <world_render/WorldRender.h>
#include <world_render/LightList.h>
#include <system/debug/I_DebugRender.h>

namespace WorldRender
{

Maths::HMat43			C_Light::s_DebugRenderCameraMatrix;

Debug::Bool g_RenderLevelLightInfo( "Graphics.WorldRender.RenderLevelLightInfo", false );
Debug::Bool g_RenderLevelLightExtraInfo( "Graphics.WorldRender.RenderLevelLightExtraInfo", false );
Debug::Float g_RenderLevelLightExtraInfoDistant( "Graphics.WorldRender.RenderLevelLightExtraInfoDistant", 40.f );
Debug::Vector	g_LightInfoGlobalPosition("Graphics.WorldRender.LightInfoGlobalPosition", 1.0f, 0.5f, 2.0f);

enum E_LightListFileVersion
{
	E_LightListFileVersion_Initial = 0,
	E_LightListFileVersion_AddedKdTree = 1,
	E_LightListFileVersion_Current = E_LightListFileVersion_AddedKdTree
};

// C_RenderLevel

C_LightList::C_LightList()
{
	m_HasSunLight = false;
	m_HasSkyLight = false;
}

C_LightList::~C_LightList()
{
}

bool C_LightList::Read( Sys::I_File* pFile )
{
	bool success = true;

	RESOURCE_MEMORY_ID("LightList");

	{
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);

		S32 version;
		ar >> version;
		if (version != E_LightListFileVersion_Current)
		{
			LOGWARN(g_Log, "Bad LightList export version, reexport level needed");
			return true;
		}
		U32 hasSunLightInt;
		ar >> hasSunLightInt;
		if (hasSunLightInt > 0)
		{
			m_HasSunLight = true;
			ar >> m_SunLight;
		}
		U32 hasSkyLightInt;
		ar >> hasSkyLightInt;
		if (hasSkyLightInt > 0)
		{
			m_HasSkyLight = true;
			ar >> m_SkyLight;
		}

		ar >> m_LightList;
	}
	return true;
}

void C_LightList::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m, const Maths::HVec3 PCREF vCamera )
{
	if (g_RenderLevelLightInfo)
	{

		if (m_HasSunLight)
			m_SunLight.DebugRender(pDebugRender, m, vCamera);

		if (m_HasSkyLight)
			m_SkyLight.DebugRender(pDebugRender, m, vCamera);

		for (S32 i = 0; i < m_LightList.Size(); i++)
			m_LightList[i].DebugRender(pDebugRender, m, vCamera);
	}
}	

Maths::HVec3 C_LightList::GetSunDirection() const
{
	if(m_HasSunLight)
	{
		return m_SunLight.m_Matrix.AZ();
	}

	return Maths::HVec3(0.57735f,	0.57735f,	-0.57735f);
}

//////////////////////////////////////////////////////////////////////////
// C_Light
//////////////////////////////////////////////////////////////////////////
void C_Light::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m, const Maths::HVec3 PCREF vCamera)
{ 
	Maths::HVec3 globalLightPos = Maths::HVec3(g_LightInfoGlobalPosition.Value().vx, g_LightInfoGlobalPosition.Value().vy, g_LightInfoGlobalPosition.Value().vz) * s_DebugRenderCameraMatrix;
	F32 globalLightSize = 0.1f;
	F32 pointLightSize = 0.25f;
	F32 lightTextOffset = 0.25f;
	Util::C_String msg = Util::C_String::Format("%f %f %f %f, R: %f",F32(m_Colour.X()), F32(m_Colour.Y()), F32(m_Colour.Z()), F32(m_Colour.W()), F32(m_Radius));	
	
	F32 drawDistanceSquared = g_RenderLevelLightExtraInfoDistant * g_RenderLevelLightExtraInfoDistant;

	switch(m_LightType)
	{
		default:
		case E_LightType_None:
		break;

		case E_LightType_Hemisphere:
		{
			Maths::C_RGBA_F32 colour(m_Colour);
			colour.Saturate();
			pDebugRender->Draw3dSphere( globalLightPos, globalLightSize, colour );
			if (g_RenderLevelLightExtraInfo)
				pDebugRender->Draw3dText(globalLightPos, msg);
		}
		break;

		case E_LightType_Area:
		{
			Maths::C_RGBA_F32 colour(m_Colour * (1.0f / 32.0f) * m_Colour.W());
			colour.Saturate();

			//Maths::HVec3 v[4];
			const Maths::HVec3 v_zero( Maths::ZERO_INIT );
			Maths::HVec3 v[4] = { v_zero, v_zero, v_zero, v_zero };
			v[0] = Maths::HVec3( -1.0f, -1.0f, 0.0f ) * m_Matrix;
			v[1] = Maths::HVec3(  1.0f, -1.0f, 0.0f ) * m_Matrix;
			v[2] = Maths::HVec3(  1.0f,  1.0f, 0.0f ) * m_Matrix;
			v[3] = Maths::HVec3( -1.0f,  1.0f, 0.0f ) * m_Matrix;

			pDebugRender->Draw3dLine( v[0], v[1], colour );
			pDebugRender->Draw3dLine( v[1], v[2], colour );
			pDebugRender->Draw3dLine( v[2], v[3], colour );
			pDebugRender->Draw3dLine( v[3], v[0], colour );

			pDebugRender->Draw3dLine( v[0], v[2], colour );
			pDebugRender->Draw3dLine( v[1], v[3], colour );
			pDebugRender->Draw3dSphere( m_Matrix.AW(), globalLightSize, colour );
			if (g_RenderLevelLightExtraInfo && (m_Matrix.AW().DistanceSquared(vCamera) < drawDistanceSquared ))
				pDebugRender->Draw3dText(m_Matrix.AW(), msg);
		}
		break;

		case E_LightType_Directional:
		{
			Maths::C_RGBA_F32 colour(m_Colour);
			colour.Saturate();
			Maths::HVec3 pos;
			//if(!g_DebugShowLightDirectionalLocations)
			//{
				pos = globalLightPos;
			//}
			//else
			//{
			//	pos = m_LightList[i].m_Matrix.AW();
			//}
			
			pDebugRender->Draw3dArrow( pos + m_Matrix.AZ() * globalLightSize,  m_Matrix.AZ() * -1.0f, globalLightSize * 2.0f, globalLightSize * 0.2f, colour );
			pDebugRender->Draw3dStar( pos + m_Matrix.AZ() * globalLightSize, globalLightSize * 0.1f, colour );
			if (g_RenderLevelLightExtraInfo)
				pDebugRender->Draw3dText(globalLightPos, msg);
		}
		break;

		case E_LightType_Point:
		{
			Maths::C_RGBA_F32 colour(m_Colour);
			colour.Saturate();
			pDebugRender->Draw3dStar( m_Matrix.AW(), pointLightSize, colour );
			//debugRenderPtr->Draw3dSphere( m_LightList[i].m_Matrix.AW(), m_LightList[i].m_PointCutOffRadius, colour );
			if (g_RenderLevelLightExtraInfo && (m_Matrix.AW().DistanceSquared(vCamera) < drawDistanceSquared ))
				pDebugRender->Draw3dText(m_Matrix.AW(), msg);
		}
		break;

		case E_LightType_Spot:
		{
			Maths::C_RGBA_F32 colour(m_Colour);
			colour.Saturate();

			//debugRenderPtr->Draw3dStar( m_LightList[i].m_Matrix.AW(), pointLightSize * 0.5f, colour );
			Maths::HVec3 ax = m_Matrix.AX();
			Maths::HVec3 ay = m_Matrix.AY();
			Maths::HVec3 az = m_Matrix.AX() ^ m_Matrix.AY();
			Maths::HVec3 aw = m_Matrix.AW();
			ax.Normalise();
			ay.Normalise();
			az.Normalise();

			Maths::HMat43 mat43( ax, ay, az, aw );

			//debugRenderPtr->Draw3dArrow( m_LightList[i].m_Matrix.AW(), dir, pointLightSize * 2.0f, pointLightSize * 0.1f, colour );

			{
				F32 dist = 2.0f;
				F32 radius = tanf( m_SpotConeAngle * 0.5f ) * dist;

				pDebugRender->Draw3dCircle(Maths::HVec3(0.0f, 0.0f,  dist) * mat43, az, radius, colour );

				pDebugRender->Draw3dLine(aw, Maths::HVec3(-radius, 0.0f,  dist) * mat43, colour );
				pDebugRender->Draw3dLine(aw, Maths::HVec3( radius, 0.0f,  dist) * mat43, colour );
				pDebugRender->Draw3dLine(aw, Maths::HVec3( 0.0f, -radius, dist) * mat43, colour );
				pDebugRender->Draw3dLine(aw, Maths::HVec3( 0.0f,  radius, dist) * mat43, colour );
			}
			if (g_RenderLevelLightExtraInfo && (m_Matrix.AW().DistanceSquared(vCamera) < drawDistanceSquared ))
				pDebugRender->Draw3dText(aw, msg);
		}
		break;
	}
}
} // namespace WorldRender
