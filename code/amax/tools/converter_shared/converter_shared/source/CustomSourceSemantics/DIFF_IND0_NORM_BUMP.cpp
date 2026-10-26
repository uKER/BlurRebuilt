#include "stdafx.h"

#include "converter_shared/renderdataconversion/CustomSemantic.h"
#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"


#pragma optimize("", off)

namespace DIFF_IND0_NORM_BUMP
{

/// a custom texture semantic handler that implements _DIFF, _IND0, _NORM, _BUMP
class Handler : public RenderDataConversion::I_CustomTextureSemantic
{
public:
	enum E_Type
	{
		E_Type_DIFF,
		E_Type_IPR0,
		E_Type_BUMP,
		E_Type_NEON,
	};

	/// parse the given source semantic and return true if we recognise it
	bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type, FX::Parameter *pParam)
	{
		if (type != RenderDataConversion::E_CustomSemanticType_Texture)
			return false;

		if (!stricmp(pszSource,"<Diff>"))
			m_Type = E_Type_DIFF;
		else if (!stricmp(pszSource,"<Bump>"))
			m_Type = E_Type_BUMP;
		else if (!stricmp(pszSource,"<Ipr0>"))
			m_Type = E_Type_IPR0;
		else if (!stricmp(pszSource,"<Neon>"))
			m_Type = E_Type_NEON;
		else
			return false;
		return true;
	}

	/// return the texture filename for this semantic for the given material
	Util::C_String GetTextureFilename(RenderDataConversion::AftkMaterialConverter *pMtl)
	{
		Util::C_String sDiff = pMtl->GetDiffuseTextureFilename();
		Util::C_String sTmp;

		switch (m_Type)
		{
		case E_Type_DIFF:
			//SetAnnotation("Default", "Warn");
			return sDiff;
		case E_Type_IPR0:
			if (sDiff.ISearch("_DIFF") == -1)
				return "";
			return sDiff.IReplace("_DIFF", "_IPR0");
		case E_Type_BUMP:
			SetAnnotation("Default", "Normal");
			if (sDiff.ISearch("_DIFF") == -1)
				return "";
			// try bumpmap
			sTmp = sDiff.IReplace("_DIFF", "_BUMP");
			if (Sys::C_FileSystem::Instance().FileExists(sTmp))
			{
				SetAnnotation("ConvertNormalMap", true);
				return sTmp;
			}
			// try normal map
			sTmp = sDiff.IReplace("_DIFF", "_NORM");
			if (Sys::C_FileSystem::Instance().FileExists(sTmp))
			{
				SetAnnotation("InvertNormalMapX", true);
				return sTmp;
			}
			return "";
		case E_Type_NEON:
			if (sDiff.ISearch("_DIFF") == -1)
				return "";
			return sDiff.IReplace("_DIFF", "_NEON");
		default:
			assert(0);
			return "";
		}
	}

private:
	E_Type m_Type;	
};

void Register()
{
	RenderDataConversion::RegisterSemantic<Handler>();
}

}
