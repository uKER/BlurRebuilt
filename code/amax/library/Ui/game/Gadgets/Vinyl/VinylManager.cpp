#include "stdafx.h"

#include "VinylManager.h"
#include "UIColourPicker.h"

#include <ui/UiInterface.h>
#include <ui/Graphics/Simple2d.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Game/Gadgets/BaseUIShape.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_MemoryStream.h>
#include <Framework/Application/CoreSystems.h>


namespace UI
{

// Valeri Bojinov Singleton
C_VinylManager C_VinylManager::m_sInstance;

C_VinylManager::C_VinylManager( ) : m_bActive( false ), m_pSceneHandle( NULL ), m_CoordRange( 512.0f, 512.0f )
{
}

C_VinylManager::~C_VinylManager( )
{
	if( m_bActive )
		End();
}

void C_VinylManager::CalcTransform2D()
{
	// Render matrix for RT0
	const F32 transformUI[ 4 ] =
	{
		1.0f / m_CoordRange.X(),	// scale x
		1.0f / m_CoordRange.Y(),	// scale y
		0.5f,						// translate x
		0.5f,						// translate y
	};

	gSimple2D.CalcTransform2D( transformUI, 1.0f, 1.0f, 0.0f, 15000.0f, m_Transform2D );
}

const bool C_VinylManager::Init( const U32 totalItems )
{
	// Create
	for( S32 i = 0; i < E_List_Count; i++ )
		if( !m_List[ i ].m_scList.Create( totalItems ) )
			return false;

	for( S32 i = 0; i < E_List_Count; i++ )
		m_List[ i ].m_MatrixCache.Resize( totalItems );

	// Clear work area lists
	m_List[ E_List_Clipboard ].m_scList.Clear();
	m_List[ E_List_ScratchPad ].m_scList.Clear();

	// Init render lists
	for( S32 i = E_List_Avatar; i < E_List_Count; i++ )
	{
		m_CurrentList = (E_List)i;
		ClearScene();
	}

	// Default to avatar
	m_CurrentList = E_List_Avatar;
	m_SceneType = E_SceneType_Avatar;
	
	m_StripMatrixCache.Resize( totalItems );

	// Copy empty scene into avatar in-case the load fails
	VinylList& al = C_VinylManager::Instance().scAvatarList();
	VinylList& rl = C_VinylManager::Instance().scList();
	al.Clear();
	CopySub( &al, al.Root(), &rl, rl.Root() );

	m_EditMode = E_EditMode_None;
	m_ShouldRender = E_ShouldRender_None;
	m_bActive = true;

	// Find the scene handle.
	m_pSceneHandle = C_AdobeSceneManager::Instance().GetHandleByName( "shapes" );
	if( m_pSceneHandle )
	{
		Maths::C_AxisBox boundingBox;
		m_pSceneHandle->ScenePtr()->BuildRenderList( C_VinylManager::Instance().ShapeRenderList(), boundingBox );
	}

	// Shape render params
	m_RenderState.m_EffectIndex = E_Effect_PolygonGradient;//E_Effect_PolygonFlat;
	m_RenderState.m_EffectParam = 0;
	m_Panel = 8;
	m_Layer = 1;

	CalcTransform2D();

	// Build clip windows
	F32 cx = CoordRange().X();
	F32 cy = CoordRange().Y();

	F32 sx = cx * -0.5f;
	F32 sy = cy * -0.5f;

	F32 x0 = sx;
	F32 x1 = sx + (cx / 3.0f);
	F32 x2 = sx + (cx * 2.0f / 3.0f);
	F32 x3 = sx + cx;

	F32 y0 = sy;
	F32 y1 = sy + (cy / 3.0f);
	F32 y2 = sy + (cy * 2.0f / 3.0f);
	F32 y3 = sy + cy;

	// Top
	m_List[ E_List_LiveryTop ].m_ClipX = x2;
	m_List[ E_List_LiveryTop ].m_ClipY = y0;
	m_List[ E_List_LiveryTop ].m_ClipW = x3 - x2;
	m_List[ E_List_LiveryTop ].m_ClipH = y2 - y0;
	m_List[ E_List_LiveryTop ].m_MoveScale = ( 2.0f / 3.0f );
	// Left
	m_List[ E_List_LiveryLeft ].m_ClipX = x0;
	m_List[ E_List_LiveryLeft ].m_ClipY = y0;
	m_List[ E_List_LiveryLeft ].m_ClipW = x2 - x0;
	m_List[ E_List_LiveryLeft ].m_ClipH = y1 - y0;
	m_List[ E_List_LiveryLeft ].m_MoveScale = ( 2.0f / 3.0f );
	// Right
	m_List[ E_List_LiveryRight ].m_ClipX = x0;
	m_List[ E_List_LiveryRight ].m_ClipY = y1;
	m_List[ E_List_LiveryRight ].m_ClipW = x2 - x0;
	m_List[ E_List_LiveryRight ].m_ClipH = y2 - y1;
	m_List[ E_List_LiveryRight ].m_MoveScale = ( 2.0f / 3.0f );
	// Front
	m_List[ E_List_LiveryFront ].m_ClipX = x0;
	m_List[ E_List_LiveryFront ].m_ClipY = y2;
	m_List[ E_List_LiveryFront ].m_ClipW = x1 - x0;
	m_List[ E_List_LiveryFront ].m_ClipH = y3 - y2;
	m_List[ E_List_LiveryFront ].m_MoveScale = ( 1.0f / 3.0f );
	// Back
	m_List[ E_List_LiveryBack ].m_ClipX = x1;
	m_List[ E_List_LiveryBack ].m_ClipY = y2;
	m_List[ E_List_LiveryBack ].m_ClipW = x2 - x1;
	m_List[ E_List_LiveryBack ].m_ClipH = y3 - y2;
	m_List[ E_List_LiveryBack ].m_MoveScale = ( 1.0f / 3.0f );
	
	// Defaults
	for( S32 i = E_List_Avatar; i < E_List_LiveryTop; i++ )
	{
		m_List[ i ].m_ClipX = sx;
		m_List[ i ].m_ClipY = sy;
		m_List[ i ].m_ClipW = cx;
		m_List[ i ].m_ClipH = cy;
	}

	// Calculate draw starting points from the clip range
	for( S32 i = E_List_Clipboard; i < E_List_Count; i++ )
	{
		m_List[ i ].m_DrawCentreX = m_List[ i ].m_ClipX + (m_List[ i ].m_ClipW * 0.5f);
		m_List[ i ].m_DrawCentreY = m_List[ i ].m_ClipY + (m_List[ i ].m_ClipH * 0.5f);
	}
	
	return true;
}

void C_VinylManager::ClearScene()
{
	scList().Clear();
	scCurrent() = scList().InsertLastChildOf( scList().Root() );
	VinylItem* p = scList().At( scCurrent() );
	if( p )
		p->Shape( 0xffff ); // Flag as the empty slot
}

void C_VinylManager::SelectedToRoot()
{
	scCurrent() = scList().GetFirstChild( scList().Root() );
}

const bool C_VinylManager::Load( )
{
	Sys::I_FileSystem* fs = Sys::GetDefaultFileSystem();
	if( !fs )
		return false;

	Sys::I_File* file = fs->Open( "vinyl.save", Sys::E_FileAccess_Read);
	if( !file )
		return false;

	Sys::BinaryDeserialiserAutoEndian serial(*file);

	const bool result = LoadImpl( serial );
	delete file;

	return result;
}

const bool C_VinylManager::LoadImpl( Sys::BinaryDeserialiserAutoEndian& serial )
{
	// Check version number
	U32 version = 0;
	serial >> version;
	if( version != m_FileVersion )
		return false;

	return scScratchPadList().Load( serial );
}

const bool C_VinylManager::Save( )
{
	Sys::I_FileSystem* fs = Sys::GetDefaultFileSystem();
	if( !fs )
		return false;

	Sys::I_File* file = fs->Open( "vinyl.save", Sys::E_FileAccess_Write);
	if( !file )
		return false;

	Sys::BinarySerialiserAutoEndian serial(*file);

	const bool result = SaveImpl( serial );
	delete file;

	return result;
}

const bool C_VinylManager::ReadBuffer( U8* pBuffer, const U32 uDataSize )
{
	Sys::C_MemoryStream stream( pBuffer, uDataSize );
	Sys::BinaryDeserialiserAutoEndian serial(stream);

	bool result = LoadImpl( serial );

	return result;
}

const bool C_VinylManager::WriteBuffer( U8* pBuffer, const U32 uDataSize )
{
	Sys::C_MemoryStream stream( uDataSize );
	Sys::BinarySerialiserAutoEndian serial(stream);

	bool result = SaveImpl( serial );
	if( !result || stream.GetBufferSize() > uDataSize )
		return false;

	memcpy( pBuffer, stream.GetBufferPtr(), (size_t)stream.GetBufferSize() );
	return true;
}

const bool C_VinylManager::SaveImpl( Sys::BinarySerialiserAutoEndian& serial )
{
	serial << m_FileVersion;

	return scScratchPadList().Save( serial );
}

const bool C_VinylManager::LoadAvatar( Sys::BinaryDeserialiserAutoEndian& serial )
{
	const bool result = LoadImpl( serial );
	if( !result )
		return false;
	
	scAvatarList().Clear();
	CopySub( &scAvatarList(), scAvatarList().Root(), &scScratchPadList(), scScratchPadList().Root() );
	return true;
}

const bool C_VinylManager::SaveAvatar( Sys::BinarySerialiserAutoEndian& serial )
{
	// Copy into clip to ensure it's 'compressed'
	scScratchPadList().Clear();
	CopySub( &scScratchPadList(), scScratchPadList().Root(), &scAvatarList(), scAvatarList().Root() );

	return SaveImpl( serial );
}

void C_VinylManager::End( )
{
	scList().Destroy();
	scClipboardList().Destroy();
	scScratchPadList().Destroy();


	for( S32 i = 0; i < E_List_Count; i++ )
		m_List[ i ].m_MatrixCache.Destroy();
	m_StripMatrixCache.Destroy();

	m_bActive = false;
}

void C_VinylManager::SelectedExplodeGroup( )
{
	if( !m_bActive )
		return;

	VinylItem* p = scList().At( scCurrent() );
	if( !p )
		return;

	int parent = p->parent;
	if( parent < 0 )  // Don't explode into root or invalid
		return;

	int fci = scList().GetFirstChild( scCurrent() );
	int lci = scList().GetLastChild( scCurrent() );
	
	VinylItem* fc = scList().At( fci );
	VinylItem* lc = scList().At( lci );
	
	if( !fc || !lc )
		return;

	// Perfrom social work and assign to new parent
	Maths::HMat43 mat;
	p->GetTransform( mat );

	for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), fci ) ); src_it; ++src_it )
	{
		VinylItem* pc = scList().At( src_it );
		if( !pc )
			continue;

		// Apply transform
		Maths::HVec3 pos( pc->PosX(), pc->PosY(), 0.0f );
		pos = pos * mat;
		pc->PosX( pos.X() );
		pc->PosY( pos.Y() );

		pc->Rotation( pc->Rotation() + p->Rotation() );

		pc->ScaleX( pc->ScaleX() * p->ScaleX() );
		pc->ScaleY( pc->ScaleY() * p->ScaleY() );

		// Re-assign parent
		pc->parent = p->parent;
	}

	// Clear parents child
	p->child = -1;

