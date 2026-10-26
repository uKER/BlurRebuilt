#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include <aftk/Aftk.h>
#include <fx/Effect.h>

#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConstantAnnotationReader.h"
#include "converter_shared\renderdataconversion\VtxInputSourceSemantics.h"
#include "converter_shared/CustomSourceSemantics/VehicleParameterExposeExportPlugin.h"
#include "converter_shared/renderdataconversion/ConvertAftkMesh.h"

namespace VehicleShaders
{

/* 
===============================================================
					class ParamHandler

NOTES: Exporter plugin for exposing material parameters
===============================================================
*/
class ParamHandler : public RenderDataConversion::ConstantAnnotationReader
{
public:
	/// constructor
    ParamHandler(FX::Parameter *pParam, const char *pszFXName) :
                RenderDataConversion::ConstantAnnotationReader(pParam, pszFXName)
    {
    }

	/// parse the given source semantic and return true if we recognise it
    bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
    {
		if (type != RenderDataConversion::E_CustomSemanticType_Constant)
			return false;

		if (!stricmp(pszSource, "CodeDamageScale"))
		{
			m_eType = E_Type_CodeDamageScale;
			return true;
		}

		if (!stricmp(pszSource, "VehicleSHNode"))
		{
			m_eType = E_Type_VehicleSHNode;
			return true;
		}

		return false;
    }

    /// get aftk data into an array of 16 floats
    bool GetAftkData(RenderDataConversion::AftkMaterialConverter *pMtl, float *p, Util::C_String *psAnimatedName)
    {
		if (!pMtl || !pMtl->GetSceneNode())
			return false;

		switch (m_eType)
		{
		case E_Type_CodeDamageScale:
			{
				bool bWheel = false;

				Util::C_String name = pMtl->GetSceneNode()->GetName().c_str();

				if (name.ISearch("_wheel_") != -1 ||
					name.ISearch("_tyre_") != -1 ||
					name.ISearch("_alloy") != -1 ||
					name.ISearch("_brakedisc_") != -1 ||
					name.ISearch("_brakecaliper_") != -1)
				{
					bWheel = true;
				}

				if (bWheel)
				{
					p[0] = 0.0f;
					return true;
				}

				//p[0] = 1.0f;

				*psAnimatedName = "CodeDamageScaleParam";

				return true;
			}
			break;
		case E_Type_VehicleSHNode:
			{
				//Check if it is a rotated wheel and the side of it
				bool bRotatedWheel = false;

				Util::C_String name = pMtl->GetSceneNode()->GetName().c_str();

				if (name.ISearch("_wheel_") != -1 ||
					name.ISearch("_tyre_") != -1 ||
					name.ISearch("_alloy") != -1 ||
					name.ISearch("_brakedisc_") != -1)
				{
					bRotatedWheel = true;
				}

				if (bRotatedWheel)
				{
					//Check side
					if ((name.ISearch("backleft") != -1) || (name.ISearch("rearleft") != -1))
					{
						*psAnimatedName = "BackLeftSHNode";
						return true;
					}
					else if (name.ISearch("frontleft") != -1)
					{
						*psAnimatedName = "FrontLeftSHNode";
						return true;
					}
					else if ((name.ISearch("backright") != -1) || (name.ISearch("rearright") != -1) )
					{
						*psAnimatedName = "BackRightSHNode";
						return true;
					}
					else if (name.ISearch("frontright") != -1)
					{
						*psAnimatedName = "FrontRightSHNode";
						return true;
					}
					
					//Blur allow or allow
					*psAnimatedName = "VehicleSHNode";
					return true;
				}
				else
				{
					*psAnimatedName = "VehicleSHNode";
					return true;
				}
			}
		}

		return false;
    }

private:
	enum E_Type
	{
		E_Type_CodeDamageScale,
		E_Type_VehicleSHNode
	};

	E_Type m_eType;
};

void Register()
{
	RenderDataConversion::RegisterAnnotationReader<ParamHandler>();
}

} // namespace VehicleShaders

