
#ifndef IPF_SHAPE_BUILDER
#define IPF_SHAPE_BUILDER

#include "Defines.h"
#include "ui/Illustrator/UIAdobePolygon.h"

namespace Ipf
{

class C_ShapeBuilder
{
public:
	static C_ShapeBuilder&	Instance()
							{
								return m_sInstance;
							}

	void					BuildOptions( const F32 fCurveStep, const bool bFixedSpacedStrokes, const bool bDisableFinning )
							{
								m_CurveStep = fCurveStep;
								m_FixedSpacedStrokes = bFixedSpacedStrokes;
								m_bDisableFinning = bDisableFinning;
							}

	void					SetSceneBounds( const Maths::C_AxisBox& bBounds )
							{
								m_SceneBounds = bBounds;
							}

	const bool				BuildPath( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon, const char* pName, const Maths::C_AxisBox& bBounds );

	const bool				BuildCompoundPath( const AIArtHandle pGroup, UI::C_AdobePolygon* pPolygon, const char* pName, const Maths::C_AxisBox& bBounds );

	void					CalculateBoundingBox( UI::TVerticies& vVerticies, Maths::C_AxisBox& bBounds, const bool bClear = true );

	void					CalculateScalarAmounts( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon );

	void					NormaliseVerticies( UI::TVerticies& vVerticies, const Maths::C_AxisBox& bBounds );

private:
	void					GetSegments( AIArtHandle pPath, UI::AdobePointList& vSegments, const Maths::C_AxisBox& bBounds, const bool bOffsetOutline = false );

	const bool				EvaluateFillSegments( const UI::AdobePointList& vSegments, const bool bClosed, UI::TVerticies& vVerticies );

	const bool				EvaluateStrokeSegments( const UI::TVerticies& vSegments, const bool bClosed, const F32 fWidth, UI::TVerticies& vStroke );

	const bool				EvaluateFixedSpaceStrokeSegments( const UI::AdobePointList& vSegments, const F32 fSpacing, UI::TVerticies& vStroke );

	void					GetOutlinePoint( Maths::HVec3& p1, Maths::HVec3& p2, Maths::HVec3& p3, Maths::HVec3& p, F32 length, float z );

	void					BuildCorner( Maths::HVec3& p1, Maths::HVec3& p2, Maths::HVec3& p3, Maths::HVec3& r1, Maths::HVec3& r2, UI::TVerticies& vStroke, F32 length, bool& extraPoint );

	const F32				BezierLength(const Maths::HVec3& b0, const Maths::HVec3& b1, const Maths::HVec3&b2, const Maths::HVec3& b3 );

	const bool				LineIntersect( const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& q1, const Maths::HVec2& q2, F32& a, F32& b, const bool bSegmentTest );

	void					SetBlendStyle( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon );

	const bool				SetFillStyle( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon );

	const bool				SetStrokeStyle( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon );

	void					Triangulate( UI::UIPolygonList& results, UI::UIPolygonIndexList& indices );

	void					DoSoftwareAA_Shape( UI::UIPolygonList& vertexList, UI::UIPolygonList& resolvedList, UI::UIPolygonIndexList& indexList );

	S32						GetCurveStepOverride( Util::C_String& name );

private:
	static C_ShapeBuilder	m_sInstance;

	F32						m_CurveStep;
	bool					m_FixedSpacedStrokes;
	bool					m_bDisableFinning;

	Maths::C_AxisBox		m_SceneBounds;
};

}; // namespace Ipf


#endif // IPF_SHAPE_BUILDER