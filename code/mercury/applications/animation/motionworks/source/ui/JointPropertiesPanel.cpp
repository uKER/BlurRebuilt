#include "stdafx.h"
#include "JointPropertiesPanel.h"
#include <animation/common/ObjectTransform.h>
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/pose/Pose.h>
#include "PropertyTypes.h"
#include "GlobalSignals.h"

#include "wx/propgrid/propgrid.h"
#include "wx/propgrid/propdev.h"
#include "wx/propgrid/advprops.h"
#include "wx/propgrid/manager.h"
#include <wx/wupdlock.h>
#include <wx/wxFlatNotebook/wxFlatNotebook.h>

//
// C_JointProperties
//

static const wxChar* kPageName = _T("Joint Properties");

class C_JointProperties
{
public:
	C_JointProperties(wxPropertyGridManager* manager):
		m_manager(manager),
		m_skeletonInstance(NULL),
		m_pose(NULL),
		m_jointIndex(-1),
		m_dynamicJointInstance(-1)
	{
		m_manager->Freeze();
		
		m_pageIndex = manager->AddPage();
		manager->SelectPage(m_pageIndex);
		
		wxPropertyGridPage* page = manager->GetPage(m_pageIndex);
		
		AddCommonProperties(page);
		AddTransformProperties(page);
		manager->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,true);
		
