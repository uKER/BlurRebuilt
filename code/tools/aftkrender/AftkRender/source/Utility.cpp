#include "AftkRenderPch.h"
#include "AftkRender.h"

#include "system/misc/StringUtil.h"
#include "system/serialise/C_FileSystem.h"
#include "attk/attk.h"

using namespace AftkRender;

bool Utility::CaptureRenderTarget(Attk::C_Image& image)
{
	const Display::C_RenderTarget& renderTarget = Display::Device()->GetRenderTarget();
	const Display::C_Surface *pSurface = renderTarget.GetColourSurface();
/*
	D3DXIMAGE_FILEFORMAT imageFormat;
	{
		Util::C_String extension = Util::ExtractFileExtension(fileName);
		extension.ConvertToLower();

		if (extension == "bmp")
			imageFormat = D3DXIFF_BMP;
		else if (extension == "jpg" || extension == "jpeg")
			imageFormat = D3DXIFF_JPG;
		else if (extension == "tga")
			imageFormat = D3DXIFF_TGA;
		else if (extension == "png")
			imageFormat = D3DXIFF_PNG;
		else if (extension == "dds")
			imageFormat = D3DXIFF_DDS;
		else if (extension == "ppm")
			imageFormat = D3DXIFF_PPM;
		else if (extension == "dib")
			imageFormat = D3DXIFF_DIB;
		else if (extension == "hdr")
			imageFormat = D3DXIFF_HDR;
		else if (extension == "pfm")
			imageFormat = D3DXIFF_PFM;
		else
			imageFormat = D3DXIFF_PNG;
	}

	if (!Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(fileName)))
		LOGERROR(g_Log, "Failed to create output directory for %s", fileName.Chars());

	HRESULT hr = D3DXSaveSurfaceToFile(
		fileName.Chars(),
		imageFormat,
		pSurface->m_pd3dSurface,
		NULL,
		NULL);

	if (hr != S_OK)
	{
		LOGERROR(g_Log, "Failed to write image %s\n", fileName.Chars());
		return false;
	}
	return true;
*/

	// Create a system memory surface & copy render target
	IDirect3DSurface9 *pd3dOffscreenSurface;
	{
		D3DSURFACE_DESC desc;
		HRESULT hr = pSurface->m_pd3dSurface->GetDesc(&desc);
		if (hr != S_OK)
		{
			LOGERROR(g_Log, "Failed to get surface description\n");
			return false;
		}
		
		//hr = Display::Device()->m_pd3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pd3dOffscreenSurface, NULL);
		hr = Display::Device()->m_pd3dDevice->CreateRenderTarget(desc.Width, desc.Height, desc.Format, D3DMULTISAMPLE_NONE, 0, TRUE, &pd3dOffscreenSurface, NULL);
		if (hr != S_OK)
		{
			LOGERROR(g_Log, "Failed to create an offscreen surface\n");
			return false;
		}

		//hr = Display::Device()->m_pd3dDevice->GetRenderTargetData(pSurface->m_pd3dSurface, pd3dOffscreenSurface);
		hr = Display::Device()->m_pd3dDevice->StretchRect(pSurface->m_pd3dSurface, NULL, pd3dOffscreenSurface, NULL, D3DTEXF_NONE);
		if (hr != S_OK)
		{
			LOGERROR(g_Log, "Failed to copy rendertarget\n");
			return false;
		}
	}

	D3DLOCKED_RECT lockedSurface;
	HRESULT hr = pd3dOffscreenSurface->LockRect(&lockedSurface, NULL, D3DLOCK_READONLY);
	if (hr != S_OK)
	{
		LOGERROR(g_Log, "Failed to lock d3d surface\n");
		pd3dOffscreenSurface->Release();
		return false;
	}

	Attk::E_PixelFormat pixelFormat;
	switch(pSurface->GetFormat())
	{
	case Display::E_Format_A8R8G8B8:
	case Display::E_Format_X8R8G8B8:
		pixelFormat = Attk::E_PixelFormat_B8G8R8A8U;
		break;
	default:
	{
		LOGERROR(g_Log, "Unsupported surface format\n");
		pd3dOffscreenSurface->Release();
		return false;
	}
	}

	Attk::C_Status status = image.NewPixelData(pSurface->GetWidth(), pSurface->GetHeight(), 1, pixelFormat, (U8*)lockedSurface.pBits);
	if (!status)
	{
		LOGERROR(g_Log, "Failed to create image\n");
		pd3dOffscreenSurface->Release();
		return false;
	}

	pd3dOffscreenSurface->UnlockRect();
	pd3dOffscreenSurface->Release();

	return (status) ? true : false;

}
bool Utility::CaptureRenderTarget(const Util::C_String& fileName, E_ImagePostProcess postProcess)
{
	Attk::C_Image image;
	if (!CaptureRenderTarget(image))
		return false;

	Attk::C_Status status;
	if (postProcess == E_ImagePostProcess_FloodFill)
	{
		Attk::C_Image floodedImage;
		status = AttkUt::C_FloodFill::FloodFill(floodedImage, image);
		if (!status)
		{
			LOGINFO(g_Log, "Failed to perform image flood operation\n");
			status = image.Write(fileName);
		}
		else
			status = floodedImage.Write(fileName);
	}
	else
		status = image.Write(fileName);

	return (status) ? true : false;
}

