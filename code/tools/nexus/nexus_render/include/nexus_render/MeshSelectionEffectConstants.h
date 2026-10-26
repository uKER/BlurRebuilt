#pragma once

namespace NexusRender
{
	class D3DShaderManager;

	class MeshSelectionEffectConstants : public EffectConstants
	{
	public:
		MeshSelectionEffectConstants();

		MeshSelectionEffectConstants(const MeshSelectionEffectConstants& rhs);

		~MeshSelectionEffectConstants();

		const MeshSelectionEffectConstants& operator=(const MeshSelectionEffectConstants& rhs);

		Tu::Status Build();

		__forceinline EffectWrapper::EEffectType GetEffectType() const;

		//Get the name of the nth texture
		__forceinline const TextureInfo* TextureInfo(U32 index) const;

		U32 GetHash() const;

		bool operator==(const EffectConstants& effectConstants) const;

	private:
		void OneTimeInit();

		const Aftk::C_MaterialNode*	m_pAftkMaterialNode;
		friend class MeshSelectionEffectWrapper;
	};

	EffectWrapper::EEffectType MeshSelectionEffectConstants::GetEffectType() const
	{
		return EffectWrapper::EFFECT_TYPE_MESH_SELECTION;
	}
}



