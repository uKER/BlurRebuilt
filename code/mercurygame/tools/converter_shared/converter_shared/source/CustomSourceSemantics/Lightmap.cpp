#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\TextureAnnotationReader.h"


namespace Lightmap
{

/// a custom texture semantic handler that implements reading multiple lightmaps
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
		if (type != RenderDataConversion::E_CustomSemanticType_Texture)
			return false;

		if (!stricmp(pszSource,"<Lightmap>"))
			m_Index = 0;
		else if (!stricmp(pszSource,"<Lightmap+0>"))
			m_Index = 0;
		else if (!stricmp(pszSource,"<Lightmap+1>"))
			m_Index = 1;
		else if (!stricmp(pszSource,"<Lightmap+2>"))
			m_Index = 2;
		else if (!stricmp(pszSource,"<Lightmap+3>"))
			m_Index = 3;
		else
			return false;

		return true;
	}

	/// return the texture filename for this semantic for the given material
	Util::C_String GetTextureFilename(RenderDataConversion::AftkMaterialConverter *pMtl)
	{
		Util::C_String sLightmap = pMtl->GetLightmapTextureFilename();

		// Note: this should only return files that actually exist,
		// otherwise a unique texture will be created.
		switch (m_Index)
		{
		case 0:
			{
				return sLightmap;
			};
		case 1:
			{
				if (sLightmap.ISearch("-A") == -1)
					return "";
				Util::C_String sNewLightmap = sLightmap.IReplace("-A", "-B");
				if (pMtl->TextureFileExists(sNewLightmap))
					return sNewLightmap;
				return "";
			};
		case 2:
			{
				if (sLightmap.ISearch("-A") == -1)
					return "";
				Util::C_String sNewLightmap = sLightmap.IReplace("-A", "-C");
				if (pMtl->TextureFileExists(sNewLightmap))
					return sNewLightmap;
				return "";
			};
		case 3:
			{
				if (sLightmap.ISearch("-A") == -1)
					return "";
				Util::C_String sNewLightmap = sLightmap.IReplace("-A", "-D");
				if (pMtl->TextureFileExists(sNewLightmap))
					return sNewLightmap;
				return "";
			};
		default:
			{
				assert(0);
				return "";
			};
		};
	}

private:
	U32 m_Index;
};

void Register()
{
	RenderDataConversion::RegisterAnnotationReader<Handler>();
}

} // namespace
