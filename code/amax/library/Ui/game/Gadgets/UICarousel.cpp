#include "Stdafx.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <Ui/Graphics/SimpleTimer.h>

#include "UICarousel.h"
#include "Ui/SoundEffects.h"
#include "ui/Game/Gadgets/UINodeList.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UICarousel, "UICarousel" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_NumItems, "num_items" )
	XT_FIELD( m_FoldAppear, "fold_appear" )
XT_END_TYPE

namespace UI
{

DECLARE_DEBUG_BOOL( g_UiCarouselCircleStyle, "UI.Carousel.CircleType", false );
DECLARE_DEBUG_BOOL( g_UiCarouselFlipRot, "UI.Carousel.FlipRot", true );
DECLARE_DEBUG_FLOAT( g_UiCarouselHackNum, "UI.Carousel.MegaHack", -40.0f );
DECLARE_DEBUG_FLOAT( g_UiCarouselHackNum2, "UI.Carousel.MegaHack2", 25.0f);

DECLARE_DEBUG_FLOAT( g_UiCarouselBase, "UI.Carousel.Base", 2.1f, Debug::Range( 0.0f, 10.0f ));
DECLARE_DEBUG_FLOAT( g_UiCarouselFactor, "UI.Carousel.Factor", 1.0f, Debug::Range( -10.0f, 10.0f ));
DECLARE_DEBUG_FLOAT( g_UiCarouselScaleX, "UI.Carousel.ScaleX", 1.0f, Debug::Range( 0.0f, 10.0f ));

DECLARE_DEBUG_FLOAT( g_UiCarouselRotationRate, "UI.Carousel.RotationRate", 90.0f, Debug::Range( 0.0f, 200.0f ) );
DECLARE_DEBUG_FLOAT( g_UiCarouselRotationDamp, "UI.Carousel.RotationDamp", 12.0f, Debug::Range( 0.0f, 100.0f ) );
DECLARE_DEBUG_FLOAT( g_UiCarouselMovedTime, "UI.Carousel.MovedTime", 0.5f, Debug::Range( 0.0f, 4.0f ) );

void UICarousel::Init()
{
	C_GadgetBase::Init();

	m_MovedTime = 0.0f;
	m_Angle = 0.0f;
	m_AngleVel = 0.0f;
	m_SelectedItem = 0;

	AddSubscriber( E_UiMsg_FrameUpdate );
//	AddSubscriber( E_UiMsg_CtrlButton_LeftShoulder );
//	AddSubscriber( E_UiMsg_CtrlButton_RightShoulder );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
}

void UICarousel::PostInit()
{
	BranchScan();
	C_GadgetBase::PostInit();
}

void UICarousel::BranchScan()
{
	FindChildren( &m_Children, false, false, E_UIButtonType_Node );

	for( S32 i = 0; i < m_Children.Size(); i++ )
		m_Children[ i ]->RemoveSubscriber( E_UiMsg_Render );

	m_SortedRef.Resize( m_NumItems );
	for( S32 i = 0; i < m_NumItems; i++ )
		m_SortedRef[ i ] = i;

	m_NodeRef.Resize( m_NumItems );
	for( S32 i = 0; i < m_NumItems; i++ )
		m_NodeRef[ i ] = 0;

	m_NodePositions.Resize( m_NumItems );
	m_NodeRotations.Resize( m_NumItems );
	UpdatePositions();

	DepthSort();
}

void UICarousel::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );
}

void UICarousel::MoveRight()
{
	const F32 step = ( m_Children.Size() == 0 ) ? 0.0f : Maths::Pi2 / (F32)(m_NumItems-1);

	m_Angle -= step;
	if( m_SelectedItem == m_Children.Size()-1 )
		m_SelectedItem=0;
	else
		m_SelectedItem++;

	m_MovedTime = g_UiCarouselMovedTime;

	UiInterface()->PlaySound( E_UiSoundEffect_CarouselRightActionIn );
}

void UICarousel::MoveLeft()
{
	const F32 step = ( m_Children.Size() == 0 ) ? 0.0f : Maths::Pi2 / (F32)(m_NumItems-1);

	m_Angle += step;
	if( m_SelectedItem == 0)
		m_SelectedItem = m_Children.Size()-1;
	else
		m_SelectedItem--;

	m_MovedTime = g_UiCarouselMovedTime;

	UiInterface()->PlaySound( E_UiSoundEffect_CarouselLeftActionIn );
} 