		manager->Thaw();
	}
	
	~C_JointProperties()
	{
		m_manager->RemovePage(m_pageIndex);
	}
	
	void SetSkeletonInstance(Animation::C_SkeletonInstance* skeletonInstance)
	{
		m_skeletonInstance = skeletonInstance;
		BuildPage();
		Refresh();
	}

	void SetPose(Animation::C_Pose* pose)
	{
		m_pose = pose;
		Refresh();
	}
	
	void SetJointIndex(S32 jointIndex)
	{
		m_jointIndex = jointIndex;
		BuildPage();
		Refresh();
	}
	
	void BuildPage()
	{
		wxWindowUpdateLocker updateLocker(m_manager);
		m_manager->ClearPage(m_pageIndex);
		wxPropertyGridPage* page = m_manager->GetPage(m_pageIndex);

		AddCommonProperties(page);
		AddTransformProperties(page);

		if (IsValid())
		{
			m_dynamicJointInstance = m_skeletonInstance->FindDynamicJoint(m_jointIndex);
			if (m_dynamicJointInstance >= 0)
			{
				switch (m_skeletonInstance->GetDynamicJointType(m_dynamicJointInstance))
				{
					case Animation::E_DynamicJointType_Aim:
						AddDynamicAimJointProperties(page);
						break;
					case Animation::E_DynamicJointType_Slider:
						AddDynamicSliderJointProperties(page);
						break;
					default:
						break;
				}
			}
			
			m_manager->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,true);
		}
		m_manager->GetGrid()->SetSplitterLeft(true);
	}
	
	void OnPropertyChange(wxPropertyGridEvent& e)
	{
		if (m_jointIndex < 0) return;
	
		wxVariant value = e.GetPropertyValue();
		if (value.IsNull())
			return;

		const wxPGProperty *prop = e.GetProperty();
				
		if (m_dynamicJointInstance >= 0)
		{
		
			if (prop == m_massProp)
			{
				m_skeletonInstance->SetDynamicJointMass(m_dynamicJointInstance, m_massProp->GetValue().GetDouble());
			}
			else if (prop == m_airResistanceProp)
			{
				m_skeletonInstance->SetDynamicJointAirResistance(m_dynamicJointInstance, m_airResistanceProp->GetValue().GetDouble());
			}
			else if (prop == m_windInfluenceProp)
			{
				m_skeletonInstance->SetDynamicJointWindInfluence(m_dynamicJointInstance, m_windInfluenceProp->GetValue().GetDouble());
			}
			else if (prop == m_buoyancyInfluenceProp)
			{
				m_skeletonInstance->SetDynamicJointBuoyancyInfluence(m_dynamicJointInstance, m_buoyancyInfluenceProp->GetValue().GetDouble());
			}
			else
			{
				switch (m_skeletonInstance->GetDynamicJointType(m_dynamicJointInstance))
				{
					case Animation::E_DynamicJointType_Aim:
					{
						if (prop == m_minPitchProp || prop == m_maxPitchProp || prop == m_minHeadingProp || prop == m_maxHeadingProp)
						{
							m_skeletonInstance->SetDynamicAimJointLimits(
								m_dynamicJointInstance,
								Maths::DegToRad(m_minPitchProp->GetValue().GetDouble()),
								Maths::DegToRad(m_maxPitchProp->GetValue().GetDouble()),
								Maths::DegToRad(m_minHeadingProp->GetValue().GetDouble()),
								Maths::DegToRad(m_maxHeadingProp->GetValue().GetDouble())
							);
						}
						else if (prop == m_pitchSpringProp || prop == m_pitchDampingProp || prop == m_headingSpringProp || prop == m_headingDampingProp)
						{
							m_skeletonInstance->SetDynamicAimJointSpringConstants(
								m_dynamicJointInstance,
								m_pitchSpringProp->GetValue().GetDouble(),
								m_pitchDampingProp->GetValue().GetDouble(),
								m_headingSpringProp->GetValue().GetDouble(),
								m_headingDampingProp->GetValue().GetDouble()
							);
						}
					
						break;
					}
					case Animation::E_DynamicJointType_Slider:
					{
						if (prop == m_sliderSpringProp || prop == m_sliderDampingProp)
						{
							m_skeletonInstance->SetDynamicSliderJointSpringConstants(m_dynamicJointInstance, m_sliderSpringProp->GetValue().GetDouble(), m_sliderDampingProp->GetValue().GetDouble());
						}
						else if (prop == m_sliderMin || prop == m_sliderMax)
						{
							m_skeletonInstance->SetDynamicSliderJointLimits(m_dynamicJointInstance, m_sliderMin->GetValue().GetDouble(), m_sliderMax->GetValue().GetDouble());
						}
						break;
					}
						
					
					default:
						break;
				}
			}
		}		
	}
	
	// Round up to nearest degree.
	inline F32 ToDegrees(F32 x) const
	{
		F32 deg = Maths::RadToDeg(x);
		deg = floorf(deg + 0.5f);
		return deg;
	}
	
	void Refresh()
	{
		if (!IsValid()) return;
		
		wxWindowUpdateLocker updateLocker(m_manager);
		
		RefreshAnimatedProperties();
		
		if (m_dynamicJointInstance >= 0)
		{
			m_massProp->SetValue(m_skeletonInstance->GetDynamicJointMass(m_dynamicJointInstance));
			m_airResistanceProp->SetValue(m_skeletonInstance->GetDynamicJointAirResistance(m_dynamicJointInstance));
			m_windInfluenceProp->SetValue(m_skeletonInstance->GetDynamicJointWindInfluence(m_dynamicJointInstance));
			m_buoyancyInfluenceProp->SetValue(m_skeletonInstance->GetDynamicJointBuoyancyInfluence(m_dynamicJointInstance));
		
			switch (m_skeletonInstance->GetDynamicJointType(m_dynamicJointInstance))
			{
				case Animation::E_DynamicJointType_Aim:
				{
					S32 targetJointIndex = m_skeletonInstance->GetDynamicAimJointTargetJointIndex(m_dynamicJointInstance);
					wxString targetJointName = "<none>";
					if (targetJointIndex >= 0)
					{
						targetJointName = m_skeletonInstance->GetSkeleton()->GetJointName(targetJointIndex).GetString();
					}
					m_targetNameProp->SetValue(targetJointName);
				
					F32 minPitch, maxPitch, minHeading, maxHeading;
					m_skeletonInstance->GetDynamicAimJointLimits(m_dynamicJointInstance, &minPitch, &maxPitch, &minHeading, &maxHeading);
					F32 pitchSpring, pitchDamping, headingSpring, headingDamping;
					m_skeletonInstance->GetDynamicAimJointSpringConstants(m_dynamicJointInstance, &pitchSpring, &pitchDamping, &headingSpring, &headingDamping);
					
					m_minPitchProp->SetValue(ToDegrees(minPitch));
					m_maxPitchProp->SetValue(ToDegrees(maxPitch));
					m_minHeadingProp->SetValue(ToDegrees(minHeading));
					m_maxHeadingProp->SetValue(ToDegrees(maxHeading));
					
					m_pitchSpringProp->SetValue(pitchSpring);
					m_pitchDampingProp->SetValue(pitchDamping);
					m_headingSpringProp->SetValue(headingSpring);				
					m_headingDampingProp->SetValue(headingDamping);
				
					break;
				}
				case Animation::E_DynamicJointType_Slider:
				{
					F32 spring, damping;
					m_skeletonInstance->GetDynamicSliderJointSpringConstants(m_dynamicJointInstance, &spring, &damping);
					
					F32 minSlide, maxSlide;
					m_skeletonInstance->GetDynamicSliderJointLimits(m_dynamicJointInstance, &minSlide, &maxSlide);
											
					m_sliderSpringProp->SetValue(spring);
					m_sliderDampingProp->SetValue(damping);
					m_sliderMin->SetValue(minSlide);
					m_sliderMax->SetValue(maxSlide);
					break;
				}
					
				
				default:
					break;
			}
		}
	}
	
	void RefreshAnimatedProperties()
	{
		if (!IsValid()) return;
		
		Animation::C_JointTransform localTransform;
		
		Animation::C_StringId name = m_skeletonInstance->GetSkeleton()->GetJointName(m_jointIndex);
		m_nameProp->SetValue(name.GetString());
		
		localTransform = m_pose->GetJointTransform(m_jointIndex);
							
		m_localRotationProp->SetValue(PGHVec4ToVariant(Maths::HVec4(Maths::GetVec4F32(localTransform.GetRotation()))));
		m_localTranslationProp->SetValue(PGHVec3ToVariant(Maths::HVec3(localTransform.GetTranslation())));
		m_localScaleProp->SetValue(PGHVec3ToVariant(Maths::HVec3(localTransform.GetScale())));
					
		Animation::C_ObjectTransform localBasePoseTransform(m_skeletonInstance->GetSkeleton()->GetLocalBasePoseJointTransform(m_jointIndex));
		
		m_localBasePoseRotationProp->SetValue(PGHVec4ToVariant(Maths::HVec4(Maths::GetVec4F32(localBasePoseTransform.GetRotation()))));
		m_localBasePoseTranslationProp->SetValue(PGHVec3ToVariant(Maths::HVec3(localBasePoseTransform.GetTranslation())));
		m_localBasePoseScaleProp->SetValue(PGHVec3ToVariant(Maths::HVec3(localBasePoseTransform.GetScale())));
	}
	
	
	bool IsValid() const
	{
		if (m_jointIndex < 0) return false;
		if (m_skeletonInstance == NULL) return false;
		if (m_skeletonInstance->GetSkeleton() == NULL) return false;
		if (m_pose == NULL) return false;
		return true;
	}
	
	wxString GetJointName() const
	{
		if (IsValid())
		{
			return m_nameProp->GetValueAsString();
		}
		return wxString();
	}
	