#if 0
	fc->prev = p->prev;
	lc->next = scCurrent();

	scList().At(p->prev)->next = fci;
	p->prev = lci;
#else
	fc->prev = scCurrent();
	lc->next = p->next;

	scList().At(p->next)->prev = lci;
	p->next = fci;
#endif

	scList().Remove( scCurrent() );
	scCurrent() = lci;
}

void C_VinylManager::SelectedEnterGroup( )
{
	if( !m_bActive )
		return;

	int index = scList().GetFirstChild( scCurrent() );
	if( index < 0 ) 
		return;

	scList().Remove( scList().GetLastSibling( scCurrent() ) );
	int empty = scList().InsertLastChildOf( scCurrent() );
	if( empty < 0 )
		return;

	scCurrent() = index;

	scList().At( empty )->Reset();
}

void C_VinylManager::SelectedExitGroup( )
{
	if( !m_bActive )
		return;	

	int index = scList().GetParent( scCurrent() );
	if( index <= 0 )  // Don't go to Root group
		return;

	scList().Remove( scList().GetLastSibling( scCurrent() ) );
	int empty = scList().InsertLastChildOf( scList().At( index )->parent );
	if( empty < 0 )
		return;

	scCurrent() = index;

	scList().At( empty )->Reset();
}

namespace
{

