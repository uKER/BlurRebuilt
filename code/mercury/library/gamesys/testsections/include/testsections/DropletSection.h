#ifndef DROPLET_SECTION_TEMPLATE_H
#define DROPLET_SECTION_TEMPLATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>

namespace Droplet
{

class C_Simulation
{
public:
	C_Simulation();
	
	void Init( const S32 width, const S32 height );

	void GenerateRandom();
	void Splat();
	void Clear();

	void Update();

	S32 GetWidth() const { return m_Width; }
	S32 GetHeight() const { return m_Height; }

	S32 GetX( const S32 x ) const { return (x + m_Width * 10000) % m_Width; }
	S32 GetY( const S32 y ) const { return (y + m_Height * 10000) % m_Height; }

	F32 GetValue( const S32 x, const S32 y ) const;

	const F32* GetData() const { return &m_Data[m_CurrBuffer][0]; }

private:

	S32 m_Width;
	S32 m_Height;

	S32 m_CurrBuffer;

	Ut::Vector<F32> m_Data[2];
};

class C_DropletSection : public Framework::C_TestSection
{
public:

	C_DropletSection(const Util::C_String& name);
	~C_DropletSection();


	void OnUpdate();
	void OnPreRender();
	void OnPostRender();
	void OnRender2DView();

private:
	//Display::C_RenderTexture2D m_RenderTexture;

	//Ut::Vector<C_RenderingData> m_Population;

	//C_ImageData m_TargetImage;

	C_Simulation m_Simulation;

	Display::C_Texture2D			m_Texture;

};

} // namespace Droplet

#endif // TEST_SECTION_TEMPLATE_H
