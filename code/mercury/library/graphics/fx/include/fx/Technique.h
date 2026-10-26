#ifndef __FX_TECHNIQUE_H__
#define __FX_TECHNIQUE_H__

#ifndef FX_ALLOW_INCLUDE
#error "Illegal include of FX library internals.h"
#endif

#include <Btl/Containers/Vector.h>

namespace FX
{

/// callback function that returns true if the registered subtechnique is enabled on the technique passed in
typedef bool SubtechniqueCallback(Technique *pTech);

/// data for a single registered subtechnique
struct Subtechnique
{
	Util::C_String *psName;		// debug variable name
	Util::C_String sName;		// subtechnique name
	Debug::Bool *pDebugVariable;
};

/// keeps track of registered techniques, calculate technique mappings
class TechniqueManager
{
public:
	/// destructor
	~TechniqueManager();

	/// register a subtechnique
	TechniqueHandle RegisterSubtechnique(const char *pszName);

	/// calculate the technique handles on each technique of an effect
	void CalculateTechniqueHandles(Effect *pEffect);

	/// return the override technique as specified by the debug variables
	int GetOverrideTechnique();

private:
	/// calculate a subtechnique bitfield for a particular technique
	TechniqueHandle CalculateSubtechniqueBitfield(Technique *pTech);

	Btl::Vector<Subtechnique>	m_Subtechniques;
};

extern TechniqueManager g_TechniqueManager;

} // namespace FX

#endif
