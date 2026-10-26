#pragma once

//#include "customcontrols/DarkDialog.h"

// For simplicity's sake, we limit the number of simultaneously intersected 
// triangles to 16
#define MAX_INTERSECTIONS 16

class CLightMapperView;
class CModelInfo;

class CPolygonPicker
{
	struct D3DVERTEX
	{
		D3DXVECTOR3 p;
		int			iColour;
		FLOAT       tu, tv;

		static const DWORD FVF;
	};

	struct INTERSECTION
	{
		DWORD dwFace;                 // mesh face that was intersected
		FLOAT fBary1, fBary2;         // barycentric coords of intersection
		FLOAT fDist;                  // distance from ray origin to intersection
		FLOAT tu, tv;                 // texture coords of intersection
	};

public:
					CPolygonPicker(void);
					~CPolygonPicker(void);

	HRESULT			Pick(SD::RayTraceResult *pResult);
	void			AddIntersectionsToMesh(SD::RayTraceResult *pResult);
	void			Init(LPDIRECT3DDEVICE9 pD3DDevice);
	void			Draw();

	DWORD					m_dwNumIntersections;
    LPDIRECT3DVERTEXBUFFER9 m_pVB;            // VB for picked triangles
	LPDIRECT3DDEVICE9		m_pd3dDevice;
    INTERSECTION			m_IntersectionArray[MAX_INTERSECTIONS]; // Intersection info
	bool					m_bClosestOnly;
	bool					m_bInitialized;
	CModelInfo				*m_pMDL;

	bool	m_bFaceSelected;
	int		m_iFace;
	int		m_iChart;
	int		m_iVtx1, m_iVtx2, m_iVtx3;
};

extern CPolygonPicker gPolygonPicker;
/*
// CPolyPickerDlg dialog

class CPolyPickerDlg : public CDarkDialog
{
	DECLARE_DYNCREATE(CPolyPickerDlg);

public:
	CPolyPickerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPolyPickerDlg();

// Dialog Data
	//{{AFX_DATA(CPolyPickerDlg)
	enum { IDD = IDD_POLYINFO };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CPolyPickerDlg)
protected:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedTestbutton();
	afx_msg void OnEnChangeFacenum();
};
*/