#ifndef __ANNOTATIONREADER_H__
#define __ANNOTATIONREADER_H__

#include "converter_shared/RenderDataConversion/CustomSemantic.h"

namespace FX
{
struct Annotation;
}

namespace RenderDataConversion
{

class EffectPermutationSelector;

/// represents a parsed 'xyzw' string
struct XYZW
{
	int iMapping[4];	// represents the mapping for each element e.g. xyw would be 0,1,3
	int nScalars;		// number of scalars e.g. xyw would be 3
	int iUsageMask;		// usage mask e.g. xyw would be 1011 (binary)
};

/// base functionality for interpreting a list of annotations
class AnnotationReader
{
friend class Effect;
friend class CustomSemanticManager;
friend class AnnotationReaderList;
public:
	/// constructor
	AnnotationReader();

protected:
	/// parse annotation
	virtual bool ParseAnnotation(FX::Annotation *pAnnotation);

	/// if this parameter has a permutation, add it to the EPS
	void SetPermutation(EffectPermutationSelector *pEPS);

	/// checks whether this parameter needs exporting in the current pass
	bool NeedsExport(E_NodeType type);

	/// returns true if this annotation reader needs exporting for the given stream index
	bool NeedsExportingForStream(int iStream);

	/// returns true if we're exporting a model or platform, false if we're exporting static streamed level mesh
	bool IsExportingModel();

	enum E_InstanceMode
	{
		E_InstanceMode_Base				= 1,
		E_InstanceMode_Instance			= 3,
		E_InstanceMode_InstanceOnly		= 2,
	};

public:
	bool m_bAllowOmit;			// true if we're allowed to omit this parameter from the material if it doesn't exist
	Util::C_String m_sDefine;	// the #define to use if the parameter has been omitted
	E_InstanceMode m_InstanceMode;	// specifies whether the data for this parameter goes on the instance or the base-instance
	bool m_bExportOnModel;		// false if the parameter won't be exported for models
	int m_iStream;				// index of the stream on the primitive that this parameter should go on
};

/// parse an xyzw string and return a usage bitfield
/// returns false if the string is not an xyzw string
bool ParseXYZWString(const char *psz, XYZW *pOut);

}

#endif
