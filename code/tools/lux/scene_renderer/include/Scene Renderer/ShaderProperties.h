#ifndef SHADER_PROPORTIES_H
#define SHADER_PROPORTIES_H

namespace SRD
{
	enum ShaderPropertiesTypeEnum
	{
		SHADER_PROPORTIES_TYPE_LIGHTMAP = 0,

		SHADER_PROPORTIES_TYPE_COUNT
	};

	class ShaderProperties
	{
	public:
		ShaderProperties();
		ShaderProperties(const ShaderProperties& rhs);
		virtual ~ShaderProperties();

		virtual ShaderPropertiesTypeEnum GetType() const = 0;

		ID3DXEffect*	GetShaderPtr();

		void			SetShaderPtr(ID3DXEffect* pShader);

	protected:
		ID3DXEffect*	m_pShader;
	};
}

#endif

