
#include <core/containers/string.h>

class C_DiskWriter
{
public:
	C_DiskWriter();
	~C_DiskWriter();
	void BeginImage(S32 width, S32 height);
	void WriteLine(U8* data, S32 len);
	void EndImage();
	void SetPath(const Util::C_String& path) { m_path = path; }
	const Util::C_String& LastFilename() const { return m_lastFilename; }
	float ImagesPerSecond();

	FILE* m_outFile;

private:
	void SkipExisting();

	Util::C_String m_path;
	Util::C_String m_lastFilename;
	int m_screenNumber;
	int m_lastTick;
	float m_imagePeriod;
};
