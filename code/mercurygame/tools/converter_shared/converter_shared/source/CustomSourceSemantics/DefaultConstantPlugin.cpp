#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include <aftk/Aftk.h>
#include <fx/Effect.h>

#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConstantAnnotationReader.h"
#include "converter_shared\renderdataconversion\VtxInputSourceSemantics.h"
#include "converter_shared/CustomSourceSemantics/SkinningProcessor.h"
#include "converter_shared/CustomSourceSemantics/SkinningExportPlugin.h"
#include "converter_shared/renderdataconversion/ConvertAftkMesh.h"

/// implements some common constant sources
namespace DefaultConstantPlugin
{

/// exporter plugin for reading world matrix into a shader constant
class WorldMatrixHandler : public RenderDataConversion::ConstantAnnotationReader
{
public:
	/// constructor
	WorldMatrixHandler(FX::Parameter *pParam, const char *pszFXName) :
		RenderDataConversion::ConstantAnnotationReader(pParam, pszFXName)
	{
	}

	/// parse the given source semantic and return true if we recognise it
	bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
	{
		if (type != RenderDataConversion::E_CustomSemanticType_Constant)
			return false;

		return !stricmp(pszSource,"<WorldMatrix>");
	}


	/// get aftk data into an array of 16 floats
	bool GetAftkData(RenderDataConversion::AftkMaterialConverter *pMtl, float *p, Util::C_String *psAnimatedName)
	{
		pMtl->GetWorldMatrix(p);
		return true;
	}
};

/// exporter plugin for reading lux custom data into a shader constant
class LuxCustomHandler : public RenderDataConversion::ConstantAnnotationReader
{
public:
	/// constructor
	LuxCustomHandler(FX::Parameter *pParam, const char *pszFXName) :
		RenderDataConversion::ConstantAnnotationReader(pParam, pszFXName)
	{
	}

	/// parse the given source semantic and return true if we recognise it
	bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
	{
		if (type != RenderDataConversion::E_CustomSemanticType_Constant)
			return false;

		if (!strcmp(pszSource,"<LuxCustom>"))
		{
			m_iOffset = 0;
			return true;
		}

		if (strncmp(pszSource,"<LuxCustom+", 11) || pszSource[strlen(pszSource)-1] != '>')
			return false;

		char buffer[512];
		strcpy(buffer, pszSource+11);
		buffer[strlen(buffer)-1] = 0;
		m_iOffset = atoi(buffer);
		return true;
	}

	/// get aftk data into an array of 16 floats
	bool GetAftkData(RenderDataConversion::AftkMaterialConverter *pMtl, float *p, Util::C_String *psAnimatedName)
	{
		pMtl->GetLuxCustomData(p, m_iOffset);
		return true;
	}
};

/// exporter plugin for reading named parameters (also supports packing)
class NamedParamHandler : public RenderDataConversion::ConstantAnnotationReader
{
	struct ParamItem
	{
		/// construct from string
		ParamItem(const char *pszSource)
		{
			char *pszName, *pszBuffer = new char[strlen(pszSource)+1];
			strcpy(pszBuffer, pszSource);
			// strip heading whitespace
			pszName = pszBuffer;
			while (*pszName == ' ')
				pszName++;
			// strip trailing whitespace
			char *px = pszName + strlen(pszName) -1;
			while (*px == ' ') px--;
				px[1] = 0;
			// split ':' from rest of string
			char *psz = strchr(pszName, ':');
			iOffset = -1;
			if (psz)
			{
				*psz = 0;
				iOffset = atoi(pszName);
				sName = psz+1;
			}
			else
				sName = pszName;
			delete pszBuffer;
		}

		Util::C_String	sName;
		int				iOffset;
	};

public:
	/// constructor
	NamedParamHandler(FX::Parameter *pParam, const char *pszFXName) :
		RenderDataConversion::ConstantAnnotationReader(pParam, pszFXName)
	{
	}

	/// parse the given source semantic and return true if we recognise it
	bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
	{
		if (type != RenderDataConversion::E_CustomSemanticType_Constant)
			return false;

		char *pszCur, *psz, *pszBuffer = new char[strlen(pszSource)+1];
		strcpy(pszBuffer, pszSource);
		pszCur = pszBuffer;
		while (psz = strchr(pszCur, ','))
		{
			*psz = 0;
			m_Params.PushBack(ParamItem(pszCur));
			pszCur = psz+1;
			while (*pszCur == ' ')
				pszCur++;
		}
		if (strlen(pszCur) > 0)
		{
			m_Params.PushBack(ParamItem(pszCur));
		}

		delete [] pszBuffer;
		return true;
	}

	/// get aftk data into an array of 16 floats
	bool GetAftkData(RenderDataConversion::AftkMaterialConverter *pMtl, float *p, Util::C_String *psAnimatedName)
	{
		bool bAnimated = false;
		int iOffset = 0;
		for (int i=0; i<m_Params.Size(); i++)
		{
			if (m_Params[i].iOffset != -1)
				iOffset = m_Params[i].iOffset;
			iOffset += pMtl->GetFloat4FromPlug(m_Params[i].sName, p+iOffset, &bAnimated);
		}
		// animated parameters is only supported on non-packed sources
		if (m_Params.Size() == 1)
		{
			if (bAnimated)
			{
				// if the parameter has an animation curve, expose it to the animation system by material parameter name
				*psAnimatedName = pMtl->GetMtlName() + "." + m_Params[0].sName;
			}
			else
			{
				// if the parameter was flagged to be manually exposed in maya, use the user specified name to expose it
				Util::C_String s = pMtl->GetExposedParameterName(m_Params[0].sName);
				if (s != "")
					*psAnimatedName = s;
			}
		}
		return true;
	}

private:
	Btl::Vector<ParamItem> m_Params;
};

void Register()
{
	RenderDataConversion::RegisterAnnotationReader<WorldMatrixHandler>();
	RenderDataConversion::RegisterAnnotationReader<LuxCustomHandler>();
	RenderDataConversion::RegisterAnnotationReader<NamedParamHandler>();
}

}
