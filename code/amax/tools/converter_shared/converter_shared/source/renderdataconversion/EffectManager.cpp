#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>
#include <renderingdata/renderingdata.h>
#include "converter_shared\helpers/AftkMaterialHelpers.h"
#include "converter_shared\renderdataconversion\Effect.h"
#include "converter_shared\renderdataconversion\EffectManager.h"

namespace RenderDataConversion
{

Debug::Logger &g_Log = Debug::GetLogger("converter_shared.converteffect");

/// print warnings, checks for duplicate warnings and limits the number of total warnings for this system
void PrintWarning(const char *format, ...)
{
	char buffer[1024];

    va_list arglist;
    va_start( arglist, format );
    vsprintf_s( buffer, sizeof(buffer), format, arglist );
    va_end( arglist );
	Util::C_String sString(buffer);

	static Btl::Set<Util::C_String> set;
	Btl::Set<Util::C_String>::Iterator i=set.Find(sString);
	if (i==set.End())
	{
		static int nUniqueWarnings = 0;
		if (nUniqueWarnings>100)
			return;
		nUniqueWarnings++;		
		set.Insert(sString);
		LOGWARN( g_Log, "%s", sString.Chars());
	}
}

/// constructor
EffectManager::EffectManager(RenderingData::Build::Scene *pSceneExport, const Util::C_String &sFXBPath)
{
	m_pSceneExport = pSceneExport;
	m_sFXBPath = sFXBPath;
}

/// destructor
EffectManager::~EffectManager()
{
	for (Btl::Map<Util::C_String,Effect*>::Iterator i=m_Map.Begin(); i!=m_Map.End(); ++i)
		delete i->GetValue();
}

/// loads the effect that is associated with the given Aftk::Material
Effect *EffectManager::GetEffect(const Util::C_String fxName)
{
	Btl::Map<Util::C_String,Effect*>::Iterator i = m_Map.Find(fxName);
	if (i != m_Map.End())
		return i->GetValue();

	// add new material
	return CreateEffect(fxName);
}

/// convert Aftk material to renderdata effect
Effect *EffectManager::CreateEffect(const Util::C_String &sName)
{
	Effect *pItem = new Effect;

	pItem->Initialise(m_pSceneExport, m_sFXBPath, sName);
	m_Map[sName] = pItem;

	return pItem;
}

};