bool UICarousel::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	switch( pMessage->Type() )
	{
	case E_UiMsg_CtrlButton_Right:
		if( m_Activated && m_Selected )
		{
			MoveRight();
		}
		break;

	case E_UiMsg_CtrlButton_Left:
		if( m_Activated && m_Selected )
		{
			MoveLeft();
		}
		break;	

	case E_UiMsg_Render :
		C_GadgetBase::MessageHandler( pMessage );

		if( !m_Activated )
			return true;

		UpdatePositions();
		DepthSort();

		for( S32 i = 0; i < m_NumItems; i++ )
		{
			const S32 sorted = m_SortedRef[ i ];
			const S32 node = m_NodeRef[ sorted ];

			if( node < m_Children.Size() )
			{
				m_Children[ node ]->m_Position = m_NodePositions[ sorted ];
				m_Children[ node ]->m_Rotation.y = m_NodeRotations[ sorted ];
				m_Children[ node ]->MessageHandler( pMessage );

				((UIRenderBranch*)m_Children[ node ])->DirtyMatrix();
			}
		}
		break;

	default:
		return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

F32 RationaliseAngle(F32 t)
{
	return fmodf(t + Maths::Pi, Maths::Pi2) - Maths::Pi;
}
void UICarousel::UpdatePositions()
{
	S32 num_items = m_NumItems-1;
	S32 half = num_items/2;
	// Set item positions
	F32 fstep = Maths::Pi2 / (F32)num_items * m_Size.y;
	F32 ra = m_Angle + (fstep * half);
	Maths::HMat33 scale( Maths::C_MatrixScale((F32)g_UiCarouselScaleX, 1.0f, 1.0f ));
	Maths::HVec3	offset( 0.0f, 0.0f, -m_Size.x );
	F32 k = g_UiCarouselBase - g_UiCarouselFactor;

	for( S32 i = 0; i < m_NumItems; i++, ra -= fstep )
	{
		Maths::HMat33	rot( Maths::RotY((F32)ra) );

		rot = rot * scale;

		if( g_UiCarouselCircleStyle )
		{
			m_NodePositions[ i ] = m_Position + (offset * rot);
		}
		else
		{
			m_NodePositions[ i ] = m_Position + offset + Maths::HVec3((F32)(ra*g_UiCarouselHackNum),0.0f,(F32)(fabsf(RationaliseAngle(ra))*g_UiCarouselHackNum2));
		}
		
		if( g_UiCarouselFlipRot )
		{
			if( m_Size.x > 0.0f )
			{
				m_NodeRotations[ i ] = sin(ra) * -90.0f * m_Size.z;
			}
		}
		else
		{
			m_NodeRotations[ i ] = Maths::RadToDeg( ra ) * m_Size.z;
		}

		if( i == half-1 && m_FoldAppear > 0.0f )
			ra += (m_FoldAppear * fstep);
	}

	// Create node reference list
	if( m_Children.Size() > 0 )
	{
		S32 index = m_SelectedItem - ((m_NumItems-1)/2);
		while( index < 0 )
			index += m_Children.Size();

		for( S32 i = 0; i < m_NumItems; i++ )
		{
			m_NodeRef[ i ] = index;
			index++;
			while( index >= m_Children.Size() )
				index -= m_Children.Size();
		}
	}
}

void UICarousel::DepthSort()
{
	bool sorting = true;
	while( sorting )
	{
		sorting = false;
		for( S32 a = 0; a < m_NumItems-1; a++ )
		{
			if( m_NodePositions[ m_SortedRef[ a ] ].Z() < m_NodePositions[ m_SortedRef[ a+1 ] ].Z() )
			{
				S32 tmp = m_SortedRef[ a ];
				m_SortedRef[ a ] = m_SortedRef[ a+1 ];
				m_SortedRef[ a+1 ] = tmp;

				sorting = true;
			}
		}
	}
}

struct UpdateAnimAccel
{
	F32 target;
	F32 rate;
	F32 damp;

public:
	UpdateAnimAccel(F32 target_, F32 rate_, F32 damp_) :
		target(target_),
		rate(rate_),
		damp(damp_)
	{
	}

public:
	inline F32 operator()(const F32& position, const F32& velocity)
	{
		return ((target - position) * rate) - (velocity * damp);
	}
};

template<typename Type, typename Method>
void Integrate_LeapFrog(Type& position, Type& velocity, Method accMethod, F32 time)
{
	F32 halfTime = time * 0.5f;

	velocity += accMethod(position, velocity) * halfTime;
	position += velocity * time;
	velocity += accMethod(position, velocity) * halfTime;
}

void UICarousel::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();

	if( !m_Activated )
		return;

	if( m_MovedTime > 0.0f )
	{
		m_MovedTime -= theFETimer.FrameTime();
		if( m_MovedTime < 0.0f )
			m_MovedTime = 0.0f;
	}

	bool update = false;
	if( m_Angle != 0.0f )
	{
		/*
		m_Angle *= 0.98f;
		*/

		Integrate_LeapFrog< F32 >( m_Angle, m_AngleVel, UpdateAnimAccel( 0.0f, g_UiCarouselRotationRate, g_UiCarouselRotationDamp ), theFETimer.FrameTime() );

//		if( Maths::Abs( m_Angle ) < 0.001f )
//			m_Angle = 0.0f;

		update = true;
	}
	else
	{
		update = (m_Size.y != 1.0f);
	}
}

// Selection interface
const bool	UICarousel::SetCurrentItemByIndex( const S32 index )
{
	m_SelectedItem = (S32)index;
	return false;
}

const bool	UICarousel::SetCurrentItemByValue( const F64 value )
{
	// Find value in child nodes
	for( S32 i = 0; i < m_Children.Size(); i++ )
	{
		UINode* pNode = (UINode*)m_Children[ i ];
		if( pNode )
			if( (F64)pNode->m_Value == value )
			{
				const F32 step = ( m_Children.Size() == 0 ) ? 0.0f : Maths::Pi2 / (F32)m_Children.Size();
				
				S32 diff = m_SelectedItem - i;
				m_Angle += (F32)diff * step;
				m_SelectedItem = (S32)i;
				return true;
			}
	}
	return false;
}

const S32	UICarousel::GetCurrentItemIndex() const
{
	return m_SelectedItem;
}

const F64	UICarousel::GetCurrentItemValue( const U32 arg ) const
{
	if( arg == 0 )
		return m_MovedTime;

	if( m_SelectedItem < m_Children.Size() )
	{
		UINode* pNode = (UINode*)m_Children[ m_SelectedItem ];
		if( pNode )
			return (F64)pNode->m_Value;
	}

	return 0.0;
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
