#include "stdafx.h"
#include "Graphics/CutSceneLights.h"

namespace CutScene
{

Debug::Bool		g_debugLightOverrideColour0		("CutScene.Lights.OverrideColour0",		false);
Debug::Bool		g_debugLightOverrideDirection0	("CutScene.Lights.OverrideDirection0",	false);
Debug::Colour	g_debugLightColour0				("CutScene.Lights.Colour0",				0.0f, 0.0f, 0.0f, 0.0f);
Debug::Float	g_debugLightWrap0				("CutScene.Lights.Wrap0",				1.0f, Debug::Range(0.5f, 1.0f));
Debug::Float	g_debugLightHeading0			("CutScene.Lights.Heading0",			0.0f, Debug::Range(-180.0f, +180.0f));
Debug::Float	g_debugLightElevation0			("CutScene.Lights.Elevation0",			0.0f, Debug::Range(-90.f, +90.0f));

Debug::Bool		g_debugLightOverrideColour1		("CutScene.Lights.OverrideColour1",		false);
Debug::Bool		g_debugLightOverrideDirection1	("CutScene.Lights.OverrideDirection1",	false);
Debug::Colour	g_debugLightColour1				("CutScene.Lights.Colour1",				0.0f, 0.0f, 0.0f, 0.0f);
Debug::Float	g_debugLightWrap1				("CutScene.Lights.Wrap1",				1.0f, Debug::Range(0.5f, 1.0f));
Debug::Float	g_debugLightHeading1			("CutScene.Lights.Heading1",			0.0f, Debug::Range(-180.0f, +180.0f));
Debug::Float	g_debugLightElevation1			("CutScene.Lights.Elevation1",			0.0f, Debug::Range(-90.f, +90.0f));

Debug::Bool		g_debugLightOverrideColour2		("CutScene.Lights.OverrideColour2",		false);
Debug::Bool		g_debugLightOverrideDirection2	("CutScene.Lights.OverrideDirection2",	false);
Debug::Colour	g_debugLightColour2				("CutScene.Lights.Colour2",				0.0f, 0.0f, 0.0f, 0.0f);
Debug::Float	g_debugLightWrap2				("CutScene.Lights.Wrap2",				1.0f, Debug::Range(0.5f, 1.0f));
Debug::Float	g_debugLightHeading2			("CutScene.Lights.Heading2",			0.0f, Debug::Range(-180.0f, +180.0f));
Debug::Float	g_debugLightElevation2			("CutScene.Lights.Elevation2",			0.0f, Debug::Range(-90.f, +90.0f));

Debug::Bool		g_debugLightOverrideColour3		("CutScene.Lights.OverrideColour3",		false);
Debug::Bool		g_debugLightOverrideDirection3	("CutScene.Lights.OverrideDirection3",	false);
Debug::Colour	g_debugLightColour3				("CutScene.Lights.Colour3",				0.0f, 0.0f, 0.0f, 0.0f);
Debug::Float	g_debugLightWrap3				("CutScene.Lights.Wrap3",				1.0f, Debug::Range(0.5f, 1.0f));
Debug::Float	g_debugLightHeading3			("CutScene.Lights.Heading3",			0.0f, Debug::Range(-180.0f, +180.0f));
Debug::Float	g_debugLightElevation3			("CutScene.Lights.Elevation3",			0.0f, Debug::Range(-90.f, +90.0f));

MathsStorage::Float4	g_LightColour[4] =
	{	MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f), 
		MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f), 
		MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f), 
		MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f), 
	};
MathsStorage::Float4	g_LightDirection[4] =
	{	MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f), 
		MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f), 
		MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f), 
		MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f), 
	};

void SetLight(const U32 iIndex, const MathsStorage::Float3& vDirection, const MathsStorage::Float3& vColour, const F32 fWrap)
{
	if(iIndex > 3)
		return;
	g_LightColour[iIndex] = MathsStorage::Float4(vColour.x, vColour.y, vColour.z, fWrap);
	g_LightDirection[iIndex] = MathsStorage::Float4(vDirection.x, vDirection.y, vDirection.z, 0.0f);
}

