#ifndef __FX_I_EFFECT_H__
#define __FX_I_EFFECT_H__

namespace FX
{

struct Parameter;
typedef U32 TechniqueHandle;
typedef U64 ImmediateParameter;

class I_Effect
{
protected:
	/// not allowed to construct these ourselves - call FX::ImportFXB instead
	I_Effect() {}
	/// not allowed to destruct these ourselves - call FX::ReleaseEffect instead
	~I_Effect() {}
public:

	/// Get a pointer to an effect parameter by name
	Parameter *GetParameter(const char *pszName) const;

	/// returns whether a parameter is used on a particular technique
	/// returns a bitfield, each bit represents a technique on the effect
	int GetTechniqueUsage(const ImmediateParameter &param);

	/// Get the index of the technique with the specified name
	/// returns -1 if the name was not found
	int GetTechnique(const char *psz);

private:
	// implementation is inherited from this class
};

} // namespace FX

#endif
