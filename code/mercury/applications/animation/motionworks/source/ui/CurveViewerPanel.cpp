#include "stdafx.h"
#include "CurveViewerPanel.h"
#include <animation/tools/CycleBuilder.h>
#include <animation/skeleton/Skeleton.h>
#include <wx/dcbuffer.h>

#undef DrawText

IMPLEMENT_CLASS( C_CurveViewerPanel, wxPanel )

enum E_ID
{
	E_ID_CurveTypeComboBox	= wxID_HIGHEST + 100,
	E_ID_RawCheckBox,
	E_ID_KeyReducedCheckBox,
	E_ID_CompressedCheckBox,
	E_ID_ShowKeysCheckBox
};

BEGIN_EVENT_TABLE( C_CurveViewerPanel, wxPanel )
	EVT_COMBOBOX(E_ID_CurveTypeComboBox, C_CurveViewerPanel::OnCurveTypeComboBoxChanged)
	EVT_CHECKBOX(E_ID_RawCheckBox, C_CurveViewerPanel::OnSourceCheckBoxChanged)
	EVT_CHECKBOX(E_ID_KeyReducedCheckBox, C_CurveViewerPanel::OnSourceCheckBoxChanged)
	EVT_CHECKBOX(E_ID_CompressedCheckBox, C_CurveViewerPanel::OnSourceCheckBoxChanged)
	EVT_CHECKBOX(E_ID_ShowKeysCheckBox, C_CurveViewerPanel::OnShowKeysCheckBoxChanged)
END_EVENT_TABLE()

C_CurveViewerPanel::C_CurveViewerPanel(
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size
	):
	wxPanel(
		parent,
		winId,
		pos,
		size
	)
{

	m_curveGraphWindow = new C_CurveGraphWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize );

	m_curveTypeStrings.Add( "Translation XYZ" );
	m_curveTypeStrings.Add( "Rotation XYZW" );

	wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
	
	hSizer->Add( new wxStaticText( this, wxID_ANY, "Channel:" ), wxSizerFlags().Border( wxALL, 4 ).Centre() );

	m_curveTypeComboBox = new wxComboBox(
		this,
		E_ID_CurveTypeComboBox,
		m_curveTypeStrings[ 0 ],
		wxDefaultPosition,
		wxDefaultSize,
		m_curveTypeStrings,
		wxCB_DROPDOWN | wxCB_READONLY
	);

	hSizer->Add( m_curveTypeComboBox, wxSizerFlags().Border( wxALL, 4 ).Centre() );

	m_rawCheckBox = new wxCheckBox(
		this,
		E_ID_RawCheckBox,
		wxT( "Show Original" )
	);

	m_keyReducedCheckBox = new wxCheckBox(
		this,
		E_ID_KeyReducedCheckBox,
		wxT( "Show Key Reduced" )
	);

	m_compressedCheckBox = new wxCheckBox(
		this,
		E_ID_CompressedCheckBox,
		wxT( "Show Compressed" )
	);

	m_rawCheckBox->SetValue( false );
	m_keyReducedCheckBox->SetValue( false );
	m_compressedCheckBox->SetValue( true );

	m_curveGraphWindow->EnableCycle( 0, m_rawCheckBox->GetValue() );
	m_curveGraphWindow->EnableCycle( 1, m_keyReducedCheckBox->GetValue() );
	m_curveGraphWindow->EnableCycle( 2, m_compressedCheckBox->GetValue() );

	wxStaticBoxSizer* sourceSizer = new wxStaticBoxSizer( wxHORIZONTAL, this, wxT( "Curves" ) );
	sourceSizer->Add( m_rawCheckBox, wxSizerFlags( 0 ).Border( wxALL, 4 )  );
	sourceSizer->Add( m_keyReducedCheckBox, wxSizerFlags( 0 ).Border( wxALL, 4 ) );
	sourceSizer->Add( m_compressedCheckBox, wxSizerFlags( 0 ).Border( wxALL, 4) );
	
	hSizer->Add( sourceSizer, wxSizerFlags( 0 ).TripleBorder( wxHORIZONTAL ).Centre() );

	m_showKeysCheckBox = new wxCheckBox(
		this,
		E_ID_ShowKeysCheckBox,
		wxT( "Show Keys" )
	);
	m_showKeysCheckBox->SetValue( true );
	hSizer->Add( m_showKeysCheckBox, wxSizerFlags( 0 ).Border( wxALL, 4 ).Centre() );

	wxBoxSizer* vSizer = new wxBoxSizer( wxVERTICAL );
	vSizer->Add( hSizer, wxSizerFlags( 0 ).Left() );
	vSizer->Add( m_curveGraphWindow, wxSizerFlags( 1 ).Expand().Border( wxALL, 4 ) );

	SetSizerAndFit( vSizer );
}


