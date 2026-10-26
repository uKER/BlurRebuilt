#include "stdafx.h"
#include "DynamicsPanel.h"
#include "PropertyTypes.h"
#include "../DynamicsContext.h"
#include <animation/skeleton/SkeletonInstance.h>

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

#include "GlobalSignals.h"

class C_DynamicsProperties
{
public:
	C_DynamicsProperties(wxPropertyGridManager* manager):
		m_manager(manager),
		m_dynamicsContext(NULL)
	{
		m_manager->Freeze();
		
		manager->AddPage(_T("Dynamics"));
		manager->SelectPage(_T("Dynamics"));
		
		wxPropertyGridPage* page = manager->GetLastPage();
				
		page->AppendCategory(_T("General"));
		m_dynamicJointsEnabledProp = page->Append(new wxBoolProperty(_T("Dynamic Joints Enabled"), wxPG_LABEL, true));
		m_gravityProp = page->Append(new C_HVec3Property(_T("Gravity"), wxPG_LABEL, Maths::HVec3(0.f, -9.81f, 0.f)));
		page->AppendCategory(_T("Wind"));
		m_windEnabledProp = page->Append(new wxBoolProperty(_T("Wind Enabled"), wxPG_LABEL, false));
		m_showWindVectorProp = page->Append(new wxBoolProperty(_T("Show Wind Vector"), wxPG_LABEL, false));
		m_windDirectionProp = page->Append(new C_HVec3Property(_T("Direction"), wxPG_LABEL, Maths::HVec3()));
		m_windDirectionVariationProp = page->Append(new wxFloatProperty(_T("Direction Variation"), wxPG_LABEL, 0.f));
		m_windStrengthProp = page->Append(new wxFloatProperty(_T("Strength"), wxPG_LABEL, 0.f));
		m_windStrengthVariationProp = page->Append(new wxFloatProperty(_T("Strength Variation"), wxPG_LABEL, 0.f));
		m_windVariationRateProp = page->Append(new wxFloatProperty(_T("Variation Rate"), wxPG_LABEL, 0.f));
		//page->AppendCategory(_T("Buoyancy"));
		//m_waterPlaneEnabled = page->Append(new wxBoolProperty(_T("Water Plane Enabled"), wxPG_LABEL, false));
		//m_waterPlaneHeight = page->Append(new wxFloatProperty(_T("Water Plane Height"), wxPG_LABEL, 0.f));
		
		
		manager->AddPage(_T("Test"));
		manager->GetLastPage()->AppendCategory(_T("Something"));
		
		manager->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,true);
		
		manager->Thaw();
	}
	
	~C_DynamicsProperties()
	{
		m_manager->RemovePage(m_manager->GetPageByName(_T("Dynamics")));
	}
	
	void SetDynamicsContext(C_DynamicsContext* dynamicsContext)
	{
		m_dynamicsContext = dynamicsContext;
		Refresh();
	}
	
	void OnPropertyChange(wxPropertyGridEvent& e)
	{
		if (m_dynamicsContext == NULL) return;
	
		wxVariant value = e.GetPropertyValue();
		if (value.IsNull())
			return;

		const wxPGProperty *prop = e.GetProperty();
		if (prop == m_dynamicJointsEnabledProp)
		{
			m_dynamicsContext->SetDynamicJointsEnabled(e.GetPropertyValueAsBool());
		}
		else if (prop == m_gravityProp)
		{
			m_dynamicsContext->SetGravity(PGHVec3FromVariant(value));
		}
		else if (prop == m_windEnabledProp)
		{
			m_dynamicsContext->SetWindEnabled(e.GetPropertyValueAsBool());
		}
		else if (prop == m_showWindVectorProp)
		{
			m_dynamicsContext->ShowWindVector(e.GetPropertyValueAsBool());
		}
		else if (prop == m_windDirectionProp)
		{
			m_dynamicsContext->SetWindDirection(PGHVec3FromVariant(value));
		}
		else if (prop == m_windDirectionVariationProp)
		{
			m_dynamicsContext->SetWindDirectionVariation((F32)e.GetPropertyValueAsDouble());
		}
		else if (prop == m_windStrengthProp)
		{
			m_dynamicsContext->SetWindStrength((F32)e.GetPropertyValueAsDouble());
		}
		else if (prop == m_windStrengthVariationProp)
		{
			m_dynamicsContext->SetWindStrengthVariation((F32)e.GetPropertyValueAsDouble());
		}
		else if (prop == m_windVariationRateProp)
		{
			m_dynamicsContext->SetWindVariationRate((F32)e.GetPropertyValueAsDouble());
		}
		//else if (prop == m_waterPlaneHeight)
		//{
		//	m_dynamicsContext->SetWaterPlaneHeight((F32)e.GetPropertyValueAsDouble());
		//}
		//else if (prop == m_waterPlaneEnabled)
		//{
		//	m_dynamicsContext->SetBuoyancyEnabled(e.GetPropertyValueAsBool());
		//}
	}
	
	void Refresh()
	{
		if (m_dynamicsContext != NULL)
		{
			m_dynamicJointsEnabledProp->SetValue(m_dynamicsContext->AreDynamicJointsEnabled());
			m_gravityProp->SetValue(PGHVec3ToVariant(Maths::HVec3(m_dynamicsContext->GetGravity())));
			
			m_windEnabledProp->SetValue(m_dynamicsContext->IsWindEnabled());
			m_windDirectionProp->SetValue(PGHVec3ToVariant(Maths::HVec3(m_dynamicsContext->GetWindDirection())));
			m_windDirectionVariationProp->SetValue(m_dynamicsContext->GetWindDirectionVariation());
			m_windStrengthProp->SetValue(m_dynamicsContext->GetWindStrength());
			m_windStrengthVariationProp->SetValue(m_dynamicsContext->GetWindStrengthVariation());
			m_windVariationRateProp->SetValue(m_dynamicsContext->GetWindVariationRate());
			m_showWindVectorProp->SetValue(m_dynamicsContext->ShowWindVector());
			
			//m_waterPlaneEnabled->SetValue(m_dynamicsContext->IsBuoyancyEnabled());
			//m_waterPlaneHeight->SetValue(m_dynamicsContext->GetWaterPlaneHeight());			
		}
	}
	
