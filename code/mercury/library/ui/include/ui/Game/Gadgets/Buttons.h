#ifndef BUTTONS_H
#define BUTTONS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>
#include <Xt/Impl/XtUt.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <btl/Containers/containers.h>

#include "ui/Graphics/Simple2d.h"
#include "ui/Game/UIScreenControl.h"
#include "ui/Game/UITimer.h"
#include "ui/Game/Effects.h"
#include "ui/Game/Timeline.h"
#include "ui/Game/UiXt.h"

#define _GADGET_BASE( _TYPE, _TYPESTRING ) \
	virtual const UIButtonTypeID	GetType()			{ return _TYPE; }			\
	static const UIButtonTypeID		TypeID()			{ return _TYPE; }			\
	static const Util::C_String32	TypeName()			{ return _TYPESTRING; }		\
	virtual const char*				TypeNameVirtual()	{ return _TYPESTRING; }

#define _GADGET_INSTANTIATE( )					virtual	void StoreXtType()	{ m_pXtType = GetXtTypeImpl( *this ); }

#define DECLARE_GADGET( _TYPE, _TYPESTRING )	\
	_GADGET_BASE( _TYPE, _TYPESTRING )	\
	_GADGET_INSTANTIATE( )

namespace UI
{

class C_UISpriteColours;
class C_TextureParam;
class C_GadgetBase;

typedef Btl::List< C_GadgetBase* >			ButtonList;
typedef Btl::Vector< C_GadgetBase* >		ButtonArray;
typedef S32									C_ButtonHandle;

class C_GadgetBaseData
{
public:
	// So xt can access the private members.
	XT_FRIEND_T( C_GadgetBaseData );

	// Enumerations
	enum E_Flag // E_Flags shares storage with E_Dirty, they must not overlap
	{
		E_Flag_ExportToLua			= 0x00000001,
		E_Flag_InheritAlpha			= 0x00000002,
		E_Flag_InheritIntensity		= 0x00000004,
		E_Flag_InheritRotation		= 0x00000008,
		E_Flag_InheritScale			= 0x00000010,
		E_Flag_InheritTranslation	= 0x00000020,
		E_Flag_BoundsCulling		= 0x00000040,
		E_Flag_Mask					= 0x0000FFFF
	};
	enum E_Dirty // E_Dirty shares storage with E_Flags, they must not overlap
	{
		E_Dirty_Colour	= 0x00010000,
		E_Dirty_Alpha	= 0x00020000,
		E_Dirty_Matrix	= 0x00040000,
		E_Dirty_Mask	= 0xFFFF0000
	};
	enum E_InternalFlag
	{
		E_InternalFlag_HasColourBinding = 0x00000001,
	};

	// Classes
#ifdef PLATFORM_PC
	class C_MouseData
	{
	public:
		XtI32	clickable;
		bool	isover;
		bool	wasover;
		XtF32	bb_top;
		XtF32	bb_bottom;
		XtF32	bb_left;
		XtF32	bb_right;
		S32		lclick_msg;
		S32		rclick_msg;
		S32		isover_msg;
	};
#endif
	class C_VectorBase
	{
	public:
		C_VectorBase( )
		{

		}

		C_VectorBase( const F32 _x, const F32 _y, const F32 _z ) :
			x( _x ),
			y( _y ),
			z( _z )
		{

		}

		operator			Maths::HVec3( ) const		
		{ 
			return Maths::HVec3( x, y, z ); 
		}

		operator			MathsStorage::Float3( ) const
		{
			return MathsStorage::Float3( x, y, z );
		}

		C_VectorBase& operator=(const Maths::HVec3& other )
		{ 
			x = other.X(); 
			y = other.Y(); 
			z = other.Z();

			return *this;
		}

		XtF32		x;
		XtF32		y;
		XtF32		z;
	};

	class C_Vector3Position : public C_VectorBase
	{
	public:
		C_Vector3Position( const F32 x = 0.0f, const F32 y = 0.0f, const F32 z = 0.0f ) : C_VectorBase( x, y, z )
		{
		}

		C_Vector3Position& operator=(const Maths::HVec3& other )
		{ 
			x = other.X(); 
			y = other.Y(); 
			z = other.Z();

			return *this;
		}
	};