void C_CurveViewerPanel::SetCycle(
	const AnimationTools::C_CycleBuilder* cycle
)
{
	m_curveGraphWindow->SetCycle( cycle );
}

void C_CurveViewerPanel::SetJointIndex( S32 jointIndex )
{
	m_curveGraphWindow->SetJointIndex( jointIndex );
}

void C_CurveViewerPanel::SetPlaybackTime( F32 time )
{
	m_curveGraphWindow->SetPlaybackTime( time );
}

void C_CurveViewerPanel::OnCurveTypeComboBoxChanged( wxCommandEvent& e )
{
	m_curveGraphWindow->SetCurveType( static_cast<C_CurveGraphWindow::E_CurveType>( m_curveTypeComboBox->GetCurrentSelection() ) );
}

void C_CurveViewerPanel::OnSourceCheckBoxChanged( wxCommandEvent& e )
{
	switch ( e.GetId() )
	{
		case E_ID_RawCheckBox: m_curveGraphWindow->EnableCycle( 0, e.IsChecked() ); break;
		case E_ID_KeyReducedCheckBox: m_curveGraphWindow->EnableCycle( 1, e.IsChecked() ); break;
		case E_ID_CompressedCheckBox: m_curveGraphWindow->EnableCycle( 2, e.IsChecked() ); break;
		default: break;
	}
}

void C_CurveViewerPanel::OnShowKeysCheckBoxChanged( wxCommandEvent& e )
{
	m_curveGraphWindow->ShowKeys( e.IsChecked() );
}

//
// C_CurveGraphWindow
//

IMPLEMENT_CLASS( C_CurveGraphWindow, wxWindow )

BEGIN_EVENT_TABLE( C_CurveGraphWindow, wxWindow )
	EVT_SIZE( C_CurveGraphWindow::OnSize )
	EVT_MOUSEWHEEL( C_CurveGraphWindow::OnMouseWheel )
	EVT_PAINT( C_CurveGraphWindow::OnPaint )
	EVT_MIDDLE_DOWN( C_CurveGraphWindow::OnMouseMiddleDown )
	EVT_MIDDLE_UP( C_CurveGraphWindow::OnMouseMiddleUp )
	EVT_MOTION( C_CurveGraphWindow::OnMouseMotion )
	EVT_MOUSE_CAPTURE_LOST( C_CurveGraphWindow::OnMouseCaptureLost )
	EVT_KEY_UP( C_CurveGraphWindow::OnKeyUp )
END_EVENT_TABLE()

C_CurveGraphWindow::C_CurveGraphWindow(
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size
):
	wxWindow( parent, winId, pos, size, wxBORDER_SUNKEN )
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );

	m_font = new wxFont(
		8,
		wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,
		wxFONTWEIGHT_NORMAL,
		false,
		wxT( "verdana" ),	// face name
		wxFONTENCODING_DEFAULT
	);

	m_curveType = E_CurveType_Translation;
	m_jointIndex = -1;
	m_cycle = 0;

	m_enabledCycles.Resize( 3 );
	m_enabledCycles.ClearBits();

	m_dragStartX = 0;
	m_dragStartY = 0;
	m_startX = 0;
	m_startY = 0;
	m_draggingCanvas = false;

	m_zoomScale = 1.f;
	m_playbackTime = 0.f;
	m_frameRate = 0.f;

	ShowKeys( true );

	//SetScrollRate( 50, 0 );
}

C_CurveGraphWindow::~C_CurveGraphWindow()
{
	delete m_font;
}


void C_CurveGraphWindow::SetCycle( const AnimationTools::C_CycleBuilder* cycle )
{
	m_cycle = cycle;
	UpdateGraph( true );
}

void C_CurveGraphWindow::EnableCycle( U32 cycleIndex, bool enabled )
{
	m_enabledCycles.SetBitValue( cycleIndex, enabled );
	UpdateGraph( false );
}

bool C_CurveGraphWindow::IsCycleEnabled( U32 cycleIndex) const
{
	return m_enabledCycles.GetBitValue( cycleIndex );
}

