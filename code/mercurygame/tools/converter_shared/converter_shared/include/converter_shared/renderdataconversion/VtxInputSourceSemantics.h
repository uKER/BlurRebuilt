#ifndef __DEFAULTVTXINPUTSEMANTICS_H__
#define __DEFAULTVTXINPUTSEMANTICS_H__

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

class DefaultVtxInputSourceSemantics : public I_CustomVtxInputSemantic
{
public:
	/// constructor
	DefaultVtxInputSourceSemantics(VtxInputAnnotationReader *pParent);

	/// parses the entire source semantic string including uv set number, swizzling etc.
	virtual bool IdentifySemantic(const char *pszSource);

	/// returns true if the given input source semantic name can be handled by this class
	virtual bool IdentifySourceSemantic(const char *pszSource);

	/// get the data for the given vertex
	virtual bool GetData(const AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, float *ppData);

	/// returns true if UV-remapping needs to be performed on the output data
	virtual bool DoRecentering();

	 /// different places to source vertex data from
	enum E_VtxSource
	{
		E_VtxSource_Position,
		E_VtxSource_Normal,
		E_VtxSource_UV,
		E_VtxSource_Tangent,
		E_VtxSource_Binormal,
		E_VtxSource_Sense,
		E_VtxSource_VtxData,
		E_VtxSource_Colour,
		E_VtxSource_UVNoRemap,
		E_VtxSource_None = 1,
	};

	int m_iIndex;				// index of source (if -1, use named source)
	Util::C_String m_sName;		// named source

	VtxInputAnnotationReader *m_pParent;

private:
	E_VtxSource m_iSource;		// source type
	static const char *m_pszSourceTypeNames[];
};

} // namespace

#endif
