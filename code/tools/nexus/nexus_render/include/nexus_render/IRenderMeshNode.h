#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class IRenderMeshNode : public RenderNode
	{
	public:
		enum EEffectMode
		{
			EFFECT_MODE_STATIC,
			EFFECT_MODE_SKINNED,
			EFFECT_MODE_BONED_INSTANCE,
			
			EFFECT_MODE_COUNT,

			EFFECT_MODE_NOT_APPLICABLE,
		};

		IRenderMeshNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkNode);
		
		virtual ~IRenderMeshNode();

		virtual void Clear();

		virtual Tu::Status Rebuild() = 0;

		virtual Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged) = 0;

		virtual Tu::Status DrawNormals(U32 lodLevel) = 0;

		virtual Tu::Status DrawTangentSpace(U32 lodLevel, U16 set) = 0;

		virtual Tu::Status DrawSelectedComponents(U32 lodLevel) = 0;

		//Draw the entire mesh using the currently active LOD.
		virtual Tu::Status Draw() = 0;

		//Draw only the portion of this mesh that uses the material at the given index.
		virtual Tu::Status Draw(U32 lodLevel, U32 clusterIndex) = 0;

		virtual const F32* GetLightmapUVOffsets() const = 0;

		virtual bool IsSkinned() const = 0;

		virtual bool IsSkinAnimated() const = 0;

		virtual U32 GetNumClusters(U32 lod) const = 0;

		virtual const MeshCluster* GetClusterPtr(U32 lodLevel, U32 clusterIndex) const = 0;

		virtual MeshCluster* GetClusterPtr(U32 lodLevel, U32 clusterIndex) = 0;

		///\brief Returns an enum indicating which vertex declaration should be used to render this mesh.
		///
		///@return The vertex declaration enum.
		virtual D3DManager::EVertexDecl GetVertexDecl() const = 0;

		virtual EEffectMode GetEffectMode() const = 0;

		virtual U8 GetNumUVSets() const = 0;

		virtual ERenderNodeType GetType() const;

		virtual bool HasType(ERenderNodeType type) const;

		__forceinline static const char* GetEffectModeAsString(EEffectMode mode);

	protected:
		static char** m_ppEffectModeAsString;
	};

	const char* IRenderMeshNode::GetEffectModeAsString(EEffectMode mode)
	{
		return m_ppEffectModeAsString[mode];
	}
}