MathsStorage::Float4 GetLightColour(const U32 iIndex)
{
#ifndef CFG_SPEED
	if(iIndex == 0 && g_debugLightOverrideColour0)
	{
		return MathsStorage::Float4(g_debugLightColour0.Value().r * g_debugLightColour0.Value().a, g_debugLightColour0.Value().g * g_debugLightColour0.Value().a, g_debugLightColour0.Value().b * g_debugLightColour0.Value().a, g_debugLightWrap0);
	}
	else if(iIndex == 1 && g_debugLightOverrideColour1)
	{
		return MathsStorage::Float4(g_debugLightColour1.Value().r * g_debugLightColour1.Value().a, g_debugLightColour1.Value().g * g_debugLightColour1.Value().a, g_debugLightColour1.Value().b * g_debugLightColour1.Value().a, g_debugLightWrap1);
	}
	else if(iIndex == 2 && g_debugLightOverrideColour2)
	{
		return MathsStorage::Float4(g_debugLightColour2.Value().r * g_debugLightColour2.Value().a, g_debugLightColour2.Value().g * g_debugLightColour2.Value().a, g_debugLightColour2.Value().b * g_debugLightColour2.Value().a, g_debugLightWrap2);
	}
	else if(iIndex == 3 && g_debugLightOverrideColour3)
	{
		return MathsStorage::Float4(g_debugLightColour3.Value().r * g_debugLightColour3.Value().a, g_debugLightColour3.Value().g * g_debugLightColour3.Value().a, g_debugLightColour3.Value().b * g_debugLightColour3.Value().a, g_debugLightWrap3);
	}
	else
#endif
	{
		if(iIndex < 4)
			return g_LightColour[iIndex];
		else
			return MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

MathsStorage::Float4 GetLightDirection(const U32 iIndex)
{
#ifndef CFG_SPEED
	if(iIndex == 0 && g_debugLightOverrideDirection0)
	{
		return MathsStorage::Float4(cosf(g_debugLightHeading0 * Maths::Pi / 180.0f) * cosf(g_debugLightElevation0 * Maths::Pi / 180.0f), sinf(g_debugLightElevation0 * Maths::Pi / 180.0f), sinf(g_debugLightHeading0 * Maths::Pi / 180.0f) * cosf(g_debugLightElevation0 * Maths::Pi / 180.0f), 0.0f);
	}																																													  
	else if(iIndex == 1 && g_debugLightOverrideDirection1)																																  
	{																																													  
		return MathsStorage::Float4(cosf(g_debugLightHeading1 * Maths::Pi / 180.0f) * cosf(g_debugLightElevation1 * Maths::Pi / 180.0f), sinf(g_debugLightElevation1 * Maths::Pi / 180.0f), sinf(g_debugLightHeading1 * Maths::Pi / 180.0f) * cosf(g_debugLightElevation1 * Maths::Pi / 180.0f), 0.0f);
	}																																													  
	else if(iIndex == 2 && g_debugLightOverrideDirection2)																																  
	{																																													  
		return MathsStorage::Float4(cosf(g_debugLightHeading2 * Maths::Pi / 180.0f) * cosf(g_debugLightElevation2 * Maths::Pi / 180.0f), sinf(g_debugLightElevation2 * Maths::Pi / 180.0f), sinf(g_debugLightHeading2 * Maths::Pi / 180.0f) * cosf(g_debugLightElevation2 * Maths::Pi / 180.0f), 0.0f);
	}																																													  
	else if(iIndex == 3 && g_debugLightOverrideDirection3)																																  
	{																																													  
		return MathsStorage::Float4(cosf(g_debugLightHeading3 * Maths::Pi / 180.0f) * cosf(g_debugLightElevation3 * Maths::Pi / 180.0f), sinf(g_debugLightElevation3 * Maths::Pi / 180.0f), sinf(g_debugLightHeading3 * Maths::Pi / 180.0f) * cosf(g_debugLightElevation3 * Maths::Pi / 180.0f), 0.0f);
	}
	else
#endif
	{
		if(iIndex < 4)
			return g_LightDirection[iIndex];
		else
			return MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

}; // namespace CutScene
