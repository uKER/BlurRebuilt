#ifndef DMAN_BATCH_GENERATE_H
#define DMAN_BATCH_GENERATE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace BatchGenerate
{
	class C_Options
	{
	public:
		C_Options()
		{
			SetGenerateNames(true);
			SetGenerateHashes(true);
			SetGenerateTTS(true);
			SetGenerateAudioXML(true);
		}

		Btl::Vector<Xt::DomRecord*>& GetRecords() { return m_records; }
		bool GenerateNames()	const { return m_generateNames; }
		bool GenerateHashes()	const { return m_generateHashes; }
		bool GenerateTTS()		const { return m_generateTTS; }
		bool GenerateAudioXML() const { return m_generateAudioXML; }

		void SetRecords(const Btl::Vector<Xt::DomRecord*>& records);
		void SetGenerateNames(bool generate)
		{ 
			m_generateNames = generate;	 
		}
		void SetGenerateHashes(bool generate)
		{ 
			m_generateHashes = generate; 
		}
		void SetGenerateTTS(bool generate)
		{ 
			m_generateTTS = generate; 
		}
		void SetGenerateAudioXML(bool generate)
		{ 
			m_generateAudioXML = generate; 
		}
					
	private:
		Btl::Vector<Xt::DomRecord*> m_records;
		bool m_generateNames;
		bool m_generateHashes;
		bool m_generateTTS;
		bool m_generateAudioXML;
	};

	class C_OptionsDlg : public wxDialog
	{
	public:

		C_OptionsDlg(wxWindow *parent, C_Options& optionsToFill);

	private:

		void OnOK(wxCommandEvent& event);

		wxCheckBox* m_generateNamesChkBox;
		wxCheckBox* m_generateHashesChkBox;
		wxCheckBox* m_generateTTSChkBox;
		wxCheckBox* m_generateAudioXMLChkBox;

		C_Options&	m_optionsToFill;

		DECLARE_EVENT_TABLE()
	};

	bool Do(const C_Options& options);
}
}

#endif //DMAN_BATCH_GENERATE_H
