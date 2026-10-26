#pragma once

namespace NexusRender
{
	class D3DTextureManager;
	class SceneRenderer;

	class EffectConstants
	{
	public:
		struct TextureInfo
		{
			std::string fileName;
			unsigned forceFullRes:1;
		};

		EffectConstants();

		EffectConstants(const EffectConstants& rhs);

		virtual ~EffectConstants();

		const EffectConstants& operator=(const EffectConstants& rhs);

		virtual void GetTextureInfo(Ut::UtVector<Ut::DynamicVector<const TextureInfo*> >& textureInfo) const {}

		virtual void LoadTextures(SceneRenderer& sceneRenderer) {}

		virtual Tu::Status LoadCropSheetTexture(SceneRenderer& sceneRenderer) {return Tu::Status::SUCCESS;}

		virtual EffectWrapper::EEffectType GetEffectType() const = 0;

		virtual U32 GetHash() const = 0;

		virtual bool operator==(const EffectConstants& effectConstants) const = 0;
	};
}


