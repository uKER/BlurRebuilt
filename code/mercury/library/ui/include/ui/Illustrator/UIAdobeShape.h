
#ifndef UI_UIADOBESHAPE_H
#define UI_UIADOBESHAPE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Illustrator/UIAdobeElement.h"
#include "ui/Illustrator/UIAdobeColour.h"
#include "ui/Illustrator/UIAdobePoint.h"

#include "ui/Illustrator/Ipf/IpfReader.h"

namespace UI
{

/// @addtogroup Illustrator
/// @{

// Forward declarations
class C_UITriListRenderInfo;

typedef Btl::Vector<C_UITriListRenderInfo*> TRenderInfoList;

//-------------------------------------------------------------------
// Class : C_AdobeShape
//-------------------------------------------------------------------
/// Shape that belongs to an AdobeScene.
class C_AdobeShape : public C_AdobeElement
{
public:
									C_AdobeShape();		///< @a

									~C_AdobeShape();	///< @a

									/// Returns true if this shape has fill render data.
	const bool						HasFill() const			
									{ 
										return m_FillStyle.Kind() != C_AdobeFillStyle::E_FillStyle_Kind_None; 
									}

									/// Returns true if this shape has outline render data.
	const bool						HasStroke() const		
									{ 
										return m_StrokeStyle.Width() != 0.0f; 
									}

									/// Returns a local bounding box in 0.0f - 1.0f
	const Maths::C_AxisBox&			BoundingBox() const		
									{ 
										return m_BoundingBox; 
									}

									/// Returns the top left offset(0.0-1.0f) based on its location in the group
	const Maths::HVec2&				ScalarOffset() const	
									{ 
										return m_ScalarOffset; 
									}

									/// Returns the scale(0.0-1.0f) based on its size propotion in the group
	const Maths::HVec2&				ScalarValue() const		
									{ 
										return m_ScalarValue; 
									}

									/// @a
	const C_AdobeFillStyle&			FillStyle() const		
									{ 
										return m_FillStyle; 
									}

									/// @a
	const C_AdobeStrokeStyle&		StrokeStyle() const		
									{ 
										return m_StrokeStyle; 
									}

									/// @a
	const C_AdobeBlendStyle&		BlendStyle() const		
									{ 
										return m_BlendStyle; 
									}

									/// @a
	const TRenderInfoList&			Fill() const			
									{ 
										return m_Fill; 
									}
									
									/// @a
	const TRenderInfoList&			Stroke() const			
									{
										return m_Stroke; 
									}

									/// @a
	void							AddFill( C_UITriListRenderInfo* pFill )
									{
										m_Fill.PushBack( pFill );
									}

									/// @a
	void							AddStroke( C_UITriListRenderInfo* pStroke )
									{
										m_Stroke.PushBack( pStroke );
									}

									/// @a
	void							Deserialise( Ipf::IpfDeserialiser& stream );


/*-------------------------------------------------------------------
	Private Data
--------------------------------------------------------------------*/
private:
	Maths::C_AxisBox				m_BoundingBox;			///< Local shape bounds in 0.0f-1.0f base on parents size. E.g Group bounds
	Maths::HVec2					m_ScalarOffset;			///< Shape offset in 0.0f-1.0f base on parent top left.
	Maths::HVec2					m_ScalarValue;			///< Shape scale in 0.0f-1.0f base on parent size.

	C_AdobeFillStyle				m_FillStyle;			///< Contains if and how to surface the shape
	C_AdobeStrokeStyle				m_StrokeStyle;			///< Contains if and how to draw the outline
	C_AdobeBlendStyle				m_BlendStyle;			///< Contains the blend mode and alpha amount

	// Pointers to shape data.
	TRenderInfoList					m_Fill;					///< Compound paths are made up of multiple paths so they could be more 
	TRenderInfoList					m_Stroke;				///< than one vertex buffer per shape.
};

/// @}

} // namespace UI

#endif // UI_UIADOBESHAPE_H
