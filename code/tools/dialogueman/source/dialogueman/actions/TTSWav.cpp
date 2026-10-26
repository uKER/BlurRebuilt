#include "stdafx.h"

#include <system/misc/BtlStringUtil.h>
#include <DialogueMan/Actions/TTSWav.h>
#include <DialogueMan/App.h>

namespace DMan
{

BEGIN_EVENT_TABLE(C_ShouldOverwrite, wxDialog)
	EVT_BUTTON(wxID_YES, C_ShouldOverwrite::OnButtonPress)
	EVT_BUTTON(wxID_YESTOALL, C_ShouldOverwrite::OnButtonPress)
	EVT_BUTTON(wxID_NOTOALL, C_ShouldOverwrite::OnButtonPress)
	EVT_BUTTON(wxID_NO, C_ShouldOverwrite::OnButtonPress)
END_EVENT_TABLE()

C_ShouldOverwrite::C_ShouldOverwrite(wxWindow *parent, const wxString& message)
:
wxDialog(
	parent,
	wxID_ANY,
	wxString(wxT("Generate TTS")))
{	
	wxBoxSizer* pMainSizer = new wxBoxSizer(wxVERTICAL);

	pMainSizer->Add(new wxStaticText(this, wxID_ANY, message), 0, wxALL, 8);

	wxBoxSizer* pButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
	pButtonsSizer->Add(new wxButton(this, wxID_YES, wxT("Yes")), 0, wxALL, 3);
	pButtonsSizer->Add(new wxButton(this, wxID_YESTOALL, wxT("Yes to All")), 0, wxALL, 3);
	pButtonsSizer->Add(new wxButton(this, wxID_NO, wxT("No")), 0, wxALL, 3);
	pButtonsSizer->Add(new wxButton(this, wxID_NOTOALL, wxT("No to All")), 0, wxALL, 3);

	pMainSizer->Add(pButtonsSizer, 0, wxALL | wxALIGN_CENTER, 8);
		
	pMainSizer->SetSizeHints(this);
    pMainSizer->Fit(this);

    SetSizer(pMainSizer);
	CenterOnParent();
	Layout();
}

void C_ShouldOverwrite::OnButtonPress(wxCommandEvent& event)
{
	EndModal(event.GetId());
}

const Btl::String C_TTSWav::token = "placeholder";

C_TTSWav::C_TTSWav() 
{
	m_gotVoice = false;
	m_yesToAll = false;
	m_noToAll = false;


	CoInitialize( NULL );  //Start COM
	if ( SUCCEEDED(m_cpVoice.CoCreateInstance(CLSID_SpVoice)) )
	{
		m_gotVoice = true;
	}
}

C_TTSWav::~C_TTSWav()
{
	if(m_gotVoice)
	{
		m_cpVoice.Release();
	}
	CoUninitialize();  //Stop COM
}

bool C_TTSWav::ToWav(const Btl::String& text, const Btl::String& wavFileName, const Voice& voice)
{	
	Btl::String& _wavFileName = const_cast<Btl::String&>(wavFileName);

	if(!m_gotVoice)
	{
		LOGERROR(wxGetApp().GetLogger(), "Problem accessing TTS Engine\n");
		return false;
	}

	Btl::String dir = Misc::ExtractDirectory(_wavFileName);
	Btl::String extension = Misc::ExtractFileExtension(_wavFileName);

	if(extension != "wav")
	{
		_wavFileName += ".wav";	
	}

	if (!Sys::C_FileSystem::Instance().FileExists(dir.Chars()) &&
		!Sys::C_FileSystem::Instance().CreateDirectory(dir.Chars()))
	{
		LOGERROR(wxGetApp().GetLogger(), "Incorrect filename: %s. Verify filename or try running Generate Names first", 
			_wavFileName.Length() == 0 ? "null" : _wavFileName.Chars());
		return false;
	}

	if(Sys::C_FileSystem::Instance().FileExists(_wavFileName.Chars()) && !IsPlaceholder(_wavFileName))
	{
		if(m_noToAll)
		{
			LOGINFO(wxGetApp().GetLogger(), "Skipped: %s", _wavFileName);
			return false;
		}

		if(!m_yesToAll)
		{
			Btl::String msg = Format("Non-TTS file \"%s\" already exists. Overwrite?", Misc::ExtractFilename(_wavFileName));

			C_ShouldOverwrite dlg(NULL, msg.Chars());
			switch(dlg.ShowModal())
			{
			case wxID_YES:
				break;
			case wxID_YESTOALL:
				m_yesToAll = true;
				break;
			case wxID_NO:
				LOGINFO(wxGetApp().GetLogger(), "Skipped: %s", _wavFileName);
				return false;
			case wxID_NOTOALL:
				LOGINFO(wxGetApp().GetLogger(), "Skipped: %s", _wavFileName);
				m_noToAll = true;
				return false;		
			}
		}
	}

	HRESULT							hr = S_OK;
	CComPtr <ISpStream>				cpStream;
	CSpStreamFormat					cAudioFmt;
	CComPtr <ISpObjectToken>		cpToken;
	CComPtr <IEnumSpObjectTokens>	cpEnum;

	hr = cAudioFmt.AssignFormat(SPSF_48kHz16BitMono);
		
	//Enumerate voice tokens with attribute "Name=voiceName" 
	if(SUCCEEDED(hr))
	{
		Btl::String voiceQuery = "Name=";
		voiceQuery += voice.speaker;
		
		//convert to widechars
		const char* src = voiceQuery.Chars();
		//get the destination size by calling MultiByteToWideChar with a NULL destination buffer
		int destSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, NULL, 0);
		wchar_t* dest = new wchar_t[destSize];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dest, destSize);
		hr = SpEnumTokens(SPCAT_VOICES, dest, NULL, &cpEnum);
		delete[] dest;
	}
    
	//Get the closest token
	if(SUCCEEDED(hr))
	{
		ULONG count; 
		hr = cpEnum->GetCount(&count);
		hr = cpEnum ->Next(1, &cpToken, NULL);
	}
	
	//set the voice 
	if(SUCCEEDED(hr))
	{
		hr = m_cpVoice->SetVoice(cpToken); //If no voice is found, it will use the windows default
		hr = m_cpVoice->SetRate(voice.speed);
		hr = m_cpVoice->SetVolume(voice.volume);
	}
	
	
	//Call SPBindToFile, a SAPI helper method,  to bind the audio stream to the file
	if(SUCCEEDED(hr))
	{
		hr = SPBindToFile( _wavFileName.Chars(),  SPFM_CREATE_ALWAYS, 
			&cpStream, & cAudioFmt.FormatId(),cAudioFmt.WaveFormatExPtr() );
	}
	
	//set the output to cpStream so that the output audio data will be stored in cpStream
	if(SUCCEEDED(hr))
	{
		hr = m_cpVoice->SetOutput( cpStream, TRUE );
	}

	//Speak the text asynchronously
	if(SUCCEEDED(hr))
	{
		//convert to widechars
		const char* src = text.Chars();
		//get the destination size by calling MultiByteToWideChar with a NULL destination buffer
		int destSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, NULL, 0);
		wchar_t* dest = new wchar_t[destSize];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dest, destSize);
		hr = m_cpVoice->Speak(dest,  SPF_DEFAULT, NULL );
		delete[] dest;
	}
	
	//close the stream
	if(SUCCEEDED(hr))
	{
		hr = cpStream->Close();
	}

	//Release the stream object
	cpStream.Release();

	bool tokenAdded = PutPlaceholderToken(_wavFileName);

	if(SUCCEEDED(hr) && tokenAdded)
	{
		LOGINFO(wxGetApp().GetLogger(), "Generated TTS WAV file: %s", _wavFileName);
		return true;
	} 
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to generate WAV file: %s", _wavFileName);
		return false;
	}
}

