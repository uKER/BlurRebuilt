#include "LogicalFsPch.h"
#include <OverlayFs/OverlayLayer.h>

namespace LogicalFs
{

C_OverlayLayer::C_OverlayLayer()
{
}

C_OverlayLayer::C_OverlayLayer(
	const Sys::C_FilePath& overlayPath,
	Sys::I_FileSystem* pFs,
	const Sys::C_FilePath& fsPath,
	C_Params params,
	const Btl::String& name)
: m_pFs(pFs), m_name(name), m_params(params)
{
	m_overlayPath = overlayPath;
	m_fsPath = fsPath;
	if (m_overlayPath.Length() > 0)
		m_overlayPath = Sys::RemoveTrailingSlash(m_overlayPath);
	if (m_fsPath.Length() > 0)
		m_fsPath = Sys::RemoveTrailingSlash(m_fsPath);
}

C_OverlayLayer::C_OverlayLayer(const C_OverlayLayer& rhs)
{
	*this = rhs;
}

C_OverlayLayer::~C_OverlayLayer()
{
}

const C_OverlayLayer& C_OverlayLayer::operator=(const C_OverlayLayer& rhs)
{
	m_overlayPath = rhs.m_overlayPath;
	m_pFs = rhs.m_pFs;
	m_fsPath = rhs.m_fsPath;
	m_name = rhs.m_name;
	m_params = rhs.m_params;
	return *this;
}

Sys::C_FilePath& C_OverlayLayer::GetOverlayPath() 
{
	return m_overlayPath;
}

const Sys::C_FilePath& C_OverlayLayer::GetOverlayPath() const
{
	return m_overlayPath;
}


Sys::I_FileSystem* C_OverlayLayer::GetFs()
{
	return m_pFs;
}

const Sys::I_FileSystem* C_OverlayLayer::GetFs() const
{
	return m_pFs;
}

Sys::C_FilePath& C_OverlayLayer::GetFsPath()
{
	return m_fsPath;
}

const Sys::C_FilePath& C_OverlayLayer::GetFsPath() const
{
	return m_fsPath;
}

C_OverlayLayer::C_Params& C_OverlayLayer::GetParams()
{
	return m_params;
}

const C_OverlayLayer::C_Params& C_OverlayLayer::GetParams() const
{
	return m_params;
}

const Btl::String& C_OverlayLayer::GetName() const
{
	return m_name;
}

}