	class C_Vector3Direction : public C_VectorBase
	{
	public:
		C_Vector3Direction( const F32 x = 0.0f, const F32 y = 0.0f, const F32 z = 0.0f ) : C_VectorBase( x, y, z )
		{
		}

		C_Vector3Direction& operator=(const Maths::HVec3& other )
		{ 
			x = other.X(); 
			y = other.Y(); 
			z = other.Z();

			return *this;
		}
	};

	class C_Vector3Rotation : public C_VectorBase
	{
	public:
		C_Vector3Rotation( const F32 x = 0.0f, const F32 y = 0.0f, const F32 z = 0.0f ) : C_VectorBase( x, y, z )
		{
		}
	};

	class C_Vector3Scale : public C_VectorBase
	{
	public:
		C_Vector3Scale( const F32 x = 1.0f, const F32 y = 1.0f, const F32 z = 1.0f ) : C_VectorBase( x, y, z )
		{
		}
	};

	class C_Vector3Size : public C_VectorBase
	{
	public:
		C_Vector3Size( const F32 x = 0.0f, const F32 y = 0.0f, const F32 z = 0.0f ) : C_VectorBase( x, y, z )
		{
		}
	};

	// Helper used for culling
	class C_Rect
	{
	public:
		C_Rect( const F32 _top, const F32 _left, const F32 _bottom, const F32 _right ) :
			left( _left ),
			right( _right ),
			top( _top ),
			bottom( _bottom )
		{
		}
		C_Rect( ) :
			top( FLT_MAX ), bottom( -FLT_MAX ), left( FLT_MAX ), right( -FLT_MAX )
		{

		}

		void UpdateExtents( const Maths::C_Vertex& v )
		{
			// X
			if( left > v.X() )
				left = v.X();
			if( right < v.X() )
				right = v.X();
			// Y
			if( top > v.Y() )
				top = v.Y();
			if( bottom < v.Y() )
				bottom = v.Y();
		}
		const bool		Overlaps( const C_Rect& _in );

		static void		ResetCulling()
		{
			m_sCullingInstance.left		= gSimple2D.ScreenLeft();
			m_sCullingInstance.right	= gSimple2D.ScreenRight();
			m_sCullingInstance.top		= 0.0f;
			m_sCullingInstance.bottom	= UI_VSCREEN_SIZEY;
		}

		static C_Rect&	CullingInstance()		{ return m_sCullingInstance; }

	public:
		F32				left;
		F32				right;
		F32				top;
		F32				bottom;

	private:
		static C_Rect	m_sCullingInstance;
	};

public:
	// Functions
	C_GadgetBaseData( C_GadgetBase* self );

	virtual ~C_GadgetBaseData( );

	const bool				ExportToLua( ) const							{ return FlagSet( E_Flag_ExportToLua ); }
	const bool				DirtySet( const E_Dirty dirty ) const			{ return (m_Flags & dirty) != 0; }
	void						DirtySet( const E_Dirty dirty, const bool set )	{ m_Flags = set ? (m_Flags|dirty) : (m_Flags & ~dirty); }
	const bool				FlagSet( const E_Flag flag ) const				{ return (m_Flags & flag) != 0; }
	void					FlagSet( const E_Flag flag, const bool set )	{ m_Flags = set ? (m_Flags|flag) : (m_Flags & ~flag); }
	const bool				IsTransformed( ) const;
	Maths::HMat43			TransformMatrix( const bool scale = true, const bool rotate = true, const bool translate = true );
	void					SetName( const char *name );
	void					SetParent(C_GadgetBase* parent);
	void					SetColourStyle( const char *colour_style );
	void					SetTextureCapacity( const S32 capacity );
	void					SetTimerCapacity( const S32 capacity );
	C_GadgetBase*			Parent() const { return m_pParentGadget; }
	XtUt::LinkList<C_GadgetBase>* ParentChildList() { return &m_ParentChildList; }
	static void				SorceryCallback( XtEventType type, XtRecord* pRecord, C_GadgetBaseData* pUIScreen, XtCallbackContext* pContext );

	void					BindColourStyle( );
	void					ResolveColourStyle( const F32 intensity, const F32 alpha );

	virtual void			Select( const bool bSelect )		{ m_Selected = bSelect; }
	virtual void			Activate( const bool bActivate )	{ m_Activated = bActivate; }

