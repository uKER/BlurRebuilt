#ifndef AFTK_RENDER_CAMERA_H
#define AFTK_RENDER_CAMERA_H

#ifdef _MSC_VER
#pragma once
#endif


namespace AftkRender
{
	class C_Camera
	{
	public:
		enum E_CameraType
		{
			E_CameraType_Perspective,
			E_CameraType_Orthographic
		};

	public:
		C_Camera(const Maths::HVec3& position = Maths::HVec3(0.f, 0.f, -10.f),
				 const Maths::HVec3& focus	  = Maths::HVec3(0.f, 0.f, 0.f),
				 const Maths::HVec3& up       = Maths::HVec3(0.f, 1.f, 0.f),
				 E_CameraType type            = E_CameraType_Perspective);

		~C_Camera();

		void SetPosition(const Maths::HVec3& p);
		Maths::HVec3& GetPosition();
		const Maths::HVec3& GetPosition() const;

		void SetUpVector(const Maths::HVec3& v);
		Maths::HVec3& GetUpVector();
		const Maths::HVec3& GetUpVector() const;

		void SetFocusPosition(const Maths::HVec3& p);
		Maths::HVec3& GetFocusPosition();
		const Maths::HVec3& GetFocusPosition() const;

		void RotateAboutAxis(F32 angle, const Maths::HVec3& axis);

		void SetNearClipDistance(F32 d);
		F32 GetNearClipDistance() const;

		void SetFarClipDistance(F32 d);
		F32 GetFarClipDistance() const;

		void SetFieldOfView(F32 fov);
		F32 GetFieldOfView() const;

		void SetAspectRatio(F32 r);
		F32 GetAspectRatio() const;

		void SetHeightExtent(F32 h);
		F32 GetHeightExtent() const;

		E_CameraType GetType() const;
		void SetType(E_CameraType type);
	
	private:
		Maths::HVec3	m_Position, m_Focus, m_Up;
		F32				m_Speed;

		F32				m_NearClip;
		F32				m_FarClip;
		F32				m_FieldOfView;
		F32				m_AspectRatio;

		F32				m_HeightExtent;
		E_CameraType	m_CameraType;
	};


	static Display::C_ViewSetup ConvertToViewSetup(const Maths::C_Rect& windowCoords, const C_Camera& camera);


	#include "Camera.inl"
}


#endif