	void CopyBefore(UI::VinylList* dstList, int after, UI::VinylList* srcList, int srcRoot)
	{
		for(VinylIterator src_it( UI::GetChildIterator( srcList, srcRoot ) ); src_it; ++src_it )
			CopySub(dstList, dstList->InsertBefore( after ), srcList, src_it.Index());
	}
}

// Like paste but stamp duplicates and inserts current
void C_VinylManager::Stamp( )
{
	if( !m_bActive )
		return;

	// SteC
	VinylItem*p = scList().At( scCurrent() );
	if( p )
	{
		if( p->Highlight() )
		{
			// Copy into stamp buffer
			scScratchPadList().Clear();
			for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
			{
				VinylItem* pSrc = scList().At( src_it );
				if( !pSrc )
					continue;

				if( !pSrc->Highlight() && (int)src_it != scCurrent() )
					continue;

				CopySub(&scScratchPadList(), scScratchPadList().InsertLastChildOf( scScratchPadList().Root() ), &scList(), src_it.Index());
				pSrc->Highlight( false );
			}

			// Highlight new stamped
			for(VinylIterator src_it( GetFirstSiblingIterator( &scScratchPadList(), scScratchPadList().GetFirstChild( scScratchPadList().Root())) ) ; src_it; ++src_it )
				scScratchPadList().At( src_it )->Highlight( true );

			// Paste into render list
			CopyBefore(&scList(), scCurrent(), &scScratchPadList(), scScratchPadList().Root());
		}
		else
		{
			VinylItem* pDst = scList().At( scList().InsertBefore( scCurrent() ) );
			if( pDst )
			{
				scCurrent() = scList().At( scCurrent() )->prev;

				pDst->Copy( *p );
			}
		}
		FlashShape();
	}
}

