// PickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "lux_app/LpvsApp.h"
#include "lux_app/PickerDlg.h"
#include "lux_app/CustomControls/DarkDialog.h"

#include "..\..\..\lux\include\lux\shmaths.h"

float g_vSH[4];

void UpdateMainLpvsView();

// CPickerDlg dialog

IMPLEMENT_DYNAMIC(CPickerDlg, CDarkDialog)
CPickerDlg::CPickerDlg(CWnd* pParent /*=NULL*/)
	: CDarkDialog(CPickerDlg::IDD, pParent)
	, m_iPolygon(0)
	, m_iChart(0)
	, m_csNodeName(_T(""))
{
	m_fSlider1 = m_fSlider2 = 0;
}

CPickerDlg::~CPickerDlg()
{
}

float transfermtx[4][4] = 
{
/*
{  0.65f, -0.00f,  0.00f, -0.62f },
{ -0.00f,  0.57f,  0.00f,  0.00f },
{  0.00f,  0.00f,  0.58f, -0.01f },
{ -0.62f,  0.00f, -0.01f,  0.79f },
*/
{ 0.841069f, 0.001700f, 0.036298f, -0.044227f }, 
{ 0.009791f, 0.432984f, -0.002159f, -0.052451f },
{ 0.040803f, -0.009348f, 0.462658f, -0.008206f },
{ -0.084912f, -0.039203f, -0.017149f, 0.496381f },

};

void CPickerDlg::CalculateLightDirSH()
{
	float vDir[3];
	float vOut[3][4];

	D3DXMATRIX mtx;

	D3DXMatrixRotationYawPitchRoll(&mtx, -m_fSlider1 * M_PI*2.f, M_PI-m_fSlider2 * M_PI*2.f, 0);

	vDir[0] = mtx(2,0);
	vDir[1] = mtx(2,1);
	vDir[2] = mtx(2,2);

	SHEvalDirectionalLight(2, vDir, 1,1,1, vOut[0],vOut[1],vOut[2]);

	char buffer[512];
	sprintf(buffer, "%5.2f %5.2f %5.2f\n", vDir[0], vDir[1], vDir[2]);
	OutputDebugString(buffer);

	g_vSH[0] = vOut[0][0];
	g_vSH[1] = vOut[0][1];
	g_vSH[2] = vOut[0][2];
	g_vSH[3] = vOut[0][3];

	sprintf(buffer, "%5.2f %5.2f %5.2f %5.2f\n", g_vSH[0], g_vSH[1], g_vSH[2], g_vSH[3]);
	OutputDebugString(buffer);

	// apply transfer matrix
	for (int i=0; i<4; i++)
	{
		g_vSH[i] = 
			transfermtx[i][0] * vOut[0][0] +
			transfermtx[i][1] * vOut[0][1] +
			transfermtx[i][2] * vOut[0][2] +
			transfermtx[i][3] * vOut[0][3];
	}


}

void CPickerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  // Get the minimum and maximum scroll-bar positions.
   int minpos;
   int maxpos;
   pScrollBar->GetScrollRange(&minpos, &maxpos); 
   maxpos = pScrollBar->GetScrollLimit();

   // Get the current position of scroll box.
   int curpos = pScrollBar->GetScrollPos();

   // Determine the new position of scroll box.
   switch (nSBCode)
   {
   case SB_LEFT:      // Scroll to far left.
      curpos = minpos;
      break;

   case SB_RIGHT:      // Scroll to far right.
      curpos = maxpos;
      break;

   case SB_ENDSCROLL:   // End scroll.
	   return CWnd::OnHScroll(nSBCode, nPos, pScrollBar);

   case SB_LINELEFT:      // Scroll left.
      if (curpos > minpos)
         curpos--;
      break;

   case SB_LINERIGHT:   // Scroll right.
      if (curpos < maxpos)
         curpos++;
      break;

   case SB_PAGELEFT:    // Scroll one page left.
   {
      // Get the page size. 
      SCROLLINFO   info;
      pScrollBar->GetScrollInfo(&info, SIF_ALL);
   
      if (curpos > minpos)
      curpos = max(minpos, curpos - (int) info.nPage);
   }
      break;

   case SB_PAGERIGHT:      // Scroll one page right.
   {
      // Get the page size. 
      SCROLLINFO   info;
      pScrollBar->GetScrollInfo(&info, SIF_ALL);

      if (curpos < maxpos)
         curpos = min(maxpos, curpos + (int) info.nPage);
   }
      break;

   case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
      curpos = nPos;      // of the scroll box at the end of the drag operation.
      break;

   case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
      curpos = nPos;     // position that the scroll box has been dragged to.
      break;
   }

	  {
		   float f = (float)curpos/100.f;

		   if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER1)
			   m_fSlider1 = f;
		   else
			   m_fSlider2 = f;

		   CalculateLightDirSH();

		   UpdateMainLpvsView();
	  }

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PICKER_POLYNO, m_iPolygon);
	DDX_Text(pDX, IDC_PICKER_CHARTNO, m_iChart);
	DDX_Text(pDX, IDC_PICKER_NODENAME, m_csNodeName);
	DDX_Slider(pDX, IDC_SLIDER1, m_iSlider1);
	DDX_Slider(pDX, IDC_SLIDER2, m_iSlider2);
}


BEGIN_MESSAGE_MAP(CPickerDlg, CDarkDialog)
	ON_WM_HSCROLL()
//	ON_BN_CLICKED(IDC_BUTTON1, OnCancelButtonClicked)
END_MESSAGE_MAP()


// CPickerDlg message handlers
