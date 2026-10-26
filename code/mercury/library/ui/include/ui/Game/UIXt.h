#ifndef UIXT_H
#define UIXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/NewCurveHermite.h>

#include <Xt/XtGame.h>
#include <btl/Containers/containers.h>

#include "UI/Game/UILua.h"
#include "UI/Game/UIXt.h"

namespace UI
{

class C_GadgetBase;
class C_FontStyle;

/// @addtogroup UiXt
/// @{

namespace UiXt
{

typedef U8							AllocationType;
typedef AllocationType *			AllocationPtr;

// Callback template to make Xt rectord refencable by pointer
template< typename _T >
static void XtReferenceRecordCB( XtEventType type, XtRecord* pRecord, _T* pBase, XtCallbackContext* pContext )
{
	if( type == kXtEtUser && pContext->tag == 'RRef' )
	{
		pContext->tag = 0;
		return;
	}
}

// Variant interface
template <typename T>
class I_Variant
{
public:
	virtual ~I_Variant( )		{ }
	virtual T*	Get()			{ return NULL; }
};

// Weak pointer variant interface
template <typename T>
class C_WeakVariant : public I_Variant<T>
{
public:
	virtual ~C_WeakVariant( )	{ }
	virtual T*	Get()			{ return m_p; }

public:
	T*	m_p;
};

// Object variant interface
template <typename T>
class C_ObjVariant : public I_Variant<T>
{
public:
	virtual  ~C_ObjVariant( )	{ }
	virtual  T*	Get()			{ return &m_o; }

public:
	T	m_o;
};

// XtRef
template <typename T>
class C_RefVariant : public I_Variant<T>
{
public:
	virtual  ~C_RefVariant( )	{ }
	virtual  T*	Get()			{ return m_h.Object(); }

public:
	XtHandle<T>	m_h;
};

// Memory allocater
class C_Allocator
{
private:
	class C_Data : public Btl::ListElementSingle< C_Data >
	{
	public:
		C_Data( AllocationPtr data, const U32 count = 1, const XtType* xt_type = NULL ) : 
			m_Data( data ),
			m_XtType( xt_type ),
			m_Count( count )
		{

		}

		~C_Data( )
		{
			if( m_XtType )
			{
				for(U32 i = 0; i < m_Count; i++)
					m_XtType->destruct( m_Data + (m_XtType->size * i) );
			}

			SAFE_DELETE_ARY( m_Data );
		}

		const AllocationPtr		Data( )		{ return m_Data; }

	private:
		AllocationPtr			m_Data;
		U32						m_Count;
		const XtType*			m_XtType;
	};
	
public:
	~C_Allocator( )
	{
		Free();
	}

	void Free( )
	{
		for( Btl::List<C_Data *>::Iterator it(m_Set.Begin()); it != m_Set.End(); )
		{
			SAFE_DELETE_PTR( *it );
			it = m_Set.Erase( it );
		}
	}

	void *New( const size_t size )
	{
		AllocationPtr data = new AllocationType[ size ];
		if( data )
		{
			m_Set.PushBack( new C_Data( data ) );
		}

		return data;
	}

	void *New( const XtType* xt_type, const U32 count = 1 )
	{
		assert( xt_type );
		AllocationPtr data = new AllocationType[ xt_type->size * count ];

		if( data )
		{
			m_Set.PushBack( new C_Data( data, count, xt_type ) );
		}

		return data;
	}

	bool Delete( void *data )
	{
		if( !data )
			return false;

		for( Btl::List<C_Data *>::Iterator it(m_Set.Begin()); it != m_Set.End(); ++it )
			if( (*it)->Data() == data )
			{
				SAFE_DELETE_PTR( *it );
				m_Set.Erase( it );
				return true;
			}

		return false;
	}

private:
	Btl::List< C_Data* >			m_Set;
};

// Class for remapping pointers from binding pairs
class C_PointerMap
{
public:
	/// Static instance
	C_PointerMap( );
	~C_PointerMap( );

