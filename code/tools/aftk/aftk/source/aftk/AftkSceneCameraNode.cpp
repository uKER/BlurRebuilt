#include <stdafx.h>

#define AFTK_SCENE_CAMERA_NODE_NUM_STATIC_PLUGS 10
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FOCAL_LENGTH 0
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_CENTRE_OF_INTEREST 1
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_HORIZONTAL_FILM_APERTURE 2
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_VERTICAL_FILM_APERTURE 3
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_SHUTTER_ANGLE 4
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_NEAR_CLIP_PLANE 5
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FAR_CLIP_PLANE 6
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_ORTHOGRAPHIC_WIDTH 7
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FOCUS_DISTANCE 8
#define AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_F_STOP 9

using namespace Aftk;

C_SceneCameraNode::C_SceneCameraNode(C_Scene* pScene) : C_SceneNode(pScene),
m_focalLength(E_GenericAttributeType_F32, 0),
m_centreOfInterest(E_GenericAttributeType_F32, 0),
m_horizontalFilmAperture(E_GenericAttributeType_F32, 0),
m_verticalFilmAperture(E_GenericAttributeType_F32, 0),
m_shutterAngle(E_GenericAttributeType_F32, 0),
m_nearClipPlane(E_GenericAttributeType_F32, 0),
m_farClipPlane(E_GenericAttributeType_F32, 0),
m_orthographicWidth(E_GenericAttributeType_F32, 0),
m_focusDistance(E_GenericAttributeType_F32, 0),
m_fStop(E_GenericAttributeType_F32, 0)
{
	OneTimeInit();
}

