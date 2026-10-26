#ifndef AMAXCAM_RAYGROUP_H
#define AMAXCAM_RAYGROUP_H

namespace Collision { class C_Ray; };

namespace AmaxCam
{

class C_RayGroup
{
public:
	C_RayGroup();
	void Init();
	void Exit();
	void StartFrame();
	S32 NextRay();
	Collision::C_Ray* GetRay(S32 index);
	S32 SchedulerGroup() const { return m_SchedulerGroup; }
	void Flush();
	
private:
	Btl::Vector<Collision::C_Ray*> m_Rays;
	S32 m_SchedulerGroup;
	S32 m_NextRay; // Counts from 0, returning the next ray
};

extern C_RayGroup g_RayGroup;

} // End of namespace

#endif