void C_CurveGraphWindow::SetJointIndex( S32 jointIndex )
{
	m_jointIndex = jointIndex;
	UpdateGraph( true );
}

void C_CurveGraphWindow::SetCurveType( E_CurveType curveType )
{
	m_curveType = curveType;
	UpdateGraph( true );
}

void C_CurveGraphWindow::ShowKeys( bool showKeys )
{
	m_showKeys = showKeys;
	UpdateGraph( false );
}

void C_CurveGraphWindow::SetPlaybackTime( F32 time )
{
	m_playbackTime = time;
	Refresh();
}

namespace
{
	void UpdateCurveBounds(
		const AnimationTools::C_CycleBuilder& cycle,
		U32 jointIndex,
		AnimationTools::C_CycleBuilder::E_Source source,
		MathsStorage::Float4& minRotation,
		MathsStorage::Float4& maxRotation,
		MathsStorage::Float3& minTranslation,
		MathsStorage::Float3& maxTranslation
	)
	{
		for ( U32 j = 0, count = cycle.GetNumRotationKeys( jointIndex, source ); j < count; ++j )
		{
			U32 frame;
			Maths::C_Quaternion rotation;
			cycle.GetRotationKey( jointIndex, j, frame, rotation, source );

			MathsStorage::Float4 r;
			Maths::Store< Maths::C_Quaternion, MathsStorage::Float4 >( rotation, &r );

			minRotation.x = Maths::Min( r.x, minRotation.x );
			minRotation.y = Maths::Min( r.y, minRotation.y );
			minRotation.z = Maths::Min( r.z, minRotation.z );
			minRotation.w = Maths::Min( r.w, minRotation.w );

			maxRotation.x = Maths::Max( r.x, maxRotation.x );
			maxRotation.y = Maths::Max( r.y, maxRotation.y );
			maxRotation.z = Maths::Max( r.z, maxRotation.z );
			maxRotation.w = Maths::Max( r.w, maxRotation.w );
		}

		for ( U32 j = 0, count = cycle.GetNumTranslationKeys( jointIndex, source ); j < count; ++j )
		{
			U32 frame;
			Maths::HVec3 translation;
			cycle.GetTranslationKey( jointIndex, j, frame, translation, source );

			MathsStorage::Float3 t;
			Maths::Store< Maths::HVec3, MathsStorage::Float3 >( translation, &t );

			minTranslation.x = Maths::Min( t.x, minTranslation.x );
			minTranslation.y = Maths::Min( t.y, minTranslation.y );
			minTranslation.z = Maths::Min( t.z, minTranslation.z );

			maxTranslation.x = Maths::Max( t.x, maxTranslation.x );
			maxTranslation.y = Maths::Max( t.y, maxTranslation.y );
			maxTranslation.z = Maths::Max( t.z, maxTranslation.z );
		}
	}
}

void C_CurveGraphWindow::UpdateGraph( bool cyclesChanged )
{
	if ( m_jointIndex < 0 )
	{
		return;
	}

	if ( cyclesChanged )
	{
		U32 maxTotalFrames = 0;

		static const F32 kMoreThanOne = 2.f;
		MathsStorage::Float4 minRotation( kMoreThanOne, kMoreThanOne, kMoreThanOne, kMoreThanOne );
		MathsStorage::Float4 maxRotation( -kMoreThanOne, -kMoreThanOne, -kMoreThanOne, -kMoreThanOne );
		MathsStorage::Float3 minTranslation( FLT_MAX, FLT_MAX, FLT_MAX );
		MathsStorage::Float3 maxTranslation( -FLT_MAX, -FLT_MAX, -FLT_MAX );

		const AnimationTools::C_CycleBuilder* c = m_cycle;
		F32 frameRate = 60.f; 

		if ( c )
		{
			frameRate = c->GetFrameRate();
			maxTotalFrames = Maths::Max( c->GetDurationFrames(), maxTotalFrames );
			UpdateCurveBounds( *c, m_jointIndex, AnimationTools::C_CycleBuilder::E_Source_Original, minRotation, maxRotation, minTranslation, maxTranslation );
			UpdateCurveBounds( *c, m_jointIndex, AnimationTools::C_CycleBuilder::E_Source_Compressed, minRotation, maxRotation, minTranslation, maxTranslation );
		}

		m_minValues[ E_CurveType_Translation ] = Maths::Min( minTranslation.x, Maths::Min( minTranslation.y, minTranslation.z ) );
		m_maxValues[ E_CurveType_Translation ] = Maths::Max( maxTranslation.x, Maths::Max( maxTranslation.y, maxTranslation.z ) );
		m_minValues[ E_CurveType_Rotation ] = Maths::Min( minRotation.x, Maths::Min( minRotation.y, Maths::Min( minRotation.z, minRotation.w ) ) );
		m_maxValues[ E_CurveType_Rotation ] = Maths::Max( maxRotation.x, Maths::Max( maxRotation.y, Maths::Max( maxRotation.z, maxRotation.w ) ) );;

		m_maxTotalFrames = maxTotalFrames;
		m_frameRate = frameRate;

		ZoomToFit();
	}
	else
	{
		Refresh();
	}
}

