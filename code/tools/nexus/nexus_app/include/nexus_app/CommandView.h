#ifndef NEXUS_COMMAND_VIEW_H
#define NEXUS_COMMAND_VIEW_H

namespace Nexus
{
	class CommandView : public CExtResizableDialog
	{
		DECLARE_DYNAMIC(CommandView)

	public:
		CommandView(CWnd* pParent = NULL);   // standard constructor
		virtual ~CommandView();

		void Print(const std::string& text);

		static void	PrintStatic(const std::string& text);

		void Clear();

	// Dialog Data
		enum { IDD = IDD_COMMAND_VIEW };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

		CommandEdit m_commandEdit;
		CChatRichEd m_outputREdit;

		CString m_outputText;

	public:
		virtual BOOL OnInitDialog();
		void OnOK();
	};
}

#endif