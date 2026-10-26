#ifndef TTS_WAV_H
#define TTS_WAV_H


namespace TTSWav
{

class C_TTSWav : public Tools::C_ToolsApplication
{
public:
	C_TTSWav(
		const char *szName,			// the name of the application
		const char *szSubtitle,		// subtitle e.g. "bizarre creations lighting tool"
		const char *szLogger,		// name of the base logger for this application
		const char *szDmpPath,		// path where .dmp files are to be written out to
		bool bExecuter = false		// true if the application needs to send executer feedback
		);

	virtual ~C_TTSWav();

	/// Generates wav files from a list of lines described in an XML file 
	/// @param xmlFileName The XML filename to read
	bool BatchXmlToWav(const Btl::String& xmlFileName);

	/// Generates a wav file from a line 
	/// @param text The line to speak
	/// @param wavFileName The wav output filename
	bool ToWav(const Btl::String& text, const Btl::String& wavFileName, const Btl::String& voiceName = "");

	/// Append custom data to wav file with "placeholder" token
	/// @see http://technology.niagarac.on.ca/courses/ctec1631/WavFileFormat.html
	/// @param wavFileName The wav output filename
	bool PutPlaceholderToken(const Btl::String& wavFileName);

	/// Checks if this WAV file is a placeholder
	bool IsPlaceholder(const Btl::String& wavFileName);

	static const Btl::String token;

private:
	/// main callback function to be overloaded by derived class
	virtual	int Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress);

	bool ParseCommandLineOptions(
		int argc, 
		char* argv[], 
		C_Options& options);

	CComPtr<ISpVoice> m_cpVoice;
	bool gotVoice;
};

} //namespace TTSWav 

#endif //#ifndef TTS_WAV_H