
#ifndef CAMERA2_PHOTOCAPTURE_H
#define CAMERA2_PHOTOCAPTURE_H

namespace Cam2
{

class C_Camera;

class C_PhotoCapture
{
public:
	C_PhotoCapture();
	~C_PhotoCapture();
	bool OnAppRender();
	void TakePhoto(S32 samples = 0, bool hideCapture = true, bool accurateInterval = false, bool forceDisableUI = false);
	void SetWidescreen(bool wide) { m_Widescreen = wide; }
	U32 TakingPhoto() const { return m_TakingPhoto; }

protected:
	bool LargePhoto() const { return m_LargePhoto; }
	bool HideCapture() const { return m_HideCapture; }
	F32 ShutterTime() const { return m_ShutterTime; }
	F32 Zoom() const { return m_Zoom; }
	F32 DeflectX() const { return m_DeflectX; }
	F32 DeflectY() const { return m_DeflectY; }
	void SetOverlap(F32 overlap);
	bool ForceDisableUI() const { return m_ForceDisableUI; }
	
private:
	bool m_TakePhoto, m_LargePhoto;
	S32 m_Samples;
	bool m_HideCapture;
	F32 m_ShutterTime;
	F32 m_Zoom, m_DeflectX, m_DeflectY;
	bool m_AccurateInterval;
	bool m_Widescreen;
	bool m_ForceDisableUI;
	U32 m_TakingPhoto;

	void RenderPhoto();
	void RenderLargePhoto();
	void RenderSingle();
	F32 PseudoNoise(int x);

	virtual void TurnStuffOff(bool /*restore*/) { }
	virtual const C_Camera* GetLiveCamera() { return 0; }
	virtual void RenderStart(S32 /*samples*/) { }
	virtual void RenderAddSample() { }
	virtual void RenderStop() { }
};

} // End of namespace

#endif