	const bool				IsSelected( ) const					{ return m_Selected; }
	const bool				IsActivated( ) const				{ return m_Activated; }

	C_TextureParam&			GetTextureParam( const S32 sIndex );
	C_TextureHandleIndex	GetTextureHandle( const S32 sIndex );

	char*					CopyCreateString( const char *name );
	void					ReplaceTimeLineLabel( const S32 sIndex, const char* pNewLabel );

	Maths::HMat43			Matrix() const						{ return Maths::Load<Maths::HMat43>(&m_GadgetMatrix); }
	void					SetMatrix(const Maths::HMat43& mat)	{ Store(mat, &m_GadgetMatrix); }

protected :
	void					ResolveTextureParams( );

	enum E_CallbackContext
	{
		E_CallbackContext_UpdateAlpha = 0,
		E_CallbackContext_UpdateIntensity,
		E_CallbackContext_UpdateColour,
	};

public:
	// Xt Vars
	XtSz8						m_Name;

private:
	C_GadgetBase*					m_pParentGadget;
	XtUt::Link<C_GadgetBase>		m_ParentLink;
	XtUt::LinkList<C_GadgetBase>	m_ParentChildList;
	MathsStorage::Float43			m_GadgetMatrix;

public:
	XtSz8						m_ColourStyle;
	XtF32						m_Alpha;
	XtF32						m_Intensity;
	XtArray<C_TextureParam>		m_TextureParams;
	XtArray<C_Timeline>			m_Timeline;

	C_Vector3Position			m_Position;
	C_Vector3Rotation			m_Rotation;
	C_Vector3Scale				m_Scale;
	C_Vector3Size				m_Size;
	E_Justify					m_Justify;
	E_Justify					m_ParentAttach;
	LayerIndex					m_Layer;
	PanelIndex					m_Panel;
	XtFlags<XtU32, E_Flag>		m_Flags;
	E_UiBlendMode				m_BlendMode;
	E_Effect					m_EffectIndex;
	E_TextureAddressMode		m_TextureAddressMode;

#ifdef _GADGET_BOUNDS_CULLING
	Maths::C_Vertex				m_BoundCorners[4];
	C_Rect						m_BoundRect;
#endif

#ifdef _UI_AUTO_CLIPWINDOW
	C_Vector3Position			m_ClipPos;
	C_Vector3Size				m_ClipSize;
#endif

	// Internal vars
	XtU32						m_InternalFlags;
	C_RenderState				m_RenderState;
	C_UISpriteColours			m_ResolveColours;
	C_UISpriteColours			m_SpriteColours;
	UiXt::C_Allocator			m_XtAllocator;

#ifdef PLATFORM_PC
	C_MouseData					m_Mouse;
#endif

protected:
	// Bad things would happen if these two where public.
	bool						m_Activated;
	bool						m_Selected;

	F32							m_AlphaInherit;
	F32							m_IntensityInherit;

private:
	C_GadgetBaseData(const C_GadgetBaseData&); // can not be copied
	C_GadgetBaseData& operator = (const C_GadgetBaseData&); // can not be copied
};

class C_GadgetBase : public C_GadgetBaseData, public Messaging::C_MessageHandler
{
public:
	DECLARE_GADGET( E_UIButtonType_Dummy, "DUMMY" );

	C_GadgetBase( ) : 
		C_GadgetBaseData(this),
		m_IgnoreController( 0 ),
		m_pXtType( NULL ),
		m_Up( 1.0f )
	{ 
		
	}

	virtual ~C_GadgetBase() 			{ }
	virtual void Init();
	virtual void Setup( lua_State* pState );
	virtual void PostInit();
	virtual void FrameUpdate();
	virtual void Render();
	virtual void UpdateEffects();
	virtual void Exit()					{ }
	virtual void ClearItems()			{ } // Called from UIButtons.ClearItems
	virtual void ResetCurrentItem()		{ } // Called from UIButtons.ClearItems

	// Message Handling
	virtual bool	AddSubscriber(Messaging::TMsgType messageType);
	virtual bool    MessageHandler(const Messaging::C_MessageBase *pMessage); // from C_MessageHandler

