/////////////////////////////////////////////////////////////
// PaintEditor
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Customisation/PaintEditor.h"
#include "Vehicle/Db/VehicleXt.h"

#include <system/Memory.h>

#include <fx/fx.h>

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

#define ENABLE_VEHICLE_CUSTOM_PAINT

namespace Vehicle
{

/* 
===============================================================
					class C_PaintJob

NOTES: Custom paint job class
===============================================================
*/

/*
===============
NAME: C_PaintJob

NOTE: Constructor
===============
*/
C_PaintJob::C_PaintJob()
{
	m_Created	= false;
	m_CurrentTexture = 0;
}

/*
===============
NAME: ~C_PaintJob

NOTE: Destructor
===============
*/
C_PaintJob::~C_PaintJob()
{

}

/*
===============
NAME: Create

NOTE: Creates paint material texture
===============
*/
bool C_PaintJob::Create(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults, XtId p_XtId, U32 vehicleIndex)
{
	RESOURCE_MEMORY_ID( "PaintJob" );
#if !defined(ENABLE_VEHICLE_CUSTOM_PAINT)
	return true;
#endif

	if (m_Created)
		return true;

	BIZ_ASSERT(vehicleIndex < ms_NumTextureResourcesVehicles);
	for (int iTex = 0; iTex < m_MaterialTextures.Size(); iTex++)
	{
		m_MaterialTextures[iTex] = &ms_PaintTextureResources[iTex][vehicleIndex];
	}

	m_Created = true;
	
	UpdateMaterial(p_pVehicle, p_pMaterialDefaults, p_XtId);

	UpdateMaterialTexture();

	return true;
}

/*
===============
NAME: Destroy

NOTE: Destroys paint material texture
===============
*/
void C_PaintJob::Destroy()
{
#if !defined(ENABLE_VEHICLE_CUSTOM_PAINT)
	return;
#endif

	//Display::Device()->SetTexture(11, NULL);

	for (int iTex = 0; iTex < m_MaterialTextures.Size(); iTex++)
		m_MaterialTextures[iTex] = NULL;

	m_CurrentTexture = 0;

	return;
}

Debug::Bool g_debugUpdatePaintMaterialTexture("Vehicle.Render.Material.Paint.UpdateMaterialTexture", false);

/*
===============
NAME: Update

NOTE: Updates paint job
===============
*/
void C_PaintJob::Update()
{
#if !defined(ENABLE_VEHICLE_CUSTOM_PAINT)
	return;
#endif

	if (g_debugUpdatePaintMaterialTexture)
		UpdateMaterialTexture();
}

Debug::Bool g_debugUseFirstPaintJobFound("Vehicle.Materials.Paint.UseFirstPaintJobFound", true);

Debug::Bool g_debugOverrideUndercoatAndScuffColour("Vehicle.Materials.Paint.OverrideDamageColours", true);

Debug::Colour g_debugUndercoatColour("Vehicle.Materials.Paint.UndercoatColour", 0.383f, 0.349f, 0.349f, 1.0f);
Debug::Colour g_debugScuffColour("Vehicle.Materials.Paint.ScuffColour", 0.658f, 0.624f, 0.624f, 1.0f);

/*
===============
NAME: UpdateMaterial

NOTE: Updates material from Sorcery
===============
*/
void C_PaintJob::UpdateMaterial(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults, XtId p_XtId, XtPaintJob* pPaintJobOverride)
{	
//MEMORY_TRACKER_PUSH("graphics.vehicle.material.paint");
	XtPaintJob* pPaintJob = NULL;

	if (p_pVehicle)
	{
		if (g_debugUseFirstPaintJobFound && p_pVehicle->m_PaintJobs.count > 0)
		{
			pPaintJob = p_pVehicle->m_PaintJobs[0];
		}
		else
		{
			for (int iPJ = 0; iPJ < p_pVehicle->m_PaintJobs.count; iPJ++)
			{
				if (p_pVehicle->m_PaintJobs[iPJ].Id() == p_XtId)
				{
					pPaintJob = p_pVehicle->m_PaintJobs[iPJ];
					break;
				}
			}
		}
	}

	if (!pPaintJob && p_pMaterialDefaults->m_DefaultPaintJob)
		pPaintJob = p_pMaterialDefaults->m_DefaultPaintJob;

	// check for override
	if (pPaintJobOverride)
		pPaintJob = pPaintJobOverride;

	if (!pPaintJob)
	{
		//MEMORY_TRACKER_POP("graphics.vehicle.material.paint");
		return;
	}

	if (!pPaintJob->m_ColourCurve || !pPaintJob->m_ReflectivityCurve || pPaintJob->m_Colours.count == 0)
	{
		//MEMORY_TRACKER_POP("graphics.vehicle.material.paint");
		return;
	}

	m_ColourCurve.Clear();
	m_ReflectivityCurve.Clear();
	m_Colours.Clear();
	
	if (g_debugOverrideUndercoatAndScuffColour)
	{
		m_UndercoatColour	= MathsStorage::Float4(g_debugUndercoatColour->r,
												 g_debugUndercoatColour->g,
												 g_debugUndercoatColour->b, 1.0f);
		m_ScuffColour		= MathsStorage::Float4(g_debugScuffColour->r,
												 g_debugScuffColour->g,
												 g_debugScuffColour->b, 1.0f);
	}
	else
	{
		m_UndercoatColour	= MathsStorage::Float4(pPaintJob->m_UndercoatColour.r,
												 pPaintJob->m_UndercoatColour.g,
												 pPaintJob->m_UndercoatColour.b, 1.0f);
		m_ScuffColour		= MathsStorage::Float4(pPaintJob->m_ScuffColour.r,
												 pPaintJob->m_ScuffColour.g,
												 pPaintJob->m_ScuffColour.b, 1.0f);
	}
	for (int iColour = 0; iColour < pPaintJob->m_Colours.count; iColour++)
	{
		m_Colours.PushBack(MathsStorage::Float3(pPaintJob->m_Colours[iColour].r,
												pPaintJob->m_Colours[iColour].g,
												pPaintJob->m_Colours[iColour].b));
	}

	m_ColourCurve = C_HermiteCurve(pPaintJob->m_ColourCurve);
	m_ReflectivityCurve = C_HermiteCurve(pPaintJob->m_ReflectivityCurve);

	//MEMORY_TRACKER_POP("graphics.vehicle.material.paint");
}

Debug::Float g_debugResprayMetallicity("Vehicle.Paint.ResprayMetallicity", 0.85f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: Respray

NOTE: Respray
===============
*/
void C_PaintJob::Respray(U32 p_Colour)
{
	//m_ColourCurve.Clear();
	m_Colours.Clear();
	
	U8 r = (p_Colour >> 16) & 0xff;
	U8 g = (p_Colour >> 8) & 0xff;
	U8 b = (p_Colour >> 0) & 0xff;
	float m = (float)g_debugResprayMetallicity;
	m_Colours.PushBack(MathsStorage::Float3(r / 255.0f, g / 255.0f, b / 255.0f));
	m_Colours.PushBack(MathsStorage::Float3(r / 255.0f * m, g / 255.0f * m, b / 255.0f * m));

	UpdateMaterialTexture();
}

/*
===============
NAME: UpdateMaterialTexture

NOTE: Updates material texture
===============
*/
void C_PaintJob::UpdateMaterialTexture()
{
#if !defined(ENABLE_VEHICLE_CUSTOM_PAINT)
	return;
#endif

	if (!m_Created)
		return;

	SwapTextures();

	//Display::Device()->SetTexture(11, NULL);
	
	Display::C_Texture2D* pTexture = GetMaterialTexture();

	Display::C_LockedRect locked = pTexture->Lock(0);
	
	DWORD* pData = (DWORD*)locked.pBits;
	//DWORD* pData	= (DWORD*)m_MaterialTexture.GetAddress();

	U32 width = pTexture->GetWidth(0);
	U32 height = pTexture->GetHeight(0);

	for (U32 iWidth = 0; iWidth < width; iWidth++)
	{			
		for (U32 iHeight = 0; iHeight < height; iHeight++)
		{
			float fHeightScale = 1.0f - (float)iWidth / (float)width;
			
			MathsStorage::Float4 texel(0, 0, 0, 0);

			if (iHeight == 0)
			{
				CalculatePaintColour(texel, fHeightScale);
				CalculatePaintReflectivity(texel, fHeightScale);
			}

			U8 r = (U8)(texel.x * 255.0f);
			U8 g = (U8)(texel.y * 255.0f);
			U8 b = (U8)(texel.z * 255.0f);
			U8 a = (U8)(texel.w * 255.0f);

			(*pData++) = (a << 24) + (r << 16) + (g << 8) + (b << 0);

			//pData += (locked.pitch - (width * sizeof(DWORD))) / 4;
		}
	}

	pTexture->Unlock(0);
}

Debug::Colour g_debugPaintColour1("Vehicle.Render.Material.Paint.PaintColour1", 1.0f, 0.015f, 0.015f, 1.0f);
Debug::Colour g_debugPaintColour2("Vehicle.Render.Material.Paint.PaintColour2", 0.251f, 0.0f, 0.007f, 1.0f);

/*
===============
NAME: CalculatePaintColour

NOTE: Calculates the paint colour
===============
*/
void C_PaintJob::CalculatePaintColour(MathsStorage::Float4& p_Texel, float p_Value)
{
	if (m_Colours.Size() == 0)
		return;

	float fNumColours	= (float)(m_Colours.Size() - 1);
	float fColour		= Maths::Lerp(0.0f, fNumColours, p_Value);
	float fFrac			= 0.0f;
	float fLerp			= modff(fColour, &fFrac);	// [Blur2PC] modf(float, float*) no longer resolves
	int iColour			= (int)fFrac;
	if (iColour == m_Colours.Size() - 1)
	{
		p_Texel.x = m_Colours[m_Colours.Size() - 1].x;
		p_Texel.y = m_Colours[m_Colours.Size() - 1].y;
		p_Texel.z = m_Colours[m_Colours.Size() - 1].z;
		return;
	}

	fLerp = Maths::Clamp(m_ColourCurve.Evaluate(fLerp), 0.0f, 1.0f);

	MathsStorage::Float3 colour1 = m_Colours[iColour];
	MathsStorage::Float3 colour2 = m_Colours[iColour+1];

	p_Texel.x = Maths::Lerp(colour1.x, colour2.x, fLerp);
	p_Texel.y = Maths::Lerp(colour1.y, colour2.y, fLerp);
	p_Texel.z = Maths::Lerp(colour1.z, colour2.z, fLerp);
}

Debug::Float g_debugPaintReflectivityMin("Vehicle.Render.Material.Paint.MinReflectivity", 0.15f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugPaintReflectivityMax("Vehicle.Render.Material.Paint.MaxReflectivity", 0.65f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: CalculatePaintReflectivity

NOTE: Calculates the paint reflectivity
===============
*/
void C_PaintJob::CalculatePaintReflectivity(MathsStorage::Float4& p_Texel, float p_Value)
{
	p_Texel.w = Maths::Clamp(m_ReflectivityCurve.Evaluate(p_Value), 0.0f, 1.0f);
}


//PaintTexture Resources
Display::C_Texture2D	C_PaintJob::ms_PaintTextureResources[NUM_PAINT_TEXTURES][20];
U32				C_PaintJob::ms_NumTextureResourcesVehicles = 0;

bool C_PaintJob::CreatePaintTextures( U32 numVehicles )
{
	ms_NumTextureResourcesVehicles = numVehicles;
	//MEMORY_TRACKER_PUSH("graphics.vehicle.material.paint");
	for (U32 j = 0; j < ms_NumTextureResourcesVehicles; j++)
	{
		for (int iTex = 0; iTex < NUM_PAINT_TEXTURES; iTex++)
		{
			if (!Display::Device()->CreateTexture2D(512, 1 , 1, Display::E_Format_Linear | Display::E_Format_SRGB, Display::E_Format_A8R8G8B8, &ms_PaintTextureResources[iTex][j]))
			{
				MEMORY_TRACKER_POP("graphics.vehicle.paint");
				return false;
			}
		}
	}
	//MEMORY_TRACKER_POP("graphics.vehicle.material.paint");
	return true;
}

void C_PaintJob::DestroyPaintTextures()
{
	for (U32 j = 0; j < ms_NumTextureResourcesVehicles; j++)
	{
		for (int iTex = 0; iTex < NUM_PAINT_TEXTURES; iTex++)
		{
			Display::Device()->DestroyTexture2D(&ms_PaintTextureResources[iTex][j]);
		}
	}
	ms_NumTextureResourcesVehicles = 0;
}

} //namespace Vehicle

