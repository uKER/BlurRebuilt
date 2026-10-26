#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\Effect.h"
#include <fx/effect.h>

#include "converter_shared\renderdataconversion\Common.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"

namespace RenderDataConversion
{

// this list of source names matches the enum in ParamAnnotationReader.h
const char *ParamAnnotationReader::m_pszSourceNames[] = {
"<None>",
"<Diffuse>",
"<Lightmap>",
"<WorldMatrix>",
"<LuxCustom>",
""
};

/// constructor
ParamAnnotationReader::ParamAnnotationReader(FX::Parameter *p, const Util::C_String &sEffectName)
{
	m_pParam = p;
	m_Source = E_Source_None;
	m_sEffectName = sEffectName;
	m_iOffset = 0;
}

/// interpret a 'Source' annotation and set variables accordingly
void ParamAnnotationReader::InterpretSourceAnnotation(FX::Annotation *pAnnotation)
{
	if (pAnnotation->Type != FX::NGLFX_ANNOTATION_STRING)
		return;

	// work out what type of source it is
	const char **pszSources = m_pszSourceNames;
	const char *pszMySource = pAnnotation->String;
	// if it wrapped in '<>', assume it's a source semantic
	if (pszMySource[0] == '<' && pszMySource[strlen(pszMySource)-1] == '>')
	{
		if (!strnicmp(pszMySource, "<LuxCustom+", 11))
		{
			m_Source = E_Source_LuxCustom;
			char buffer[512];
			strcpy(buffer, pszMySource+11);
			buffer[strlen(buffer)-1] = 0;
			m_iOffset = atoi(buffer);
			return;
		}

		int iSource = 0;
		while (pszSources[iSource][0] != 0)
		{
			if (!stricmp(pszSources[iSource], pszMySource))
			{
				m_Source = (E_Source)iSource;
				return;
			}
			iSource++;
		}
		// unrecognised source semantic
		PrintWarning( "%s: Unrecognised source semantic '%s' on parameter '%s'\n",
			m_sEffectName.Chars(), pszMySource, m_pParam->Name.Chars() );
		return;
	}
	m_Source = E_Source_NamedParam;
	m_sNamedParamName = pszMySource;
}

/// interpret an annotation
bool ParamAnnotationReader::ParseAnnotation(FX::Annotation *pAnnotation)
{
	if (!stricmp(pAnnotation->Name, "Source"))
	{
		InterpretSourceAnnotation(pAnnotation);
		return true;
	}

	bool bResult = AnnotationReader::ParseAnnotation(pAnnotation);

	if (!m_pParam->bShared && m_InstanceMode != E_InstanceMode_Base)
		PrintWarning("Parameter %s is InstanceMode!=Base without being declared as 'shared'.\n", m_pParam->Name.Chars());

	return bResult;
}

}

