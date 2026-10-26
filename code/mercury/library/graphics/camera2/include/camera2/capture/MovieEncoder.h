
//#define COMPILE_OGG

namespace Sys
{
	class I_File;
}

namespace Cam2
{

class C_LockScreenInfo;
class C_OggEncoder;
class C_OggPacket;

class C_MovieSettings
{
public:
	C_MovieSettings();
	Util::C_String filename;
	S32 width, height, bitrate, quality;
	S32 fps;
};

class C_MovieEncoder
{
public:
	C_MovieEncoder();
	bool Open(const C_MovieSettings& settings);
	void Close();
	void AddFrame(const C_LockScreenInfo& si);

private:
	C_MovieSettings m_Settings;
	C_OggEncoder* m_Encoder;
	U8* m_VideoData;
	Sys::I_File* m_File;
	S32 m_TotalBytes; // Counter of video data written

	void WritePacket(C_OggPacket& op);
	void FlushOgg();
	void AddBytes(S32 amount);
	void GetYuvFrame(const C_LockScreenInfo& si);
};

}