namespace
{
	void DrawCentredText( wxDC& dc, const wxString& str )
	{
		wxSize ext = dc.GetTextExtent( str );
		wxSize size = dc.GetSize();

		wxCoord x = ( size.GetWidth() - ext.GetWidth() ) / 2;
		wxCoord y = ( size.GetHeight() - ext.GetHeight() ) / 2;

		dc.DrawText( str, x, y );
	}
}

void C_CurveGraphWindow::OnPaint( wxPaintEvent& e )
{
	wxBufferedPaintDC dc( this );

	// Draw the graph!

	S32 clientWidth;
	S32 clientHeight;
	GetClientSize( &clientWidth, &clientHeight );

	wxBrush bgBrush( wxColour( 150, 150, 180 ) );


	dc.SetFont( *m_font );
	dc.SetBackground( bgBrush );
	dc.Clear();

	if ( m_jointIndex < 0 )
	{
		DrawCentredText( dc, wxT( "No joint selected" ) );
		return;
	}

	DrawGrid( dc );

	//DrawCentredText( dc, "CurveGraphWindow" );

	// Draw scale.

	wxString minLabel = wxString::Format( "%.4f", ClientToCurveY( clientHeight - 9 ) );
	wxString maxLabel = wxString::Format( "%.4f", ClientToCurveY( 8 ) );

	wxSize minLabelSize = dc.GetTextExtent( minLabel );
	wxSize maxLabelSize = dc.GetTextExtent( maxLabel );

	//dc.DrawText( wxString::Format( "zoom scale: %f", m_zoomScale ), 50, 50 );
	
	{
		S32 spacingY = 12;
		S32 statsY = 10;
		S32 statsX = 50;

		//dc.DrawText( wxString::Format( "X: %.3f", ClientToCurveX( m_clientMouseX ) ), statsX, statsY );
		//statsY += spacingY;
		//dc.DrawText( wxString::Format( "Y: %.3f", ClientToCurveY( m_clientMouseY ) ), statsX, statsY );
		//statsY += spacingY;
		
		if ( m_cycle )
		{
			if ( m_curveType == E_CurveType_Rotation )
			{
				dc.DrawText( wxString::Format( "Original Key Count: %d", m_cycle->GetNumRotationKeys( m_jointIndex, AnimationTools::C_CycleBuilder::E_Source_Original ) ), statsX, statsY );
				statsY += spacingY;
				dc.DrawText( wxString::Format( "Compressed Key Count: %d", m_cycle->GetNumRotationKeys( m_jointIndex, AnimationTools::C_CycleBuilder::E_Source_Compressed ) ), statsX, statsY );
				statsY += spacingY;
			}
			else if ( m_curveType == E_CurveType_Translation )
			{
				dc.DrawText( wxString::Format( "Original Key Count: %d", m_cycle->GetNumTranslationKeys( m_jointIndex, AnimationTools::C_CycleBuilder::E_Source_Original ) ), statsX, statsY );
				statsY += spacingY;
				dc.DrawText( wxString::Format( "Compressed Key Count: %d", m_cycle->GetNumTranslationKeys( m_jointIndex, AnimationTools::C_CycleBuilder::E_Source_Compressed ) ), statsX, statsY );
				statsY += spacingY;
			}
		}
	}

	// Get value at viewport minimum. 
	
	dc.DrawText( maxLabel, 8, 8 );
	dc.DrawText( minLabel, 8, clientHeight - 8 - minLabelSize.GetHeight() );

	dc.SetPen( *wxBLACK_PEN );
	wxCoord zeroY = CurveToClientY( 0.f );
	dc.DrawLine(
		0,
		zeroY,
		clientWidth,
		zeroY
	);

	dc.DrawLine(
		2,
		8,
		2,
		clientHeight - 9
	);

	dc.DrawText( wxT( "0" ), 8, zeroY );

	if ( m_cycle )
	{
		F32 playbackTimeInFrames = m_frameRate * m_playbackTime;

		F32 dur = m_cycle->GetDurationFrames();
		playbackTimeInFrames = playbackTimeInFrames - dur * floorf( playbackTimeInFrames / dur );

		dc.SetPen( *wxBLACK_DASHED_PEN );
		
		dc.DrawLine(
			CurveToClientX( playbackTimeInFrames ),
			0,
			CurveToClientX( playbackTimeInFrames ),
			clientHeight
			);
			
		dc.DrawText( wxString::Format( "Frame: %.2f, Current: %.2f", ClientToCurveX( m_clientMouseX ), playbackTimeInFrames ), 8, 30 );
	}
	
	if ( m_cycle )
	{
		if ( m_cycle->IsValid() )
		{
			if ( IsCycleEnabled( 0 ) )
			{
				DrawCurve( dc, *m_cycle, AnimationTools::C_CycleBuilder::E_Source_Original );
			}
			
			if ( IsCycleEnabled( 2 ) )
			{
				DrawCurve( dc, *m_cycle, AnimationTools::C_CycleBuilder::E_Source_Compressed );
			}
		}
	}
}