private:
	C_DynamicsContext*		m_dynamicsContext;
	wxPropertyGridManager*	m_manager;
	wxPGProperty*			m_dynamicJointsEnabledProp;
	wxPGProperty*			m_gravityProp;
	wxPGProperty*			m_windEnabledProp;
	wxPGProperty*			m_windDirectionProp;
	wxPGProperty*			m_windDirectionVariationProp;
	wxPGProperty*			m_windStrengthProp;
	wxPGProperty*			m_windStrengthVariationProp;
	wxPGProperty*			m_windVariationRateProp;
	wxPGProperty*			m_showWindVectorProp;
	wxPGProperty*			m_waterPlaneHeight;
	wxPGProperty*			m_waterPlaneEnabled;
};


C_DynamicsPanel::C_DynamicsPanel(
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size
):
	wxPanel(parent, winId, pos, size)
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
	m_dynamicsProperties = new C_DynamicsProperties(m_propertyGridManager);
		
	m_propertyGridManager->GetGrid()->SetWindowStyle(wxNO_BORDER);
	//m_propertyGridManager->GetGrid()->SetMarginColour(*wxWHITE);
	m_propertyGridManager->GetGrid()->SetCaptionBackgroundColour(*wxWHITE);
	m_propertyGridManager->GetGrid()->SetCellBackgroundColour(*wxWHITE);
	
	mainSizer->Add(m_propertyGridManager, wxSizerFlags(1).Expand().Border(wxALL, 1));
	//m_propertyGridManager->GetGrid()->SetInternalFlag(wxPG_FL_SPLITTER_PRE_SET);
	
	Connect(wxID_ANY, wxID_ANY, wxEVT_PG_CHANGED, wxObjectEventFunction((void (wxPanel::*)(wxPropertyGridEvent&))&C_DynamicsPanel::OnPropertyChange), 0, this);
	
	C_GlobalSignals::Instance().skeletonLoaded.connect(this, &C_DynamicsPanel::OnSkeletonLoaded);
	
	SetSizerAndFit(mainSizer);
}

C_DynamicsPanel::~C_DynamicsPanel()
{
	delete m_dynamicsProperties;
}

void C_DynamicsPanel::SetDynamicsContext(C_DynamicsContext* dynamicsContext)
{
	m_dynamicsProperties->SetDynamicsContext(dynamicsContext);
}

void C_DynamicsPanel::OnPropertyChange(wxPropertyGridEvent& e)
{
	m_dynamicsProperties->OnPropertyChange(e);
}

void C_DynamicsPanel::OnSkeletonLoaded(const Animation::C_Skeleton* skeleton, Animation::C_SkeletonInstance* skeletonInstance, Animation::C_Pose* pose, const char* name)
{
	RefreshProperties();
}

void C_DynamicsPanel::RefreshProperties()
{
	m_dynamicsProperties->Refresh();
	m_propertyGridManager->Refresh();
}




