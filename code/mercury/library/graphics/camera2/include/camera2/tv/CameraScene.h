
#ifndef CAMERA2_CAMERASCENE_H
#define CAMERA2_CAMERASCENE_H

#pragma warning (push, 3)
#include <btl/containers/vector.h>
#include <misc_shared/archive/archive.h>
#pragma warning (pop)

namespace Cam2
{
class C_AnimCamera;
class C_AnimData;
class C_Camera;
class C_TvZone;

class C_CameraScene
{
public:
	static const S32 Version = 22;
	Btl::Vector<C_AnimData*> m_cams;
	Btl::Vector<C_TvZone*> m_zones;

	C_CameraScene();
	~C_CameraScene();
	void Load(Util::C_String filename);
	void Update(F32 period);
	S32 WhichZone(const Maths::HVec3& point) const;
	S32 FindAnimData(const char* name) const;
	const C_AnimData* GetAnimData(S32 index) const;
	const C_AnimData* GetAnimData(const char* name) const;
	bool PlayAnimation(const char* name);
	const C_AnimCamera* AnimCamera() const { return m_AnimCamera; }
	const C_Camera* Camera() const;

private:
	C_AnimCamera* m_AnimCamera;
};

extern C_CameraScene& CameraScene();

} // End of namespace

#endif
