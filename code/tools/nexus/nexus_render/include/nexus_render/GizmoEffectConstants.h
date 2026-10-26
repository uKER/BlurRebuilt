#ifndef NEXUS_RENDER_GIZMO_EFFECT_CONSTANTS_H
#define NEXUS_RENDER_GIZMO_EFFECT_CONSTANTS_H

namespace NexusRender
{
	class D3DShaderManager;

	class GizmoEffectConstants : public EffectConstants
	{
	public:
		GizmoEffectConstants();

		GizmoEffectConstants(const GizmoEffectConstants& rhs);

		~GizmoEffectConstants();

		const GizmoEffectConstants& operator=(const GizmoEffectConstants& rhs);

		Tu::Status Build();

		Tu::Status SetColour(const D3DXVECTOR4& colour);

		Tu::Status UpdateWorldViewProjectionMatrix(const D3DXMATRIX& worldView);

		__forceinline EffectWrapper::EEffectType GetEffectType() const;

		//Return the number of textures that will be loaded for the current material
		__forceinline U32 GetNumTextures() const;

		U32 GetHash() const;

		bool operator==(const EffectConstants& effectConstants) const;

	protected:
		void OneTimeInit();

		D3DXHANDLE	m_mWorldViewProjectionHandle;
		D3DXHANDLE	m_colourHandle;
	};


	EffectWrapper::EEffectType GizmoEffectConstants::GetEffectType() const
	{
		return EffectWrapper::EFFECT_TYPE_GIZMO;
	}

	U32 GizmoEffectConstants::GetNumTextures() const
	{
		return 0;
	}
}

#endif