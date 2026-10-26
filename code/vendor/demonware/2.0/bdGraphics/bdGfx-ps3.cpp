// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdGfx.h>

#include <bdCore/bdCore.h>
#include <bdGraphics/bdMesh.h>

#include <cell/dbgfont.h>
#include <cell/sysmodule.h>
#include <sys/spu_initialize.h>

BD_REGISTER_SINGLETON(bdGfxImpl)

#define BD_PSGL_MAX_SPUS (1u)
#define BD_PSGL_PERSISTENT_MEMORY_SIZE (64 << 20)

//extern PSGLbufferParameters g_bdDisplayParameters;

bdGfxImpl::bdGfxImpl() :
 m_glDevice(BD_NULL),
 m_glContext(BD_NULL),
 m_deviceWidth(0),
 m_deviceHeight(0)
{
}

bdGfxImpl::~bdGfxImpl()
{
}

void loadModules()
{
	int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_GCM_SYS);
	switch( ret )
	{
		case CELL_OK:
			// The module is successfully loaded,^M
		break;
					 
		case CELL_SYSMODULE_ERROR_DUPLICATED:
			 // The module was already loaded,
		break;
												 
		case CELL_SYSMODULE_ERROR_UNKNOWN:
		case CELL_SYSMODULE_ERROR_FATAL:
		break;
		
	}

	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	switch( ret )
	{
		case CELL_OK:
				// The module is successfully loaded,
		 break;
				 
		case CELL_SYSMODULE_ERROR_DUPLICATED:
				// The module was already loaded,
		break;
											 
		case CELL_SYSMODULE_ERROR_UNKNOWN:
		case CELL_SYSMODULE_ERROR_FATAL:
			break;
		
	}
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_IO);
	switch( ret )
	{
		case CELL_OK:
			 // The module is successfully loaded,
			break;
					 
		case CELL_SYSMODULE_ERROR_DUPLICATED:
			// The module was already loaded,
		 break;
		
		case CELL_SYSMODULE_ERROR_UNKNOWN:
		case CELL_SYSMODULE_ERROR_FATAL:
			break;
	}


}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	//PSGLinitOptions initOptions;

	//initOptions.enable = PSGL_INIT_MAX_SPUS | PSGL_INIT_INITIALIZE_SPUS/*PSGL_INIT_MAX_SPUS | PSGL_INIT_INITIALIZE_SPUS | PSGL_INIT_PERSISTENT_MEMORY_SIZE */;
	//initOptions.maxSPUs = 1;
	//initOptions.initializeSPUs = GL_TRUE;
	sys_spu_initialize(4, 3);
	loadModules();
	psglInit(NULL);
	psglFXInit();

	//m_glDevice = psglCreateDevice(&g_bdDisplayParameters);
	m_glDevice = psglCreateDeviceAuto(GL_ARGB_SCE, GL_DEPTH_COMPONENT24, GL_MULTISAMPLING_NONE_SCE);

	if (!m_glDevice)
	{
		return false;
	}

	m_glContext = psglCreateContext();

	if (!m_glContext)
	{
		return false;
	}

	psglMakeCurrent(m_glContext, m_glDevice);
	psglResetCurrentContext();
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);

	bdFloat32 background[4];
	m_backgroundColor.asFloat(background);
	glClearColor(background[0], background[1], background[2], background[3]);
	glShadeModel(GL_SMOOTH);
	glClearDepthf(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	psglSwap();
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	CellDbgFontConfig fontConfig;

	psglGetDeviceDimensions( m_glDevice, &m_deviceWidth, &m_deviceHeight );

	fontConfig.screenHeight = m_deviceHeight;
	fontConfig.screenWidth = m_deviceWidth;
	glViewport(0, 0, m_deviceWidth, m_deviceHeight);

	glEnableClientState(GL_VERTEX_ARRAY);

	fontConfig.bufSize = BD_MAX_SCREEN_TEXT;
	
	if (cellDbgFontInit(&fontConfig) != CELL_OK)
	{
		return false;
	}

	return true;
}

bdBool bdGfxImpl::runFrame() const
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspectivef(45.0f, bdFloat32(m_deviceWidth) / m_deviceHeight, 1, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAtf(	0, 0, 15,	// The position of the camera
		0, 0, 0,	// The target point of the camera
		0, 1, 0);	// The up vector

	for (bdUInt i = 0; i < m_meshes.getSize(); i++)
	{
		// Store the modelview matrix
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		// Draw the mesh
		m_meshes[i]->draw();

		// Restore the modelview matrix
		glPopMatrix();
	}

	if (m_text[0] != '\0')
	{
		cellDbgFontPuts(0.1f, 0.1f, 1.3f, BD_UINT32_MAX, m_text);
		cellDbgFontDraw();
	}

	glFlush();

	psglSwap();
	
	return true;
}

void bdGfxImpl::cleanup()
{
	// XXX: todo.
}

//void bdGfxImpl::render() const
//{
//	LPDIRECT3DDEVICE8 device = bdD3D::getInstance()->getDevice();
//	D3DXMATRIX matView;
//	D3DXMATRIX matProj;
//	D3DXVECTOR3 position(0, 0, -15);
//	D3DXVECTOR3 at(0, 0, 0);
//	D3DXVECTOR3 up(0, 1, 0);
//
//	device->Clear(	0L,
//					NULL,
//					D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
//					ConvertColor(m_backgroundColor),
//					1.0f,
//					0L);
//
//	D3DXMatrixLookAtLH(&matView, &position, &at, &up);
//	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 640.0f / 480.0f, 1, 1000);
//
//	device->SetTransform(D3DTS_VIEW, &matView);
//	device->SetTransform(D3DTS_PROJECTION, &matProj);
//
//	device->BeginScene();
//
//	for (bdUInt i = 0; i < m_meshes.getSize(); i++)
//	{
//		m_meshes[i]->draw();
//	}
//
//	if (m_text[0] != '\0')
//	{
//		bdUInt16 *wideText;
//
//		// We need to convert the text to Unicode
//		wideText = new bdUInt16[bdStrlen(m_text) + 1];
//		mbstowcs(wideText, m_text, bdStrlen(m_text) + 1);
//
//		// Draw the text
//		m_font.drawText(m_textPositionX, m_textPositionY, m_textColor, wideText);
//
//		delete [] wideText;
//	}
//
//	device->EndScene();
//
//	device->Present(NULL, NULL, NULL, NULL);
//}


