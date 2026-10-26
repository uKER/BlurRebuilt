#ifndef AFTK_UT_CREATE_MATERIAL_H
#define AFTK_UT_CREATE_MATERIAL_H

namespace AftkUt
{

/// utility class that makes it easy to create an aftk material
class MaterialCreator
{
public:
									/// constructor
									MaterialCreator(std::string name);

									/// add a texture to the material
	void							AddTexture(std::string name, std::string filename);
									/// add a parameter to the material
	void							AddParameter(std::string name, float fValue);

									/// create the material using the given parameters/textures
	const Aftk::C_MaterialResource* CreateMaterial(Aftk::C_Scene* pScene, U32 materialTypeId);

private:
	struct TextureInfo
	{
		std::string name;
		std::string filename;
	};
	typedef Ut::Vector<TextureInfo> TextureList;

	struct FloatInfo
	{
		std::string name;
		float fValue;
	};
	typedef Ut::Vector<FloatInfo> FloatList;

	TextureList		m_textures;
	FloatList		m_floats;
	std::string		m_name;
};

}

#endif
