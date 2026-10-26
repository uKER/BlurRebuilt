#ifndef NEXUS_MESH_WELDING_OPTIONS_H
#define NEXUS_MESH_WELDING_OPTIONS_H

#include "afxcmn.h"
#include "afxwin.h"

namespace Nexus
{
	class MeshWeldingOptions : public CDialog
	{
		DECLARE_DYNAMIC(MeshWeldingOptions)

	public:
		MeshWeldingOptions(CWnd* pParent = NULL);   // standard constructor
		virtual ~MeshWeldingOptions();

	// Dialog Data
		enum { IDD = IDD_MESH_WELD_OPTIONS };

		F32 m_weldDistance;
		bool m_cancelled;

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

	private:
		CNumSpinCtrl m_weldDistanceSpin;
		CNumEdit m_weldDistanceEdit;
	public:
		virtual BOOL OnInitDialog();
		afx_msg void OnBnClickedOk();
		afx_msg void OnBnClickedCancel();
	};
}

#endif
