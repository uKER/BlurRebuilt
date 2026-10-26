#ifndef NEXUS_COMMAND_EDIT_H
#define NEXUS_COMMAND_EDIT_H

namespace Nexus
{
	class CommandEdit : public CEdit
	{
	public:
		CommandEdit();

		~CommandEdit();

		void ClearCommand();

		void AddHistory(char* history);

	protected:
		void KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags, bool pressed);

		List<std::string> m_commandHistory;
		ListItem<std::string>* m_pHistoryItem;

	public:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	};
}

#endif