private:

	void AddCommonProperties(wxPropertyGridPage* page)
	{
		m_nameProp = page->Append(new wxStringProperty(_T("Name"), wxPG_LABEL));
		//page->EnableProperty(m_nameProp, false);
		page->SetPropertyReadOnly(m_nameProp, true);
	}

	void AddTransformProperties(wxPropertyGridPage* page)
	{
		page->AppendCategory(_T("Local Transform"));
		m_localRotationProp = page->Append(new C_HVec4Property(_T("Rotation"), wxPG_LABEL, Maths::HVec4(0.f, 0.f, 0.f, 1.f)));
		m_localTranslationProp = page->Append(new C_HVec3Property(_T("Translation"), wxPG_LABEL, Maths::HVec3()));
		m_localScaleProp = page->Append(new C_HVec3Property(_T("Scale"), wxPG_LABEL, Maths::HVec3()));
		page->SetPropertyReadOnly(m_localRotationProp, true);
		page->SetPropertyReadOnly(m_localTranslationProp, true);
		page->SetPropertyReadOnly(m_localScaleProp, true);


		wxPGProperty* basePoseCat = page->AppendCategory(_T("Local Base Pose Transform"));
		m_localBasePoseRotationProp = page->Append(new C_HVec4Property(_T("Rotation"), _T("BasePoseRotation"), Maths::HVec4(0.f, 0.f, 0.f, 1.f)));
		m_localBasePoseTranslationProp = page->Append(new C_HVec3Property(_T("Translation"), _T("BasePoseTranslation"), Maths::HVec3()));
		m_localBasePoseScaleProp = page->Append(new C_HVec3Property(_T("Scale"), _T("BasePoseScale"), Maths::HVec3(1.f, 1.f, 1.f)));
		//m_localBindPoseScaleProp = page->Append(new C_HVec3Property(_T("Scale"), wxPG_LABEL, Maths::HVec3()));
		page->SetPropertyReadOnly(m_localBasePoseRotationProp, true);
		page->SetPropertyReadOnly(m_localBasePoseTranslationProp, true);
		page->SetPropertyReadOnly(m_localBasePoseScaleProp, true);
		//page->Collapse(basePoseCat);
		//page->SetPropertyReadOnly(m_localBindPoseScaleProp, true);

		//page->EnableProperty(m_localRotationProp, false);
		//page->EnableProperty(m_localTranslationProp, false);
		//page->EnableProperty(m_localScaleProp, false);
	}
	
	void AddCommonDynamicJointProperties(wxPropertyGridPage* page)
	{
		m_massProp = page->Append(new wxFloatProperty(_T("Mass")));
		m_airResistanceProp = page->Append(new wxFloatProperty(_T("Air Resistance")));
		m_windInfluenceProp = page->Append(new wxFloatProperty(_T("Wind Influence")));
		m_buoyancyInfluenceProp = page->Append(new wxFloatProperty(_T("Buoyancy Influence")));
	}
	
	void AddDynamicAimJointProperties(wxPropertyGridPage* page)
	{
		wxPGProperty* cat = page->AppendCategory(_T("Dynamic Aim Joint Parameters"));
		
		AddCommonDynamicJointProperties(page);
		
		m_targetNameProp = page->Append(new wxStringProperty(_T("Target Joint Name")));
		page->SetPropertyReadOnly(m_targetNameProp);
		//m_upVectorNameProp = page->Append(new wxStringProperty(_T("Up Vector Joint Name")));
		
		wxPGProperty* springCat = page->AppendIn(cat, new wxPropertyCategory(_T("Spring Constants")));
		m_pitchSpringProp = page->AppendIn(springCat, new wxFloatProperty(_T("Pitch Spring")));
		m_pitchDampingProp = page->AppendIn(springCat, new wxFloatProperty(_T("Pitch Damping")));
		m_headingSpringProp = page->AppendIn(springCat, new wxFloatProperty(_T("Heading Spring")));
		m_headingDampingProp = page->AppendIn(springCat, new wxFloatProperty(_T("Heading Damping")));
		
		wxPGProperty* limitsCat = page->AppendIn(cat, new wxPropertyCategory(_T("Limits")));
		
		m_minPitchProp = page->AppendIn(limitsCat, new wxFloatProperty(_T("Min Pitch Angle")));
		m_maxPitchProp = page->AppendIn(limitsCat, new wxFloatProperty(_T("Max Pitch Angle")));
		m_minHeadingProp = page->AppendIn(limitsCat, new wxFloatProperty(_T("Min Heading Angle")));
		m_maxHeadingProp = page->AppendIn(limitsCat, new wxFloatProperty(_T("Max Heading Angle")));
	}

	void AddDynamicSliderJointProperties(wxPropertyGridPage* page)
	{
		page->AppendCategory(_T("Dynamic Slide Joint Parameters"));
		AddCommonDynamicJointProperties(page);
		m_sliderSpringProp = page->Append(new wxFloatProperty(_T("Spring")));
		m_sliderDampingProp = page->Append(new wxFloatProperty(_T("Damping")));
		m_sliderMin = page->Append(new wxFloatProperty(_T("Min Slide")));
		m_sliderMax = page->Append(new wxFloatProperty(_T("Max Slide")));
	}

	S32								m_jointIndex;
	S32								m_dynamicJointInstance;
	Animation::C_SkeletonInstance*	m_skeletonInstance;
	Animation::C_Pose*				m_pose;
	
	wxPropertyGridManager*			m_manager;
	S32								m_pageIndex;	
	wxPGProperty*					m_nameProp;
	wxPGProperty*					m_targetNameProp;
	wxPGProperty*					m_upVectorNameProp;
	
	wxPGProperty*					m_localRotationProp;
	wxPGProperty*					m_localTranslationProp;
	wxPGProperty*					m_localScaleProp;

	wxPGProperty*					m_localBasePoseRotationProp;
	wxPGProperty*					m_localBasePoseTranslationProp;
	wxPGProperty*					m_localBasePoseScaleProp;
	
	wxPGProperty*					m_massProp;
	wxPGProperty*					m_airResistanceProp;
	wxPGProperty*					m_windInfluenceProp;
	wxPGProperty*					m_buoyancyInfluenceProp;
	wxPGProperty*					m_pitchSpringProp;
	wxPGProperty*					m_pitchDampingProp;
	wxPGProperty*					m_headingSpringProp;
	wxPGProperty*					m_headingDampingProp;
	wxPGProperty*					m_minPitchProp;
	wxPGProperty*					m_maxPitchProp;
	wxPGProperty*					m_minHeadingProp;
	wxPGProperty*					m_maxHeadingProp;
	
	wxPGProperty*					m_sliderSpringProp;
	wxPGProperty*					m_sliderDampingProp;
	wxPGProperty*					m_sliderMin;
	wxPGProperty*					m_sliderMax;
};	