void C_CurveGraphWindow::OnSize( wxSizeEvent& e )
{

	ClampZoom();
	UpdateGraph( false );
}

Debug::Int gZoomSensitivity("AnimationPreview.ZoomSensitivity", 120, Debug::Range(0, 1000) );

void C_CurveGraphWindow::OnMouseWheel( wxMouseEvent& e )
{
	if ( e.GetWheelRotation() > 0 )
	{
		m_zoomScale *= 1.25f;
	}
	else
	{
		m_zoomScale /= 1.25f;
	}

	ClampZoom();

	Refresh();
}

void C_CurveGraphWindow::OnMouseMiddleDown( wxMouseEvent& e )
{
	CaptureMouse();
	m_draggingCanvas = true;
	m_dragStartX = e.GetX();
	m_dragStartY = e.GetY();

	Refresh();
}

void C_CurveGraphWindow::OnMouseMiddleUp( wxMouseEvent& e )
{
	if ( m_draggingCanvas )
	{
		ReleaseMouse();
		m_draggingCanvas = false;
	}
}

void C_CurveGraphWindow::OnMouseMotion( wxMouseEvent& e )
{
	if ( m_draggingCanvas )
	{
		S32 deltaX = e.GetX() - m_dragStartX;
		S32 deltaY = e.GetY() - m_dragStartY;

		m_dragStartX = e.GetX();
		m_dragStartY = e.GetY();

		m_startX -= deltaX;
		//m_startY += deltaY;

		if ( m_startX < 0 )
		{
			m_startX = 0;
		}

		F32 lastFrame = ( GetClientSize().GetWidth() - 1 + m_startX ) / m_zoomScale;

		if ( lastFrame > m_maxTotalFrames )
		{
			m_startX = ( m_maxTotalFrames * m_zoomScale ) - GetClientSize().GetWidth() + 1;
		}

		Refresh();
	}
	else
	{
		m_clientMouseX = e.GetX();
		m_clientMouseY = e.GetY();

		SetFocus();
	}
}

void C_CurveGraphWindow::OnMouseCaptureLost( wxMouseCaptureLostEvent& e )
{
	m_draggingCanvas = false;
}

void C_CurveGraphWindow::OnKeyUp( wxKeyEvent& e )
{
	if ( e.GetKeyCode() == 'F' )
	{
		ZoomToFit();
	}
}

