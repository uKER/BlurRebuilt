#include "stdafx.h"

#include <wx/dir.h>
#include <wx/progdlg.h>
#include <wx/clipbrd.h>

#include <DialogueMan/Actions/TTSFilesInFolder.h>
#include <DialogueMan/Actions/TTSWav.h>
#include <system/misc/BtlStringUtil.h>


namespace DMan
{

namespace TTSFilesInFolder
{

bool Do(const C_Options& options)
{
	LOGINFO(wxGetApp().GetLogger(), ">>> STARTING TO DISPLAY TTS FILES IN FOLDER \"%s\"", 
		options.GetFolder());
	
	wxDir dir(options.GetFolder());

    if ( !dir.IsOpened() )
    {
        LOGERROR(wxGetApp().GetLogger(), "Cannot open folder \"%s\"", 
			options.GetFolder());
        return false;
    }
		
	wxProgressDialog progress(_T("Searching for TTS Files..."),
								_T("Please Wait..."),
								100,    // range
								NULL,   // parent
								wxPD_CAN_ABORT |
								wxPD_APP_MODAL |
								wxPD_AUTO_HIDE);

    wxString filename;
	wxString allFilenames;

	bool cont = dir.GetFirst(&filename, "*.wav");
    while ( cont )
    {
		if (!progress.Pulse())
			break;
		
		wxString fullFilename = wxString(Misc::AddTrailingSlash(options.GetFolder().c_str(), '\\').Chars()) + filename;
		if( C_TTSWav::IsPlaceholder(fullFilename.c_str()) )
		{
			LOGINFO(wxGetApp().GetLogger(), "%s", fullFilename);
			allFilenames += fullFilename;
			allFilenames += "\n";
		}
        cont = dir.GetNext(&filename);
    }

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(allFilenames) );
		wxTheClipboard->Close();
	}
	wxTheClipboard->Close();

	LOGINFO(wxGetApp().GetLogger(), "<<< DONE DISPLAYING TTS FILES IN FOLDER \"%s\"", 
		options.GetFolder());

	return true;
}

} //namespace GenerateNames

} //namespace DMan