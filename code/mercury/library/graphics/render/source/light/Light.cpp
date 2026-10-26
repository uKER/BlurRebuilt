#include "stdafx.h"
#include "Render/Light/Light.h"

Debug::Bool		g_debugLightUpdate			("Graphics.Render.Lights.DebugUpdate", 			true);
Debug::Bool		g_debugLightRender			("Graphics.Render.Lights.DebugRender", 			true);

Debug::Bool		g_debugLight0_Show			("Graphics.Render.Lights.Light0.Show",			false);
Debug::Bool		g_debugLight0_Enabled		("Graphics.Render.Lights.Light0.Enabled",		false);
Debug::Float	g_debugLight0_R				("Graphics.Render.Lights.Light0.R",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight0_G				("Graphics.Render.Lights.Light0.G",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight0_B				("Graphics.Render.Lights.Light0.B",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight0_Intensity		("Graphics.Render.Lights.Light0.Intensity",		1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight0_X				("Graphics.Render.Lights.Light0.X",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight0_Y				("Graphics.Render.Lights.Light0.Y",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight0_Z				("Graphics.Render.Lights.Light0.Z",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight0_Radius		("Graphics.Render.Lights.Light0.Radius",			1.f,	Debug::Range(0.f, 50.f));

Debug::Bool		g_debugLight1_Show			("Graphics.Render.Lights.Light1.Show",			false);
Debug::Bool		g_debugLight1_Enabled		("Graphics.Render.Lights.Light1.Enabled",		false);
Debug::Float	g_debugLight1_R				("Graphics.Render.Lights.Light1.R",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight1_G				("Graphics.Render.Lights.Light1.G",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight1_B				("Graphics.Render.Lights.Light1.B",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight1_Intensity		("Graphics.Render.Lights.Light1.Intensity",		1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight1_X				("Graphics.Render.Lights.Light1.X",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight1_Y				("Graphics.Render.Lights.Light1.Y",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight1_Z				("Graphics.Render.Lights.Light1.Z",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight1_Radius		("Graphics.Render.Lights.Light1.Radius",			1.f,	Debug::Range(0.f, 50.f));

Debug::Bool		g_debugLight2_Show			("Graphics.Render.Lights.Light2.Show",			false);
Debug::Bool		g_debugLight2_Enabled		("Graphics.Render.Lights.Light2.Enabled",		false);
Debug::Float	g_debugLight2_R				("Graphics.Render.Lights.Light2.R",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight2_G				("Graphics.Render.Lights.Light2.G",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight2_B				("Graphics.Render.Lights.Light2.B",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight2_Intensity		("Graphics.Render.Lights.Light2.Intensity",		1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight2_X				("Graphics.Render.Lights.Light2.X",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight2_Y				("Graphics.Render.Lights.Light2.Y",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight2_Z				("Graphics.Render.Lights.Light2.Z",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight2_Radius		("Graphics.Render.Lights.Light2.Radius",			1.f,	Debug::Range(0.f, 50.f));

Debug::Bool		g_debugLight3_Show			("Graphics.Render.Lights.Light3.Show",			false);
Debug::Bool		g_debugLight3_Enabled		("Graphics.Render.Lights.Light3.Enabled",		false);
Debug::Float	g_debugLight3_R				("Graphics.Render.Lights.Light3.R",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight3_G				("Graphics.Render.Lights.Light3.G",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight3_B				("Graphics.Render.Lights.Light3.B",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight3_Intensity		("Graphics.Render.Lights.Light3.Intensity",		1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight3_X				("Graphics.Render.Lights.Light3.X",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight3_Y				("Graphics.Render.Lights.Light3.Y",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight3_Z				("Graphics.Render.Lights.Light3.Z",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight3_Radius		("Graphics.Render.Lights.Light3.Radius",			1.f,	Debug::Range(0.f, 50.f));

Debug::Bool		g_debugLight4_Show			("Graphics.Render.Lights.Light4.Show",			false);
Debug::Bool		g_debugLight4_Enabled		("Graphics.Render.Lights.Light4.Enabled",		false);
Debug::Float	g_debugLight4_R				("Graphics.Render.Lights.Light4.R",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight4_G				("Graphics.Render.Lights.Light4.G",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight4_B				("Graphics.Render.Lights.Light4.B",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight4_Intensity		("Graphics.Render.Lights.Light4.Intensity",		1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight4_X				("Graphics.Render.Lights.Light4.X",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight4_Y				("Graphics.Render.Lights.Light4.Y",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight4_Z				("Graphics.Render.Lights.Light4.Z",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight4_Radius		("Graphics.Render.Lights.Light4.Radius",			1.f,	Debug::Range(0.f, 50.f));

Debug::Bool		g_debugLight5_Show			("Graphics.Render.Lights.Light5.Show",			false);
Debug::Bool		g_debugLight5_Enabled		("Graphics.Render.Lights.Light5.Enabled",		false);
Debug::Float	g_debugLight5_R				("Graphics.Render.Lights.Light5.R",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight5_G				("Graphics.Render.Lights.Light5.G",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight5_B				("Graphics.Render.Lights.Light5.B",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight5_Intensity		("Graphics.Render.Lights.Light5.Intensity",		1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight5_X				("Graphics.Render.Lights.Light5.X",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight5_Y				("Graphics.Render.Lights.Light5.Y",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight5_Z				("Graphics.Render.Lights.Light5.Z",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight5_Radius		("Graphics.Render.Lights.Light5.Radius",			1.f,	Debug::Range(0.f, 50.f));

Debug::Bool		g_debugLight6_Show			("Graphics.Render.Lights.Light6.Show",			false);
Debug::Bool		g_debugLight6_Enabled		("Graphics.Render.Lights.Light6.Enabled",		false);
Debug::Float	g_debugLight6_R				("Graphics.Render.Lights.Light6.R",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight6_G				("Graphics.Render.Lights.Light6.G",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight6_B				("Graphics.Render.Lights.Light6.B",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight6_Intensity		("Graphics.Render.Lights.Light6.Intensity",		1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight6_X				("Graphics.Render.Lights.Light6.X",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight6_Y				("Graphics.Render.Lights.Light6.Y",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight6_Z				("Graphics.Render.Lights.Light6.Z",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight6_Radius		("Graphics.Render.Lights.Light6.Radius",			1.f,	Debug::Range(0.f, 50.f));

Debug::Bool		g_debugLight7_Show			("Graphics.Render.Lights.Light7.Show",			false);
Debug::Bool		g_debugLight7_Enabled		("Graphics.Render.Lights.Light7.Enabled",		false);
Debug::Float	g_debugLight7_R				("Graphics.Render.Lights.Light7.R",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight7_G				("Graphics.Render.Lights.Light7.G",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight7_B				("Graphics.Render.Lights.Light7.B",				1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight7_Intensity		("Graphics.Render.Lights.Light7.Intensity",		1.f,	Debug::Range(0.f, 2.f));
Debug::Float	g_debugLight7_X				("Graphics.Render.Lights.Light7.X",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight7_Y				("Graphics.Render.Lights.Light7.Y",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight7_Z				("Graphics.Render.Lights.Light7.Z",				0.f,	Debug::Range(-100.f, 100.f));
Debug::Float	g_debugLight7_Radius		("Graphics.Render.Lights.Light7.Radius",			1.f,	Debug::Range(0.f, 50.f));

namespace Render
{
namespace Light
{

// C_Light

C_Light::C_Light() :
	m_Enabled(false),
	m_Type(E_LightType_Spot),
	m_Radius(1.f),
	m_Intensity(1.f),
	m_Colour(1.f, 1.f, 1.f)
{
}

C_Light::~C_Light()
{
}

void C_Light::Initialise(const Maths::HVec3 PCREF position, const Maths::HVec3 PCREF direction)
{
	m_Type = E_LightType_Spot;
	m_Position = position;
	m_Direction = direction;
}

void C_Light::Initialise(const Maths::HVec3 PCREF position, const F32 radius)
{
	m_Type = E_LightType_Point;
	m_Position = position;
	m_Radius = radius;
}

void C_Light::Initialise(const Maths::HVec3 PCREF direction)
{
	m_Type = E_LightType_Directional;
	m_Direction = direction;
}

void C_Light::DebugUpdate()
{
}

void C_Light::DebugRender()
{
	const Maths::RGBA colour(m_Colour.X(), m_Colour.Y(), m_Colour.Z());

	switch(m_Type)
	{
	case E_LightType_Spot:
		Display::BasicRender()->DrawColoured3DSphere(m_Position, m_Radius, colour);
		Display::BasicRender()->DrawColoured3DLine(m_Position, m_Position+m_Direction, colour);
		break;
	case E_LightType_Point:
		Display::BasicRender()->DrawColoured3DSphere(m_Position, m_Radius, colour);
		break;
	case E_LightType_Directional:
		Display::BasicRender()->DrawColoured3DLine(m_Position, m_Position+m_Direction, colour);
		break;
	}
}

// C_LightManager

C_LightManager C_LightManager::s_Instance;

C_LightManager::C_LightManager()
{
}

C_LightManager::~C_LightManager()
{
	Clear();
}

void C_LightManager::Clear()
{
	m_LightList.Clear();
}

void C_LightManager::SetLightCount(const S32 count, const E_LightType type)
{
	m_LightList.Resize(count);
	const S32 lightCount = m_LightList.Size();
	for(S32 lightIndex=0; lightIndex<lightCount; lightIndex++)
	{
		C_Light& light = m_LightList[lightIndex];
		light.SetType(type);
	}
}

const S32 C_LightManager::GetDisabledLightIndex()
{
	const S32 lightCount = m_LightList.Size();
	for(S32 lightIndex=0; lightIndex<lightCount; lightIndex++)
	{
		C_Light& light = m_LightList[lightIndex];

		if(!light.GetEnabled())
		{
			return lightIndex;
		}
	}
	return -1;
}

void C_LightManager::DebugUpdate()
{
	if(!g_debugLightUpdate)
		return;

	const S32 lightCount = m_LightList.Size();
	for(S32 lightIndex=0; lightIndex<lightCount; lightIndex++)
	{
		C_Light& light = m_LightList[lightIndex];
		
		switch(lightIndex)
		{
		case 0:
			light.SetEnabled(g_debugLight0_Enabled);
			light.SetColour(Maths::HVec3((F32)g_debugLight0_R, (F32)g_debugLight0_G, (F32)g_debugLight0_B));
			light.SetIntensity(g_debugLight0_Intensity);
			light.SetPosition(Maths::HVec3((F32)g_debugLight0_X, (F32)g_debugLight0_Y, (F32)g_debugLight0_Z));
			light.SetRadius(g_debugLight0_Radius);
			break;
		case 1:
			light.SetEnabled(g_debugLight1_Enabled);
			light.SetColour(Maths::HVec3((F32)g_debugLight1_R, (F32)g_debugLight1_G, (F32)g_debugLight1_B));
			light.SetIntensity(g_debugLight1_Intensity);
			light.SetPosition(Maths::HVec3((F32)g_debugLight1_X, (F32)g_debugLight1_Y, (F32)g_debugLight1_Z));
			light.SetRadius(g_debugLight1_Radius);
			break;
		case 2:
			light.SetEnabled(g_debugLight2_Enabled);
			light.SetColour(Maths::HVec3((F32)g_debugLight2_R, (F32)g_debugLight2_G, (F32)g_debugLight2_B));
			light.SetIntensity(g_debugLight2_Intensity);
			light.SetPosition(Maths::HVec3((F32)g_debugLight2_X, (F32)g_debugLight2_Y, (F32)g_debugLight2_Z));
			light.SetRadius(g_debugLight2_Radius);
			break;
		case 3:
			light.SetEnabled(g_debugLight3_Enabled);
			light.SetColour(Maths::HVec3((F32)g_debugLight3_R, (F32)g_debugLight3_G, (F32)g_debugLight3_B));
			light.SetIntensity(g_debugLight3_Intensity);
			light.SetPosition(Maths::HVec3((F32)g_debugLight3_X, (F32)g_debugLight3_Y, (F32)g_debugLight3_Z));
			light.SetRadius(g_debugLight3_Radius);
			break;
		case 4:
			light.SetEnabled(g_debugLight4_Enabled);
			light.SetColour(Maths::HVec3((F32)g_debugLight4_R, (F32)g_debugLight4_G, (F32)g_debugLight4_B));
			light.SetIntensity(g_debugLight4_Intensity);
			light.SetPosition(Maths::HVec3((F32)g_debugLight4_X, (F32)g_debugLight4_Y, (F32)g_debugLight4_Z));
			light.SetRadius(g_debugLight4_Radius);
			break;
		case 5:
			light.SetEnabled(g_debugLight5_Enabled);
			light.SetColour(Maths::HVec3((F32)g_debugLight5_R, (F32)g_debugLight5_G, (F32)g_debugLight5_B));
			light.SetIntensity(g_debugLight5_Intensity);
			light.SetPosition(Maths::HVec3((F32)g_debugLight5_X, (F32)g_debugLight5_Y, (F32)g_debugLight5_Z));
			light.SetRadius(g_debugLight5_Radius);
			break;
		case 6:
			light.SetEnabled(g_debugLight6_Enabled);
			light.SetColour(Maths::HVec3((F32)g_debugLight6_R, (F32)g_debugLight6_G, (F32)g_debugLight6_B));
			light.SetIntensity(g_debugLight6_Intensity);
			light.SetPosition(Maths::HVec3((F32)g_debugLight6_X, (F32)g_debugLight6_Y, (F32)g_debugLight6_Z));
			light.SetRadius(g_debugLight6_Radius);
			break;
		case 7:
			light.SetEnabled(g_debugLight7_Enabled);
			light.SetColour(Maths::HVec3((F32)g_debugLight7_R, (F32)g_debugLight7_G, (F32)g_debugLight7_B));
			light.SetIntensity(g_debugLight7_Intensity);
			light.SetPosition(Maths::HVec3((F32)g_debugLight7_X, (F32)g_debugLight7_Y, (F32)g_debugLight7_Z));
			light.SetRadius(g_debugLight7_Radius);
			break;
		}
	}
}

void C_LightManager::DebugRender()
{
	if(!g_debugLightRender)
		return;

	Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

	const S32 lightCount = m_LightList.Size();
	for(S32 lightIndex=0; lightIndex<lightCount; lightIndex++)
	{
		C_Light& light = m_LightList[lightIndex];

		if(!light.GetEnabled())
			continue;

		switch(lightIndex)
		{
		case 0:
			if(!g_debugLight0_Show)
				continue;
			break;
		case 1:
			if(!g_debugLight1_Show)
				continue;
			break;
		case 2:
			if(!g_debugLight2_Show)
				continue;
			break;
		case 3:
			if(!g_debugLight3_Show)
				continue;
			break;
		case 4:
			if(!g_debugLight4_Show)
				continue;
			break;
		case 5:
			if(!g_debugLight5_Show)
				continue;
			break;
		case 6:
			if(!g_debugLight6_Show)
				continue;
			break;
		case 7:
			if(!g_debugLight7_Show) 
				continue;
			break;
		}
		
		light.DebugRender();
	}

	Display::Device()->SetFillMode(Display::E_FillMode_Default);
}

} // namespace Light
} // namespace Render
