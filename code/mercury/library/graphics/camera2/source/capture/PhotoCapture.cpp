
#include "stdafx.h"
#include <camera2/capture/PhotoCapture.h>

#include <camera2/camera.h>

// PhotoCapture code - This adjusts the camera matrix in order to repeatedly render the screen and
// create a higher-quality version

// It's designed to be used with Accumulation buffer in Render, but this is done using a
// virtual RenderAddSample method and implemented in the app
// (so that the Camera library isn't dependant on the Render library)

namespace Cam2
{

static Debug::Bool g_TakeLargePhoto("Camera2.PhotoMode.TakeLargePhoto", false);
static Debug::Int g_PhotoSamples("Camera2.PhotoMode.PhotoSamples", 5, Debug::Range(1, 16));
static Debug::Int g_LargeTiles("Camera2.PhotoMode.LargeTiles", 5, Debug::Range(1, 8));
static Debug::Float g_Overlap("Camera2.PhotoMode.Overlap", 0.25f, Debug::Range(0.0f, 0.5f));
static Debug::Bool g_MoveCamera("Camera2.PhotoMode.MoveCamera", true);
static Debug::Float g_ApertureScale("Camera2.PhotoMode.ApertureScale", 0.024f, Debug::Range(0.0f, 0.1f));
static Debug::Bool g_HideCapture("Camera2.PhotoMode.HideCapture", false);
extern Debug::Bool g_ShutterOverride;
extern Debug::Float g_ShutterAngle;

C_PhotoCapture::C_PhotoCapture()
{
	m_TakePhoto = m_LargePhoto = false;
	m_Samples = 0;
	m_HideCapture = false;
	m_ShutterTime = 0.f;
	m_Zoom = 1.f;
	m_DeflectX = m_DeflectY = 0.f;
	m_AccurateInterval = false;
	m_Widescreen = true;
	m_ForceDisableUI = false;
}

C_PhotoCapture::~C_PhotoCapture()
{
}

void C_PhotoCapture::SetOverlap(F32 overlap)
{
	g_Overlap = overlap;
}

// This is called in the GameApp::Render() method - if we are capturing something, it returns true
bool C_PhotoCapture::OnAppRender()
{
	// Check for "Take Photo" (either from the debug or from the game):
	if (m_TakePhoto)
	{
		RenderPhoto();
		m_TakePhoto = false;
		return true;
	}

	// Check for "Take Large Photo":
	if (g_TakeLargePhoto)
	{
		m_Samples = g_PhotoSamples;
		RenderLargePhoto();
		g_TakeLargePhoto = false;
		return true;
	}
	
	return false;
}

void C_PhotoCapture::TakePhoto(S32 samples, bool hideCapture, bool accurateInterval, bool forceDisableUI)
{
	m_TakePhoto = true;
	if (samples > 0)
		m_Samples = samples;
	else
		m_Samples = g_PhotoSamples;
	
	m_HideCapture = hideCapture || g_HideCapture;
	m_AccurateInterval = accurateInterval;
	m_ForceDisableUI = forceDisableUI;
}

void C_PhotoCapture::RenderPhoto()
{
	m_LargePhoto = false;
	// Disable stuff which won't work with photo mode:
	TurnStuffOff(false);
	m_TakingPhoto = 0xffffffff;
	m_Zoom = 1.f;
	m_DeflectX = m_DeflectY = 0.f;
	RenderSingle();
	TurnStuffOff(true);
	m_TakingPhoto = 0;
}

void C_PhotoCapture::RenderLargePhoto()
{
	m_LargePhoto = true;
	// Disable stuff which won't work with large photos:
	TurnStuffOff(false);
	m_TakingPhoto = 0xffffffff;

	// Work out overlap:
	S32 tiles = g_LargeTiles;
	F32 overlap = 1.f - g_Overlap;
	m_Zoom = (F32)tiles * overlap;

	for (S32 ty = 0; ty < tiles; ty++)
	{
		m_DeflectY = (F32)ty * -2.f - 1.f + (F32)tiles;
		m_DeflectY *= overlap;

		for (S32 tx = 0; tx < tiles; tx++)
		{
			m_DeflectX = (F32)tx * 2.f + 1.f - (F32)tiles;
			m_DeflectX *= overlap;
			RenderSingle();
		}
	}

	m_Zoom = 1.f;
	m_DeflectX = m_DeflectY = 0.f;

	TurnStuffOff(true);
	m_LargePhoto = false;
	m_TakingPhoto = 0;
}

void C_PhotoCapture::RenderSingle()
{
	const C_Camera* live = GetLiveCamera();
	if (!live)
		return;

	F32 shutterAngle = live->ShutterAngle();
	F32 idepth = live->Idepth();
	F32 aperture = Maths::Max(live->Aperture(), 0.f);

	bool prevOverride = g_ShutterOverride;
	F32 prevShutterAngle = g_ShutterAngle;

	if (g_MoveCamera)
	{
		g_ShutterOverride = true;
		g_ShutterAngle = 0.f;
	}

	S32 samples = m_Samples;
	S32 samples2 = samples * samples;
	
	F32 heightScale = m_Widescreen ? 1.f : 0.75f; // In 4:3 we need to squash the height to get a 16:9 image
		
	RenderStart(samples2);
	
	S32 sampleNum = 0;
	for (S32 sy = 0; sy < samples; sy++)
	for (S32 sx = 0; sx < samples; sx++, sampleNum++)
	{
		// Work out a fractional value going from 0 to 1:
		F32 frac = sampleNum / (F32)samples2;

		F32 dx = 2.f * (F32)(sx + 0.5f) / (F32)samples - 1.f;
		F32 dy = 2.f * (F32)(sy + 0.5f) / (F32)samples - 1.f;
		
		// A noise to sample points to reduce aliasing further:
		F32 rx = PseudoNoise(sampleNum);
		F32 ry = PseudoNoise(123 + sampleNum);
		dx += rx / (F32)samples;
		dy += ry / (F32)samples;

		// Work out where the shutter time will go between:
		if (m_AccurateInterval)
			m_ShutterTime = 1.f + (frac - 1.0f) * shutterAngle; // Accurate (mid frame to end of frame)
		else
			m_ShutterTime = 1.f + (frac - 0.5f) * shutterAngle; // Copy motion blur shader (so PhotoMode pics look the same)
		
		C_Camera::DeflectMatrix(m_Zoom, m_DeflectX + dx / 1280.f, m_DeflectY + dy / 720.f, heightScale);

		F32 movex = dx * aperture * g_ApertureScale;
		F32 movey = dy * aperture * g_ApertureScale;
		C_Camera::StrafeMatrix(movex, movey, -movex * idepth, -movey * idepth);

		if (g_MoveCamera)
			C_Camera::SetShutterEnd(m_ShutterTime);

		RenderAddSample();
	}

	C_Camera::DeflectMatrix(1.f, 0.f, 0.f, 1.f);
	C_Camera::StrafeMatrix(0.f, 0.f, 0.f, 0.f);
	C_Camera::SetShutterEnd(1.f);
	g_ShutterOverride = prevOverride;
	g_ShutterAngle = prevShutterAngle;

	RenderStop();
}

// Predictable random number generator for input 'x' (output is -1 to +1)
F32 C_PhotoCapture::PseudoNoise(int x)
{
	x = (x << 13) ^ x;
	x = x * (x * x * 19379 + 819233) + 1266122899;
	return ((x & 0x7fffffff) / 1073741823.5f) - 1.f;
}

} // End of namespace