C_Node* C_SceneCameraNode::Clone(C_Scene* pScene) const
{
	C_SceneCameraNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneCameraNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneCameraNode(pScene);
		(C_SceneCameraNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_SceneCameraNode::~C_SceneCameraNode()
{
	ClearThis();
}

E_ObjType C_SceneCameraNode::GetType() const
{
	return E_ObjType_SceneCamera;
}

bool C_SceneCameraNode::HasType(E_ObjType type) const
{
	if (C_SceneNode::HasType(type))
		return true;
	if (type == C_SceneCameraNode::GetType()) 
		return true;

	return false;
}

void C_SceneCameraNode::SetVerticalFieldOfView(F32 fov)
{
	F32 verticalAperture;
	m_verticalFilmAperture.GetValue(verticalAperture);
	//Derive FOV coefficients and apply.
	F32 focalLength = 0.5f*verticalAperture / tanf(0.5f * fov);
	m_focalLength.SetValue(focalLength);
}

F32 C_SceneCameraNode::GetVerticalFieldOfView() const
{
	F32 verticalAperture;
	m_verticalFilmAperture.GetValue(verticalAperture);
	F32 focalLength;
	m_focalLength.GetValue(focalLength);
	//Calculate FOV and return.
	return 2.f*atanf( (0.5f*verticalAperture) / (focalLength));
}

void C_SceneCameraNode::SetHorizontalFieldOfView(F32 fov)
{
	F32 horizontalAperture;
	m_horizontalFilmAperture.GetValue(horizontalAperture);
	//Derive FOV coefficients and apply.
	F32 focalLength = 0.5f*horizontalAperture / tanf(0.5f * fov);
	m_focalLength.SetValue(focalLength);
}

F32 C_SceneCameraNode::GetHorizontalFieldOfView() const
{
	F32 horizontalAperture;
	m_horizontalFilmAperture.GetValue(horizontalAperture);
	F32 focalLength;
	m_focalLength.GetValue(focalLength);
	//Calculate FOV and return.
	return 2.f*atanf( (0.5f*horizontalAperture) / focalLength);
}

C_StaticPlug<F32>& C_SceneCameraNode::GetFocalLengthPlug()
{
	return m_focalLength;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetFocalLengthPlug() const
{
	return m_focalLength;
}

//Connect this to a constraint node for target camera support.
C_StaticPlug<F32>& C_SceneCameraNode::GetCentreOfInterestPlug()
{
	return m_centreOfInterest;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetCentreOfInterestPlug() const
{
	return m_centreOfInterest;
}

C_StaticPlug<F32>& C_SceneCameraNode::GetHorizontalFilmAperturePlug()
{
	return m_horizontalFilmAperture;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetHorizontalFilmAperturePlug() const
{
	return m_horizontalFilmAperture;
}

C_StaticPlug<F32>& C_SceneCameraNode::GetVerticalFilmAperturePlug()
{
	return m_verticalFilmAperture;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetVerticalFilmAperturePlug() const
{
	return m_verticalFilmAperture;
}

C_StaticPlug<F32>& C_SceneCameraNode::GetShutterAnglePlug()
{
	return m_shutterAngle;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetShutterAnglePlug() const
{
	return m_shutterAngle;
}

C_StaticPlug<F32>& C_SceneCameraNode::GetNearClipPlanePlug()
{
	return m_nearClipPlane;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetNearClipPlanePlug() const
{
	return m_nearClipPlane;
}

C_StaticPlug<F32>& C_SceneCameraNode::GetFarClipPlanePlug()
{
	return m_farClipPlane;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetFarClipPlanePlug() const
{
	return m_farClipPlane;
}

C_StaticPlug<F32>& C_SceneCameraNode::GetOrthographicWidthPlug()
{
	return m_orthographicWidth;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetOrthographicWidthPlug() const
{
	return m_orthographicWidth;
}

C_StaticPlug<F32>& C_SceneCameraNode::GetFocusDistancePlug()
{
	return m_focusDistance;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetFocusDistancePlug() const
{
	return m_focusDistance;
}

C_StaticPlug<F32>& C_SceneCameraNode::GetFStopPlug()
{
	return m_fStop;
}

const C_StaticPlug<F32>& C_SceneCameraNode::GetFStopPlug() const
{
	return m_fStop;
}

void C_SceneCameraNode::SetIsOrthographic(bool isOrthographic)
{
	m_isOrthographic = isOrthographic;
}

bool C_SceneCameraNode::IsOrthographic() const
{
	return m_isOrthographic;
}

void C_SceneCameraNode::SetDepthOfFieldEnabled(bool depthOfFieldEnabled)
{
	m_depthOfFieldEnabled = depthOfFieldEnabled;
}

bool C_SceneCameraNode::GetDepthOfFieldEnabled() const
{
	return m_depthOfFieldEnabled;
}

bool C_SceneCameraNode::operator== (const C_SceneCameraNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneCameraNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneCameraNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneCamera)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneCameraNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(GetType()))
	{
		return false;
	}

	C_SceneCameraNode& rhs = (C_SceneCameraNode&) _rhs;



	return true;
}

const I_Plug* C_SceneCameraNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = C_SceneNode::GetNumStaticPlugs();

	if (instance < staticOffset)
		return C_SceneNode::GetStaticPlugPtr(instance);

	instance -= staticOffset;
	switch (instance)
	{
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FOCAL_LENGTH:
			return &m_focalLength;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_CENTRE_OF_INTEREST:
			return &m_centreOfInterest;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_HORIZONTAL_FILM_APERTURE:
			return &m_horizontalFilmAperture;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_VERTICAL_FILM_APERTURE:
			return &m_verticalFilmAperture;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_SHUTTER_ANGLE:
			return &m_shutterAngle;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_NEAR_CLIP_PLANE:
			return &m_nearClipPlane;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FAR_CLIP_PLANE:
			return &m_farClipPlane;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_ORTHOGRAPHIC_WIDTH:
			return &m_orthographicWidth;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FOCUS_DISTANCE:
			return &m_focusDistance;
		case AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_F_STOP:
			return &m_fStop;
		default:
			break;
	};

	return 0;
}

U32	C_SceneCameraNode::GetNumStaticPlugs() const
{
	return C_SceneNode::GetNumStaticPlugs() + 
		AFTK_SCENE_CAMERA_NODE_NUM_STATIC_PLUGS;
}


// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS
/*
C_SceneCameraNode::C_SceneCameraNode()
{
	C_SceneCameraNode::C_SceneCameraNode(0);
}
*/
void C_SceneCameraNode::OneTimeInit()
{
	U32 staticOffset = C_SceneNode::GetNumStaticPlugs();

	m_focalLength.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FOCAL_LENGTH);
	m_focalLength.SetParentPtr(this);
	m_focalLength.SetName("focalLength");
	
	m_centreOfInterest.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_CENTRE_OF_INTEREST);
	m_centreOfInterest.SetParentPtr(this);
	m_centreOfInterest.SetName("centreOfInterest");
	
	m_horizontalFilmAperture.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_HORIZONTAL_FILM_APERTURE);
	m_horizontalFilmAperture.SetParentPtr(this);
	m_horizontalFilmAperture.SetName("horizontalFilmAperture");
	
	m_verticalFilmAperture.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_VERTICAL_FILM_APERTURE);
	m_verticalFilmAperture.SetParentPtr(this);
	m_verticalFilmAperture.SetName("verticalFilmAperture");
	
	m_shutterAngle.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_SHUTTER_ANGLE);
	m_shutterAngle.SetParentPtr(this);
	m_shutterAngle.SetName("shutterAngle");
	
	m_nearClipPlane.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_NEAR_CLIP_PLANE);
	m_nearClipPlane.SetParentPtr(this);
	m_nearClipPlane.SetName("nearClipPlane");
	
	m_farClipPlane.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FAR_CLIP_PLANE);
	m_farClipPlane.SetParentPtr(this);
	m_farClipPlane.SetName("farClipPlane");

	m_orthographicWidth.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_ORTHOGRAPHIC_WIDTH);
	m_orthographicWidth.SetParentPtr(this);
	m_orthographicWidth.SetName("orthographicWidth");

	m_focusDistance.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_FOCUS_DISTANCE);
	m_focusDistance.SetParentPtr(this);
	m_focusDistance.SetName("focusDistance");

	m_fStop.SetLogicalIndex(staticOffset+AFTK_SCENE_CAMERA_NODE_STATIC_PLUG_F_STOP);
	m_fStop.SetParentPtr(this);
	m_fStop.SetName("fStop");

	ClearThis();
}

