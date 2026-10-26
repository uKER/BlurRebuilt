#ifndef DMAN_AUDIO_XML_H
#define DMAN_AUDIO_XML_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace AudioXML
{
	class C_Options
	{
	public:
		C_Options()
		{
			SetRecord(NULL);
			SetAudioXMLPath( wxGetApp().GetConfig().audioXMLPath.Chars() );
			SetBankPCPath( wxGetApp().GetConfig().bankPCPath.Chars() );
			SetBank360Path( wxGetApp().GetConfig().bank360Path.Chars() );
			SetBankPS3Path(	wxGetApp().GetConfig().bankPS3Path.Chars() );
			SetBuild360Path( wxGetApp().GetConfig().build360Path.Chars() );
		}

		C_Record* GetRecord()		const { return m_pRecord; }
		wxString  GetAudioXMLPath()	const { return m_audioXmlPath; }
		wxString  GetBankPCPath()	const { return m_bankPCPath; }
		wxString  GetBank360Path()	const { return m_bank360Path; }
		wxString  GetBankPS3Path()	const { return m_bankPS3Path; }
		wxString  GetBuild360Path()	const { return m_build360Path; }

		void SetRecord(C_Record* pRecord) 
		{ 
			m_pRecord = pRecord; 
		}

		void SetAudioXMLPath(const wxString& audioXMLPath) 
		{ 
			m_audioXmlPath = audioXMLPath; 
		}

		void SetBankPCPath(const wxString& bankPCPath) 
		{ 
			m_bankPCPath = bankPCPath; 
		}

		void SetBank360Path(const wxString& bank360Path) 
		{ 
			m_bank360Path = bank360Path; 
		}

		void SetBankPS3Path(const wxString& bankPS3Path) 
		{ 
			m_bankPS3Path = bankPS3Path; 
		}

		void SetBuild360Path(const wxString& build360Path) 
		{ 
			m_build360Path = build360Path; 
		}
						
	private:
		C_Record* m_pRecord;
		wxString  m_audioXmlPath;
		wxString  m_bankPCPath;
		wxString  m_bank360Path;
		wxString  m_bankPS3Path;
		wxString  m_build360Path;
	};

	class C_OptionsDlg : public wxDialog
	{
	public:

		C_OptionsDlg(wxWindow *parent, C_Options& optionsToFill);

	private:

		void OnOutputChanged(wxCommandEvent& event);
		void OnOK(wxCommandEvent& event);

		wxTextCtrl* m_pAudioXmlPathTxtCtrl;
		wxTextCtrl* m_pBankPCPathTxtCtrl;
		wxTextCtrl* m_pBank360PathTxtCtrl;
		wxTextCtrl* m_pBankPS3PathTxtCtrl;
		wxTextCtrl* m_pBuild360PathTxtCtrl;

		C_Options&	m_optionsToFill;

		DECLARE_EVENT_TABLE()
	};

	bool Do(const C_Options& options);

	
}
}

#endif //DMAN_AUDIO_XML_H