void C_CurveGraphWindow::DrawCurve( wxDC& dc, const AnimationTools::C_CycleBuilder& cycle, U32 sourceId ) const
{
	AnimationTools::C_CycleBuilder::E_Source source = static_cast< AnimationTools::C_CycleBuilder::E_Source >( sourceId );


	wxColour kRed25( 255, 200, 200 );
	wxColour kRed50( 255, 100, 100 );
	wxColour kRed100( 255, 0, 0 );

	wxColour kGreen25( 200, 255, 200 );
	wxColour kGreen50( 100, 255, 100 );
	wxColour kGreen100( 0, 255, 0 );

	wxColour kBlue25(   0, 200, 255 );
	wxColour kBlue50( 100, 100, 255 );
	wxColour kBlue100(   0, 0, 255 );

	wxColour kWhite25( 64, 64, 64 );
	wxColour kWhite50( 128, 128, 128 );
	wxColour kWhite100( 255, 255, 255 );


	static const wxPen curvePens[ 4 ][ 3 ] =
	{
		{ wxPen( kRed25 ), wxPen( kRed50 ), wxPen( kRed100 ) },
		{ wxPen( kGreen25 ), wxPen( kGreen50 ), wxPen( kGreen100 ) },
		{ wxPen( kBlue25 ), wxPen( kBlue50 ), wxPen( kBlue100 ) },
		{ wxPen( kWhite25 ), wxPen( kWhite50 ), wxPen( kWhite100 ) }
	};

	// Draw interpolated curves.
	wxCoord prevX;
	wxCoord prevY[ 4 ];

	S32 clientWidth;
	S32 clientHeight;
	GetClientSize( &clientWidth, &clientHeight );
	
	S32 viewStartX = m_startX;
	S32 viewStartY = m_startY;

	for ( S32 x = 0; x < clientWidth; ++x )
	{
		// Calculate time in frames for this pixel.
		F32 cycleTimeInFrames = ( x + viewStartX ) / m_zoomScale;
		
		F32 cycleTimeInSeconds = cycleTimeInFrames / cycle.GetFrameRate();
		

		F32 value = 0.f;

		if ( cycleTimeInFrames < 0.f || cycleTimeInFrames > cycle.GetDurationFrames() )
		{
			break;
		}

		Animation::C_JointTransform t = cycle.GetLocalJointTransform( m_jointIndex, cycleTimeInSeconds, source );

		Maths::StorageVector3 translation;
		Maths::StorageVec4F32 rotation;

		Maths::Store( t.GetTranslation(), &translation );
		Maths::Store( Maths::GetVec4F32( t.GetRotation() ), &rotation );
		
		wxCoord currX = x;
		wxCoord currY[ 4 ];

		U32 numCurves = 0;
		switch ( m_curveType )
		{
			case E_CurveType_Rotation:
				currY[ 0 ] = CurveToClientY( rotation.m_Data.x );
				currY[ 1 ] = CurveToClientY( rotation.m_Data.y );
				currY[ 2 ] = CurveToClientY( rotation.m_Data.z );
				currY[ 3 ] = CurveToClientY( rotation.m_Data.w );
				numCurves = 4;
				break;

			case E_CurveType_Translation:
				currY[ 0 ] = CurveToClientY( translation.m_Data.x );
				currY[ 1 ] = CurveToClientY( translation.m_Data.y );
				currY[ 2 ] = CurveToClientY( translation.m_Data.z );
				numCurves = 3;
				break;

			default:
				assert( false );
				break;
		}

		if ( x > 0 )
		{
			for ( U32 curveIndex = 0; curveIndex < numCurves; ++curveIndex )
			{
				dc.SetPen( curvePens[ curveIndex ][ sourceId ] );
				dc.DrawLine(
					prevX,
					prevY[ curveIndex ],
					currX,
					currY[ curveIndex ]
				);
			}
		}

		prevX = currX;
		
		for ( U32 curveIndex = 0; curveIndex < 4; ++curveIndex )
		{
			prevY[ curveIndex ] = currY[ curveIndex ];
		}
	}

	//
	// Draw key frames.
	// 
	if ( m_showKeys )
	{
		if ( m_curveType == E_CurveType_Translation )
		{
			for ( U32 keyIndex = 0, count = cycle.GetNumTranslationKeys( m_jointIndex, source ); keyIndex < count; ++keyIndex )
			{
				U32 frame;
				Maths::HVec3 t;
				cycle.GetTranslationKey( m_jointIndex, keyIndex, frame, t, source );

				Maths::StorageVector3 translation;
				Maths::Store( Maths::Vector3( t ), &translation );

				S32 x = static_cast< S32 >( ( frame * m_zoomScale ) - viewStartX );

				dc.SetBrush( *wxGREY_BRUSH );
				for ( U32 curveIndex = 0; curveIndex < 3; ++curveIndex )
				{
					dc.SetPen( curvePens[ curveIndex ][ sourceId ] );
					
					S32 radius = 3;
					S32 y = CurveToClientY( translation.m_Data.v[ curveIndex ] );

					dc.DrawRectangle(
						x - radius,
						y - radius,
						radius * 2,
						radius * 2
					);
				}
			}			
		}
		else if ( m_curveType == E_CurveType_Rotation )
		{
			for ( U32 keyIndex = 0, count = cycle.GetNumRotationKeys( m_jointIndex, source ); keyIndex < count; ++keyIndex )
			{
				U32 frame;
				Maths::C_Quaternion r;
				cycle.GetRotationKey( m_jointIndex, keyIndex, frame, r, source );

				Maths::StorageVec4F32 rotation;
				Maths::Store( Maths::GetVec4F32( r ), &rotation );

				S32 x = static_cast< S32 >( ( frame * m_zoomScale ) - viewStartX );

				dc.SetBrush( *wxGREY_BRUSH );
				for ( U32 curveIndex = 0; curveIndex < 4; ++curveIndex )
				{
					dc.SetPen( curvePens[ curveIndex ][ sourceId ] );
					
					S32 radius = 3;
					S32 y = CurveToClientY( rotation.m_Data.v[ curveIndex ] );

					dc.DrawRectangle(
						x - radius,
						y - radius,
						radius * 2,
						radius * 2
					);
				}
			}
		}
	}
	
}

