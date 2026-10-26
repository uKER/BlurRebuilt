#ifndef PHOTOMODE_APPPHOTOCAPTURE_H
#define PHOTOMODE_APPPHOTOCAPTURE_H

#include <camera2/capture/PhotoCapture.h>

namespace Display2 { struct C_Texture; }

namespace Game
{
	namespace Actors
	{
		class C_VehicleActor;
	}
}

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
	bool Init();
	void Exit();
	Display2::C_Texture* GetTexture();

	bool LockBitmap(unsigned char*& retBits, S32& retPitch, S32& retWidth, S32& retHeight, S32& retFormat);
	void UnlockBitmap();

private:
	Render::Photo::C_AccumulateStrip* m_Accumulate;
	bool* m_OldBool;
	S32* m_OldInt;
	F32* m_OldFloat;
	Display2::C_Texture* m_OffscreenTexture;
	S32 m_ToneMapShader;

	void TurnStuffOff(bool restore) OVERRIDE;
	const Cam2::C_Camera* GetLiveCamera() OVERRIDE;
	void RenderStart(S32 samples) OVERRIDE;
	void RenderAddSample() OVERRIDE;
	void RenderStop() OVERRIDE;
	void MoveCars(F32 frac);
#ifndef CFG_GOLD
	static void MovePerkStash(F32 timeValue);
#endif
};

PhotoMode::C_AppPhotoCapture& AppPhotoCapture();

}

#endif // PHOTOMODE_PHOTOCAPTURE_H
