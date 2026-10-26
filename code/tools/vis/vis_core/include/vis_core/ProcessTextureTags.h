#pragma once

class C_TextureTags
{
public:
	static bool Process(const Aftk::C_Scene& scene);

private:
	static bool BuildTextureList(
		Ut::UtMap<Util::C_String, bool>& textureSet,
		const Aftk::C_Scene& scene);

	static bool SetTextureTags(
		const Ut::UtMap<Util::C_String, bool>& textureSet);

	static bool IsComplexMaterial(const Aftk::C_ShaderNode& shaderNode, bool& forceAlpha);

	static bool IsComplexMaterial(const Aftk::C_MaterialNode& mtl, bool& forceAlpha);
};