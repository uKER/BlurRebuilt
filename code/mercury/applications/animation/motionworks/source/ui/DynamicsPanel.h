#ifndef DYNAMICSPANEL_H
#define DYNAMICSPANEL_H

#include <sigslot/sigslot.h>

class wxPropertyGridEvent;
class wxPropertyGridManager;
class C_DynamicsProperties;
class C_DynamicsContext;

namespace Animation
{
	class C_Skeleton;
	class C_SkeletonInstance;
	class C_Pose;
}

class C_DynamicsPanel: public wxPanel, public sigslot::has_slots<>
{
public:
	C_DynamicsPanel(
		wxWindow* parent,
		wxWindowID winId,
		const wxPoint& pos,
		const wxSize& size
	);
	
	virtual ~C_DynamicsPanel();
	
	void SetDynamicsContext(C_DynamicsContext* dynamicsContext);
	void RefreshProperties();

	void OnPropertyChange(wxPropertyGridEvent& e);
	void OnSkeletonLoaded(const Animation::C_Skeleton* skeleton, Animation::C_SkeletonInstance* skeletonInstance, Animation::C_Pose* pose, const char* name);
	
private:

	wxPropertyGridManager*	m_propertyGridManager;
	C_DynamicsProperties*	m_dynamicsProperties;
	C_DynamicsContext*		m_dynamicsContext;
};

#endif