namespace
{
	int DirectChildCount(UI::VinylList* list, int root)
	{
		int count = 0;

		for(UI::VinylIterator it = GetChildIterator(list, root); it; ++it)
			count++;

		return count;
	}
}

void C_VinylManager::Paste( const bool allowEmpty )
{
	if( !m_bActive )
		return;

	int rootChildCount = DirectChildCount(&scClipboardList(), scClipboardList().Root());

	if (rootChildCount == 0)
	{
	}
	else if (rootChildCount == 1)
	{
		CopyBefore(&scList(), scCurrent(), &scClipboardList(), scClipboardList().Root());

		// Move to pasted item
		scCurrent() = scList().At( scCurrent() )->prev;
	}
	else
	{
		// Create group
		int index = scList().InsertBefore( scCurrent() );
		if( index >= 0 )
		{
			scList().At( index )->Reset();
			scCurrent() = index;

			CopySub(&scList(), index, &scClipboardList(), scClipboardList().Root() );
		}

		// Clear highlight of pasted group and set pivot
		F32 x = 0;
		F32 y = 0;
		F32 cnt = 0;
		for(VinylIterator src_it( UI::GetChildIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem* p = scList().At( src_it );
			if( !p ) 
				continue;

			p->Highlight( false );
			x += p->PosX();
			y += p->PosY();

			cnt++;
		}
		x /= cnt;
		y /= cnt;

		// Setup parent
		VinylItem* p = scList().At( scCurrent() );
		if( p )
		{
			p->PosX( x );
			p->PosY( y );
			p->ScaleX( 1.0f );
			p->ScaleY( 1.0f );
			p->Rotation( 0.0f );
		}
		// Remove offset
		for(VinylIterator src_it( UI::GetChildIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem* c = scList().At( src_it );
			if( !c ) 
				continue;

			c->PosX( c->PosX() - x );
			c->PosY( c->PosY() - y );
		}
	}
}

void C_VinylManager::Copy( )
{
	if( !m_bActive )
		return;

	scClipboardList().Clear();

	for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
	{
		const VinylItem* pSrc = scList().At( src_it );
		if( !pSrc )
			continue;

		if( !pSrc->Highlight() && (int)src_it != scCurrent() )
			continue;

		CopySub(&scClipboardList(), scClipboardList().InsertLastChildOf( scClipboardList().Root() ), &scList(), src_it.Index());
	}
}

void C_VinylManager::Cut( )
{
	if( !m_bActive )
		return;

	int current = scCurrent();
	Copy();
	for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; )
	{
		const VinylItem* pSrc = scList().At( src_it );
		if( !pSrc )
			break;

		if( !pSrc->Highlight() && (int)src_it != current )
		{
			++src_it;
			continue;
		}

		if( pSrc->Shape() == 0xffff && pSrc->child < 0)
		{
			++src_it;
			continue;
		}

		int index = src_it;
		// Move the current selected to 
		if( (int)index == scCurrent() )
			scCurrent() = ( pSrc->next >= 0 ) ? pSrc->next : pSrc->prev;

		// Move and remove
		if( index == scList().GetLastSibling( index ) )
		{
			scList().Remove( index );
			break;
		}

		++src_it;
		scList().Remove( index );

		FlashShape();
	}
	// Special case : empty group? Remove!
	if( scList().GetFirstSibling( scCurrent() ) == scList().GetLastSibling( scCurrent() ) && scList().At( scCurrent() )->parent != scList().Root() )
	{
		SelectedExitGroup();

		int index = scCurrent();
		scCurrent() = scList().At( scCurrent() )->prev;
		scList().Remove( index );
	}
}

void C_VinylManager::Rotate( const F32 rotationInc, const bool nudge )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() && !nudge )
	{
		F32 cnt = 0;
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			p->IncRotation( rotationInc, nudge );
			cnt++;
		}

		// Rotate the translations around selection point
		Maths::HVec3 pv( p->PosX(), p->PosY(), 0.0f );
		Maths::HMat33 rot( Maths::C_MatrixRotatePHR( 0.0f, 0.0f, Maths::DegToRad( rotationInc ) ) );

		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			Maths::HVec3 v( p->PosX(), p->PosY(), 0.0f );

			v -= pv;
			v = v * rot;
			v += pv;

			p->PosX( v.X() );
			p->PosY( v.Y() );
		}
	}
	else
	{
		p->IncRotation( rotationInc, nudge );
	}
}

void C_VinylManager::Alpha( const F32 alphaInc, const bool nudge )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			p->IncAlpha( 0, alphaInc, nudge );
		}
	}
	else
	{
		p->IncAlpha( 0, alphaInc, nudge );
	}
}

