#ifndef DISPLAY_GCMSHADER_H
#define DISPLAY_GCMSHADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"

namespace NewDisplay
{
namespace GCM
{

struct C_ShaderParameter
{
	U32				m_gcmOffset;
	CGprogram		m_cgProgram;
	CGparameter		m_cgParameter;
};

}; // namespace GCM
}; // namespace NewDisplay

#endif // DISPLAY_GCMSHADER_H
