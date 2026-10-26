// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON PS2 PSP-INFRA PSP-ADHOC PS3 WII OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdGfx.h>

bdGfxBase::bdGfxBase()
:	m_backgroundColor(51, 102, 153),
	m_textColor(255, 255, 255)
{
	m_text[0] = '\0';
	m_textPositionX = 5;
	m_textPositionY = 5;
}

bdGfxBase::~bdGfxBase()
{
}

void bdGfxBase::setText(const bdNChar8 *const str)
{
	// Check that str is a valid string
	if (str)
	{
		// Check that str and m_text are not the same
		if (bdStrcmp(str, m_text) != 0)
		{
			// Copy over the string into the local buffer
			bdStrlcpy(m_text, str, BD_MAX_SCREEN_TEXT);
		}
	}
	else
	{
		// Set to the null string
		m_text[0] = '\0';
	}
}

void bdGfxBase::getText(bdNChar8 *const buffer, bdUInt bufferSize)
{
	if (buffer && (bufferSize > 0))
	{
		bdStrlcpy(buffer, m_text, bufferSize);
	}
}

void bdGfxBase::setTextPosition(bdFloat32 x, bdFloat32 y)
{
	m_textPositionX = x;
	m_textPositionY = y;
}

void bdGfxBase::getTextPosition(bdFloat32 &x, bdFloat32 &y)
{
	x = m_textPositionX;
	y = m_textPositionY;
}

void bdGfxBase::setTextColor(bdColor color)
{
	m_textColor = color;
}

bdColor bdGfxBase::getTextColor()
{
	return m_textColor;
}

bdColor bdGfxBase::getBackgroundColor() const
{
	return m_backgroundColor;
}

void bdGfxBase::setBackgroundColor(const bdColor &color)
{
	m_backgroundColor = color;
}

bdBool bdGfxBase::addMesh(bdMesh *mesh)
{
	bdUInt index;
	bdBool result;

	// Do not add the same mesh twice
	if (!m_meshes.findFirst(mesh, index))
	{
		// Mesh wasn't in list, so add it
		m_meshes.pushBack(mesh);
		result = true;
	}
	else
	{
		// Mesh was already in list. Do not add it.
		result = false;
	}

	return result;
}

bdBool bdGfxBase::removeMesh(bdMesh *mesh)
{
	bdBool result = false;
	bdUInt index = 0;

	// Search the array for mesh
	if (m_meshes.findFirst(mesh, index))
	{
		// Found it, so remove it
		m_meshes.removeAt(index);
		result = true;
	}
	else
	{
		// Couldn't find the mesh in the array
		result = false;
	}

	return result;
}