wxArrayString C_TTSWav::EnumerateSpeakers()
{
	wxArrayString speakers;

	if(!m_gotVoice)
	{
		LOGERROR(wxGetApp().GetLogger(), "Problem accessing TTS Engine\n");
		return speakers;
	}

	CComPtr <IEnumSpObjectTokens> cpEnum;
    ISpObjectToken *pToken = NULL;

    WCHAR *pszCurTokenId = NULL;
    ULONG ulIndex = 0, ulNumTokens = 0, ulCurToken = -1;

	HRESULT hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);

	if (hr == S_OK) {
        hr = cpEnum->GetCount( &ulNumTokens );
        
        if (SUCCEEDED(hr) && ulNumTokens != 0) {
            

			wchar_t* src;

            while (cpEnum->Next(1, &pToken, NULL) == S_OK) {
                hr = SpGetDescription(pToken, &src);
				BOOL usedDefaultChar;
				//get the destination size by calling WideCharToMultiByte with a NULL destination buffer
				int destSize = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, src, -1, NULL, 0, NULL, &usedDefaultChar);
				char* dest = new char[destSize];
				WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, src, -1, dest, destSize, NULL, &usedDefaultChar);
				speakers.Add(dest);
                ulIndex++;                
                pToken->Release();
                pToken = NULL;
				delete[] dest;
            }                   
        }
    }

	return speakers;
}

bool C_TTSWav::PutPlaceholderToken(const Btl::String& wavFileName)
{
	//Open file for read and write
	std::fstream wavFile(
		wavFileName.Chars(),
		std::ios::in | std::ios::out | std::ios::binary );

	if (wavFile.is_open())
	{
		wavFile.seekg(0, std::ios::end);		
		std::fstream::pos_type prevSize;
		prevSize = wavFile.tellg();
		
		U32 tokenLen = token.Length();
		U32 newSize = (U32)prevSize + tokenLen - 8; //length chunk doesnt include RIFF(4) and its own size(4)
					
		//Fix length chunk (since we will add token at the end)
		wavFile.seekp(4, std::ios::beg); //length chunk starts at 4th byte 
		char* pNewSize = (char*)&newSize;
		wavFile << pNewSize[0] << pNewSize[1] << pNewSize[2] << pNewSize[3];

		//Append custom data
		wavFile.seekp(0, std::ios::end);
		wavFile << token.Chars();

		//Close File
		wavFile.close();

		//assert(IsPlaceholder(wavFileName));

		return true;
	}
	LOGERROR(wxGetApp().GetLogger(), "Failed to add custom data to WAV file: %s", wavFileName);
	return false;
}

bool C_TTSWav::IsPlaceholder(const Btl::String& wavFileName)
{
	 std::ifstream wavFile(
		wavFileName.Chars(),
		std::ios::in | std::ios::binary | std::ios::ate ); //open at the end

	if (wavFile.is_open())
	{
		
		char* ph = new char[token.Length() + 1];
		int size = wavFile.tellg();
		wavFile.seekg(size - token.Length(), std::ios::beg);
		wavFile.read(ph, token.Length());
		ph[token.Length()] = '\0';
		bool equal = !strcmp(ph, token.Chars());
		delete[] ph;
		wavFile.close();
		return equal;
	}
	LOGERROR(wxGetApp().GetLogger(), "Failed to get custom placeholder data on: %s", wavFileName);
	return false;
}

} //namespace DMan
