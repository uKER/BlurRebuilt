#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include <aftk/Aftk.h>
#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConstantAnnotationReader.h"
#include <converter_shared/helpers/AftkHelpers.h>

namespace LodFadePlugin
{

static float g_fLODFadeDistance = 0.2f;

/// exporter plugin for reading world matrix into a shader constant
class LodFadeHandler : public RenderDataConversion::ConstantAnnotationReader
{
public:
	/// constructor
	LodFadeHandler(FX::Parameter *pParam, const char *pszFXName) :
		RenderDataConversion::ConstantAnnotationReader(pParam, pszFXName)
	{
	}

	/// parse the given source semantic and return true if we recognise it
	bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
	{
		if (type != RenderDataConversion::E_CustomSemanticType_Constant)
			return false;

		return !stricmp(pszSource,"<LodFade>");
	}

	/// get aftk data into an array of 16 floats
	bool GetAftkData(RenderDataConversion::AftkMaterialConverter *pMtl, float *p, Util::C_String *psAnimatedName)
	{
		const Aftk::C_SceneNode* pNode = pMtl->GetSceneNode();

		float distanceMin, distanceMax;
		bool instance = (AftkHelpers::IsInstance(pNode) || pNode->GetNumInstancesOfThisNode() > 0);

		if (instance)
			AftkHelpers::GetInstanceDrawDistance(*pNode, distanceMin, distanceMax);
		else
			AftkHelpers::GetPolygonDrawDistance(*pNode, distanceMin, distanceMax);

		//distanceMin = Maths::Max(distanceMin, 0.f);
		float f = g_fLODFadeDistance;
		distanceMin *= (1.f - f);

		float m1, m2, a1, a2;

		if (f > 0.f)
		{
			if (distanceMin > 0.f)
			{
				m1 = 1.f / (distanceMin * f);
				a1 = 1.f - (distanceMin - 1.f) / (distanceMin * f);
			}
			else
			{
				m1 = 0.f;
				a1 = FLT_MAX;
			}

			m2 = -1.f / (distanceMax * f);
			a2 = 1.f / f;
		}
		else
		{
			m1 = 0.f;
			a1 = 1.f;

			m2 = 0.f;
			a2 = 1.f;
		}

		*(p++) = m1;
		*(p++) = m2;
		*(p++) = a1;
		*(p++) = a2;

		return true;
	}
};

void SetLODFadeDistance(float f)
{
	g_fLODFadeDistance = f;
}

void Register()
{
	RenderDataConversion::RegisterAnnotationReader<LodFadeHandler>();
}

}
