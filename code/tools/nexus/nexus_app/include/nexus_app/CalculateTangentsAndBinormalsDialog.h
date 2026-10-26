#ifndef NEXUS_CALCULATE_TANGENTS_AND_BINORMALS_DIALOG_H
#define NEXUS_CALCULATE_TANGENTS_AND_BINORMALS_DIALOG_H

#include "afxcmn.h"
#include "afxwin.h"

namespace Nexus
{
	class CalculateTangentsAndBinormalsDialog : public CDialog
	{
		DECLARE_DYNAMIC(CalculateTangentsAndBinormalsDialog)

	public:
		CalculateTangentsAndBinormalsDialog(CWnd* pParent = NULL);   // standard constructor
		virtual ~CalculateTangentsAndBinormalsDialog();

	// Dialog Data
		enum { IDD = IDD_TANGENT_SPACE_OPTIONS };

		F32 m_mag;
		F32 m_angle;
		F32 m_threshold;
		bool m_cancelled;

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

	public:
		afx_msg void OnBnClickedCtbRespectsplit();
		BOOL m_bRespectSplitCheck;
		BOOL m_bNormaliseCheck;
		BOOL m_bWeldCheck;
	private:
		CNumSpinCtrl m_magSpin;
		CNumSpinCtrl m_angleSpin;
		CNumSpinCtrl m_weldDistanceSpin;
	public:
		afx_msg void OnDeltaposCtbTcreasespin(NMHDR *pNMHDR, LRESULT *pResult);
		virtual BOOL OnInitDialog();
	private:
		CNumEdit m_magEdit;
		CNumEdit m_angleEdit;
		CNumEdit m_weldDistanceEdit;
	public:
		afx_msg void OnBnClickedOk();
		afx_msg void OnBnClickedCancel();
	};
}

#endif
