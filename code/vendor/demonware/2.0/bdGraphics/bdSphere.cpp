// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON PS2 PSP-INFRA PSP-ADHOC PS3 WII OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdSphere.h>

#include <bdGraphics/bdGfx.h>

const bdUInt bdSphere::m_numSlices = 10;
const bdUInt bdSphere::m_numStacks = 10;
bdUInt bdSphere::m_numSpheres = 0;
bdUInt bdSphere::m_numSphereVertices = 0;
bdVertex *bdSphere::m_sphereVertexBuffer = BD_NULL;

bdSphere::bdSphere(bdFloat32 radius)
{
	m_meshType = BD_MESH_TRIANGLE_LIST;
	m_meshDestroy = false;
	setColor(bdColor(255, 0, 0));

	if (m_numSpheres == 0)
	{
		init(radius);
		m_sphereVertexBuffer = m_vertexBuffer;
		m_numSphereVertices = m_numVertices;
	}
	else
	{
		m_vertexBuffer = m_sphereVertexBuffer;
		m_numVertices = m_numSphereVertices;
	}

	m_visible = true;
	m_numSpheres++;
}

bdSphere::~bdSphere()
{
	if (--m_numSpheres == 0)
	{
		m_meshDestroy = true;
	}
	else
	{
		m_meshDestroy = false;
	}
}

