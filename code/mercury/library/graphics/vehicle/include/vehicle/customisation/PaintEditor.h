/////////////////////////////////////////////////////////////
// PaintEditor
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_PAINT_EDITOR_H__
#define __VEHICLE_PAINT_EDITOR_H__

#include "Vehicle/Utils/Curve.h"

#include <xt/xtgame.h>

namespace Vehicle
{

struct XtVehicle;
struct XtMaterialDefaults;
struct XtPaintJob;

#define NUM_PAINT_TEXTURES 2

/* 
===============================================================
					class C_PaintJob

NOTES: Custom paint job class
===============================================================
*/
class C_PaintJob
{
public:

	C_PaintJob();

	~C_PaintJob();

	//! Creates paint material texture
	bool Create(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults, XtId p_XtId, U32 vehicleIndex);

	//! Destroys paint material texture
	void Destroy();

	//! Updates paint job
	void Update();

	//! Updates material from Sorcery
	void UpdateMaterial(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults, XtId p_XtId, XtPaintJob* pPaintJobOverride = NULL);

	//! Updates material texture
	void UpdateMaterialTexture();

	//! Resprays colour
	void Respray(U32 p_Colour);

	//! Calculates the paint colour	
	void CalculatePaintColour(MathsStorage::Float4& p_Texel, float p_Value);

	//! Calculates the paint reflectivity
	void CalculatePaintReflectivity(MathsStorage::Float4& p_Texel, float p_Value);

	Display::C_Texture2D* GetMaterialTexture() { return m_MaterialTextures[m_CurrentTexture]; }
	void SwapTextures() { m_CurrentTexture++; if (m_CurrentTexture >= NUM_PAINT_TEXTURES) m_CurrentTexture = 0; }

	const MathsStorage::Float4& GetUndercoatColour() const { return m_UndercoatColour; }
	const MathsStorage::Float4& GetScuffColour() const { return m_ScuffColour; }

public:
	
	Btl::Vector<Display::C_Texture2D*, Btl::VectorParameters_Array<NUM_PAINT_TEXTURES> > m_MaterialTextures;	//!< Material texture for paint (RGB colour and reflectivity)
	U8							m_CurrentTexture;

	bool						m_Created;						//!< Whether the paint textures have been created

	MathsStorage::Float4		m_UndercoatColour;
	MathsStorage::Float4		m_ScuffColour;

	Btl::Vector<MathsStorage::Float3>	m_Colours;						//!< Array of colours making up the paint

	C_HermiteCurve				m_ColourCurve;					//!< Curve to control colour blending
	C_HermiteCurve				m_ReflectivityCurve;			//!< Curve to control view-based reflectivity

	//PaintTexture Resources
	static U32	ms_NumTextureResourcesVehicles;
	static Display::C_Texture2D	ms_PaintTextureResources[NUM_PAINT_TEXTURES][20];
	
	static bool CreatePaintTextures(U32 numVehicles);
	static void DestroyPaintTextures();
};

} //namespace Vehicle

#endif

