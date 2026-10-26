#include "stdafx.h"
#include "ConstraintsPanel.h"
#include <animation/tools/ExportConstraints.h>
#include <animation/skeleton/Skeleton.h>
#include "../PreviewState.h"
#include "GlobalSignals.h"
#include <wx/listctrl.h>
#include <wx/wupdlock.h>

C_ConstraintsPanel::C_ConstraintsPanel(
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size,
	long style
):
	wxPanel(parent, winId, pos, size, style),
	m_listCtrl(NULL),
	m_constraints(NULL),
	m_skeleton(NULL)
{

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	
	m_listCtrl = new wxListCtrl(
		this,
		wxID_ANY,
		wxDefaultPosition,
		wxDefaultSize,
		wxLC_REPORT | wxLC_VRULES // | wxLC_SINGLE_SEL
	);
	
	m_listCtrl->InsertColumn(E_Column_Name, _T("Joint"));
	m_listCtrl->InsertColumn(E_Column_Type, _T("Type"));
	m_listCtrl->InsertColumn(E_Column_Order, _T("Order"));
	m_listCtrl->InsertColumn(E_Column_Details, _T("Details"));
		
	sizer->Add( m_listCtrl, wxSizerFlags(1).Expand() );
	
	m_infoText = new wxStaticText(this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
	
	sizer->Add( m_infoText, wxSizerFlags(0).Expand().Border(wxALL, 4) );
	
	SetSizer(sizer);
	
	Connect(m_listCtrl->GetId(), wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler(C_ConstraintsPanel::OnColumnClicked));
	Connect(m_listCtrl->GetId(), wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(C_ConstraintsPanel::OnItemSelected));
	Connect(m_listCtrl->GetId(), wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(C_ConstraintsPanel::OnItemSelected));
	Connect(m_listCtrl->GetId(), wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(C_ConstraintsPanel::OnItemActivated));

	m_sortColumn = E_Column_Order;
	m_sortAscending = true;
}


void C_ConstraintsPanel::SetConstraints( const AnimationTools::C_ExportConstraints* constraints, const Animation::C_Skeleton* skeleton )
{
	m_constraints = constraints;
	m_skeleton = skeleton;
	
	Populate();
}


void C_ConstraintsPanel::OnColumnClicked(wxListEvent& e)
{
	if ( m_constraints == NULL || m_skeleton == NULL || m_listCtrl == NULL ) return;
	
	E_Column newCol = static_cast<E_Column>(e.GetColumn());
	
	if (newCol == E_Column_Details)
	{
		// Never sort by details column.
		return;
	}
	
	if (newCol == m_sortColumn)
	{
		// Same column, toggle between ascending/descending sort.
		m_sortAscending = !m_sortAscending;
	}
	else
	{
		// Different column, default to ascending sort.
		m_sortAscending = true;
		m_sortColumn = newCol;
	}
	
	Sort(m_sortColumn, m_sortAscending);
}

void C_ConstraintsPanel::OnItemSelected(wxListEvent& e)
{
	U32 selCount = m_listCtrl->GetSelectedItemCount();
	
	if (selCount > 0)
	{
		m_infoText->SetLabel(wxString::Format("%d constraint%s selected.", selCount, selCount == 1 ? "" : "s"));
	}
	else
	{
		if (m_constraints != NULL)
		{
			m_infoText->SetLabel(wxString::Format("%d constraints.", m_constraints->GetNumConstraints()));
		}
		else
		{
			m_infoText->SetLabel(_T(""));
		}
	}
	
	gClipPreviewState->selectedConstraints.Clear();
	
	// Mark selected constraints.
	long itemIndex = -1;
	do
	{
		itemIndex = m_listCtrl->GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		
		if (itemIndex != -1)
		{
			// Get order.
			const AnimationTools::C_ExportConstraint* c = reinterpret_cast<const AnimationTools::C_ExportConstraint*>(m_listCtrl->GetItemData(itemIndex));
			gClipPreviewState->selectedConstraints.PushBack(c->order);
		}
	
	} while (itemIndex != -1);
}

void C_ConstraintsPanel::OnItemActivated(wxListEvent& e)
{
	const AnimationTools::C_ExportConstraint* c = reinterpret_cast<const AnimationTools::C_ExportConstraint*>(e.GetItem().GetData());
	if (c == NULL) return;
	
	C_GlobalSignals::Instance().jointSelected(c->nodeIndex);
}

inline F32 LimitToDeg(F32 limit)
{
	return Maths::RadToDeg(asinf(limit) * 2.f);
}

static const char* kAxisNames[3] = 	{ "X", "Y", "Z" };

wxString AxisToSimpleString(const MathsStorage::Float3& axis)
{
	F32 lengthSquared = axis.x*axis.x+axis.y*axis.y+axis.z*axis.z;
	
	bool full = true;
	
	if (Maths::Abs(1.f - lengthSquared) < 0.001f)
	{
		U32 largestAxis = 0;
		F32 largest = 0.f;
		for (U32 i = 0; i < 3; ++i)
		{
			if (Maths::Abs(axis.v[i]) > largest)
			{
				largest = Maths::Abs(axis.v[i]);
				largestAxis = i;
			}
		}
		
		return wxString::Format("%s%s", axis.v[largestAxis] < 0.f ? "-" : "+", kAxisNames[largestAxis]);		
	}
	
	return wxString::Format("(%.3f,%.3f,%.3f)", axis.x, axis.y, axis.z);
}

wxString LimitsToSimpleString(const MathsStorage::Float3& minLimits, const MathsStorage::Float3& maxLimits, F32 inactiveThresold)
{
	wxString str;
	
	bool notFirst = false;

	for (U32 i = 0; i < 3; ++i)
	{
		F32 minLimit = minLimits.v[i];
		F32 absMinLimit = Maths::Abs(minLimits.v[i]);
		if (absMinLimit < inactiveThresold)
		{
			if (notFirst)
			{
				str += "; ";	
			}
			else
			{
				str += "(";
				notFirst = true;
			}
			str += wxString::Format("Min%s: %.3f", kAxisNames[i], minLimit);
		}
		
		F32 maxLimit = maxLimits.v[i];
		F32 absMaxLimit = Maths::Abs(maxLimits.v[i]);
		if (absMaxLimit < inactiveThresold)
		{
			if (notFirst)
			{
				str += "; ";	
			}
			else
			{
				str += "(";
				notFirst = true;
			}
			str += wxString::Format("Max%s: %.3f", kAxisNames[i], maxLimit);
		}
	}
	
	if (notFirst)
	{
		str += ")";
	}
	else
	{
		str = "None";
	}

	return str;	

}


wxString AngularLimitsToSimpleString(const MathsStorage::Float3& minLimits, const MathsStorage::Float3& maxLimits)
{
	MathsStorage::Float3 minAngles;
	MathsStorage::Float3 maxAngles;
	
	for (U32 i = 0; i < 3; ++i)
	{
		minAngles.v[i] = LimitToDeg(minLimits.v[i]);
		maxAngles.v[i] = LimitToDeg(maxLimits.v[i]);
	}
	
	return LimitsToSimpleString(minAngles, maxAngles, 179.99f);
}

wxString PositionLimitsToSimpleString(const MathsStorage::Float3& minLimits, const MathsStorage::Float3& maxLimits)
{
	return LimitsToSimpleString(minLimits, maxLimits, 99999.f);
}

void C_ConstraintsPanel::Populate()
{
	if (m_constraints == NULL || m_skeleton == NULL || m_listCtrl == NULL) return;
	
	wxWindowUpdateLocker updateLocker(m_listCtrl);
	
	m_listCtrl->DeleteAllItems();
	
	for (U32 i = 0; i < m_constraints->GetNumConstraints(); ++i)
	{
		const AnimationTools::C_ExportConstraint* c = m_constraints->GetConstraint(i);
	
		wxString jointName = m_skeleton->GetJointName(c->nodeIndex).GetString();
		long itemIndex = m_listCtrl->InsertItem(i, jointName); 
		
		m_listCtrl->SetItemPtrData(itemIndex, wxUIntPtr(c));
		
		wxString constraintName;
		wxString info;
		//wxColour bgColour;
	
		switch (m_constraints->GetConstraintType(i))
		{
			case Animation::E_ConstraintType_Aim:
			{
				constraintName = "Aim";
				const AnimationTools::C_ExportAimConstraint* aim = static_cast<const AnimationTools::C_ExportAimConstraint*>(c);
				
				wxString targetName = m_skeleton->GetJointName(aim->targetNodeIndex).GetString();
				
				info.Printf("Target: %s, Aim Axis:%s, Limits: %s",
					targetName.c_str(),
					AxisToSimpleString(aim->aimVector).c_str(),
					AngularLimitsToSimpleString(aim->minLimits, aim->maxLimits).c_str()
				);
								
				break;
			}
			case Animation::E_ConstraintType_Orientation:
			{
				constraintName = "Orientation";
				const AnimationTools::C_ExportOrientationConstraint* ori = static_cast<const AnimationTools::C_ExportOrientationConstraint*>(c);
				
				wxString inputName1 = m_skeleton->GetJointName(ori->inputNodeIndex1).GetString();
				wxString inputName2 = m_skeleton->GetJointName(ori->inputNodeIndex2).GetString();
				
				info.Printf("Inputs: (%s, %s), Blend: %.2f, Limits: %s",
					inputName1.c_str(),
					inputName2.c_str(),
					ori->blendFraction,
					AngularLimitsToSimpleString(ori->minLimits, ori->maxLimits).c_str()
				);
								
				break;
			}
			case Animation::E_ConstraintType_Position:
			{
				constraintName = "Position";
				const AnimationTools::C_ExportPositionConstraint* pos = static_cast<const AnimationTools::C_ExportPositionConstraint*>(c);
				
				wxString inputName1 = m_skeleton->GetJointName(pos->inputNodeIndex1).GetString();
				wxString inputName2 = m_skeleton->GetJointName(pos->inputNodeIndex2).GetString();
				
				info.Printf("Inputs: (%s, %s), Blend: %.2f, Limits: %s, Offset: (%.2f, %.2f, %.2f)",
					inputName1.c_str(),
					inputName2.c_str(),
					pos->blendFraction,
					PositionLimitsToSimpleString(pos->minLimits, pos->maxLimits),
					pos->localOffset.x, pos->localOffset.y, pos->localOffset.z
				);
								
				break;
			}

			case Animation::E_ConstraintType_DrivenKeyRotation:
				constraintName = "Driven Key Rotation";
				break;
			case Animation::E_ConstraintType_DrivenKeyTranslation:
				constraintName = "Driven Key Translation";
				break;
			case Animation::E_ConstraintType_Custom:
				constraintName = "Custom";
				break;

			case Animation::E_ConstraintType_DynamicAimJoint:
			{
				constraintName = "Dynamic Aim";
				
				const AnimationTools::C_ExportDynamicAimJoint* aim = static_cast<const AnimationTools::C_ExportDynamicAimJoint*>(c);
				
				wxString inputName = "?unknown?";
				if (aim->targetNodeIndex >= 0)
				{
					inputName = m_skeleton->GetJointName(aim->targetNodeIndex).GetString();
				}
								
				info.Printf("Target: %s, LocalUp %s, UpJoint '%s', PitchLimits: (%.1f, %.1f), HeadingLimits: (%.1f, %.1f), Mass %.3f, PitchSpring %.3f, PitchDamping %.3f, HeadingSpring %.3f, HeadingDamping %.3f, AirDamping %.3f",
					inputName.c_str(),
					AxisToSimpleString(aim->upVector),
					aim->upVectorNodeIndex >= 0 ? m_skeleton->GetJointName(aim->upVectorNodeIndex).GetString() : "none",
					Maths::RadToDeg(aim->minPitch),
					Maths::RadToDeg(aim->maxPitch),
					Maths::RadToDeg(aim->minHeading),
					Maths::RadToDeg(aim->maxHeading),
					aim->mass,
					aim->pitchSpring,
					aim->pitchDamping,
					aim->headingSpring,
					aim->headingDamping,
					1.f - aim->drag
					// pitch min soft limit ratio
					// pitch max soft limit ratio
					// heading min soft limit ratio
					// heading max soft limit ratio
				);
	
				break;
			}
			case Animation::E_ConstraintType_DynamicSliderJoint:
			{
				constraintName = "Dynamic Slider";
				const AnimationTools::C_ExportDynamicSliderJoint* slider = static_cast<const AnimationTools::C_ExportDynamicSliderJoint*>(c);
				
				info.Printf("Limits: (%.1f, %.1f), Mass %.3f, Spring %.3f, Damping %.3f, Air %.3f",
					slider->minLimit,
					slider->maxLimit,
					slider->mass,
					slider->spring,
					slider->damping,
					1.f - slider->drag
					// min soft limit ratio
					// max soft limit ratio
				);
				
				break;
			}
				
			default:
				break;
		}
		
		m_listCtrl->SetItem(itemIndex, E_Column_Name, jointName);
		m_listCtrl->SetItem(itemIndex, E_Column_Type, constraintName);
		m_listCtrl->SetItem(itemIndex, E_Column_Order, wxString::Format("%d", i));
		m_listCtrl->SetItem(itemIndex, E_Column_Details, info);
	}
	
	// Sort as before.
	Sort(m_sortColumn, m_sortAscending);
		
	m_listCtrl->SetColumnWidth(E_Column_Name, wxLIST_AUTOSIZE);
	m_listCtrl->SetColumnWidth(E_Column_Type, wxLIST_AUTOSIZE_USEHEADER);
	m_listCtrl->SetColumnWidth(E_Column_Order, wxLIST_AUTOSIZE_USEHEADER);
	m_listCtrl->SetColumnWidth(E_Column_Details, wxLIST_AUTOSIZE);
	
	m_infoText->SetLabel(wxString::Format("%d constraints.", m_constraints->GetNumConstraints()));
}

int wxCALLBACK ConstraintJointNameCompareFunc(long item1, long item2, long data)
{
	const AnimationTools::C_ExportConstraint* c1 = reinterpret_cast<const AnimationTools::C_ExportConstraint*>(item1);
	const AnimationTools::C_ExportConstraint* c2 = reinterpret_cast<const AnimationTools::C_ExportConstraint*>(item2);
	
	if (data) Ut::Swap(c1, c2);
	
	if ( c1->nodeIndex < c2->nodeIndex ) return -1;
	if ( c1->nodeIndex > c2->nodeIndex ) return +1;
	return 0;
}

int wxCALLBACK ConstraintOrderCompareFunc(long item1, long item2, long data)
{
	const AnimationTools::C_ExportConstraint* c1 = reinterpret_cast<const AnimationTools::C_ExportConstraint*>(item1);
	const AnimationTools::C_ExportConstraint* c2 = reinterpret_cast<const AnimationTools::C_ExportConstraint*>(item2);
	
	if (data) Ut::Swap(c1, c2);
	
	if ( c1->order < c2->order ) return -1;
	if ( c1->order > c2->order ) return +1;
	return 0;
}

int wxCALLBACK ConstraintTypeCompareFunc(long item1, long item2, long data)
{
	const AnimationTools::C_ExportConstraint* c1 = reinterpret_cast<const AnimationTools::C_ExportConstraint*>(item1);
	const AnimationTools::C_ExportConstraint* c2 = reinterpret_cast<const AnimationTools::C_ExportConstraint*>(item2);
	
	if (data) Ut::Swap(c1, c2);
	
	if ( c1->GetType() < c2->GetType() ) return -1;
	if ( c1->GetType() > c2->GetType() ) return +1;
	return 0;
}

void C_ConstraintsPanel::Sort(E_Column sortColumn, bool sortAscending)
{
	wxWindowUpdateLocker updateLocker(m_listCtrl);

	switch ( sortColumn )
	{
		case E_Column_Name:
			// sort by constrained joint name.
			m_listCtrl->SortItems(&ConstraintJointNameCompareFunc, sortAscending ? 0 : 1);
			break;
		case E_Column_Type:
			// sort by constraint type.
			m_listCtrl->SortItems(&ConstraintTypeCompareFunc, sortAscending ? 0 : 1);
			break;
		case E_Column_Order:
			// sort by order.
			m_listCtrl->SortItems(&ConstraintOrderCompareFunc, sortAscending ? 0 : 1);
			break;
			
		default:
			break;
	}

}