	/// Add a binding between two addresses
	void				AddBinding( const void* _src, void *_dst );
	/// Clear the binding list
	void				ClearBindings( );
	/// Store a pointer that is referenced - these referenced structures can be created in ListExpand()
	void				AddReference( const void* _src );
	/// Create data that has been referenced by pointers but not already cloned
	void				CreateReferencedData( const XtRecord* root, C_Allocator& allocator );
	/// Add an address to be resolved later
	void				AddToRemapList( void** _src );
	/// Remap the list of pointers from their source to their newly created dest
	void				ListRemap( );

private:
	/// Has _src has a binding pair?
	const bool			HasBinding( const void* _src );
	/// Remap the contents of __src with it's binding pair
	const bool			Remap( void** _src );

private:
	// Binding of _src data to cloned _dst pointers
	struct C_AddressPair
	{
		const void*	src;
		void*		dst;

	public:
		C_AddressPair(const void* src_, void* dst_) : src(src_), dst(dst_) { }
	};

private:
	Btl::Vector< C_AddressPair >		m_AddressPairs;
	Btl::Vector< void ** >				m_ResolveList;
	Btl::Vector< const void * >			m_RefList;
};

// function prototypes & templated fun
void MakeLuaFromDefImpl(lua_State *pState,XtType* def,const U8 *structPtr, C_PointerMap& pointerMap, const XtRecord* root = NULL );
void GetLuaFromDefImpl(lua_State *pState,XtType* def,U8 *structPtr, C_Allocator& allocator, C_PointerMap& pointerMap, const XtRecord* root = NULL );

template <typename Type>
void MakeLuaFromDef(lua_State *pState,const Type &def, C_PointerMap& pointerMap, const XtRecord* root )
{
	MakeLuaFromDefImpl( pState, XtGetType<Type>(), (const U8 *)&def, pointerMap, root );
}

template <typename Type>
void GetLuaFromDef(lua_State *pState,Type &def, C_Allocator& allocator, C_PointerMap& pointerMap, const XtRecord* root )
{
	GetLuaFromDefImpl(pState,XtGetType<Type>(),(U8 *)&def, allocator, pointerMap, root );
}

// Find member address link 
class C_MemberAddrLink
{
public:
	C_MemberAddrLink( ) : 
		m_pComponentName( NULL ), 
		m_pDef( NULL ), 
		m_pComponent( NULL ), 
		m_pNext( NULL )
	{

	}

	~C_MemberAddrLink( )
	{
		SAFE_DELETE_PTR( m_pNext );
	}

public:
	const char *		m_pComponentName;
	XtType *			m_pDef;
	void *				m_pComponent;
	C_MemberAddrLink *	m_pNext;
};

// Find member address (with callstack links)
void* FindMemberAddrImpl(XtType* def, const U8 *structPtr, const char *name, C_MemberAddrLink* link  );

// Callback context
enum { E_CallbackContext_Setup = 0xffff };

class C_CallbackAssignContext : public XtCallbackContext
{
public:
	C_CallbackAssignContext( C_MemberAddrLink* link ) : 
		XtCallbackContext( E_CallbackContext_Setup ),
		m_pLink( link )
	{ 
		assert( link );
	}

public:
	C_MemberAddrLink*		m_pLink;
};

// Xt Colour def
/*
// Styles
class C_FontStyle
{
public:
	static void	SorceryCallback( XtEventType type, XtRecord* pRecord, C_FontStyle* pFontStyle, XtCallbackContext* pContext );

public:
	XtSz8			m_Name;
	XtSz8			m_FontName;

	XtF32			m_DropShadow;
	XtF32			m_TrackingOveride;
	
//	XtOption		m_DropShadowPropotional;
//	XtOption		m_Italics;

	XtI8			m_ShaderIndex;
};
*/

class C_FontStyleList
{
public:
	typedef Ut::UtSet<C_FontStyle*>	List;
	
public:
	C_FontStyleList( ) : m_bIsDirty( false )
	{
	}

//	void						FrameUpdate( lua_State *pState );
	const C_FontStyle *			FindByName( const char *pName );
	void						LoadPolygonFonts( );
	void						DefferBind( C_FontStyle* style )	{ m_DefferedBindList.PushBack( style ); }
	void						FlushBind( );
	void						ClearBind( );
	List &						GetList( )							{ return m_List; }
	void						SetDirty( )							{ m_bIsDirty = true; }
	const bool					IsDirty( ) const					{ return m_bIsDirty; }

