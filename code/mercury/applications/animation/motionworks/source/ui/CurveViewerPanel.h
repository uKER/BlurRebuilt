#ifndef CURVEVIEWERPANEL_H
#define CURVEVIEWERPANEL_H

#include <btl/Containers/bitvector.h>

namespace AnimationTools
{
	class C_CycleBuilder;
}

class C_CurveGraphWindow;

class C_CurveViewerPanel: public wxPanel
{
public:
	C_CurveViewerPanel(
		wxWindow* parent,
		wxWindowID winId,
		const wxPoint& pos,
		const wxSize& size
	);

	void SetCycle( const AnimationTools::C_CycleBuilder* cycle );

	void SetJointIndex( S32 jointIndex );
	void SetPlaybackTime( F32 time );
	
private:
	void OnCurveTypeComboBoxChanged( wxCommandEvent& e );
	void OnSourceCheckBoxChanged( wxCommandEvent& e );
	void OnShowKeysCheckBoxChanged( wxCommandEvent& e );

	wxArrayString m_curveTypeStrings;
	C_CurveGraphWindow* m_curveGraphWindow;
	wxComboBox* m_curveTypeComboBox;
	wxCheckBox* m_rawCheckBox;
	wxCheckBox* m_keyReducedCheckBox;
	wxCheckBox* m_compressedCheckBox;
	wxCheckBox* m_showKeysCheckBox;
	
	DECLARE_CLASS(C_CurveViewerPanel)
	DECLARE_EVENT_TABLE()
};

class C_CurveGraphWindow: public wxWindow
{
public:
	enum E_CurveType
	{
		E_CurveType_Translation	= 0,
		E_CurveType_Rotation,
		E_CurveType_Count
	};

	C_CurveGraphWindow(
		wxWindow* parent,
		wxWindowID winId,
		const wxPoint& pos,
		const wxSize& size
	);

	virtual ~C_CurveGraphWindow();

	void SetCycle( const AnimationTools::C_CycleBuilder* cycle );
	void EnableCycle( U32 cycleIndex, bool enabled=true );
	bool IsCycleEnabled( U32 cycleIndex) const;
	void SetJointIndex( S32 jointIndex );
	void SetCurveType( E_CurveType curveType );
	void ShowKeys( bool showKeys = true );
	void SetPlaybackTime( F32 time );

	void UpdateGraph( bool cyclesChanged );

	//virtual void OnDraw( wxDC& dc );

private:
	void OnPaint( wxPaintEvent& e );
	void OnSize( wxSizeEvent& e );
	void OnMouseWheel( wxMouseEvent& e );
	void OnMouseMiddleDown( wxMouseEvent& e );
	void OnMouseMiddleUp( wxMouseEvent& e );
	void OnMouseMotion( wxMouseEvent& e );
	void OnMouseCaptureLost( wxMouseCaptureLostEvent& e );
	void OnKeyUp( wxKeyEvent& e );
	
	void DrawCurve( wxDC& dc, const AnimationTools::C_CycleBuilder& cycle, U32 sourceId ) const;


	F32 GetFractionalCurveValue( F32 value ) const;

	S32 CurveToClientY( F32 curveY ) const;
	F32 ClientToCurveY( S32 clientY ) const;
	S32 CurveToClientX( F32 curveX ) const;
	F32 ClientToCurveX( S32 clientX ) const;

	void ZoomToFit();
	void ClampZoom();

	void DrawGrid( wxDC& dc ) const;

	wxFont* m_font;

	E_CurveType m_curveType;
	
	const AnimationTools::C_CycleBuilder* m_cycle;

	Btl::BitVector m_enabledCycles;
	S32 m_jointIndex;
	F32 m_zoomScale;	// pixels per frame
	U32 m_maxTotalFrames;
	F32 m_frameRate;
	F32 m_playbackTime;
	F32 m_minValues[ E_CurveType_Count ];
	F32 m_maxValues[ E_CurveType_Count ];
	S32 m_startX;
	S32 m_startY;
	S32 m_dragStartX;
	S32 m_dragStartY;
	S32 m_zoomCounter;
	S32 m_clientMouseX;
	S32 m_clientMouseY;
	
	bool m_draggingCanvas;
	bool m_showKeys;

	DECLARE_CLASS(C_CurveGraphWindow)
	DECLARE_EVENT_TABLE()
};

#endif
