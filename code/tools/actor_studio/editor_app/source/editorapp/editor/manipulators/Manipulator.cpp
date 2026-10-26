#include "stdafx.h"

#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Shaders/Constants.h"

#include "shaders/vsConstants.h"
#include "shaders/psConstants.h"

#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Core.h>

#include <EditorApp/components/ManipulatorPropertyPanel.h>
#include <EditorLib/context/EditorContext.h>
#include <EditorApp/Editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/ManipulatorContextMapping.h>
#include <EditorApp/Editor/manipulators/ManipulatorContextFactory.h>
#include <EditorApp/Editor/manipulators/ManipulatorHelpers.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>

#include <EditorApp/Misc/EvtHandlerHelpers.h>

#include <Editorlib/GlobalSignals.h>



EXTERN_SHADER(vsBasicNormalColour);
EXTERN_SHADER(psBasicNdotLVertexColour);


using namespace Editor;

C_ManipulatorManager::C_Slots::C_Slots(C_ManipulatorManager *pManipulatorManager)
: m_pManipulatorManager(pManipulatorManager)
{
}

void C_ManipulatorManager::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreContextChanged.connect(this, &C_ManipulatorManager::C_Slots::OnCoreContextChanged);
	s.dgNodeUpdated.connect(this, &C_ManipulatorManager::C_Slots::OnDgNodeUpdated);
}

void C_ManipulatorManager::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreContextChanged.disconnect(this);
	s.dgNodeUpdated.disconnect(this);
}

void C_ManipulatorManager::C_Slots::OnCoreContextChanged(const Btl::String& context, const void* contextData)
{
	m_pManipulatorManager->OnCoreContextChanged(context, contextData);
}

void C_ManipulatorManager::C_Slots::OnDgNodeUpdated(const EditorLib::I_DgNode& node, const EditorLib::C_UpdateHintCollection* pHc)
{
	m_pManipulatorManager->Dirty();
}



C_ManipulatorManager::C_ManipulatorManager()
: m_pActiveManipulator(0)
, m_pPropertyPanel(0)
, m_mode(0)
, m_Slots(this)
, m_dirty(true)
{
	m_Slots.Connect();
}

C_ManipulatorManager::~C_ManipulatorManager()
{
	m_Slots.Disconnect();

	Ut::UtMap<Btl::String, I_Manipulator*>::Iterator it;
	for (it=m_Manipulators.Begin(); it!=m_Manipulators.End(); it++)
		delete it->GetValue();
}

bool C_ManipulatorManager::AddManipulator(const Btl::String &name, Editor::I_Manipulator *pManipulator)
{
	if (m_Manipulators.Find(name)==m_Manipulators.End())
	{
		pManipulator->SetToolManager(this);
		m_Manipulators[name] = pManipulator;
		return true;
	}
	else
		return false;
}

bool C_ManipulatorManager::ManipulatorExists(const Btl::String &name) const
{
	return m_Manipulators.Find(name) != m_Manipulators.End();
}

I_Manipulator* C_ManipulatorManager::GetManipulator(const Btl::String& name)
{
	Ut::UtMap<Btl::String, I_Manipulator*>::Iterator it = m_Manipulators.Find(name);
	if (it != m_Manipulators.End())
		return it->GetValue();
	else
		return 0;
}

const I_Manipulator* C_ManipulatorManager::GetManipulator(const Btl::String& name) const
{
	Ut::UtMap<Btl::String, I_Manipulator*>::ConstIterator it = m_Manipulators.Find(name);
	if (it != m_Manipulators.End())
		return it->GetValue();
	else
		return 0;
}

I_Manipulator* C_ManipulatorManager::GetActiveManipulator()
{
	if (m_dirty && m_pActiveManipulator)
	{
		m_pActiveManipulator->OnSelectionChanged();
		m_dirty = false;
	}
		
	return m_pActiveManipulator;
}