void C_VinylManager::ColourRotate( const F32 amount, const bool nudge )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			p->IncColourRotation( amount, nudge );
		}
	}
	else
	{
		p->IncColourRotation( amount, nudge );
	}
}

void C_VinylManager::ColourTranslate( const F32 amount, const bool nudge )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			p->IncColourTranslation( amount, nudge );
		}
	}
	else
	{
		p->IncColourTranslation( amount, nudge );
	}
}

void C_VinylManager::ColourScale( const F32 amount, const bool nudge )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			p->IncColourScale( amount, nudge );
		}
	}
	else
	{
		p->IncColourScale( amount, nudge );
	}
}


void C_VinylManager::Translate( const bool moveX, const F32 amount, const bool nudge )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	Maths::HVec3 amount3( moveX ? amount : 0.0f, moveX ? 0.0f : amount, 0.0f );
	Maths::HMat43 m0;
	GetTransformHeirachy( &scList(), p->parent, m0 );
	amount3 = amount3 * Maths::HMat33( m0 ).Inverse();

	if( p->Highlight() && !nudge )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			p->IncTranslation( true, amount3.X(), nudge );
			p->IncTranslation( false, amount3.Y(), nudge );
		}
	}
	else
	{
		p->IncTranslation( true, amount3.X(), nudge );
		p->IncTranslation( false, amount3.Y(), nudge );
	}
}

void C_VinylManager::Scale( const bool scaleX, const F32 amount, const bool nudge )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			p->IncScale( scaleX, amount, false );
		}
	}
	else
	{
		p->IncScale( scaleX, amount, nudge );
	}
}

void C_VinylManager::Shear( const bool shearX, const F32 amount, const bool nudge )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );

			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			p->IncShear( shearX, amount, false );
		}
	}
	else
	{
		p->IncShear( shearX, amount, nudge );
	}
}

void C_VinylManager::SelectedHighlightToggle( )
{
	if( !m_bActive )
		return;

	VinylItem* sp = scList().At( scCurrent() );
	if( sp && !(sp->Shape() == 0xffff && sp->child < 0) )
		sp->Highlight( !sp->Highlight() );
}

const bool C_VinylManager::SelectedHightlight()
{
	if( !m_bActive )
		return false;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return false;

	return p->Highlight();
}

const S32 C_VinylManager::SetSelection( const S32 dir )
{
	S32 cnt = 0;
	VinylIterator src_it( GetSiblingIterator( &scList(), scCurrent() ) );

	if (src_it)
	{
		if( dir > 0 )
		{
			// Inc
			for( S32 i = 0; i < dir && src_it; ++src_it, i++)
				cnt++;
				
			if (!src_it)
			{
				--src_it;
				cnt--;
			}

			scCurrent() = src_it;
		}
		else
		if( dir < 0 )
		{
			// Dec
			for( S32 i = 0; i < -dir && src_it; --src_it, i++)
				cnt++;
				
			if (!src_it)
			{
				++src_it;
				cnt--;
			}

			scCurrent() = src_it;
		}
	}

	if( cnt > 0 )
		FlashShape();

	return cnt;
}

const C_AdobeShape* C_VinylManager::SelectedShape( )
{
	if( !m_bActive )
		return NULL;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return NULL;

	if( p->Shape() >= m_ShapeRenderList.Size() )
		return NULL;

	return m_ShapeRenderList[ p->Shape() ];
}

void C_VinylManager::SelectedResetItem( )
{
	if( !m_bActive )
		return;

	VinylItem* p = scList().At( scCurrent() );
	if( !p )
		return;
	p->Reset();
}

void C_VinylManager::InsertNewItem( )
{
	int index = scList().InsertBefore( scCurrent() );
	if( index < 0 )
		return;

	scCurrent() = index;
	VinylItem* p = scList().At( scCurrent() );
	if( !p )
		return;

	p->Reset();

	// Place at the centre of the current clip area
	p->PosX( List().m_DrawCentreX );
	p->PosY( List().m_DrawCentreY );
}

const bool C_VinylManager::HighlightSelected()
{
	if( !m_bActive )
		return false;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return false;

	return p->Highlight();	
}

const U16 C_VinylManager::SelectedShapeIndex()
{
	if( !m_bActive )
		return 0xffff;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return 0xffff;

	return p->Shape();
}

void C_VinylManager::SelectedShapeIndex( const U16 index )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );
			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			if( p->child >= 0 )
				continue;

			p->Shape( index );
		}
	}
	else
	{
		p->Shape( index );
	}
}

void C_VinylManager::SelectedUseGradient( const bool useGradient )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );
			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			if( p->child >= 0 )
				continue;

			p->Gradient( useGradient );
		}
	}
	else
	{
		p->Gradient( useGradient );
	}
}