	static C_FontStyleList &	Instance( )							{ return m_sInstance; }

private:
	List						m_List;
	bool						m_bIsDirty;
	static C_FontStyleList		m_sInstance;
	Btl::Vector<C_FontStyle *>	m_DefferedBindList;
};

// Colour styles
class C_ColourStyle
{
public:
	static void	SorceryCallback( XtEventType type, XtRecord* pRecord, C_ColourStyle* pUIScreen, XtCallbackContext* pContext );

public:
	XtSz8		m_Name;
	XtRgb		m_TopLeft;
	XtRgb		m_TopRight;
	XtRgb		m_BottomLeft;
	XtRgb		m_BottomRight;
};

class C_ColourStyleList
{
public:
	typedef Ut::UtSet<C_ColourStyle*>	List;
	
public:
	void						FrameUpdate( lua_State *pState );
	const C_ColourStyle *		FindByName( const char *pName );
	List &						GetList( )						{ return m_List; }
	void						SetDirty( )						{ m_bIsDirty = true; }
	const bool					IsDirty( ) const				{ return m_bIsDirty; }

	static C_ColourStyleList&	Instance( )						{ return m_sInstance; }

private:
	List						m_List;
	bool						m_bIsDirty;
	static C_ColourStyleList	m_sInstance;
};

// Screens
class C_Screen
{

public:
	static void						SorceryCallback( XtEventType type, XtRecord* pRecord, C_Screen* pUIScreen, XtCallbackContext* pContext );

	void							MakeLuaTable( lua_State *pState, const S32 context, const bool bForceNewTable, C_PointerMap& pointerMap ) const;
	void							AddElements( lua_State *pState, const XtRecord* root, const S32 context, const char* pTableName, C_PointerMap& pointerMap ) const;
	const U32						AddElementByName( const char* name, const XtRecord* root, const S32 context, const bool cloneHeirachy = true ) const;	
	const C_FontStyle *				FindFontStyleByName( const char *name ) const;
	const C_ColourStyle *			FindColourStyleByName( const char *name ) const;
	const C_GadgetBase *			FindGadgetByName( const char *name ) const;
	const bool						NameExists( const char* name ) const;

public:
	XtHandle< C_Screen >			m_Self;
	XtSz8							m_Name;
	XtSz8							m_OnNextCmd;
	XtSz8							m_OnBackCmd;
	XtSz8							m_GuiTableName;
	XtArray<C_FontStyle>			m_FontStyles;
	XtArray<C_ColourStyle>			m_ColourStyles;
	XtArray<C_GadgetBase *>			m_Elements;
private:
	Btl::Vector<Util::C_String>		m_ScreenNames;
};

class C_ScreenList
{
public:
	enum E_DirtyFlags
	{
		E_DirtyFlags_Screen		= (1<<0),
		E_DirtyFlags_Colour		= (1<<24),
		E_DirtyFlags_Font		= (1<<25),

		E_DirtyFlags_All		= 0xffff
	};

	typedef Ut::UtSet< C_Screen * >	List;

public:
	C_ScreenList( ) : m_IsDirtyFlags( 0 )
	{

	}
	void							InitScreen( lua_State *pState, const char *name );
	const C_Screen*					FindByName( const char * pName );

	void							FrameUpdate( )							{ m_IsDirtyFlags = 0; }
	List&							GetList( )								{ return m_List; }
	void							SetDirty( const E_DirtyFlags flag )		{ m_IsDirtyFlags |= flag; }
	const S32						IsDirty( ) const						{ return m_IsDirtyFlags; }