void C_SceneCameraNode::Clear()
{
	ClearThis();
	C_SceneNode::Clear();
}

void C_SceneCameraNode::ClearThis()
{
	m_focalLength.SetValue(0.f);
	m_centreOfInterest.SetValue(0.f);
	m_verticalFilmAperture.SetValue(0.f);
	m_horizontalFilmAperture.SetValue(0.f);
	m_shutterAngle.SetValue(0.f);
	m_nearClipPlane.SetValue(0.f);
	m_farClipPlane.SetValue(0.f);
	m_orthographicWidth.SetValue(0.f);
	m_focusDistance.SetValue(0.f);
	m_fStop.SetValue(0.f);

	m_isOrthographic = false;
	m_depthOfFieldEnabled = false;
}

C_SceneCameraNode::C_SceneCameraNode(const C_SceneCameraNode& rhs) : C_SceneNode(0),
m_focalLength(E_GenericAttributeType_F32, 0),
m_centreOfInterest(E_GenericAttributeType_F32, 0),
m_horizontalFilmAperture(E_GenericAttributeType_F32, 0),
m_verticalFilmAperture(E_GenericAttributeType_F32, 0),
m_shutterAngle(E_GenericAttributeType_F32, 0),
m_nearClipPlane(E_GenericAttributeType_F32, 0),
m_farClipPlane(E_GenericAttributeType_F32, 0),
m_orthographicWidth(E_GenericAttributeType_F32, 0),
m_focusDistance(E_GenericAttributeType_F32, 0),
m_fStop(E_GenericAttributeType_F32, 0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneCameraNode& C_SceneCameraNode::operator =(const C_SceneCameraNode& rhs)
{
	C_SceneNode::operator=(rhs);

	ClearThis();

	m_focalLength = rhs.m_focalLength;
	m_centreOfInterest = rhs.m_centreOfInterest;
	m_horizontalFilmAperture = rhs.m_horizontalFilmAperture;
	m_verticalFilmAperture = rhs.m_verticalFilmAperture;
	m_shutterAngle = rhs.m_shutterAngle;
	m_nearClipPlane = rhs.m_nearClipPlane;
	m_farClipPlane = rhs.m_farClipPlane;
	m_orthographicWidth = rhs.m_orthographicWidth;
	m_focusDistance = rhs.m_focusDistance;
	m_fStop = rhs.m_fStop;

	return *this;
}


#endif
