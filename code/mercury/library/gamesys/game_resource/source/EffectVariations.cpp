#include "GameResourcePch.h"

#include "game_resource/EffectVariations.h"
#include <system/misc/BtlStringUtil.h>
#include <algorithm>

namespace GameResource
{

EffectVariations g_EffectVariations;

/// get instance
EffectVariations &EffectVariations::Instance()
{
	return g_EffectVariations;
}

/// set the current variation
void EffectVariations::SetVariation(const char *pszVariation)
{
	m_sVariation = pszVariation;
}

/// get the current variation
const Btl::String &EffectVariations::GetVariation()
{
	return m_sVariation;
}

/// extract variations from a string (separated by '_' character) and add to an array of variations
void EffectVariations::ExtractVariations(const char *pszString, Btl::Vector<Btl::String> &vars)
{
	char *pszBuffer;

	pszBuffer = new char[strlen(pszString)+1];
	strcpy(pszBuffer, pszString);

	// extract variations from m_sVariation into a list
	while (1)
	{
		char *px = strrchr(pszBuffer, '_');
		if (!px)
			break;
		*px = 0;
		px++;
		vars.PushBack(px);
	}

	delete pszBuffer;
}

bool StringGreater ( const Btl::String &e1, const Btl::String &e2 )
{
	for (int i=0; i<e1.Length(); i++)
	{
		if (i == e2.Length())
			return false;
		if (e1[i]>='a' && e2[i]<'a')
			return true;
		if (e2[i]>='a' && e1[i]<'a')
			return false;
		if (e1[i] < e2[i])
			return true;
		if (e2[i] < e1[i])
			return false;
	}
	return true;
}

/// apply the current variation to the given .fx filename
const Btl::String EffectVariations::ApplyVariation(const char *pszFilename)
{
	Btl::Vector<Btl::String> vars;

	// strip the path and file extension
	Btl::String sFilename = Misc::RemoveFileExtension(Misc::ExtractFilename(pszFilename)).Chars();
	char *pszBuffer = new char[sFilename.Length()+1];
	strcpy(pszBuffer, sFilename.Chars());
	char *px = strchr(pszBuffer, '_');
	// if the first variation is more than 4 characters then skip it, assuming it's something like _skin _paint etc.
	if (px)
	{
		char *pxx = strchr(px+1, '_');
		if (!pxx)
			pxx = px+strlen(px);
		if (pxx && (pxx - px) >= 4)
			px = pxx;
	}

	// extract variations
	if (px)
		ExtractVariations(px, vars);
	ExtractVariations(m_sVariation.Chars(), vars);

	// sort variations alphabetically
	if (vars.Size())
		std::sort(&vars[0], &vars[0]+vars.Size(), StringGreater);

	// strip variations from original filename
	if (px)
		*px = 0;

	// generate new filename
	Btl::String sNewFilename = (Misc::ExtractDirectory(pszFilename) + pszBuffer).Chars();
	for (int i=0; i<vars.Size(); i++)
		sNewFilename += Btl::String("_") + vars[i];
	sNewFilename += Btl::String(".fxb");

	delete pszBuffer;

	return sNewFilename;
}

} // namespace