void C_VinylManager::SelectedColourIndex( const U32 edge_index, const U16 col_index )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );
			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			if( p->child >= 0 )
				continue;

			if( edge_index == 0 )
				p->Colour0( col_index );
			else
				p->Colour1( col_index );
		}
	}
	else
	{
		if( edge_index == 0 )
			p->Colour0( col_index );
		else
			p->Colour1( col_index );
	}
}

const U16 C_VinylManager::SelectedColourIndex( const U32 edge_index )
{
	if( !m_bActive )
		return 0;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return 0;

	return ( edge_index == 0 ) ? p->Colour0() : p->Colour1();
}

const bool C_VinylManager::SelectedUseGradient()
{
	if( !m_bActive )
		return false;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return false;

	return p->Gradient();
}

void C_VinylManager::SelectedAlpha( const U32 edge_index, const F32 alpha )
{
	if( !m_bActive )
		return;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return;

	if( p->Highlight() )
	{
		for(VinylIterator src_it( GetFirstSiblingIterator( &scList(), scCurrent() ) ); src_it; ++src_it )
		{
			VinylItem*p = scList().At( src_it );
			if( !p->Highlight() && (int)src_it != scCurrent() )
				continue;

			if( p->child >= 0 )
				continue;

			if( edge_index == 0)
				p->Alpha0( alpha );
			else
				p->Alpha1( alpha );
		}
	}
	else
	{
		if( edge_index == 0)
			p->Alpha0( alpha );
		else
			p->Alpha1( alpha );
	}
}

const F32 C_VinylManager::SelectedAlpha( const U32 edge_index )
{
	if( !m_bActive )
		return 1.0f;

	VinylItem*p = scList().At( scCurrent() );
	if( !p )
		return 1.0f;

	return ( (edge_index == 0) ? p->Alpha0() : p->Alpha1() );
}

void C_VinylManager::EditMode( const E_EditMode editMode )
{
	m_EditMode = editMode;
}

const C_VinylManager::E_EditMode C_VinylManager::EditMode( ) const
{
	return m_EditMode;
}

void C_VinylManager::SceneType( const C_VinylManager::E_SceneType t )
{
	// Enusure we're on the correct list when switching scenes
	switch( t )
	{
	case E_SceneType_Avatar:
		CurrentList( E_List_Avatar );
		break;

	case E_SceneType_Group:
		CurrentList( E_List_Group );
		break;

	case E_SceneType_Livery:
		CurrentList( E_List_LiveryTop );
		break;
	default:
		LOGERROR( g_Log, DBSTR " : Invalid scene type %d", (S32)t );
		return;
	}

	m_SceneType = t;
}

const C_VinylManager::E_SceneType C_VinylManager::SceneType( ) const
{ 
	return m_SceneType; 
}

const C_AdobeShape*	C_VinylManager::ResolveShape( U16 index )
{
	if( index >= m_ShapeRenderList.Size() )
		return NULL;

	return m_ShapeRenderList[ index ];
}

C_UIColour C_VinylManager::ResolveColour( U16 index, F32 alpha )
{
	if( index == 0xffff )
		return 0xff7fff7f;

	C_UIColour col;
	col = C_UIColourPicker::Instance().GetColour( index>>8, index&0xff );
	col.A01( alpha );

	return col;
}


void C_VinylManager::FlashShape( )
{
	C_UiTimelineMessage message( "shape_selected", true, 0.0f, true );
	gButtons.BroadcastMessageNow( 0xffffffff, &message );
}

void GetTransformHeirachy( VinylList* list, int current, Maths::HMat43& mat )
{
	for( VinylItem *p = list->At( current ); p; p = list->At( p->parent ) )
	{
		Maths::HMat43 m0;
		p->GetTransform( m0 );

		mat = mat * m0;
	}
}

const bool CalcTransform(
	const C_AdobeShape* pShape,
	const Maths::HVec3& drawSize, 
	const Maths::HMat43& drawMat,
	const F32 up, 
	TransformIndex& ti, 
	Maths::HVec3& out, 
	F32& sizeX, 
	F32& sizeY )
{
	if( !pShape )
		return false;

	// Transform?
	ti = gSimple2D.PopPostTransformMatrix();
	Maths::HMat43& mat = gSimple2D.GetPostTransformMatrix( ti );
	mat = drawMat;

	TransformIndex at = gSimple2D.GetAutoTransform();
	if( at )
		mat = mat * gSimple2D.GetPostTransformMatrix( at );

	Maths::HVec3 pos = gSimple2D.GetJustifiedCorner( E_Justify_MiddleCentre, 0, up == -1.0f ) * drawSize * pShape->BoundingBox().Max();

	sizeX = drawSize.X();
	sizeY = drawSize.Y() * up;

	F32 x = pos.X();
	F32 y = pos.Y() * -up;

	out = Maths::HVec3( x, y, 0.0f );

	return true;
}

