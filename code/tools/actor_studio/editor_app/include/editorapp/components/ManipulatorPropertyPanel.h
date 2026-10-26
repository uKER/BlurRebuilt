#ifndef EDITOR_TOOL_PROPERTY_PANEL
#define EDITOR_TOOL_PROPERTY_PANEL

#ifdef _MSC_VER
#pragma once
#endif

class wxPropertyGridManager;
class wxPropertyGridEvent;

namespace Editor
{
	class I_ManipulatorProperties;

	class C_ManipulatorPropertyPanel : public wxPanel
	{
	public:
		C_ManipulatorPropertyPanel(wxWindow *parent, wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxDefaultSize,
							long style = wxTAB_TRAVERSAL|wxNO_BORDER);

		~C_ManipulatorPropertyPanel();

		void OnPropertyChange(wxPropertyGridEvent& e);
		
		template <typename PropType, typename Manipulator>
		I_ManipulatorProperties* Register(const char* pName, Manipulator* pManipulator)
		{
			Btl::Map<Btl::String, I_ManipulatorProperties*>::Iterator it = 
				m_PropertiesMap.Find(Btl::String(pName));

			if (it == m_PropertiesMap.End())
			{
				m_pPropertyGridManager->AddPage(pName);
				m_pPropertyGridManager->SelectPage(pName);
				
				I_ManipulatorProperties *pManipulatorProps = new typename PropType(m_pPropertyGridManager, pManipulator);
				m_PropertiesMap[pName] = pManipulatorProps;
				m_pCurrentManipulatorProps = pManipulatorProps;

				return pManipulatorProps;
			}
			else
				return 0;
		}

		void SetActivePage(const char* pName);
		void RefreshProperties();

		void OnIdle(wxIdleEvent& e);

	private:
		wxPropertyGridManager		*m_pPropertyGridManager;
		I_ManipulatorProperties		*m_pCurrentManipulatorProps;
		Btl::Map<Btl::String, I_ManipulatorProperties*> m_PropertiesMap;

		Btl::String m_pageChange;

		DECLARE_EVENT_TABLE()
	};

}

#endif