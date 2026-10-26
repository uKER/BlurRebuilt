#ifndef NEXUS_RENDER_FIRST_PERSON_CAMERA_H
#define NEXUS_RENDER_FIRST_PERSON_CAMERA_H

namespace NexusRender
{
	class FirstPersonCamera : public Camera
	{
	public:
		///
		/// constructor
		FirstPersonCamera(C_D3DWindow* pD3DWindow);
		///
		/// destructor
		~FirstPersonCamera(); 

		Camera::ECameraType GetType() const;

		bool OnUpdateDevice();

		void Frame(const Tu::Bounds& bounds);

		/// Reset this camera to it's intial state. Note: This disassociates this camera with
		/// the D3D window. Any allocated memory is released.
		void Clear();

		/// To be called whenever the viewport is resized.
		void OnResize();

	private:
		///
		/// constructor
		FirstPersonCamera();
		/// Reset camera state variables without dellocating any memory.
		void Reset();
	};
}

#endif
