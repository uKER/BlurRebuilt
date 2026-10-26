#ifndef SKELETONPANEL_H
#define SKELETONPANEL_H

#include <wx/treectrl.h>
#include <btl/containers/vector.h>
#include <btl/utility/string.h>
#include <sigslot/sigslot.h>

namespace Animation
{
	class C_Skeleton;
	class C_SkeletonInstance;
	class C_Pose;
}

class C_SkeletonPanel: public wxPanel, public sigslot::has_slots<>
{
public:
	C_SkeletonPanel(
		wxWindow* parent,
		wxWindowID winId,
		const wxPoint& pos,
		const wxSize& size
	);

	void OnJointSelected(S32 jointIndex);
	
	void OnSkeletonLoaded(const Animation::C_Skeleton* skeleton, Animation::C_SkeletonInstance* skeletonInstance, Animation::C_Pose* pose, const char* name);
	
private:

	wxTreeItemId FindItemWithJointIndex(const wxTreeItemId& item, S32 jointIndex) const;

	void C_SkeletonPanel::BuildTreeFromSkeleton(
		wxTreeCtrl& treeCtrl,
		const Animation::C_Skeleton* skeleton,
		const Btl::String& filter
	) const;

	void OnItemMenu( wxTreeEvent& e );
	void OnItemSelected( wxTreeEvent& e );
	void OnFilterChanged( wxCommandEvent& e );
	void OnExpandAllChildren( wxCommandEvent& e );
	void OnCollapseAllChildren( wxCommandEvent& e );
	
	
	class C_JointTreeItemData: public wxTreeItemData
	{
	public:
		C_JointTreeItemData( S32 jointIndex ):
			m_jointIndex( jointIndex )
		{
		}
	
		S32 m_jointIndex;
	};

	const Animation::C_Skeleton* m_skeleton;
	wxStaticText* m_skeletonNameStaticText;
	wxButton* m_skeletonChooserButton;
	wxTreeCtrl* m_treeCtrl;
	wxTextCtrl* m_filterTextCtrl;
	
	Btl::Vector< wxTreeItemId > m_jointTreeItemIds;
	Btl::String m_filter;
	wxTreeItemId m_focusItem;
	bool m_ignoreJointSelectedSignal;
	
	DECLARE_CLASS(C_SkeletonPanel)
	DECLARE_EVENT_TABLE()
};

#endif
