#include "StdAfx.h"
#include "Samples/Section/SectionPCGL.h"

//#define COMPILE_CG_FROM_FILE

namespace Samples
{

struct C_Vertex
{
	F32 x, y;
};

C_SectionPCGL::C_SectionPCGL(const Util::C_String& name) : C_Section(name)
{
}

C_SectionPCGL::~C_SectionPCGL()
{
}

bool C_SectionPCGL::OnCreate()
{
#ifdef GRAPHICS_GL
	static bool created = false;
	if (created)
		return true;
	created = true;

#ifdef COMPILE_CG_FROM_FILE
	Util::C_String vsFileName = Display::FormatString("D:\\devroot\\code\\mercury\\library\\display\\include\\display\\shaders\\test\\vsTest.vsh");
	m_cgVertexShader = cgCreateProgramFromFile(Display::Device()->GetCGContext(), CG_SOURCE, vsFileName.Chars(), CG_PROFILE_VP30, "main", NULL);
	char vsListing[1024];
	sprintf(vsListing, "Creating program from file = %s\n", cgGetLastListing(Display::Device()->GetCGContext()));
	LOGERROR(Display::g_Log, vsListing);
#else
	Util::C_String vsFileName = Display::FormatString(Display::g_GameRoot + "Shaders/vsTest.pcglv");
	m_cgVertexShader = cgCreateProgramFromFile(Display::Device()->GetCGContext(), CG_OBJECT, vsFileName.Chars(), CG_PROFILE_VP30, "main", NULL);
#endif
	assert(m_cgVertexShader);
	cgGLLoadProgram(m_cgVertexShader);
	
	assert(m_cgVertexShader);
	m_cgClipVector = cgGetNamedParameter(m_cgVertexShader, "g_ClipVector");
	assert(m_cgClipVector);

#ifdef COMPILE_CG_FROM_FILE
	Util::C_String psFileName = Display::FormatString("D:\\devroot\\code\\mercury\\library\\display\\include\\display\\shaders\\test\\psTest.psh");
	m_cgPixelShader = cgCreateProgramFromFile(Display::Device()->GetCGContext(), CG_SOURCE, psFileName.Chars(), CG_PROFILE_FP30, "main", NULL);
	char psListing[1024];
	sprintf(psListing, "Creating program from file = %s\n", cgGetLastListing(Display::Device()->GetCGContext()));
	LOGERROR(Display::g_Log, psListing);
#else
	Util::C_String psFileName = Display::FormatString(Display::g_GameRoot + "Shaders/psTest.pcglp");
	m_cgPixelShader = cgCreateProgramFromFile(Display::Device()->GetCGContext(), CG_OBJECT, psFileName.Chars(), CG_PROFILE_FP30, "main", NULL);	
#endif
	assert(m_cgPixelShader);
	cgGLLoadProgram(m_cgPixelShader);

	glGenBuffers(1, &m_glVertexBuffer);
	assert(m_glVertexBuffer != 0);
	Display::GL::GLError();
	
	glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
	Display::GL::GLError();
	
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(C_Vertex), 0, GL_DYNAMIC_DRAW);
	Display::GL::GLError();
#endif
	return true;
}

bool C_SectionPCGL::OnCreateVolatile()
{
	return true;
}

void C_SectionPCGL::OnDestroyVolatile()
{
}

void C_SectionPCGL::OnDestroy()
{
}

void C_SectionPCGL::OnUpdate(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionPCGL::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionPCGL");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 64, 0, 255));

	Display::GL::GLError();

	Display::Device()->SetCullMode(Display::E_CullMode_None);

#if 0
	glScissor(0, 0, Display::Device()->GetWidth(), Display::Device()->GetHeight());
	
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(false);
	
	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_ALWAYS, 0);

	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	glBlendEquation(GL_FUNC_ADD);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	Display::GL::GLError();
#endif

#if 0
	cgGLEnableProfile(CG_PROFILE_VP30);
	cgGLEnableProfile(CG_PROFILE_FP30);

	cgGLBindProgram(m_cgVertexShader);
	cgSetParameter4fv(m_cgClipVector, (F32*)&Display::Device()->m_ClipVector);
	cgGLBindProgram(m_cgPixelShader);

	Display::GL::GLError();

	glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
	C_Vertex* pVertex = (C_Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	pVertex->x =  500.0f;
	pVertex->y = -500.0f;
	pVertex++;
	pVertex->x = -500.0f;
	pVertex->y = -500.0f;
	pVertex++;
	pVertex->x = 0.0f;
	pVertex->y = 500.0f;
	pVertex++;
	glUnmapBuffer(GL_ARRAY_BUFFER);

	Display::GL::GLError();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(C_Vertex), BUFFER_OFFSET(0));
	Display::GL::GLError();

	glDrawArrays(GL_TRIANGLES, 0, 3);
	Display::GL::GLError();
#endif

#if 1
	Util::C_String str;
	str.Printf("PCGL Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 95.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));

	Display::GL::GLError();
#endif
}

}; // namespace Samples
