#ifndef HIERACHY_RENDERER_H
#define HIERACHY_RENDERER_H

namespace SRD
{
	class HierachyRenderer
	{
	public:
		//Create and associate with a window and scene.
		HierachyRenderer(D3dWindow* pD3dWindow, const SD::SpatialHierarchy* pSpatialHierarchy);
		//Destructor.
		virtual ~HierachyRenderer();
		
		///Associate this scene renderer with a particular D3D window.
		void SetD3dWindowPtr(D3dWindow* pD3dWindow);
		///
		///Associate this scene renderer with a particular scene hierachy.
		void SetSpatialHierarchyPtr(const SD::SpatialHierarchy* pSpatialHierarchy);
		///
		///Render the current scene to the display device
		virtual void OnUpdateDevice() = 0;
		///
		virtual bool RebuildVertexBuffers() = 0;
		///
		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		//Default constructor.
		HierachyRenderer();

		//Copy constructor - not implemented.
		HierachyRenderer(const HierachyRenderer& rhs);
		const HierachyRenderer& operator=(const HierachyRenderer& rhs);

		void Reset();

		///Store the spatial hierachy that we are to render.
		///
		const SD::SpatialHierarchy* m_pSpatialHierarchy;
		///
		///Store the window that we are attached to.
		D3dWindow* m_pD3dWindow;

	#endif
	};

}

#endif