const I_Manipulator* C_ManipulatorManager::GetActiveManipulator() const
{
	if (m_dirty && m_pActiveManipulator)
	{
		m_pActiveManipulator->OnSelectionChanged();
		m_dirty = false;
	}
	return m_pActiveManipulator;
}

const char* C_ManipulatorManager::GetActiveManipulatorName() const
{
	if (m_pActiveManipulator)
	{
		Ut::UtMap<Btl::String, I_Manipulator*>::ConstIterator it = m_Manipulators.Begin();
		while (it != m_Manipulators.End())
		{
			if (it->GetValue() == m_pActiveManipulator)
				return it->GetKey().Chars();
			it++;
		}
	}

	return 0;
}

I_Manipulator* C_ManipulatorManager::SetActiveManipulator(const Btl::String& name)
{
	Ut::UtMap<Btl::String, I_Manipulator*>::Iterator it = m_Manipulators.Find(name);
	if (it == m_Manipulators.End())
		return 0;

	C_RenderViewport *pViewport = NULL;
	if (m_pActiveManipulator)
	{
		pViewport = m_pActiveManipulator->GetAttachedViewport();
		m_pActiveManipulator->AttachToViewport(0);
	}

	m_pActiveManipulator = it->GetValue();

	if (m_pActiveManipulator && pViewport)
		m_pActiveManipulator->AttachToViewport(pViewport);

	if (m_pPropertyPanel)
		m_pPropertyPanel->SetActivePage(it->GetKey().Chars());

	m_pActiveManipulator->OnSelectionChanged();

	C_RenderCore::Instance().DirtyAllViewports();

	return m_pActiveManipulator;
}

void C_ManipulatorManager::SetActiveManipulator(I_Manipulator* pManipulator)
{
	if (pManipulator == m_pActiveManipulator)
		return;

	Ut::UtMap<Btl::String, I_Manipulator*>::Iterator it = m_Manipulators.Begin();
	while (it != m_Manipulators.End())
	{
		if (it->GetValue() == pManipulator)
		{
			C_RenderViewport *pViewport = NULL;
			if (m_pActiveManipulator)
			{
				pViewport = m_pActiveManipulator->GetAttachedViewport();
				m_pActiveManipulator->AttachToViewport(0);
			}

			m_pActiveManipulator = it->GetValue();

			if (m_pActiveManipulator && pViewport)
				m_pActiveManipulator->AttachToViewport(pViewport);

			if (m_pPropertyPanel)
				m_pPropertyPanel->SetActivePage(it->GetKey().Chars());

			m_pActiveManipulator->OnSelectionChanged();

			C_RenderCore::Instance().RefreshAllViewports();
			return;
		}
		it++;
	}
}

void C_ManipulatorManager::AttachPropertyPanel(C_ManipulatorPropertyPanel* pPropertyPanel)
{
	m_pPropertyPanel = pPropertyPanel;
}

C_ManipulatorPropertyPanel* C_ManipulatorManager::GetAttachedPropertyPanel()
{
	return m_pPropertyPanel;
}

void C_ManipulatorManager::Dirty()
{
	m_dirty = true;
}

bool C_ManipulatorManager::ToggleMode(E_ManipulatorMode mode)
{
	U32 flag = 1 << mode;
	if (m_mode & flag)
		m_mode = m_mode & ~flag;
	else
		m_mode = m_mode | flag;

	C_RenderCore::Instance().RefreshAllViewports();
	return (m_mode & flag) > 0;
}

bool C_ManipulatorManager::IsModeEnabled(E_ManipulatorMode mode) const
{
	U32 flag = 1 << mode;
	return (m_mode & flag) > 0;
}

bool C_ManipulatorManager::IsNormalMode() const
{
	return m_mode == 0;
}

