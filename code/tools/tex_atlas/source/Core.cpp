#include "stdafx.h"
#include <tex_atlas/core.h>
#include <tools_common/FileUtilities.h>

namespace TexAtlas
{

Core* g_pCore = 0;

Core::Core()
{
	m_isCreated = false;
	m_pPrintMessageCallbackFunc = 0;
	m_pPrintWarningCallbackFunc = 0;
	m_pPrintErrorCallbackFunc = 0;
	m_pProgressCallback = 0;
}

Core::~Core()
{
}

Tu::Status Core::Create()
{
	if (g_pCore)
		return Tu::Status::FAILURE;
	
	g_pCore = this;

	std::stringstream ss;
	ss << "-==| Crop-Sheet Generator (CSGen) " << CS_GEN_VERSION << " Created |==-";
	DoMessage(ss.str());

	return Tu::Status::SUCCESS;
}

void Core::Destroy()
{
	std::stringstream ss;
	ss << "-==| Crop-Sheet Generator (CSGen) " << CS_GEN_VERSION << " Shutting down |==-";
	DoMessage(ss.str());

	m_isCreated = false;
	g_pCore = 0;
}

bool Core::IsCreated() const
{
	return m_isCreated;
}

Tu::Status Core::BuildGridCropSheet(
	AftkScene* pAftkScene,
	const std::string& dbFileName,
	const std::string& csFileName,
	U32 csSize,
	U32 borderSize)
{
	GridCropSheet cropSheet;
	return cropSheet.Build(
		pAftkScene,
		dbFileName,
		csFileName,
		csSize,
		borderSize,
		m_pProgressCallback);
}

Tu::Status Core::BuildFixedSizeCropSheet(
	AftkScene* pAftkScene,
	const std::string& dbFileName,
	const std::string& csFileName,
	U32 csSize,
	U32 subImageSize,
	U32 borderSize)
{
	FixedSizeCropSheet cropSheet;
	return cropSheet.Build(
		pAftkScene,
		dbFileName,
		csFileName,
		csSize,
		subImageSize,
		borderSize,
		m_pProgressCallback);
}

Tu::Status Core::BuildScaleCropSheet(
	AftkScene* pAftkScene,
	const std::string& dbFileName,
	const std::string& csFileName,
	U32 csSize,
	F32 scale,
	U32 borderSize,
	bool allowRotation)
{
	ScaleCropSheet cropSheet;
	return cropSheet.Build(
		pAftkScene,
		dbFileName,
		csFileName,
		csSize,
		scale,
		borderSize,
		allowRotation,
		m_pProgressCallback);
}

Tu::Status Core::BuildAutoScaleCropSheet(
	AftkScene* pAftkScene,
	const std::string& dbFileName,
	const std::string& csFileName,
	U32 csSize,
	U32 borderSize,
	bool allowRotation)
{
	AutoScaleCropSheet cropSheet;
	return cropSheet.Build(
		pAftkScene,
		dbFileName,
		csFileName,
		csSize,
		borderSize,
		allowRotation,
		m_pProgressCallback);
}

void Core::DoError(const std::string& text)
{
	if (m_pPrintErrorCallbackFunc)
	{
		m_pPrintErrorCallbackFunc(text);
	}
}

void Core::DoWarning(const std::string& text)
{
	if (m_pPrintWarningCallbackFunc)
	{
		m_pPrintWarningCallbackFunc(text);
	}
}

void Core::DoMessage(const std::string& text)
{
	if (m_pPrintMessageCallbackFunc)
	{
		m_pPrintMessageCallbackFunc(text);
	}
}

Progress* Core::ProgressCallback()
{
	return m_pProgressCallback;
}

void Core::SetProgressCallback(Progress* pProgress)
{
	m_pProgressCallback = pProgress;
}

void Core::SetPrintMessageCallback(void (*pPrintCallbackFunc)(const std::string& text))
{
	m_pPrintMessageCallbackFunc = pPrintCallbackFunc;
}

void Core::SetPrintWarningCallback(void (*pPrintCallbackFunc)(const std::string& text))
{
	m_pPrintWarningCallbackFunc = pPrintCallbackFunc;
}

void Core::SetPrintErrorCallback(void (*pPrintCallbackFunc)(const std::string& text))
{
	m_pPrintErrorCallbackFunc = pPrintCallbackFunc;
}


}