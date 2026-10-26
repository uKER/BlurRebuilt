#ifndef FLAT_HIERACHY_RENDERER_H
#define FLAT_HIERACHY_RENDERER_H

namespace SRD
{
	class FlatHierachyRenderer : public HierachyRenderer
	{
	public:
		//Create and associate with a window and scene.
		FlatHierachyRenderer(D3dWindow* pD3dWindow, const SD::SpatialHierachy* pSpatialHierachy);
		//Destructor.
		virtual ~FlatHierachyRenderer();
		///
		///Asignment operator - not implemented.
		FlatHierachyRenderer& operator=(const FlatHierachyRenderer& rhs);
		///
		///Render the current scene to the display device
		virtual void OnUpdateDevice();
		///
		virtual bool RebuildVertexBuffers();
		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		//Default constructor.
		FlatHierachyRenderer();
		//Copy constructor - not implemented.
		FlatHierachyRenderer(const FlatHierachyRenderer& rhs);

		void Reset();
		///
		///Array of vertex buffers for this scene.
		Ut::UtVector<Ut::DynamicVector<IDirect3DVertexBuffer9*> >	m_vertexBuffers;
		Ut::UtVector<Ut::DynamicVector<S32> >						m_vertexBufferSizes;
		///
		bool m_vertexBuffersBuiltFlag;

	#endif
	};
}

#endif