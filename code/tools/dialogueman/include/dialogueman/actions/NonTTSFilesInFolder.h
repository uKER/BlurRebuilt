#ifndef DMAN_NON_TTS_FILES_IN_FOLDER_H
#define DMAN_NON_TTS_FILES_IN_FOLDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace NonTTSFilesInFolder
{
	class C_Options
	{
	public:
		C_Options()
		{
			SetFolder( "" );
		}

		wxString GetFolder() const { return m_folder; }

		void SetFolder(const wxString& folder) 
		{ 
			m_folder = folder; 
		}
		
	private:
		wxString m_folder;
	};

	bool Do(const C_Options& options);
}
}

#endif //DMAN_NON_TTS_FILES_IN_FOLDER_H