// Render
Maths::HMat43 C_VinylManager::GetBoundsMatrix( const C_AdobeShape* pShape, const Maths::HVec3& shapeSize )
{
	// Set position
	Maths::HMat43 mat( Maths::C_MatrixScale( shapeSize.X(), shapeSize.Y(), shapeSize.Z() ) );
	Maths::HVec3 boundsSize = shapeSize * Maths::HVec3( pShape->BoundingBox().Max().X(), pShape->BoundingBox().Max().Y(), 0.0f );
	mat.Translation( gSimple2D.GetJustifiedCorner( E_Justify_MiddleCentre, 0, false ) * boundsSize );

	return mat;
}

void C_VinylManager::RenderShape( const C_AdobeShape* pShape )
{
	for ( S32 i = 0; i < pShape->Fill().Size(); i++ )
	{
		C_UITriListRenderInfo* pRenderInfo = pShape->Fill()[ i ];
		if( !pRenderInfo )
			continue;

		gSimple2D.GetRenderer().RenderTriList( pRenderInfo->m_VertexBuffer, pRenderInfo->m_IndexBuffer, pRenderInfo->m_uIndexCount, pRenderInfo->m_uVertexCount );
	}
}

void C_VinylManager::RenderGradientShape( const C_AdobeShape* pShape, const Maths::HMat43& m0, const Maths::HMat43& col_mat, const Maths::HVec3& shapeSize, const C_UIColour& fillColour0, const C_UIColour& fillColour1 )
{
	if( !pShape )
		return;

	// Transform
	Maths::HMat43 mat( GetBoundsMatrix( pShape, shapeSize ) * m0 );

	// Set colours
	F32 pColour[ C_Renderer::m_NumGradientColours ][ 4 ];

	pColour[ 0 ][ 0 ] = fillColour0.R01();
	pColour[ 0 ][ 1 ] = fillColour0.G01();
	pColour[ 0 ][ 2 ] = fillColour0.B01();
	pColour[ 0 ][ 3 ] = fillColour0.A01();

	pColour[ 1 ][ 0 ] = fillColour0.R01();
	pColour[ 1 ][ 1 ] = fillColour0.G01();
	pColour[ 1 ][ 2 ] = fillColour0.B01();
	pColour[ 1 ][ 3 ] = fillColour0.A01();

	pColour[ 2 ][ 0 ] = fillColour1.R01();
	pColour[ 2 ][ 1 ] = fillColour1.G01();
	pColour[ 2 ][ 2 ] = fillColour1.B01();
	pColour[ 2 ][ 3 ] = fillColour1.A01();

	pColour[ 3 ][ 0 ] = fillColour1.R01();
	pColour[ 3 ][ 1 ] = fillColour1.G01();
	pColour[ 3 ][ 2 ] = fillColour1.B01();
	pColour[ 3 ][ 3 ] = fillColour1.A01();

	gSimple2D.GetRenderer().SetPolygonGradientSettings( mat, col_mat, &pColour[ 0 ] [ 0 ], NULL );
	gSimple2D.GetRenderer().SetEffect( E_Effect_PolygonGradient, 0, E_TextureAddressMode_Wrap );
	RenderShape( pShape );
}

void C_VinylManager::RenderFlatShape( const C_AdobeShape* pShape, const Maths::HMat43& m0, const Maths::HVec3& shapeSize, const C_UIColour& fillColour )
{
	if( !pShape )
		return;

	// Transform
	Maths::HMat43 mat( GetBoundsMatrix( pShape, shapeSize ) * m0 );

	// Set colour
	F32 pColour[ 4 ];
	pColour[ 0 ] = fillColour.R01();
	pColour[ 1 ] = fillColour.G01();
	pColour[ 2 ] = fillColour.B01();
	pColour[ 3 ] = fillColour.A01();

	gSimple2D.GetRenderer().SetPolygonFlatSettings( mat, pColour );
	gSimple2D.GetRenderer().SetEffect( E_Effect_PolygonFlat, 0, E_TextureAddressMode_Wrap );

	RenderShape( pShape );
}

void C_VinylManager::RenderList( C_List& rlist, const int index, const Maths::HVec3& shapeSize, const bool isScene, const C_UIColour colourOveride, const bool doOveride )
{
	VinylList& list = rlist.m_scList;

	for(VinylIterator it( GetSiblingIterator( &list, index ) ); it; ++it )
	{
		VinylItem *p = list.At( it );

		if( p->child >= 0 )
		{
			RenderList( rlist, p->child, shapeSize, isScene, colourOveride, doOveride );
			continue;
		}

		const C_AdobeShape* pShape = C_VinylManager::Instance().ResolveShape( p->Shape() );

		Maths::HMat43& mat( isScene ? rlist.m_MatrixCache[ p->self ] : m_StripMatrixCache[ p->self ] );

		if( doOveride || !p->Gradient() )
		{
			RenderFlatShape( pShape, mat, shapeSize, doOveride ? colourOveride : C_VinylManager::Instance().ResolveColour( p->Colour0(), p->Alpha0() ) );
		}
		else
		{
			// This is poo!
			Maths::HMat43 col_mat;
			p->GetColourTransform( col_mat );

			RenderGradientShape( pShape, mat, col_mat, shapeSize, C_VinylManager::Instance().ResolveColour( p->Colour0(), p->Alpha0() ), C_VinylManager::Instance().ResolveColour( p->Colour1(), p->Alpha1() ) );
		}
	}
}

