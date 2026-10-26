#ifndef SR_TARGET_CAMERA_H
#define SR_TARGET_CAMERA_H

namespace SRD
{
	class TargetCamera : public Camera
	{
	public:
		///
		/// constructor
		TargetCamera(D3dWindow* pD3dWindow);
		///
		/// destructor
		virtual ~TargetCamera(); 

		virtual SrCameraTypeEnum GetType() const;

		virtual bool OnUpdateDevice();

		virtual void Frame(const LPVS::Bounds& bounds);

		void MoveInZ(F32 distance);

		void SetZDistance(F32 distance);

		F32 GetZDistance() const;

		/// Reset this camera to it's intial state. Note: This disassociates this camera with
		/// the D3D window. Any allocated memory is released.
		void Clear();

		/// To be called whenever the viewport is resized.
		virtual void OnResize();

	protected:
		///
		/// constructor
		TargetCamera();
		/// Reset camera state variables without dellocating any memory.
		void Reset();

		//Distance from the origin along the camera's Z (at) vector.
		F32 m_zDistance;
	};
}

#endif