	static C_ScreenList&			Instance( )								{ return m_sInstance; }

private:
	static C_ScreenList				m_sInstance;						
	List							m_List;
	S32								m_IsDirtyFlags;
};

/// Create a lua table from an Xt Type of type_name (with table_name overide)
const bool CreateLuaTableFromEnumType( lua_State *pState, const char *type_name, const char *table_name = NULL );

/// Get the Xt name of an enum for a given type name
const char* GetEnumNameFromType( const char* pTypeName, const S32 sEnumValue );

} // UiXt

// ---------------------
// C_UIXtHermiteKeyFrame
// ---------------------
/// sorcery interface for Hermite key frame data
struct C_UIXtHermiteKeyFrame
{
	F32 	m_fValue;
	F32 	m_fTangentIn;
	F32 	m_fTangentOut;
	F32 	m_fTime;
};

// ---------------------
// C_UIXtHermiteProxy
// ---------------------
/// interface allowing spline evaluator to talk directly to xt data.
class C_UIXtHermiteProxyImpl
{
public:
	void SetData(const XtArray<C_UIXtHermiteKeyFrame>* pXtData)
	{
		m_pXtData = pXtData;
	}

	void LoadSourceKnot(Curve::C_HermiteSplineKnotU<F32>& knot, U32 index)
	{
		BIZ_ASSERT(m_pXtData);
		const C_UIXtHermiteKeyFrame& xtKnot = (*m_pXtData)[index];
		//Load functions come from "#include <core/xtstdx.h>"
		knot.value = xtKnot.m_fValue;
		knot.tangentIn = Maths::Tan(Maths::DegToRad(xtKnot.m_fTangentIn));
		knot.tangentOut = Maths::Tan(Maths::DegToRad(xtKnot.m_fTangentOut));
		knot.tangentScale = 1.f;
		knot.u = xtKnot.m_fTime;
	}

	void StoreSourceKnot(const Curve::C_HermiteSplineKnotU<F32>& knot, U32 index)
	{
		BIZ_ASSERT(!"Xt splines are read only!");
	}

	U32 GetNumSourceKnots() const
	{
		return m_pXtData ? m_pXtData->count : 0;
	}

private:
	const XtArray<C_UIXtHermiteKeyFrame>* m_pXtData;
};

class C_UIXtHermiteProxy : public Curve::C_CurveDataProxy< 
	Curve::C_HermiteSplineKnotU<F32>, C_UIXtHermiteProxyImpl >
{
};

typedef Curve::C_HermiteSpline< F32, C_UIXtHermiteProxy, 
	Curve::C_HermiteCtrlPt1DTanAngle<F32, C_UIXtHermiteProxy> > UIHermiteSpline1DAngular;

class C_UIXtHermite1D : public UIHermiteSpline1DAngular
{
public:
	///Constructor.
	C_UIXtHermite1D(Curve::C_HermiteSplineFlags flags = Curve::C_HermiteSplineFlags()) : UIHermiteSpline1DAngular(flags) {}

	///Destructor.
	virtual ~C_UIXtHermite1D() {}	

	typedef UIHermiteSpline1DAngular::VecType VecType;
	typedef UIHermiteSpline1DAngular::StorageType::KnotType KnotType;
};



// ------------------
// C_UIXtHermiteCurve
// ------------------
/// sorcery interface for a Hermite curve
struct C_UIXtHermiteCurve
{
	XtSz8								m_Name;
	XtArray< C_UIXtHermiteKeyFrame >	m_KeyFrames;
};


// ------------------
// UIPadding
// ------------------
struct Padding
{
	F32		top;
	F32		bottom;
	F32		left;
	F32		right;
};

// ------
// C_UIXt
// ------
/// the UI's Xt structure
class C_UIXt
{
public:
	C_UIXt() : m_DbCreate( false )	
	{  

	}

	~C_UIXt()						
	{  

	}

public:
	/// sorcery callback
	static void		SorceryCallback( XtEventType type, XtRecord* pRecord, C_UIXt* pUIXtDef, XtCallbackContext* );

	void			LoadDefinitions( const char* pFileName, XtDb* db = NULL );		///< load in from Sorcery
	void			Cleanup();														///< cleanup

public:

private:
	bool								m_DbCreate;
	XtCallbackContext*					m_pXtCC;
	XtDb*								m_pXtDb;
	XtPack*								m_pXtPack;
};

/// @}

// global instance to the UI's Sorcery interface
extern C_UIXt g_UIXt;

}; // namespace UI

#endif // UIXT_H
