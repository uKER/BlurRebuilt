#ifndef JOINTPROPERTIESPANEL_H
#define JOINTPROPERTIESPANEL_H

#include <sigslot/sigslot.h>

namespace Animation
{
	class C_Skeleton;
	class C_SkeletonInstance;
	class C_Pose;
}

class wxPropertyGridEvent;
class wxPropertyGridManager;
class C_JointProperties;

class C_JointPropertiesPanel: public wxPanel, public sigslot::has_slots<>
{
public:
	C_JointPropertiesPanel(
		wxWindow* parent,
		wxWindowID winId,
		const wxPoint& pos,
		const wxSize& size
	);
	
	virtual ~C_JointPropertiesPanel();
	
	void SetSkeletonInstance(Animation::C_SkeletonInstance* skeletonInstance);
	void SetPose(Animation::C_Pose* pose);
	void SetJointIndex(S32 jointIndex);
		
	void OnAnimationUpdated(F32 dt);
	void RefreshProperties();
	void RefreshAnimatedProperties();

	void OnPropertyChange(wxPropertyGridEvent& e);
	void OnSkeletonLoaded(const Animation::C_Skeleton* skeleton, Animation::C_SkeletonInstance* skeletonInstance, Animation::C_Pose* pose, const char* name);
	
	void UpdateParentNotebookTabCaption();
	
private:

	wxPropertyGridManager*			m_propertyGridManager;
	C_JointProperties*				m_jointProperties;
};




#endif
