#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\TextureAnnotationReader.h"
#include "converter_shared\renderdataconversion\TextureTags.h"

namespace DIFF_IND0_NORM_BUMP
{

/// a custom texture semantic handler that implements _DIFF, _IND0, _NORM, _BUMP
class Handler : public RenderDataConversion::TextureAnnotationReader
{
public:
	enum E_Type
	{
		E_Type_DIFF,
		E_Type_IPR0,
		E_Type_BUMP,
		E_Type_NEON,
	};

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

		if (!strnicmp(pszSource,"<Diff",5))
			m_Type = E_Type_DIFF;
		else if (!strnicmp(pszSource,"<Bump",5))
			m_Type = E_Type_BUMP;
		else if (!strnicmp(pszSource,"<Ipr0",5))
			m_Type = E_Type_IPR0;
		else if (!strnicmp(pszSource,"<Neon",5))
			m_Type = E_Type_NEON;
		else
			return false;

		// no source parameter name specified
		if (!strcmp(pszSource+5, ">"))
		{
			m_sSourceName = "";
			return true;
		}

		if (pszSource[strlen(pszSource)-1] != '>')
			return false;

		// source parameter name specified
		Util::C_String sTmp = pszSource;
		sTmp[sTmp.Length()-1] = 0;
		m_sSourceName = &sTmp[6];

		return true;
	}

	/// return the texture filename for this semantic for the given material
	Util::C_String GetTextureFilename(RenderDataConversion::AftkMaterialConverter *pMtl)
	{
		Util::C_String sDiff;
		Util::C_String sTmp;
		if (m_sSourceName.Length() == 0)
			sDiff = pMtl->GetDiffuseTextureFilename();
		else
			sDiff = pMtl->GetTextureFilenameFromPlug(m_sSourceName);

		RenderDataConversion::C_TextureTags textureTags;
		RenderDataConversion::C_DefaultTextureTagsReader defaultReader;

		// Note: this should only return files that actually exist,
		// otherwise a unique texture will be created.
		switch (m_Type)
		{
		case E_Type_DIFF:
			m_bWarnMissing = true;
			return sDiff;
		case E_Type_IPR0:

			defaultReader.ReadTextureTags(sDiff, textureTags);
			if (textureTags.m_Override_IPR0.Length())
			{
				if (pMtl->TextureFileExists(textureTags.m_Override_IPR0))
					return textureTags.m_Override_IPR0;
			}

			if (sDiff.ISearch("_DIFF") != -1)
			{
				sTmp = sDiff.IReplace("_DIFF", "_IPR0");

				if (pMtl->TextureFileExists(sTmp))
					return sTmp;
			}

			// search for legacy club naming
			if (sDiff.ISearch("A0_") != -1)
			{
				sTmp = sDiff.IReplace("A0_", "S0_");

				if (pMtl->TextureFileExists(sTmp))
					return sTmp;
			}

			return "";

		case E_Type_BUMP:
			// reset values in case they were changed by a previous texture
			m_bConvertNormalMap = false;
			m_bInvertNormalMapX = false;

			defaultReader.ReadTextureTags(sDiff, textureTags);
			if (textureTags.m_Override_BUMP_NORM.Length())
			{
				if (pMtl->TextureFileExists(textureTags.m_Override_BUMP_NORM))
				{
					if (textureTags.m_Override_BUMP_NORM.ISearch("_BUMP") != -1)
					{
						m_bConvertNormalMap = true;
						return textureTags.m_Override_BUMP_NORM;
					}
					else if (textureTags.m_Override_BUMP_NORM.ISearch("_NORM") != -1)
					{
						m_bInvertNormalMapX = true;
						return textureTags.m_Override_BUMP_NORM;
					}
				}
			}

			m_iDefaultColour = 0xffff7f7f;
			if (sDiff.ISearch("_DIFF") != -1)
			{
				// try bumpmap
				sTmp = sDiff.IReplace("_DIFF", "_BUMP");
				if (pMtl->TextureFileExists(sTmp))
				{
					m_bConvertNormalMap = true;
					return sTmp;
				}
				// try normal map
				sTmp = sDiff.IReplace("_DIFF", "_NORM");
				if (pMtl->TextureFileExists(sTmp))
				{
					m_bInvertNormalMapX = true;

					return sTmp;
				}
			}
			
			// search for legacy club naming
			if (sDiff.ISearch("A0_") != -1)
			{
				sTmp = sDiff.IReplace("A0_", "N0_");

				if (pMtl->TextureFileExists(sTmp))
				{
					m_bInvertNormalMapX = true;
					return sTmp;
				}
			}

			return "";
		case E_Type_NEON:

			defaultReader.ReadTextureTags(sDiff, textureTags);
			if (textureTags.m_Override_NEON.Length())
			{
				if (pMtl->TextureFileExists(textureTags.m_Override_NEON))
					return textureTags.m_Override_NEON;

			}

			if (sDiff.ISearch("_DIFF") != -1)
			{
				sTmp = sDiff.IReplace("_DIFF", "_NEON");

				if (pMtl->TextureFileExists(sTmp))
					return sTmp;
			}

			// search for legacy club naming
			if (sDiff.ISearch("A0_") != -1)
			{
				sTmp = sDiff.IReplace("A0_", "E0_");

				if (pMtl->TextureFileExists(sTmp))
					return sTmp;
			}

			return "";

		default:
			assert(0);
			return "";
		}
	}

private:
	E_Type m_Type;
	Util::C_String m_sSourceName;
};

void Register()
{
	RenderDataConversion::RegisterAnnotationReader<Handler>();
}

} // namespace
