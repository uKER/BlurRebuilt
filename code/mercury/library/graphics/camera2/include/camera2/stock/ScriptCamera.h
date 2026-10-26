
#ifndef CAMERA2_SCRIPTCAMERA_H
#define CAMERA2_SCRIPTCAMERA_H

#include <camera2/Camera.h>

namespace Cam2
{

// Presentation key frames:
class PreKey
{
public:
	PreKey();
	F32 period; // Period of time to move to this frame
	S32 flags; // &1 = object relative
	F32 x,y,z, ow,ox,oy,oz, fov;
	F32 idepth, aperture;
	F32 gameSpeed;
};

struct CamState
{
public:
	CamState();
	Maths::HVec3 position;
	Maths::C_Quaternion orient, shaken;
	F32 fov, idepth, aperture;
	F32 gameSpeed;
};

class C_ScriptCamera : public C_Camera
{
public:
	C_ScriptCamera();
	virtual ~C_ScriptCamera();

	void OnUpdate() OVERRIDE;
	void LoadScript();
	float GameSpeed() const;

private:
	char *m_script; // The current presentation camera script
	S32 m_scriptPos; // Position (in characters) in the current script
	char m_scriptLine[256]; // Current line in the script
	S32 m_scriptCount; // Script line counter (for information only)

	PreKey m_key[4]; // Four keyframes (from the camera script)
	PreKey m_dot[4]; // Frames used for Catmull-Rom interpolation)
	F32 m_counter;
	float m_GameSpeed;

	S32 ScriptNextLine();
	void ParseKeyFrame(PreKey *key, char *text);
	void CheckScriptFlip();
	void UpdateScript();
	void PresentCatmull(PreKey *key, F32 frac);
	void GetFrame(CamState *pcam, PreKey *paf);
};

} // End of namespace Cam2

#endif
