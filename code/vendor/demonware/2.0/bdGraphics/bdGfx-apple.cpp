// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdGraphics/bdGfx.h>

#if defined(BD_PLATFORM_OSX)
#include <bdGraphics/bdCocoaBridge-osx.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#elif defined(BD_PLATFORM_IPHONE)
#include <bdGraphics/bdUIKitBridge-iphone.h>
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#error Platform is Apple but not OSX or iPhone.
#endif

BD_REGISTER_SINGLETON(bdGfxImpl)

#define BITS_PER_PIXEL		  32.0
#define DEPTH_SIZE			  32.0
#define DEFAULT_TIME_INTERVAL   0.001

bdGfxImpl::bdGfxImpl()
{
}

bdGfxImpl::~bdGfxImpl()
{
}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	return initGL();
}

bdBool bdGfxImpl::runFrame() const
{
	render();
	return true;
}

void bdGfxImpl::cleanup()
{
}

void bdGfxImpl::setText(const bdNChar8 *const str)
{
#ifdef BD_PLATFORM_OSX
	bdCocoaUpdateDisplayString(str);
#else
	bdUIKitUpdateDisplayString(str);
#endif
}

void bdGfxImpl::setBackgroundColor(const bdColor &color)
{
	bdFloat32 col[4];

	// Store the background color
	bdGfxBase::setBackgroundColor(color);

	// Convert the color to a floating point representation of the components
	color.asFloat(col);

	// Set the OpenGL clear color
	glClearColor(col[0], col[1], col[2], col[3]);
}

bdBool bdGfxImpl::initGL()
{
	bdFloat32 color[4];

	// Convert the background color to a floating-point representation
	m_backgroundColor.asFloat(color);

	// Set the OpenGL clear color
	glClearColor(color[0], color[1], color[2], color[3]);

	// Set the shading model to smooth (as opposed to flat)
	glShadeModel(GL_SMOOTH);

	// Set up the depth buffer. Set the clear value for the depth buffer to 1.0 (for glClear()), enable depth testing
	// and let pixels pass the test if their depth is <= the current value in the depth buffer.
	//glClearDepth(1.0f); // TODO: Determine if this is needed on OSX.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Clear and swap the buffers now to fill in the window ASAP
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Turn off lighting.
	glDisable(GL_LIGHTING);

	// Turn off back face culling because direct x and OpenGL use different (left- and right-handed) coordinate systems
	glDisable(GL_CULL_FACE);

	// Use the nicest looking perspective correction
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set up the viewport
	glViewport(0, 0, 640, 480);

	// Enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);

	return true;
}

void bdGfxImpl::render() const
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifdef BD_PLATFORM_OSX
	gluPerspective(45.0f, 640.0f / 480.0f, 1, 1000);
#else
	// Calculate gluPerspective function by 'hand' and use glFrustumf instead.
	float fovy = 45.0f;
	float aspect = 320.0f / 480.0f;
	float zNear = 1;
	float zFar = 1000;

	float xmin, xmax, ymin, ymax;
	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustumf(xmin, xmax, ymin, ymax, zNear, zFar);
#endif

	// Set up the viewpoint using the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#ifdef BD_PLATFORM_OSX
	gluLookAt(	0, 0, 15,	// The position of the camera
				0, 0, 0,	// The target point of the camera
				0, 1, 0);	// The up vector
#else
	lookAt(	0, 0, 15,	// The position of the camera
			0, 0, 0,	// The target point of the camera
			0, 1, 0);	// The up vector
#endif

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
}

#ifdef BD_PLATFORM_IPHONE
void bdGfxImpl::lookAt(GLfloat camX, GLfloat camY, GLfloat camZ,
						GLfloat centerX, GLfloat centerY, GLfloat centerZ,
						GLfloat upx, GLfloat upy, GLfloat upz) const
{
	GLfloat m[16];
	GLfloat x[3], y[3], z[3];
	GLfloat magnitude;

	/* Make rotation matrix */

	/* Z vector */
	z[0] = camX - centerX;
	z[1] = camY - centerY;
	z[2] = camZ - centerZ;
	
	magnitude = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);

	if (magnitude) {
		z[0] /= magnitude;
		z[1] /= magnitude;
		z[2] /= magnitude;
	}

	/* Y vector */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;
	
	/* X vector = Y cross Z */
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];

	/* Recompute Y = Z cross X */
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];

	// cross product gives area of parallelogram, which is <1 for non-perpendicular unit-length vectors; so normalize x, y here
	magnitude = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);

	if (magnitude) {
		x[0] /= magnitude;
		x[1] /= magnitude;
		x[2] /= magnitude;
	}

	magnitude = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);

	if (magnitude) {
		y[0] /= magnitude;
		y[1] /= magnitude;
		y[2] /= magnitude;
	}

#define M(row,column)  m[column * 4 + row]
	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M

	glMultMatrixf(m);

	/* Translate cam to Origin */
	glTranslatef(-camX, -camY, -camZ);
}
#endif
