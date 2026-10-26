#ifndef DMAN_GENERATE_NAMES_H
#define DMAN_GENERATE_NAMES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace GenerateNames
{
	class C_Options
	{
	public:
		C_Options()
		{
			SetRecord(NULL);
			SetRowNameFormat( wxGetApp().GetConfig().locId.Chars() );
			SetWavNameFormat( wxGetApp().GetConfig().tts.fileName.Chars() );
		}

		C_Record* GetRecord()		const { return m_pRecord; }
		wxString GetRowNameFormat() const { return m_rowNameFormat; }
		wxString GetWavNameFormat() const { return m_wavNameFormat; }

		void SetRowNameFormat(const wxString& rowNameFormat) 
		{ 
			m_rowNameFormat = rowNameFormat; 
		}

		void SetWavNameFormat(const wxString& wavNameFormat) 
		{ 
			m_wavNameFormat = wavNameFormat; 
		}

		void SetRecord(C_Record* pRecord) 
		{ 
			m_pRecord = pRecord; 
		}
						
	private:
		C_Record* m_pRecord;
		wxString  m_rowNameFormat;
		wxString  m_wavNameFormat;
	};

	class C_OptionsDlg : public wxDialog
	{
	public:
		C_OptionsDlg(wxWindow *parent, C_Options& optionsToFill);
			  
	private:
		void OnOK(wxCommandEvent& event);

		wxTextCtrl*		m_pRowNameFormatTxtCtrl;
		wxTextCtrl*		m_pWavNameFormatTxtCtrl;

		C_Options&		m_optionsToFill;

		static wxString	defaultRowNameFormat;
		static wxString	defaultWavNameFormat;

		DECLARE_EVENT_TABLE()
	};

	bool Do(const C_Options& options);
}
}

#endif //DMAN_GENERATE_NAMES_H