void Utility::FrameObjectPerspective(
	C_Camera& camera,
	const Maths::C_AxisBox& aabbox,
	const Maths::HVec3& viewVector)
{		
	//calculate view position using camera fov & aspect ratio and supplied view vector
	F32 size;
	{
		const Maths::HVec3 boxSize = aabbox.Max() - aabbox.Min();
		size = 0.5f*Maths::Max(boxSize[0], Maths::Max(boxSize[1], boxSize[2]));
	}
	if (size < AFTK_EPSILON)
		size = 0.5f;

	F32 distanceFromObj = Maths::Max(camera.GetNearClipDistance(), size/tan(Maths::DegToRad(camera.GetFieldOfView()*0.5f)));

	camera.SetPosition(aabbox.Centre() - ((distanceFromObj+size) * viewVector));
	camera.SetFocusPosition(aabbox.Centre());
}

void Utility::FrameObjectPerspective(
	C_Camera& camera,
	const Aftk::C_SceneNode* pSceneNode,
	const Maths::HVec3& viewVector)
{
	Aftk::C_Bounds bounds = pSceneNode->GetWorldBounds();
	for (Aftk::C_SceneNodeConstIterator it(pSceneNode->GetChildPtr()); it; ++it)
		bounds += (it)->GetWorldBounds();

	FrameObjectPerspective(camera, Maths::C_AxisBox(
		Maths::HVec3(bounds.GetMin()[0], bounds.GetMin()[1], bounds.GetMin()[2]),
		Maths::HVec3(bounds.GetMax()[0], bounds.GetMax()[1], bounds.GetMax()[2])),
		viewVector);
}

void Utility::FrameObjectOrthographic(
	C_Camera& camera,
	const Maths::C_AxisBox& aabbox,
	const Maths::HVec3& viewVector)
{
	//calculate view position using camera fov & aspect ratio and supplied view vector
	F32 size;
	{
		const Maths::HVec3 boxSize = aabbox.Max() - aabbox.Min();
		size = Maths::Max(boxSize[0], Maths::Max(boxSize[1], boxSize[2]));
	}
	if (size < AFTK_EPSILON)
		size = 0.5f;

	F32 distanceFromObj = Maths::Max(camera.GetNearClipDistance(), size/tan(Maths::DegToRad(camera.GetFieldOfView()*0.5f)));
	camera.SetPosition(aabbox.Centre() - ((distanceFromObj+size) * viewVector));
	camera.SetFocusPosition(aabbox.Centre());
	camera.SetHeightExtent(size);
}

void Utility::FrameObjectOrthographic(
	C_Camera& camera,
	const Aftk::C_SceneNode* pSceneNode,
	const Maths::HVec3& viewVector)
{

	Aftk::C_Bounds bounds = pSceneNode->GetWorldBounds();
	for (Aftk::C_SceneNodeConstIterator it(pSceneNode->GetChildPtr()); it; ++it)
		bounds += (it)->GetWorldBounds();

	FrameObjectOrthographic(camera, Maths::C_AxisBox(
		Maths::HVec3(bounds.GetMin()[0], bounds.GetMin()[1], bounds.GetMin()[2]),
		Maths::HVec3(bounds.GetMax()[0], bounds.GetMax()[1], bounds.GetMax()[2])),
		viewVector);
}


void Utility::GetVertexColourChannelAndType(
	const Aftk::C_SceneMeshNode* pAftkMesh,
	S32& channel,
	Aftk::E_GenericAttributeType& type)
{
	//Look for the first vertex data channel.
	channel = -1;
	type = Aftk::E_GenericAttributeType_Unknown;
	for (U32 iChannel = 0; iChannel < pAftkMesh->GetNumVertexDataChannels(); iChannel++)
	{
		const Aftk::C_VertexDataChannelInfo* pChannelInfo = 
			&pAftkMesh->GetVertexDataChannelInfo(iChannel);
		if (pChannelInfo->GetName() == "luxVertexColour" || pChannelInfo->GetName() == "vertexColour")
		{
			channel = iChannel;
			type = pChannelInfo->GetDataType();
			break;
		}
	}
}

U32 Utility::GetLightMapUVSet(
	const Aftk::C_SceneMeshNode *pAftkSceneMeshNode)
{
	U32 lightMapUVSet = -1;
	for (U32 i=0; i<pAftkSceneMeshNode->GetNumTextureCoordinateSets(); ++i)
	{
		if (pAftkSceneMeshNode->GetTextureCoordinateSetName(i) == "luxlightmap")
		{
			lightMapUVSet = i;
			break;
		}
	}
	return lightMapUVSet;
}

