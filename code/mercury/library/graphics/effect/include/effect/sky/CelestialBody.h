#ifndef EFFECT_SKY_CELESTIAL_BODY_H
#define EFFECT_SKY_CELESTIAL_BODY_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <SceneGraph/RenderNode.h>
#include <SceneGraph/GatherNode.h>

namespace Effect
{
namespace Sky
{

class C_CelestialBody : public Display::I_Renderer, public SceneGraph::I_RenderNode, public SceneGraph::I_GatherNode
{
public:

	C_CelestialBody();
	~C_CelestialBody();

	bool OnCreate();
	void OnDestroy();

	void OnRender(SceneGraph::C_Context& context) const;
	void OnGather(const SceneGraph::C_View& view) const;
	void Accept(SceneGraph::I_Visitor& visitor) const;
	void SetRenderPass(const U32 renderPass);

	void Render() const;

	void SetDirection(const Maths::HVec3& vDirection);
	void SetDirection(const Display::C_Float3& vDirection);
	void SetHeadingOffset(const F32 fHeadingOffset)			{ m_fHeadingOffset = fHeadingOffset; }
	void SetElevationOffset(const F32 fElevationOffset)		{ m_fElevationOffset = fElevationOffset; }
	void SetSize(const F32 fSize)							{ m_fSize = fSize; }
	void SetColour(const Maths::C_RGBA_F32& vColour)		{ Maths::Store(vColour, &m_vColour); }
	void SetColour(const Display::C_Float4& vColour)		{ m_vColour = vColour; }
	void SetTexture(const Util::C_String& textureName)		{ m_TextureName = textureName; }
	Util::C_String GetTexture() const						{ return m_TextureName; }

	bool ReloadTexture();

private:

	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader;
	Display::C_PixelShaderParameter	m_LuminanceParameter;

	Display::C_Texture2D			m_Texture;
	Util::C_String					m_TextureName;

	Display::C_Float3				m_vDirection;
	F32								m_fSize;
	Display::C_Float4				m_vColour;
	F32								m_fHeadingOffset;
	F32								m_fElevationOffset;

	U32								m_iRenderPass;
	
};

}; // namespace Sky
}; // namespace Effect

#endif // EFFECT_SKY_CELESTIAL_BODY_H
