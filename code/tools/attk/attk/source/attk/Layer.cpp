#include "stdafx.h"

#include <attk/Definitions.h>
#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>

using namespace Attk;

C_Layer::C_Layer(C_BaseImage* pImage, U32 width, U32 height, U32 depth)
{
	m_pImage = pImage;
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_opacity = 1.f;
	m_enabled = true;
	m_blendMode = E_BlendMode_Multiply;
}

C_Layer::C_Layer(const C_Layer& rhs)
{
	*this = rhs;
}

C_Layer::~C_Layer()
{
}

const C_Layer& C_Layer::operator=(const C_Layer& rhs)
{
	m_name = rhs.m_name;
	m_enabled = rhs.m_enabled;
	m_opacity = rhs.m_opacity;
	m_width = rhs.m_width;
	m_height = rhs.m_height;
	m_depth = rhs.m_depth;
	m_blendMode = rhs.m_blendMode;

	return *this;
}

void C_Layer::SetName(const std::string& name)
{
	m_name = name;
}

const std::string& C_Layer::GetName() const
{
	return m_name;
}

void C_Layer::SetEnabled(bool enabled)
{
	m_enabled = enabled;
}

bool C_Layer::IsEnabled() const
{
	return m_enabled;
}

void C_Layer::SetOpacity(F32 opacity)
{
	m_opacity = opacity;
}

void C_Layer::SetBlendMode(E_BlendMode mode)
{
	m_blendMode = mode;
}

F32 C_Layer::GetOpacity() const
{
	return m_opacity;
}

U32 C_Layer::GetWidth() const
{
	return m_width;
}

U32 C_Layer::GetHeight() const
{
	return m_height;
}

U32 C_Layer::GetDepth() const
{
	return m_depth;
}

U32 C_Layer::GetWidth(U32 mipLevel) const
{
	if (mipLevel == 0)
		return m_width;

	return m_width / 4*mipLevel;
}

U32 C_Layer::GetHeight(U32 mipLevel) const
{
	if (mipLevel == 0)
		return m_height;

	return m_height / 4*mipLevel;
}

U32 C_Layer::GetDepth(U32 mipLevel) const
{
	if (mipLevel == 0)
		return m_depth;

	return m_depth / 4*mipLevel;
}

E_BlendMode C_Layer::GetBlendMode() const
{
	return m_blendMode;
}