F32 C_CurveGraphWindow::GetFractionalCurveValue( F32 value ) const
{
	return 1.f - ( ( value - m_minValues[ m_curveType ] ) * Maths::Reciprocal( m_maxValues[ m_curveType ] - m_minValues[ m_curveType ] ) );
}

S32 C_CurveGraphWindow::CurveToClientY( F32 curveY ) const
{
	
	F32 yRange = m_maxValues[ m_curveType ] - m_minValues[ m_curveType ];
	
	F32 y = ( curveY - m_minValues[ m_curveType ] ) * Maths::Reciprocal( yRange );
	y = 1.f - y;
	y *= GetClientSize().GetHeight() - 16;
	y += m_startY + 8;

	return static_cast< S32 >( y );
}

F32 C_CurveGraphWindow::ClientToCurveY( S32 clientY ) const
{
	F32 y = ( clientY - 8 - m_startY ) * Maths::Reciprocal( static_cast< F32 >( GetClientSize().GetHeight() - 16 ) );
	y = 1.f - y;

	y *= m_maxValues[ m_curveType ] - m_minValues[ m_curveType ];
	y += m_minValues[ m_curveType ];

	return y;
}

S32 C_CurveGraphWindow::CurveToClientX( F32 curveX ) const
{
	return ( curveX * m_zoomScale ) - m_startX;
}

F32 C_CurveGraphWindow::ClientToCurveX( S32 clientX ) const
{
	return static_cast< F32 >( ( clientX + m_startX ) / m_zoomScale );
}

void C_CurveGraphWindow::ZoomToFit()
{
	m_startX = 0;
	m_startY = 0;
	m_zoomScale = static_cast< F32 >( GetClientSize().GetWidth() - 1 ) / m_maxTotalFrames;
	Refresh();
}

void C_CurveGraphWindow::ClampZoom()
{
	if ( m_maxTotalFrames > 0 )
	{
		F32 fitScale = static_cast< F32 >( GetClientSize().GetWidth() - 1 ) / m_maxTotalFrames;

		if ( m_zoomScale < fitScale )
		{
			m_zoomScale = fitScale;
		}
	}
}

void C_CurveGraphWindow::DrawGrid( wxDC& dc ) const
{
	if ( m_zoomScale <= 1.f )
	{
		return;
	}

	S32 clientWidth, clientHeight;
	GetClientSize( &clientWidth, &clientHeight );
	
	F32 startTime = floorf( ClientToCurveX( 0 ) );
	F32 endTime   = floorf( ClientToCurveX( clientWidth - 1 ) );

	if ( endTime > m_maxTotalFrames )
	{
		endTime = static_cast< F32 >( m_maxTotalFrames );
	}

	S32 numFrames = static_cast< S32 >( endTime - startTime );

	dc.SetPen( *wxLIGHT_GREY_PEN );
	for ( S32 i = 0; i < numFrames; ++i )
	{
		S32 x = CurveToClientX( static_cast< F32 >( startTime + i ) );

		dc.DrawLine( x, 0, x, clientHeight );
	}
}