	void			CalcTransformHierarchy( TransformIndex& ti );
	bool			CalcTransformHierarchy( Maths::HMat43& mat, const bool scale = true, const bool rotate = true, const bool translate = true );

	void			ResolveInheritedColours( );
	const bool		CheckParentDepth( );
	const S32		FindChildren( ButtonArray* children, const bool all, const bool all_context = false, const E_UIButtonType type = E_UIButtonType_None );
	C_GadgetBase*	FindChildByName( const char* pName );
	C_GadgetBase*	ArrayFindByName( ButtonArray& array, const char* pName );
	virtual void	SendPrivateTimelineMessaage( const char* msg, const bool active, const F32 time = -1, const bool reset = false );

	virtual void	Select( const bool bSelect, const bool bSendTimelineMsg = true );
	virtual void	Activate( const bool bActivate, const bool bSendTimelineMsg = true );

	const S32		LuaSerialise( lua_State* pState, const U32 context, UiXt::C_PointerMap& pointerMap );
	const S32		LuaDeserialise( lua_State* pState, UiXt::C_PointerMap& pointerMap );
	void			BindTimeline( );

#ifdef PLATFORM_PC
	void UpdateMouseOver( );			// Standard AABB check
#endif
	void SetDefaultMouseBoundingBox();

protected:
	template <typename Type>
	XtType* GetXtTypeImpl( const Type& def )	{ return XtGetType<Type>(); }

	void ResetDirtyFlags( );
	void CheckSleepingTimeline( );
	void ReplaceToken( const Util::C_String& token, const Util::C_String& replacement, XtSz8& sz8 );

#ifdef _GADGET_BOUNDS_CULLING
	void			CalcBoundRect( );
	const bool		IsCulled( );
#endif

	const F32		Up( ) const					{ return m_Up; }

public:

	C_ButtonId			m_ID;
	XtType*				m_pXtType;
	S8					m_IgnoreController;
	F32					m_Up;

#ifdef PLATFORM_PC
	F32 				m_PrevMousePosX;
	F32 				m_PrevMousePosY;
#endif
};

// Helper gadgets
class UISafeArea : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_SafeArea, "SAFE_AREA" );

	virtual void Setup( lua_State* pState );
};

class UIRenderState : public C_GadgetBase
{
public:
	enum E_AlterFlags
	{
		E_AlterFlags_CullMode,
		E_AlterFlags_ClipWindow,
		E_AlterFlags_DepthBuffer,
		E_AlterFlags_StencilBuffer,
		E_AlterFlags_ColourWrite,
	};

public:
	DECLARE_GADGET( E_UIButtonType_RenderState, "RENDER_STATE" );
	
	virtual void Init();
	virtual void Render();

public:
	XtFlags<XtU8, E_AlterFlags>			m_AlterFlags;
	bool								m_PushState;
	C_Simple2D::C_StackedRenderState	m_State;
};

class UIRenderBranch : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_RenderBranch, "RENDER_BRANCH" );

	virtual void		PostInit();
	virtual bool		MessageHandler(const Messaging::C_MessageBase *pMessage);

	virtual void		Scan();
	virtual void		DirtyMatrix();

public:
	bool				m_DoSort;

protected:
	ButtonArray			m_BranchChildren;
};

// Dummy node which can interpolate matrices from it's base directly to a stored
class UIDummyLerp : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_DummyLerp, "DUMMY_LERP" );

	void				Init();
	void				FrameUpdate();
	void				CopyMatrix( const Maths::HMat43& src );

public:
	XtF32				m_fLerp;

private:
	Maths::HMat43		m_StoreMat;
};

// Inline method implementations

inline C_TextureParam& C_GadgetBaseData::GetTextureParam( const S32 sIndex )
{
	static C_TextureParam s_Null( gUITextureManager.GetTextureByName( "null" ) );

	if ( sIndex >= 0 && sIndex < m_TextureParams.count )
		return m_TextureParams[ sIndex ];

	return s_Null;
}

inline C_TextureHandleIndex C_GadgetBaseData::GetTextureHandle( const S32 sIndex )
{
	static C_TextureHandleIndex s_Null = gUITextureManager.GetTextureByName( "null" );

	if ( sIndex >= 0 && sIndex < m_TextureParams.count )
		return m_TextureParams[ sIndex ].m_TextureID;

	return s_Null;
}


};

#endif
