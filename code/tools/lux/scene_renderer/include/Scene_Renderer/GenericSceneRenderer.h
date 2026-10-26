#ifndef GENERIC_SCENE_RENDERER_H
#define GENERIC_SCENE_RENDERER_H

namespace SRD
{
	class CViewGrid;

	class GenericSceneRenderer : public SceneRenderer
	{
	public:
		//Create and associate with a window and scene.
		GenericSceneRenderer(D3dWindow* pD3dWindow, const Aftk::C_Scene* pScene);
		virtual ~GenericSceneRenderer();

		virtual bool Create();
		virtual void Destroy();

		virtual bool OnResetDevice();

		///Render the current scene to the display device
		virtual void OnUpdateDevice();
		///
		virtual bool RebuildVertexBuffers();

		virtual bool Clear();

	protected:
		struct RealTimeShaderFragmentMapping
		{
			ID3DXEffect*							m_pShader;
			Ut::UtVector< Ut::DynamicVector<U32> >	m_fragmentArrayIndices;
		};

		virtual void Reset();

		void ExtractObjectRenderingFragments(
			U32 currentRenderBatchIndex,
			std::map< 
				const Aftk::C_MaterialResource*, 
				Ut::UtVector< Ut::DynamicVector<ObjectRenderingFragment > > >& fragmentArraysByMaterial);

		bool BuildVertexBuffers();

		bool BuildMaterialShaderMapping();

		ID3DXEffect* LoadAndInitialiseShader(const std::string& fileName);

		GenericSceneRenderer();
		GenericSceneRenderer(const GenericSceneRenderer& rhs);
		const GenericSceneRenderer& operator=(const GenericSceneRenderer& rhs);
		bool DoesSceneUseLightmaps();

		ObjectRenderingBatch*		m_pRenderableObjects;
		U32							m_numRenderableNodes;

		//2D array of object fragments to render
		Ut::UtVector< Ut::DynamicVector< 
			Ut::UtVector< Ut::DynamicVector<ObjectRenderingFragment> > > >	m_fragmentArrays;
		
		//Array of indices into the fragment array array.
		
		//Map of shaders for particular materials. Note that there are likley to be 
		//some effect pointers that are used for multiple material nodes. This is as shaders
		//are a resource that is managed in a similar way to texture maps, via the D3dShaderManager
		std::map< 
			const Aftk::C_MaterialResource* , ShaderProperties* >	m_shaderMapping;

		//Indices of the fragment arrays that use a particular material.
		std::map< 
			ID3DXEffect* , Ut::UtVector< Ut::DynamicVector<U32> > >			m_fragmentArrayIndices;

		// the grid
		CViewGrid *m_pViewGrid;

		LPD3DXMESH m_pSphereMesh;

	};
}

#endif
