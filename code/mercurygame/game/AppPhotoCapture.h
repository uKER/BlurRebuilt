#ifndef PHOTOMODE_APPPHOTOCAPTURE_H
#define PHOTOMODE_APPPHOTOCAPTURE_H

#include <camera2/capture/PhotoCapture.h>
#include <Display.h>

namespace Render
{
	namespace Photo
	{
		class C_AccumulateStrip;
	}
}

namespace PhotoMode
{

class C_AppPhotoCapture : public Cam2::C_PhotoCapture
{
public:
	C_AppPhotoCapture();
	~C_AppPhotoCapture();
	Render::Photo::C_AccumulateStrip* Accumulate() { return m_Accumulate; }

private:
	Render::Photo::C_AccumulateStrip* m_Accumulate;
	bool* m_OldBool;
	Display::C_Texture2D* m_OffscreenTexture;

	void TurnStuffOff(bool largePhoto, bool restore) OVERRIDE;
	const Cam2::C_Camera* GetLiveCamera() OVERRIDE;
	void RenderStart(S32 samples) OVERRIDE;
	void RenderAddSample() OVERRIDE;
	void RenderStop() OVERRIDE;
};

}

#endif // PHOTOMODE_PHOTOCAPTURE_H