//
// C_JointPropertiesPanel
//

C_JointPropertiesPanel::C_JointPropertiesPanel(
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size
):
	wxPanel(parent, winId, pos, size),
	m_jointProperties(NULL)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	
	m_propertyGridManager = new wxPropertyGridManager(
		this,
		wxID_ANY,
		wxDefaultPosition,
		wxDefaultSize,
		wxNO_BORDER |
		wxPG_SPLITTER_AUTO_CENTER |
		wxPGMAN_DEFAULT_STYLE 
	);
	
	m_jointProperties = new C_JointProperties(m_propertyGridManager);
		
	m_propertyGridManager->GetGrid()->SetWindowStyle(wxNO_BORDER);
	//m_propertyGridManager->GetGrid()->SetMarginColour(*wxWHITE);
	m_propertyGridManager->GetGrid()->SetCaptionBackgroundColour(*wxWHITE);
	m_propertyGridManager->GetGrid()->SetCellBackgroundColour(*wxWHITE);
	
	mainSizer->Add(m_propertyGridManager, wxSizerFlags(1).Expand().Border(wxALL, 1));
	//m_propertyGridManager->GetGrid()->SetInternalFlag(wxPG_FL_SPLITTER_PRE_SET);
	
	typedef void (wxPanel::*PanelEventFunction)(wxPropertyGridEvent&);
	
	Connect(wxID_ANY,
			wxID_ANY,
			wxEVT_PG_CHANGED,
			wxObjectEventFunction((PanelEventFunction)&C_JointPropertiesPanel::OnPropertyChange), 0, this);
	
	SetSizerAndFit(mainSizer);
	
	C_GlobalSignals::Instance().jointSelected.connect(this, &C_JointPropertiesPanel::SetJointIndex);
	C_GlobalSignals::Instance().skeletonLoaded.connect(this, &C_JointPropertiesPanel::OnSkeletonLoaded);
	C_GlobalSignals::Instance().animationUpdated.connect(this, &C_JointPropertiesPanel::OnAnimationUpdated);
}