bdBool bdSphere::init(bdFloat32 radius)
{
	bdUInt vertexIndex = 0;
	bdFloat32 *sinCache1a;
	bdFloat32 *cosCache1a;
	bdFloat32 *sinCache1b;
	bdFloat32 *cosCache1b;
	bdFloat32 zLow;
	bdFloat32 zHigh;
	bdFloat32 sintemp1;
	bdFloat32 sintemp2;

	BD_ASSERT(radius > 0.0f, "Radius must be greater than 0");

	clear();

	m_meshType = BD_MESH_TRIANGLE_LIST;
	m_numIndices = 0;
	m_numVertices = (6 * m_numSlices) + ((m_numStacks - 2) * m_numSlices * 6);
	m_vertexBuffer = new bdVertex[m_numVertices];

	sinCache1a = bdAllocate<bdFloat32>(m_numSlices + 1);
	cosCache1a = bdAllocate<bdFloat32>(m_numSlices + 1);
	sinCache1b = bdAllocate<bdFloat32>(m_numStacks + 1);
	cosCache1b = bdAllocate<bdFloat32>(m_numStacks + 1);

	bdUInt i;
	for (i = 0; i < m_numSlices; i++)
	{
		bdFloat32 angle = 2 * BD_PI * i / m_numSlices;
		sinCache1a[i] = bdSinf32(angle);
		cosCache1a[i] = bdCosf32(angle);
	}

	bdUInt j;
	for (j = 0; j <= m_numStacks; j++)
	{
		bdFloat32 angle = BD_PI * j / m_numStacks;
		sinCache1b[j] = radius * bdSinf32(angle);
		cosCache1b[j] = radius * bdCosf32(angle);
	}

	sinCache1b[0] = 0;
	sinCache1b[m_numStacks] = 0;

	sinCache1a[m_numSlices] = sinCache1a[0];
	cosCache1a[m_numSlices] = cosCache1a[0];

	// First do the caps at the 'poles' of the sphere
	sintemp2 = sinCache1b[1];
	zHigh = cosCache1b[1];

	for (i = m_numSlices; i > 0; i--)
	{
		// The pole vertex
		m_vertexBuffer[vertexIndex].m_x = 0.0f;
		m_vertexBuffer[vertexIndex].m_y = 0.0f;
		m_vertexBuffer[vertexIndex].m_z = radius;
		vertexIndex++;

		m_vertexBuffer[vertexIndex].m_x = sintemp2 * sinCache1a[i - 1];
		m_vertexBuffer[vertexIndex].m_y = sintemp2 * cosCache1a[i - 1];
		m_vertexBuffer[vertexIndex].m_z = zHigh;
		vertexIndex++;

		m_vertexBuffer[vertexIndex].m_x = sintemp2 * sinCache1a[i];
		m_vertexBuffer[vertexIndex].m_y = sintemp2 * cosCache1a[i];
		m_vertexBuffer[vertexIndex].m_z = zHigh;
		vertexIndex++;
	}

	// Now the other pole
	sintemp2 = sinCache1b[m_numStacks - 1];
	zHigh = cosCache1b[m_numStacks - 1];

	for (i = 0; i < m_numSlices; i++)
	{
		// The pole vertex
		m_vertexBuffer[vertexIndex].m_x = 0.0f;
		m_vertexBuffer[vertexIndex].m_y = 0.0f;
		m_vertexBuffer[vertexIndex].m_z = -radius;
		vertexIndex++;

		m_vertexBuffer[vertexIndex].m_x = sintemp2 * sinCache1a[i + 1];
		m_vertexBuffer[vertexIndex].m_y = sintemp2 * cosCache1a[i + 1];
		m_vertexBuffer[vertexIndex].m_z = zHigh;
		vertexIndex++;

		m_vertexBuffer[vertexIndex].m_x = sintemp2 * sinCache1a[i];
		m_vertexBuffer[vertexIndex].m_y = sintemp2 * cosCache1a[i];
		m_vertexBuffer[vertexIndex].m_z = zHigh;
		vertexIndex++;
	}

	for (j = 1; j < m_numStacks - 1; j++)
	{
		zLow = cosCache1b[j];
		zHigh = cosCache1b[j + 1];
		sintemp1 = sinCache1b[j];
		sintemp2 = sinCache1b[j + 1];

		for (bdUInt i = 0; i < m_numSlices; i++)
		{
			// Triangle 1
			m_vertexBuffer[vertexIndex].m_x = sintemp1 * sinCache1a[i];
			m_vertexBuffer[vertexIndex].m_y = sintemp1 * cosCache1a[i];
			m_vertexBuffer[vertexIndex].m_z = zLow;
			vertexIndex++;

			m_vertexBuffer[vertexIndex].m_x = sintemp2 * sinCache1a[i];
			m_vertexBuffer[vertexIndex].m_y = sintemp2 * cosCache1a[i];
			m_vertexBuffer[vertexIndex].m_z = zHigh;
			vertexIndex++;

			m_vertexBuffer[vertexIndex].m_x = sintemp1 * sinCache1a[i + 1];
			m_vertexBuffer[vertexIndex].m_y = sintemp1 * cosCache1a[i + 1];
			m_vertexBuffer[vertexIndex].m_z = zLow;
			vertexIndex++;

			// Triangle 2
			m_vertexBuffer[vertexIndex].m_x = sintemp1 * sinCache1a[i + 1];
			m_vertexBuffer[vertexIndex].m_y = sintemp1 * cosCache1a[i + 1];
			m_vertexBuffer[vertexIndex].m_z = zLow;
			vertexIndex++;

			m_vertexBuffer[vertexIndex].m_x = sintemp2 * sinCache1a[i];
			m_vertexBuffer[vertexIndex].m_y = sintemp2 * cosCache1a[i];
			m_vertexBuffer[vertexIndex].m_z = zHigh;
			vertexIndex++;

			m_vertexBuffer[vertexIndex].m_x = sintemp2 * sinCache1a[i + 1];
			m_vertexBuffer[vertexIndex].m_y = sintemp2 * cosCache1a[i + 1];
			m_vertexBuffer[vertexIndex].m_z = zHigh;
			vertexIndex++;
		}
	}

	bdDeallocate<bdFloat32>(sinCache1a);
	bdDeallocate<bdFloat32>(cosCache1a);
	bdDeallocate<bdFloat32>(sinCache1b);
	bdDeallocate<bdFloat32>(cosCache1b);

	return true;
}

