#ifndef DMAN_GENERATE_WAVS_H
#define DMAN_GENERATE_WAVS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace GenerateWavs
{
	class C_Options
	{
	public:
		C_Options()
		{
			SetRecord(NULL);
			SetPath( wxGetApp().GetConfig().tts.path.Chars() );
		}

		C_Record* GetRecord()		const { return m_pRecord; }
		wxString  GetPath()			const { return m_path; }
		Btl::OpenHashSet<Character> GetCharactersInRecord() const;

		void SetRecord(C_Record* pRecord);

		void SetPath(const wxString& path) 
		{ 
			m_path = path; 
		}
						
	private:
		C_Record* m_pRecord;
		wxString  m_path;
	};

	class C_OptionsDlg : public wxDialog
	{
	public:
		C_OptionsDlg(wxWindow *parent, C_Options& optionsToFill);

	private:
		void OnOK(wxCommandEvent& event);

		wxTextCtrl* m_pPathTxtCtrl;
		C_Options&	m_optionsToFill;

		DECLARE_EVENT_TABLE()
	};

	class C_TTSSettingsTable : public wxGridTableBase
	{
	public:

		enum {
			COL_CHARACTER,
			COL_SPEAKER,
			COL_SPEED,
			COL_VOLUME,
			COL_MAX
		};

		C_TTSSettingsTable(C_Options& options);

		virtual int GetNumberRows();
		virtual int GetNumberCols();
		virtual bool IsEmptyCell( int row, int col );
		virtual wxString GetValue( int row, int col );
		virtual void SetValue( int row, int col, const wxString& value );

		virtual wxString GetColLabelValue( int col );
		virtual wxString GetRowLabelValue( int row );

		virtual wxString GetTypeName( int row, int col );
		virtual bool CanGetValueAs( int row, int col, const wxString& typeName );
		virtual bool CanSetValueAs( int row, int col, const wxString& typeName );

		virtual long GetValueAsLong( int row, int col );
		virtual void SetValueAsLong( int row, int col, long value );

	private:
		Btl::Vector<Character> m_rowsToCharacter;
	};

	bool Do(const C_Options& options);
}
}

#endif //DMAN_GENERATE_WAVS_H
