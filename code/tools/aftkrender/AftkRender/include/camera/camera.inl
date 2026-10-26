

inline C_Camera::C_Camera(
	const Maths::HVec3& position,
	const Maths::HVec3& focus,
	const Maths::HVec3& up,
	C_Camera::E_CameraType type)
: m_Position(position)
, m_Focus(focus)
, m_Up(up)
, m_CameraType(type)
, m_Speed(1.f)
, m_NearClip(0.1f)
, m_FarClip(10000.f)
, m_FieldOfView(60.f)
, m_AspectRatio(1.f)
, m_HeightExtent(0.f)
{

}

inline C_Camera::~C_Camera()
{
}

inline void C_Camera::SetPosition(const Maths::HVec3& p)
{
	m_Position = p;
}

inline Maths::HVec3& C_Camera::GetPosition()
{
	return m_Position;
}

inline const Maths::HVec3& C_Camera::GetPosition() const
{
	return m_Position;
}

inline void C_Camera::SetUpVector(const Maths::HVec3& v)
{
	m_Up = v;
}

inline Maths::HVec3& C_Camera::GetUpVector()
{
	return m_Up;
}

inline const Maths::HVec3& C_Camera::GetUpVector() const
{
	return m_Up;
}

inline void C_Camera::SetFocusPosition(const Maths::HVec3& p)
{
	m_Focus = p;
}

inline Maths::HVec3& C_Camera::GetFocusPosition()
{
	return m_Focus;
}

inline const Maths::HVec3& C_Camera::GetFocusPosition() const
{
	return m_Focus;
}

inline void C_Camera::RotateAboutAxis(F32 angle, const Maths::HVec3& axis)
{
	Maths::HVec3 n = axis;
	Maths::HVec3 v(m_Focus - m_Position);
	Maths::HVec3 f;

	v.Normalise();

	F32 sinAngle, cosAngle;
	Maths::SinCos(&sinAngle, &cosAngle, angle);

    F32 oneMinusCosAngle = 1.0f-cosAngle;

	F32 focusX, focusY, focusZ;

    focusX  = (n.X()*n.X() + cosAngle*(1-n.X()*n.X())) * v.X();
    focusX += (n.X()*n.Y()*oneMinusCosAngle - sinAngle*n.Z()) * v.Y();
    focusX += (n.X()*n.Z()*oneMinusCosAngle + sinAngle*n.Y()) * v.Z();

    focusY  = (n.X()*n.Y()*oneMinusCosAngle + sinAngle*n.Z()) * v.X();
    focusY += (n.Y()*n.Y() + cosAngle*(1-n.Y()*n.Y())) * v.Y();
    focusY += (n.Y()*n.Z()*oneMinusCosAngle - sinAngle*n.X()) * v.Z();

    focusZ  = (n.X()*n.Z()*oneMinusCosAngle - sinAngle*n.Y()) * v.X();
    focusZ += (n.Y()*n.Z()*oneMinusCosAngle + sinAngle*n.X()) * v.Y();
    focusZ += (n.Z()*n.Z() + cosAngle*(1-n.Z()*n.Z())) * v.Z();

	m_Focus = m_Position + Maths::HVec3(focusX, focusY, focusZ);
}

inline void C_Camera::SetNearClipDistance(F32 d)
{
	m_NearClip = d;
}

inline F32 C_Camera::GetNearClipDistance() const
{
	return m_NearClip;
}

inline void C_Camera::SetFarClipDistance(F32 d)
{
	m_FarClip = d;
}

inline F32 C_Camera::GetFarClipDistance() const
{
	return m_FarClip;
}

inline void C_Camera::SetFieldOfView(F32 fov)
{
	m_FieldOfView = fov;
}

inline F32 C_Camera::GetFieldOfView() const
{
	return m_FieldOfView;
}

inline void C_Camera::SetAspectRatio(F32 r)
{
	m_AspectRatio = r;
}

inline F32 C_Camera::GetAspectRatio() const
{
	return m_AspectRatio;
}

inline void C_Camera::SetHeightExtent(F32 h)
{
	m_HeightExtent = h;
}

inline F32 C_Camera::GetHeightExtent() const
{
	return m_HeightExtent;
}

inline C_Camera::E_CameraType C_Camera::GetType() const
{
	return m_CameraType;
}

inline void C_Camera::SetType(C_Camera::E_CameraType type)
{
	m_CameraType = type;

	//initialise camera extent
	if (m_HeightExtent<Maths::Constants<F32>::Epsilon())
		m_HeightExtent = m_NearClip*Maths::Sin(Maths::DegToRad(m_FieldOfView)*0.5f);
}

inline Display::C_ViewSetup ConvertToViewSetup(const Maths::C_Rect& windowCoords, const C_Camera& camera)
{
	Display::C_ViewSetup setup;
	setup.m_WindowCoords = windowCoords;

	const Maths::HVec3& position = camera.GetPosition();
	const Maths::HVec3& direction = camera.GetFocusPosition() - camera.GetPosition();

	Maths::HVec3 toX, toY, toZ;
	Maths::HMat33 lookMatrix;
	toZ = direction;
	toZ.Normalise();

	toX = camera.GetUpVector()^toZ;
	toX.Normalise();

	toY = toZ^toX;
	toY.Normalise();

	lookMatrix.AX(toX);
	lookMatrix.AY(toY);
	lookMatrix.AZ(toZ);

	setup.m_ViewSetup3D.m_AspectRatio = camera.GetAspectRatio();
	setup.m_ViewSetup3D.m_CameraToWorld = Maths::HMat43(lookMatrix);
	setup.m_ViewSetup3D.m_CameraToWorld.AW(position);
	setup.m_ViewSetup3D.m_FieldOfView = camera.GetFieldOfView();
	setup.m_ViewSetup3D.m_NearClip = camera.GetNearClipDistance();
	setup.m_ViewSetup3D.m_FarClip = camera.GetFarClipDistance();
	setup.m_ViewSetup3D.m_PerspDist = camera.GetFarClipDistance();

	F32 h;
	if (camera.GetType() == C_Camera::E_CameraType_Orthographic)
	{
		setup.m_ViewSetup3D.m_PerspDist = 0.f;
		h = 0.5f*camera.GetHeightExtent();
		if (h<Maths::Constants<F32>::Epsilon())
			h = Maths::Tan(Maths::DegToRad(camera.GetFieldOfView())*0.5f);
	}
	else
		h = camera.GetFarClipDistance()*Maths::Tan(Maths::DegToRad(camera.GetFieldOfView())*0.5f);

	F32 w = h*camera.GetAspectRatio();
	setup.m_ViewSetup3D.m_ViewExtents.SetBounds(-w, -h, w, h);

	return setup;
}

