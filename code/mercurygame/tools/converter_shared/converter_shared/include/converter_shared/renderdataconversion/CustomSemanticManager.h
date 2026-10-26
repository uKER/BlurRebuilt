#ifndef __CUSTOMSEMANTICMANAGER_H__
#define __CUSTOMSEMANTICMANAGER_H__

#include "converter_shared/renderdataconversion/CustomSemanticType.h"

namespace RenderDataConversion
{

class I_CustomSemanticFactory;
class I_CustomVtxInputSemanticFactory;

/// interpret a list of annotations belonging to a texture parameter
class CustomSemanticManager
{
public:
	/// destructor
	~CustomSemanticManager();

	/// register a custom input semantic with the manager
	void RegisterSemantic(const I_CustomSemanticFactory *p);
	void RegisterSemantic(const I_CustomVtxInputSemanticFactory *p);

	/// returns a pointer to an object that can interpret the given parameter
	ParamAnnotationReader *FindAnnotationReader(FX::Parameter *pParam, const char *pszFXName);

	/// returns a pointer to an object that can interpret the given vtx input source semantic
	I_CustomVtxInputSemantic *FindVtxInputSemanticHandler(VtxInputAnnotationReader *pParent, const char *pszMySource);

private:
	/// find the source annotation and return the name of the source
	const char *GetSource(Btl::Vector<FX::Annotation> &annotations);

	Btl::Vector<const I_CustomSemanticFactory*> m_Semantics;
	Btl::Vector<const I_CustomVtxInputSemanticFactory*> m_VtxSemantics;
};

extern CustomSemanticManager g_CustomSemanticManager;

}

#endif