void C_VinylManager::RenderShape( const C_Sprite* pSprite )
{
	C_List&		rlist = *((C_List*)pSprite->m_RenderState.m_EffectParam); 
	VinylList&	list = rlist.m_scList;
	int			at = (int)pSprite->m_RenderState.m_EffectParam2;

	VinylItem *p = list.At( at );
	if( !p )
		return;

	const C_AdobeShape* pShape = C_VinylManager::Instance().ResolveShape( p->Shape() );
	if( !pShape )
		return;

	const Maths::HMat43& mat( ( pSprite->m_PostTransformIndex == 0 ) ? rlist.m_MatrixCache[ p->self ] : m_StripMatrixCache[ p->self ] );
	const Maths::HVec3 shapeSize( pSprite->m_Size.x, pSprite->m_Size.y, pSprite->m_Size.z );
	const bool doOveride = (pSprite->m_UserData != 0);

	Display::Device()->SetBlend( Display::E_Blend_SrcAlpha, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add, Display::E_Blend_One, Display::E_Blend_One, Display::E_BlendOp_Max );

	if( doOveride || !p->Gradient() )
	{
		RenderFlatShape( pShape, mat, shapeSize, doOveride ? pSprite->m_Colours[0] : C_VinylManager::Instance().ResolveColour( p->Colour0(), p->Alpha0() ) );
	}
	else
	{
		Maths::HMat43 col_mat;
		p->GetColourTransform( col_mat );

		RenderGradientShape( pShape, mat, col_mat, shapeSize, C_VinylManager::Instance().ResolveColour( p->Colour0(), p->Alpha0() ), C_VinylManager::Instance().ResolveColour( p->Colour1(), p->Alpha1() ) );
	}

}

void C_VinylManager::RenderList( const C_Sprite* pSprite )
{
	C_List&		rlist = *((C_List*)pSprite->m_RenderState.m_EffectParam); 
	int			root = (int)pSprite->m_RenderState.m_EffectParam2;

	Display::Device()->SetBlend( Display::E_Blend_SrcAlpha, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add, Display::E_Blend_One, Display::E_Blend_One, Display::E_BlendOp_Max );

	RenderList( rlist, root, Maths::HVec3( pSprite->m_Size.x, pSprite->m_Size.y, pSprite->m_Size.z ), (pSprite->m_PostTransformIndex == 0), pSprite->m_Colours[0], (pSprite->m_UserData != 0) );

}

void C_VinylManager::Render( )
{
#if 0
	if( !m_bActive )
		return;

	VinylList* pList = NULL;

	switch( m_ShouldRender ) 
	{
	case E_ShouldRender_Avatar:
		pList = &scAvatarList();
		break;

	case E_ShouldRender_Render:
		pList = &scList();
		break;

	case E_ShouldRender_Clipboard:
		pList = &scClipboardList();
		break;

	case E_ShouldRender_ScratchPad:
		pList = &scScratchPadList();
		break;

	default:
		LOGERROR( g_Log, DBSTR "Invalid should render\n");
		break;
	}

	const Maths::HVec3 shapeSize( 64.0f, 64.0f, 1.0f );
	if( pList )
	{
		gSimple2D.PreRender();
		gSimple2D.SetTransformMatrix2D();

		C_VinylManager::Instance().CacheMatrices();
		C_VinylManager::Instance().RenderList( *pList, pList->Root(), shapeSize, true, 0xffffffff, false );
		
		gSimple2D.PostRender();
	}

	m_ShouldRender = E_ShouldRender_None;
#endif
}

// Render list
void C_List::CacheMatrices( )
{
	// Cache recursively
	Maths::HMat43 mat;
	mat.SetIdentity();

	CacheMatrices( m_scList.Root(), mat );
}

void C_List::CacheMatrices( const int index, Maths::HMat43& mat )
{
	for(VinylIterator it( GetSiblingIterator( &m_scList, index ) ); it; ++it )
	{
		VinylItem *p = m_scList.At( it );
		if( !p )
			continue;

		Maths::HMat43 m0;
		p->GetTransform( m0 );

		m_MatrixCache[ p->self ] = (m0 * mat);

		if( p->child >= 0 )
			CacheMatrices( p->child, m_MatrixCache[ p->self ] );
	}
}

} // namespace UI