void C_ManipulatorManager::OnCoreContextChanged(const Btl::String& context, const void* contextData)
{
	I_Manipulator* manipulator = SetActiveManipulator(context);
	if (manipulator)
		manipulator->SetData(contextData);
	else
		EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
}














U32 I_Manipulator::m_screenSize = 80;

BEGIN_EVENT_TABLE(I_Manipulator, wxEvtHandler)
  EVT_MOUSE_EVENTS(I_Manipulator::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(I_Manipulator::OnMouseCaptureLostEvent)
  EVT_KEY_DOWN(I_Manipulator::OnKeyDown)
  EVT_KEY_UP(I_Manipulator::OnKeyUp)
END_EVENT_TABLE()

I_Manipulator::I_Manipulator()
: m_pViewport(0)
, m_pToolManager(0)
, m_pContext(0)
, m_previousContext(E_ManipulatorContextType_Unknown)
, m_space(E_ManipulatorSpace_Local)
{
	AftkRender::C_ResourcePtr<Display::C_VertexShader> pBasicVertexShader =
		AftkRender::C_Renderer::GetShaderManager().LoadVertexShader("shaders/vsBasicNormalColour.shader", REF_SHADER(vsBasicNormalColour));
	assert(pBasicVertexShader != 0);

	AftkRender::C_ResourcePtr<Display::C_PixelShader> pBasicPixelShader = 
		AftkRender::C_Renderer::GetShaderManager().LoadPixelShader("shaders/psBasicNdotLVertexColour.shader", REF_SHADER(psBasicNdotLVertexColour));
	assert(pBasicPixelShader != 0);
}

I_Manipulator::~I_Manipulator()
{
	delete m_pContext;

	if (m_pViewport)
		if (!m_pViewport->RemoveEventHandler(this))
			LOGWARN(g_Log, "Failed to remove tool handler - was it previously registered??\n");
}

bool I_Manipulator::IsA(E_ManipulatorType type) const
{
	if (type == E_ManipulatorType_IManipulator)
		return true;

	return false;
}

void I_Manipulator::BindRender3D()
{
	Display::Device()->SetVertexShader(AftkRender::C_Renderer::GetShaderManager().GetVertexShader("shaders/vsBasicNormalColour.shader"));
	Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psBasicNdotLVertexColour.shader"));	
	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
}

F32 I_Manipulator::GetRenderScale(const C_RenderViewport& viewport, const Maths::HVec3& position)
{
	//Calculate scale factor for manipulators to appear 80 pixels in size
	const AftkRender::C_Camera& camera = viewport.GetCamera();
	F32 scale = 1.f;

	if (camera.GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
	{
		Maths::HVec3 zDist = position - camera.GetPosition();

		wxSize s = viewport.GetSize();
		scale = 2.f*tanf(Maths::DegToRad(0.5f*camera.GetFieldOfView()))*zDist.Length()*(F32)m_screenSize/s.y;
	}
	else
	{
		wxSize s = viewport.GetSize();
		scale = camera.GetHeightExtent()*(F32)m_screenSize/s.y;
	}

	return scale;

}

U32	I_Manipulator::GetScreenSize()
{
	return m_screenSize;
}

void I_Manipulator::SetScreenSize(U32 s)
{
	m_screenSize = Maths::Clamp<U32>(s,20, 200);
}

bool I_Manipulator::GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space)
{
	if (!m_pContext)
		return false;

	return m_pContext->GetInitialMatrix(matrix, space);
}

void I_Manipulator::AttachToViewport(C_RenderViewport *pViewport)
{
	if (m_pViewport)
		if (!m_pViewport->RemoveEventHandler(this))
			LOGWARN(g_Log, "Failed to remove tool handler - was it previously registered??\n");

	m_pViewport = pViewport;
	if (m_pViewport)
		EvtHandlerHelpers::PushBack(m_pViewport, this);
}

C_RenderViewport* I_Manipulator::GetAttachedViewport()
{
	return m_pViewport;
}

const C_RenderViewport* I_Manipulator::GetAttachedViewport() const
{
	return m_pViewport;
}

void I_Manipulator::SetToolManager(C_ManipulatorManager *pManager)
{
	m_pToolManager = pManager;
}

void I_Manipulator::SetCoordinateSpace(E_ManipulatorSpace space)
{
	m_space = space;
	//force update
	OnSelectionChanged();
	C_RenderCore::Instance().RefreshAllViewports();
}

E_ManipulatorSpace I_Manipulator::GetCoordinateSpace() const
{
	return m_space;
}

void I_Manipulator::OnMouseEvent(wxMouseEvent &e)
{
	//pass event down the event handler chain
	e.Skip();
}

void I_Manipulator::OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e)
{
	e.Skip();
}

