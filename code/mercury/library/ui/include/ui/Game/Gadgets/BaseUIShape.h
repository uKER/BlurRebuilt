
#ifndef UI_BASE_SHAPE_H
#define UI_BASE_SHAPE_H

#include "ui\Game\Gadgets\Buttons.h"

#include "ui\Illustrator\UIAdobeSceneManager.h"
#include "ui\Illustrator\UIAdobeShape.h"
#include "ui\Illustrator\UIAdobeGroup.h"
#include "ui\Illustrator\UIAdobeScene.h"

namespace UI
{
// Forward declartion
class C_AdobeShape;

enum E_MatrixRotationOrder
{
	E_MatrixRotationOrder_TSR = 0,	///< Transformation, Scale, Rotation.
	E_MatrixRotationOrder_RTS,		///< Rotation, Transformation, Scale. 
};

enum E_StrokeEffect
{
	E_StrokeEffectAlpha							= E_Effect_PolygonStrokeAlpha,
	E_StrokeEffectIntensityAsAlpha				= E_Effect_PolygonStrokeIntensityAsAlpha,
	E_StrokeEffectTrail							= E_Effect_PolygonTrail,
	E_StrokeEffect_AM_AudioEqAlpha				= E_Effect_AM_AudioEqAlpha,			
	E_StrokeEffect_AM_AudioEqIntensityAsAlpha	= E_Effect_AM_AudioEqIntensityAsAlpha,
};

/*---------------------------------------------------------------------------------------
	class : BaseUIShape
---------------------------------------------------------------------------------------*/
class BaseUIShape : public C_GadgetBase
{
public:
	virtual void					Init();
	virtual void					Setup( lua_State* pState );
	virtual void 					FrameUpdate();

	virtual void					SetVertexParams( const F32 min, const F32 max )
									{
										m_StrokeMinVertexParam = min;
										m_StrokeMaxVertexParam = max;
									}

	virtual void					RenderFill( const C_AdobeShape* pShape, const Maths::HVec3& vPos, const F32 fSizeX, const F32 fSizeY, const TransformIndex uColourIndex );

	virtual void					RenderStroke( const C_AdobeShape* pShape, const Maths::HVec3& vPos, const F32 fSizeX, const F32 fSizeY, const TransformIndex uColourIndex );

	const bool 						ChangeSceneName( const char* pName );

	const bool 						ChangeObjectName( const char* pName );

	Maths::C_AxisBox				SceneBounds() const { return m_SceneBounds; }

protected:
	const C_AdobeShape*				FindShape( const char* name ) const;

	const bool 						ResolveSceneName();
	const bool						ResolveObjectName();

	const bool						HasAlpha() const;

	void							SetBlendMode( const C_AdobeShape* pShape );

/// exposed for xt.
public:
	XtSz8							m_SceneName;
	XtSz8							m_ObjectName;
	E_Justify						m_ShapeJustification;
	E_StrokeEffect					m_StrokeEffect;
	bool							m_bFinnedOuter;
	bool							m_UseGroupBounds;

	F32								m_StrokeMinVertexParam;
	F32								m_StrokeMaxVertexParam;
	
protected:
	static const U32				m_SceneInvalid = 0xffffffff;

	C_AdobeScene::AdobeRenderList	m_ShapeRenderList;

	Maths::C_AxisBox				m_SceneBounds;
	const C_AdobeHandle*			m_pSceneHandle;
};

}; // namespace UI

#endif // UI_BASE_SHAPE_H