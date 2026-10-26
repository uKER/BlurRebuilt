#ifndef __VTXINPUTANNOTATIONREADER_H__
#define __VTXINPUTANNOTATIONREADER_H__

namespace FX
{
struct InputSemantic;
}

namespace RenderDataConversion
{

class AftkMeshConverter;

/*
 * A list of supported vertex input annotations:
 *
 * Packing is specified in the following way: xyz(source) e.g.
 * float4 vTexCoord : TEXCOORD0 annotation(xy(uv:0) | zw(uv:luxlightmap)) ;
 * 
 * source can be one of the following:
 *  
 *	position
 *	normal
 *	uv[:uvset]
 *	tangent[:uvset]
 *	binormal[:uvset]
 *	data[:vtxdatach]
 *  colour[:vtxdatach]
 *
 *	additional annotations:
 *	- ability to pretransform an SH vector: shpretransform(true)
 *	- applying a scale factor
 *	- palletising
 *	- specify data format: format(F16_2) etc.
 *	
 */

/// the default source and vertex format for a particular semantic
struct DefaultVtxElement
{
	const char *pszName, *pszSource;
	RenderingData::VertexElementType type;
};

/// vtx element source description
struct VtxSourceSemantic
{
	I_CustomVtxInputSemantic *pSource;
	XYZW src;
	XYZW dest;
};

/// interpret a list of annotations belonging to a vertex input semantic
class VtxInputAnnotationReader : public AnnotationReader
{
public:
	/// constructor
	VtxInputAnnotationReader(Effect *pEffect, FX::InputSemantic *pAnnotation);

	/// destructor
	~VtxInputAnnotationReader();

	/// get the data format for this vertex input semantic
	/// returns false if this semantic should be skipped
	bool GetVertexDescElement(RenderingData::VertexFormat::VertexElement *pElement, S32 *piStream);

	/// get data from aftk vertex in vertex buffer format
	/// data pointer is automatically incremented
	bool GetVertexData(const AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, U8 **ppData);

	/// return a string that can identify this vtx input annotation so we can print out a warning about it
	Util::C_String GetWarningParams();

	/// return all of the annotations in a string (for debugging)
	Util::C_String DebugDesc();

private:
	/// decide on the output format based on the annotations
	void ParseFormatAnnotation();

	/// parse format annotation and set m_Type if possible. returns false if the annotation was not recognised
	bool ParseFormatAnnotation(const char *pszFormat);

	/// add a vertex input data source
	void AddSourceElement(XYZW *pXYZW, const char *pszSourceDesc);

	static const char *m_pszFormatStrings[];
	static const int m_nFormatScalars[];
	static const DefaultVtxElement m_DefaultElementValues[];

	Effect *m_pEffect;
	FX::InputSemantic *m_pSemantic;
	RenderingData::VertexElementType m_Type;
	Ut::Vector<VtxSourceSemantic> m_Sources;
	bool m_bBias;
};

/// returns a string containing the semantic name e.g. POSITION0 (for printing warnings etc.)
Util::C_String GetSemanticName(FX::InputSemantic *pSemantic);

}

#endif
