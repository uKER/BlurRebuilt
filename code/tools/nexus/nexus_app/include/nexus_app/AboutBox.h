#ifndef NEXUS_ABOUT_BOX_H
#define NEXUS_ABOUT_BOX_H

namespace Nexus
{
	class AboutBox : public CDialog
	{
		DECLARE_DYNAMIC(AboutBox)

	public:
		AboutBox(CWnd* pParent = NULL);   // standard constructor
		virtual ~AboutBox();

	// Dialog Data
		enum { IDD = IDD_ABOUTBOX };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		CStatic m_versionTextCtrl;
		CStatic m_aftkTextCtrl;
		
		DECLARE_MESSAGE_MAP()
	public:
		virtual BOOL OnInitDialog();
	};
}

#endif