void I_Manipulator::OnSelectionChanged()
{
	EditorLib::E_ObjType selectionType = ManipulatorHelpers::DetermineSelectionType();

	//Determine context type via table lookup.
	E_ManipulatorContextType contextType = C_ManipulatorContextMapping::Instance().FindContext(
		selectionType, GetType());

	//Don't do any work if we don't have to.
	if (contextType == m_previousContext)
		return;

	//Remove old context.
	if (m_pContext)
		delete m_pContext;

	//Create new context
	m_pContext = C_ManipulatorContextFactory::Instance().Create(contextType);
	if (!m_pContext)
	{
		m_previousContext = E_ManipulatorContextType_Unknown;
		return;
	}

	//Set current context type
	m_previousContext = contextType;
}

void I_Manipulator::OnKeyDown(wxKeyEvent &e)
{
	e.Skip();
}

void I_Manipulator::OnKeyUp(wxKeyEvent &e)
{
	e.Skip();
}

void I_Manipulator::Render(const C_RenderViewport *pViewport) const
{
}

void I_Manipulator::SetData(const void* userData)
{
}

Maths::C_Line I_Manipulator::CalculateRay(S32 clickX, S32 clickY)
{
	if (!m_pViewport)
		return Maths::C_Line();

	wxSize windowSize = m_pViewport->GetSize();

	const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
	if (camera.GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
	{
		F32 x = 2.f*(F32)clickX/(F32)windowSize.x - 1.f;
		F32 y = 1.f - 2.f*(F32)clickY/(F32)windowSize.y;

		y *= tanf(Maths::DegToRad(0.5f*camera.GetFieldOfView()));
		x *= tanf(Maths::DegToRad(0.5f*camera.GetFieldOfView())) * camera.GetAspectRatio();

		//orthonormalise camera axii
		Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
		toZ.Normalise();

		Maths::HVec3 toX = camera.GetUpVector()^toZ;
		toX.Normalise();

		Maths::HVec3 toY = toZ^toX;
		toY.Normalise();

		Maths::HVec3 rayDir = toZ + x*toX + y*toY;

		return Maths::C_Line(Maths::C_Vertex(camera.GetPosition()), Maths::C_Vector(rayDir*camera.GetFarClipDistance()));

	}
	else
	{
		F32 x = (F32)clickX/(F32)windowSize.x - 0.5f;
		F32 y = 0.5f - (F32)clickY/(F32)windowSize.y;

		y *= camera.GetHeightExtent();
		x *= camera.GetHeightExtent() * camera.GetAspectRatio();

		//orthonormalise camera axii
		Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
		toZ.Normalise();

		Maths::HVec3 toX = camera.GetUpVector()^toZ;
		toX.Normalise();

		Maths::HVec3 toY = toZ^toX;
		toY.Normalise();

		Maths::HVec3 origin = camera.GetPosition() + x*toX + y*toY;

		return Maths::C_Line(Maths::C_Vertex(origin), Maths::C_Vector(toZ*camera.GetFarClipDistance()));
	}
}

I_ManipulatorContext* I_Manipulator::GetContext()
{
	return m_pContext;
}












