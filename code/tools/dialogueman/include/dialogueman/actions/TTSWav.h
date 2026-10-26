#ifndef DMAN_TTS_WAV_H
#define DMAN_TTS_WAV_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

struct Voice;

class C_ShouldOverwrite : public wxDialog
{
public:
	C_ShouldOverwrite(wxWindow *parent, const wxString& message);
private:
	void OnButtonPress(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

class C_TTSWav 
{
public:
	C_TTSWav();

	virtual ~C_TTSWav();
		
	/// Generates a wav file from a line 
	/// @param text The line to speak
	/// @param wavFileName The wav output filename
	bool ToWav(const Btl::String& text, const Btl::String& wavFileName, const Voice& voice);

	wxArrayString EnumerateSpeakers();

	/// Append custom data to wav file with "placeholder" token
	/// @see http://technology.niagarac.on.ca/courses/ctec1631/WavFileFormat.html
	/// @param wavFileName The wav output filename
	bool PutPlaceholderToken(const Btl::String& wavFileName);

public:

	/// Checks if this WAV file is a placeholder
	static bool IsPlaceholder(const Btl::String& wavFileName);

	static const Btl::String token;

private:

	CComPtr<ISpVoice> m_cpVoice;
	bool m_gotVoice;

	bool m_yesToAll;
	bool m_noToAll;
};

} //namespace DMan 

#endif //DMAN_TTS_WAV_H