C_JointPropertiesPanel::~C_JointPropertiesPanel()
{
	delete m_jointProperties;
}

void C_JointPropertiesPanel::SetSkeletonInstance(Animation::C_SkeletonInstance* skeletonInstance)
{
	m_jointProperties->SetSkeletonInstance(skeletonInstance);
}

void C_JointPropertiesPanel::SetPose(Animation::C_Pose* pose)
{
	m_jointProperties->SetPose(pose);
}

void C_JointPropertiesPanel::SetJointIndex(S32 jointIndex)
{
	m_jointProperties->SetJointIndex(jointIndex);
	UpdateParentNotebookTabCaption();
}
	
void C_JointPropertiesPanel::OnAnimationUpdated(F32 dt)
{
	wxWindow* focus = FindFocus();
	
	if (focus != NULL)
	{
		wxWindow* child = FindWindowById(focus->GetId(), this);
		if (child != NULL || focus == this)
		{
			// Don't update if a child window has the focus.
			return;
		}
	}
	
	RefreshAnimatedProperties();
	Refresh();
}

void C_JointPropertiesPanel::RefreshProperties()
{
	m_jointProperties->Refresh();
}

void C_JointPropertiesPanel::RefreshAnimatedProperties()
{
	m_jointProperties->RefreshAnimatedProperties();
}

void C_JointPropertiesPanel::OnPropertyChange(wxPropertyGridEvent& e)
{
	m_jointProperties->OnPropertyChange(e);
}

void C_JointPropertiesPanel::OnSkeletonLoaded(const Animation::C_Skeleton* skeleton, Animation::C_SkeletonInstance* skeletonInstance, Animation::C_Pose* pose, const char* name)
{
	m_jointProperties->SetSkeletonInstance(skeletonInstance);
	m_jointProperties->SetPose(pose);
	//m_jointProperties->SetJointIndex(-1);
	UpdateParentNotebookTabCaption();
}

void C_JointPropertiesPanel::UpdateParentNotebookTabCaption()
{
	const wxClassInfo* parentClassInfo = GetParent()->GetClassInfo();
	if (!stricmp(parentClassInfo->GetClassName(), "wxFlatNotebook"))
	{
		wxFlatNotebook* parentNotebook = static_cast<wxFlatNotebook*>(GetParent());
		
		int pageIndex = parentNotebook->GetPageIndex(this);
		if (pageIndex >= 0)
		{
			wxString caption = m_jointProperties->GetJointName();
			if (caption.IsEmpty())
			{
				caption = "No Joint Selected";
			}
			parentNotebook->SetPageText(pageIndex, caption);
		}
	}
}
