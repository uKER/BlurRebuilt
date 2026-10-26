#ifndef SR_CAMERA_H
#define SR_CAMERA_H

namespace NexusRender
{
	__declspec(align(16)) class Camera
	{
	public:
		enum ECameraType
		{
			CAMERA_TYPE_FIRST_PERSON,
			CAMERA_TYPE_TARGET,

			CAMERA_TYPE_USER
		};

		///
		/// constructor
		Camera(C_D3DWindow* pD3DWindow);
		///
		/// destructor
		virtual ~Camera(); 

		virtual void Frame(const Aftk::C_Bounds& bounds);

		virtual bool OnUpdateDevice() = 0;

		///Set the camera's field of view.
		void SetFov(F32 fov);

		//Set the proporties of this camera using the given matrix
		void SetFromMatrix(Maths::HMat43& matrix);

		///
		//Set the proporties of this camera using the given matrix
		void SetFromMatrix(D3DXMATRIX& matrix);

		Maths::HVec3 GetPosition() const;

		__forceinline void GetPosition(D3DXVECTOR3& position) const;

		/// set the camera's position by hand (won't affect its orientation)
		void SetOrigin(const Maths::HVec3& origin);

		Maths::HVec3 GetOrigin() const;

		void GetOrigin(D3DXVECTOR3& origin) const;
		
		void SetRotation(F32 pitch, F32 yaw, F32 roll);

		///Return the camera's field of view
		__forceinline F32 GetFov() const;

		/// move the camera forwards (the way its facing) distance units.
		__forceinline void MoveForward(F32 distance);

		/// move the camera distance units to the right
		__forceinline void MoveRight(F32 distance);

		/// move the camera fDist units up
		__forceinline void MoveUp(float distance);

		/// move the camera in some arbitrary direction - assumes direction is of unit length
		__forceinline void MoveInDirection(float distance, const Maths::HVec3& direction);

		///Rotate the camera in pitch
		__forceinline void RotatePitch(F32 angle);

		///Yaw the camera
		__forceinline void RotateYaw(F32 angle);

		///Roll the camera
		__forceinline void RotateRoll(float angle);

		/// Return the last used transformation matrix
		__forceinline void GetViewMatrix(D3DXMATRIX& matrix) const;

		__forceinline void GetProjectionMatrix(D3DXMATRIX& matrix) const;

		__forceinline void GetWorldMatrix(D3DXMATRIX& matrix) const;

		__forceinline void GetFacing(D3DXVECTOR3& facing) const;

		__forceinline void GetViewMatrix(Maths::HMat44& matrix) const;

		__forceinline void GetProjectionMatrix(Maths::HMat44& matrix) const;

		__forceinline void GetFacing(Maths::HVec3& facing) const;

		__forceinline const Frustum& GetFrustum() const;

		virtual ECameraType GetType() const = 0;

		/// Reset this camera to it's intial state. Note: This disassociates this camera with
		/// the D3D window. Any allocated memory is released.
		void Clear();

		/// To be called whenever the viewport is resized.
		virtual void OnResize();

		void* operator new (size_t size) 
		{
			return AllocAligned(size);
		}

		void operator delete (void *p)
		{	
			FreeAligned(p);	
		}

	protected:
		///
		/// constructor
		Camera();

		/// Reset camera state variables without dellocating any memory.
		void Reset();

		NexusRender::Frustum	m_frustum;

		// The four vectors that define the camera's position and orientation.
		D3DXVECTOR3		m_positionWorld;
		D3DXVECTOR3		m_origin;
		D3DXVECTOR3		m_facing;
		D3DXVECTOR3		m_up;
		D3DXVECTOR3		m_right;

		// floats to keep track of how far we've rotated since the last update which
		// can then be used by OnUpdateDevice() to recalculate the new orientation vectors.
		F32				m_rotAboutUp;
		F32				m_rotAboutRight;
		F32				m_rotAboutFacing;

		//Clipping planes
		F32				m_zNear;
		F32				m_zFar;
		F32				m_fov;
		///Used to detect a change in the field of view.
		F32				m_oldFov;

		F32				m_centreZ;

		//Camera projection matrix.
		D3DXMATRIX		m_projectionMatrix;
		D3DXMATRIX		m_viewMatrix;
		C_D3DWindow*		m_pWindow; 
	};

	void Camera::GetPosition(D3DXVECTOR3& position) const
	{
		position = m_origin;
	}

	void Camera::GetFacing(D3DXVECTOR3& facing) const
	{
		facing = m_facing;
	}

	void Camera::GetViewMatrix(D3DXMATRIX& matrix) const
	{
		matrix = m_viewMatrix;
	}

	void Camera::GetProjectionMatrix(D3DXMATRIX& matrix) const
	{
		matrix = m_projectionMatrix;
	}

	void Camera::GetViewMatrix(Maths::HMat44& matrix) const
	{
		matrix = D3DToNightmareMatrix44(m_viewMatrix);
	}

	void Camera::GetProjectionMatrix(Maths::HMat44& matrix) const
	{
		matrix = D3DToNightmareMatrix44(m_projectionMatrix);
	}

	void Camera::GetFacing(Maths::HVec3& facing) const
	{
		facing.X(m_facing[0]);
		facing.Y(m_facing[1]);
		facing.Z(m_facing[2]);
	}

	void Camera::MoveForward(F32 dist)
	{
		m_origin += dist*m_facing;
	}   

	void Camera::MoveRight(F32 dist)
	{
		m_origin += dist*m_right;
	}

	void Camera::MoveUp(F32 dist)
	{
		m_origin += dist*m_up;
	}

	void Camera::MoveInDirection(F32 dist, const Maths::HVec3& direction)
	{
		Maths::HVec3 translation = direction * dist;
		m_origin[0] = translation[0];
		m_origin[0] = translation[1];
		m_origin[0] = translation[2];
	}

	void Camera::RotatePitch(F32 angle)
	{
		m_rotAboutRight += angle;
	}

	void Camera::RotateYaw(F32 angle)
	{
		m_rotAboutUp += angle;
	}

	void Camera::RotateRoll(F32 angle)
	{
		m_rotAboutFacing += angle;
	}

	F32 Camera::GetFov() const
	{
		return m_fov;
	}

	const Frustum& Camera::GetFrustum() const
	{
		return m_frustum;
	}
}

#endif
