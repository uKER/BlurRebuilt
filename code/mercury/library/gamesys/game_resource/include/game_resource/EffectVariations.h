#ifndef __EFFECT_VARIATIONS_H__
#define __EFFECT_VARIATIONS_H__

namespace GameResource
{

/// global class that stores the currently used effect variation (amax day, amax night, low lod, etc.)
class EffectVariations
{
public:
	/// get instance
	static EffectVariations &Instance();

	/// set the current variation
	void SetVariation(const char *pszVariation);

	/// get the current variation
	const Btl::String &GetVariation();

	/// apply the current variation to the given .fx filename
	const Btl::String ApplyVariation(const char *pszFilename);

private:
	/// extract variations from a string (separated by '_' character) and add to an array of variations
	void ExtractVariations(const char *pszString, Btl::Vector<Btl::String> &vars);

	Btl::String m_sVariation;
};

} // namespace

#endif
