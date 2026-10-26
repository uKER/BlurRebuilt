
#ifndef CAMERA2_FEED_H
#define CAMERA2_FEED_H

#pragma warning (disable: 4481) // nonstandard extension used: override specifier 'override'

#include "CamObj.h"

// Forward declarations
namespace Display
{
	class C_ViewSetup3D;
}

namespace Cam2
{

// An I_Feed shows the video feed from a camera (or set of cameras)
class C_Camera;
class I_Feed
{
public:
	I_Feed();
	virtual ~I_Feed() { }
	virtual void Update(F32) { }
	virtual const C_Camera& Live() const = 0; // The camera which is currently being shown

	S32 Type() const { return m_type; }
	Maths::HVec3 Position() const;
	Maths::C_Quaternion Orient() const;
	Maths::C_Quaternion Shaken() const;
	Maths::HMat43 Mat43() const;
	F32 Fov() const;

	void SetObject(const C_CamObj& object) { m_object = object; }
	const C_CamObj& Object() const { return m_object; }
	void SetScreenOffset(F32 offx, F32 offy) { m_ScreenOffsetX = offx; m_ScreenOffsetY = offy; }
	void ToViewSetup(Display::C_ViewSetup3D& view, bool blurEnd) const;

	static void DeflectMatrix(F32 zoom, F32 dx, F32 dy, F32 heightScale = 1.f);
	static void SetShutterEnd(F32 end);
	static void SetRenderOffset(F32 offset);	// [Blur2PC] see Feed.cpp
	static void Blur2PCAdvanceTick();	// [Blur2PC] once per decoupled update
	static U32 Blur2PCTick();
	static void StrafeMatrix(F32 movex, F32 movey, F32 anglex, F32 angley);
	static void SetGlobalShutter(bool enable, F32 angle);

protected:
	void SetType(S32 t) { m_type = t; }
	
private:
	S32 m_type;
	C_CamObj m_object; // Focus object
	F32 m_ScreenOffsetX, m_ScreenOffsetY;
};

// Specific Feed class - call Show to assign a feed
class C_Feed : public I_Feed
{
public:
	C_Feed() { m_feed = 0; }
	virtual void Update(F32) { }
	virtual const C_Camera& Live() const;
	void Show(const Cam2::I_Feed* feed) { m_feed = feed; }
	const Cam2::I_Feed* Current() const { return m_feed; }

private:
	const Cam2::I_Feed* m_feed;
};

} // End of namespace Cam2

#endif
