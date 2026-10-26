#include "StdAfx.h"
#include "source/testsection.h"

#define NGL_API

#include <system/serialise/C_FileSystem.h>
#include <system/serialise/BinarySerialise.h>

namespace Display
{
#if PLATFORM_PC
namespace D3D9
{
void LoadTexture2D(C_Texture2D *pTexture, Sys::I_BinaryStream *pStream);
}
#elif PLATFORM_PS3
namespace GCM
{
void LoadTexture2D(C_Texture2D *pTexture, Sys::I_BinaryStream *pStream);
}
#elif PLATFORM_XENON
namespace D3D9
{
void LoadTexture2D(C_Texture2D *pTexture, Sys::I_BinaryStream *pStream);
}
#endif
};

namespace Samples
{

bool CreateTexture2DFromRaw(const Util::C_String& fileName, Display::C_Texture2D* pTexture)
{
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(fileName);

#if PLATFORM_PC
	Display::D3D9::LoadTexture2D(pTexture, pFile);
#elif PLATFORM_PS3
	Display::GCM::LoadTexture2D(pTexture, pFile);
#elif PLATFORM_XENON
	Display::D3D9::LoadTexture2D(pTexture, pFile);
#endif

	delete pFile;

	return true;
}

C_SectionTextureExportTest::C_SectionTextureExportTest(const Util::C_String& name) : C_Section(name)
{
}

C_SectionTextureExportTest::~C_SectionTextureExportTest()
{
}

bool C_SectionTextureExportTest::OnCreate()
{
	CreateTexture2DFromRaw("mytexture.bin", &m_Texture);

	return true;
}

bool C_SectionTextureExportTest::OnCreateVolatile()
{
	return true;
}

void C_SectionTextureExportTest::OnDestroyVolatile()
{
}

void C_SectionTextureExportTest::OnDestroy()
{
	Display::Device()->Destroy(&m_Texture);
}

void C_SectionTextureExportTest::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	if(input.IsButtonDn(Samples::E_Button_A))
	{
		CreateTexture2DFromRaw("mytexture.bin", &m_Texture);
	}
}

void C_SectionTextureExportTest::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionEmpty");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	if (m_Texture.IsCreated())
		Display::Quad::DrawFullScreen(&m_Texture);
	
	Util::C_String str;
	str.Printf("%.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 512.f, 384.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
}

}; // namespace Samples
