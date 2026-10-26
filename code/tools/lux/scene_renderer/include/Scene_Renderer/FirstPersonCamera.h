#ifndef SR_FIRST_PERSON_CAMERA_H
#define SR_FIRST_PERSON_CAMERA_H

namespace SRD
{
	class FirstPersonCamera : public Camera
	{
	public:
		///
		/// constructor
		FirstPersonCamera(D3dWindow* pD3dWindow);
		///
		/// destructor
		virtual ~FirstPersonCamera(); 

		virtual SrCameraTypeEnum GetType() const;

		virtual bool OnUpdateDevice();

		virtual void Frame(const LPVS::Bounds& bounds);

		/// Reset this camera to it's intial state. Note: This disassociates this camera with
		/// the D3D window. Any allocated memory is released.
		void Clear();

		/// To be called whenever the viewport is resized.
		virtual void OnResize();

	protected:
		///
		/// constructor
		FirstPersonCamera();
		/// Reset camera state variables without dellocating any memory.
		void Reset();
	};
}

#endif
