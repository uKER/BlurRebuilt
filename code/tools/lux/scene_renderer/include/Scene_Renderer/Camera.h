#ifndef SR_CAMERA_H
#define SR_CAMERA_H

namespace SRD
{
	enum SrCameraTypeEnum
	{
		SR_CAMERA_TYPE_FIRST_PERSON,
		SR_CAMERA_TYPE_TARGET,

		SR_CAMERA_TYPE_COUNT
	};

	class Camera
	{
	public:
		///
		/// constructor
		Camera(D3dWindow* pD3dWindow);
		///
		/// destructor
		virtual ~Camera(); 

		virtual void Frame(const LPVS::Bounds& bounds);

		virtual bool OnUpdateDevice() = 0;

		///Set the camera's field of view.
		void SetFov(F32 fov);

		//Set the properties of this camera using the given matrix
		void SetFromMatrix(Matrix44& matrix);

		///
		//Set the properties of this camera using the given matrix
		void SetFromMatrix(D3DXMATRIX& matrix);

		/// set the camera's position by hand (won't affect its orientation)
		void SetPosition(const Position3& position);

		Position3 GetPosition() const;
		
		void SetRotation(F32 pitch, F32 yaw, F32 roll);

		///Return the camera's field of view
		F32 GetFov() const;

		/// move the camera forwards (the way its facing) distance units.
		void MoveForward(F32 distance);

		/// move the camera distance units to the right
		void MoveRight(F32 distance);

		/// move the camera fDist units up
		void MoveUp(float distance);

		/// move the camera in some arbitrary direction - assumes direction is of unit length
		void MoveInDirection(float distance, const Vector3F32& direction);

		///Rotate the camera in pitch
		void RotatePitch(F32 angle);

		///Yaw the camera
		void RotateYaw(F32 angle);

		///Roll the camera
		void RotateRoll(float angle);

		/// Return the last used transformation matrix
		void GetViewMatrix(D3DXMATRIX& matrix) const;

		void GetProjectionMatrix(D3DXMATRIX& matrix) const;

		void GetWorldMatrix(D3DXMATRIX& matrix) const;

		virtual SrCameraTypeEnum GetType() const = 0;

		/// Reset this camera to it's intial state. Note: This disassociates this camera with
		/// the D3D window. Any allocated memory is released.
		void Clear();

		/// To be called whenever the viewport is resized.
		virtual void OnResize();

		D3dWindow*		m_pD3dWindow; 

	protected:
		///
		/// constructor
		Camera();
		/// Reset camera state variables without dellocating any memory.
		void Reset();

		// The four vectors that define the camera's position and orientation.
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
		D3DXMATRIX		m_worldMatrix;
	};
}

#endif
