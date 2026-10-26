#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\TextureAnnotationReader.h"
#include "converter_shared\renderdataconversion\CubemapInform.h"

namespace LocalisedCubemap
{

const RenderDataConversion::I_CubemapInform *g_pCubemapInform = NULL;

/// a custom texture semantic handler that implements _DIFF, _IND0, _NORM, _BUMP
class Handler : public RenderDataConversion::TextureAnnotationReader
{
public:
	/// constructor
	Handler(FX::Parameter *pParam, const char *pszFXName) :
		RenderDataConversion::TextureAnnotationReader(pParam, pszFXName)
	{
	}

	/// parse the given source semantic and return true if we recognise it
	bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
	{
		return stricmp(pszSource, "<LocalisedCubemap>") == 0;
	}

	/// return the texture filename for this semantic for the given material
	Util::C_String GetTextureFilename(RenderDataConversion::AftkMaterialConverter *pMtl)
	{
		if (!g_pCubemapInform)
		{
			m_bAllowOmit = true;	// if no cubemap inform was set then we don't export cubemaps
			return "";
		}

		m_bWarnMissing = true;
		return g_pCubemapInform->GetCubemapFilename(pMtl->GetWorldBoundingBox());
	}
};

void Register()
{
	RenderDataConversion::RegisterAnnotationReader<Handler>();
}

void SetCubemapInform(const RenderDataConversion::I_CubemapInform *p)
{
	g_pCubemapInform = p;
}

} // namespace
