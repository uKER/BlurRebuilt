#ifndef __CUSTOMSEMANTICMANAGER_H__
#define __CUSTOMSEMANTICMANAGER_H__

#include "converter_shared/renderdataconversion/CustomSemanticType.h"

namespace RenderDataConversion
{

class I_CustomSemanticFactory;

/// interpret a list of annotations belonging to a texture parameter
class CustomSemanticManager
{
public:
	/// destructor
	~CustomSemanticManager();

	/// register a custom input semantic with the manager
	void RegisterSemantic(const I_CustomSemanticFactory *p);

	/// returns a pointer to an object that can interpret the given texture source semantic
	I_CustomSemantic *ParseSourceSemantic(const char *pszSource, E_CustomSemanticType type, FX::Parameter *pParam, I_ParseAnnotation *pParent);

private:
	Ut::Vector<const I_CustomSemanticFactory*> m_Semantics;
};

extern CustomSemanticManager g_CustomSemanticManager;

}

#endif
