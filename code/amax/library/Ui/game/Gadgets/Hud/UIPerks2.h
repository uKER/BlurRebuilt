#ifndef UIPERKS_H
#define UIPERKS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <scenegraph\SceneGraph.h>
#include <ui/Game/Gadgets/Buttons.h>	// [Blur2PC] C_GadgetBase

#include <ui/Game/Gadgets/ButtonController.h>
#include "Ui/UIManager.h"
#include "UIPerks.h" // C_UIPerkInfo

#include "UI/Game/Multiplayer/MultiplayerDefines.h"

namespace Game
{
	namespace Actors
	{
		class C_VehicleLogicActor;
		class C_VehicleActor;
	}
}

namespace UI
{

// -----------------------
// C_UiPerkSlotEffectParam
// -----------------------
class C_UiPerkSlotEffectParam
{
public:
	C_UiPerkSlotEffectParam()
		: m_fScale( 1.0f )
		, m_fAlpha( 1.0f )
		, m_fModScale( 0.0f )
		, m_fModAlpha( 0.0f )
		, m_fBlobScale( 0.0f )
		, m_fBlobAlpha( 0.0f )
	{
		//
	}

public:
	F32		m_fScale;
	F32		m_fAlpha;
	F32		m_fModScale;
	F32		m_fModAlpha;
	F32		m_fBlobScale;
	F32		m_fBlobAlpha;
};

// -------
// UIPerks
// -------
class UIPerks2 : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Perks2, "PERKS2" )

public: // exposed for Xt
	//

public:
	void 			Init();
	bool 			MessageHandler( const Messaging::C_MessageBase* pMessage );
	void			FrameUpdate();
	void 			Render();
	void			Setup( lua_State* pState );
	void			Exit();
#if PLATFORM_PC
	// [Blur2PC] world transform that moves the car-attached perks sprites (built at the update pose) to the
	// car's render pose for the frame being rendered; false when there is no such gadget
	static bool		Blur2PCCarPanelDelta( Maths::HMat43& delta );
#endif

private:
	C_UIPerkInfo*	GetPerkInfo( const E_PICKUP_TYPE type, const S32 sAmount = 0, const bool bHasExtraBoltMod = false );
	F32				GetWidth();
	S32				GetNumSlots();
	bool			CanDoubleDip();
	void			UpdateActivatingIn();

public:
	// Exposed for Xt
	bool				m_AttachedToCar;

	XtF32				m_FoldoutScale;
	XtF32				m_ScaleAmount;
	XtF32				m_ScaleSpeed;
	XtF32				m_Padding;
	XtF32				m_ItemSize;
	XtF32				m_DrawAspect;
	XtF32				m_ScaleEpsilon;

	XtF32				m_AngleStep;
	XtF32				m_SlotStep;
	XtF32				m_Bender;

	E_Effect			m_SelectedEffectIndex;

	XtF32				m_TextHeight;
	C_Vector3Direction	m_TextOffset;

	XtArray< C_UiPerkSlotEffectParam >		m_SlotEffect;

private:
	void				AddPerk( const E_PICKUP_TYPE type );
	void				RemovePerk();

private:
	struct C_Item
	{
		F32				m_Advance;
		F32				m_Scale;

		void Init()
		{
			m_Advance = 0.0f;
			m_Scale = 1.0f;
		}

		void FrameUpdate( const F32 ft, const F32 speed, const F32 target_scale )
		{
			m_Advance -= (m_Advance * ft * speed);
			m_Scale += (target_scale - m_Scale) * ft * speed;
		}
	};

	S32									m_CurrentItem;
	S32									m_NumItems;
	S32									m_sLocalPlayerIndex;
	F32									m_fPickupActivatedIn;

	VehiclePickupInfo*					m_pPickupInfo;
	Game::Actors::C_VehicleLogicActor*	m_pLogicActor;
	Game::Actors::C_VehicleActor*		m_pVehicleActor;
	Btl::Vector< C_Item >				m_Items;
	Maths::HVec3						m_Origin;
	C_RenderState						m_SelectedRS;

	Maths::HMat43						m_PrevMatrix;
#if PLATFORM_PC
	Maths::HMat43						m_Blur2PCBuiltMatrix;	// [Blur2PC] car matrix the sprites were last built with
	bool								m_Blur2PCBuilt;
	static UIPerks2*					s_pBlur2PCCarGadget;
#endif
	Maths::HVec3						m_Vel;
	U32									m_LightVisibilityFlags;

	SceneGraph::C_PointLightHandle		m_PointLightHandle;

	Maths::C_Quaternion m_PrevQuat;
	Maths::HVec3 s_prevIdealDir;

	F32 PrevAngle;

	bool								m_LastGaspActive;
	bool								m_ScramblerActive;
	bool								m_bHasExtraBoltMod;
	bool								m_bPickupActivated;

	Btl::Vector< E_MultiplayerVehicleAbilities >	m_GiftedAbility;

	C_UISpriteColours					m_ColourMod1;
	C_UISpriteColours					m_ColourMod2;
	C_UISpriteColours					m_ColourMod3;
	C_UISpriteColours					m_ColourModSp;
};

}; // namespace UI

#endif /* UIPERKS_H */
