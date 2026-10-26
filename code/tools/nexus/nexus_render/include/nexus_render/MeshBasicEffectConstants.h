#pragma once

namespace NexusRender
{
	class D3DShaderManager;

	class MeshBasicEffectConstants : public EffectConstants
	{
	public:
		MeshBasicEffectConstants();

		MeshBasicEffectConstants(const MeshBasicEffectConstants& rhs);

		~MeshBasicEffectConstants();

		const MeshBasicEffectConstants& operator=(const MeshBasicEffectConstants& rhs);

		Tu::Status Build(
			const Aftk::C_MaterialNode* pAftkMaterialNode);

		FORCE_INLINE EffectWrapper::EEffectType GetEffectType() const;

		U32 GetHash() const;

		bool operator==(const EffectConstants& effectConstants) const;

	private:
		void OneTimeInit();

		friend class MeshBasicEffectWrapper;
	};

	EffectWrapper::EEffectType MeshBasicEffectConstants::GetEffectType() const
	{
		return EffectWrapper::EFFECT_TYPE_MESH_BASIC;
	}
}



