#ifndef DISPLAY_RESOURCE_BUNDLE_SINGLE_H_
#define DISPLAY_RESOURCE_BUNDLE_SINGLE_H_

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		class C_ResourceBundleSingle : public I_ResourceBundle
		{
		public:
			
			C_ResourceBundleSingle();

			virtual ~C_ResourceBundleSingle();

			virtual bool Create(const Sys::C_FilePath& fileName);
			virtual bool Create(Sys::I_File& file);

			virtual void Destroy();

			virtual const S32 GetTexture2DCount();
			virtual C_RefTexture* GetTexture2D(const S32 textureIndex);
			virtual C_RefTexture* GetTexture2D(const Util::C_String& textureName);

			virtual const U32 ResourceCount() const;
			virtual const char* GetResourceName(const S32 index);

			U32 CalculateVideoMemorySize();

		private:
			virtual U8* AllocateVideoMemory(size_t bytes);

			virtual void FreeVideoMemory();

			virtual U8* AllocateSystemMemory(size_t bytes);

			virtual void FreeSystemMemory(U8* p);

			virtual void OnCreateHeader(
				const C_BundleHeader& bundleHeader,
				const U8* pResourceHeaders,
				U32 numTexture2D,
				U32 numTextureCubeMap,
				U32 numTextureArray2D,
				U32 numTexture3D);

			virtual bool RequiresResourceCount();

			virtual void OnCreateTexture2D(
				const C_BundleHeader& bundleHeader,
				const C_ResourceTexture2D& resourceHeader);

			virtual void OnCreateTextureCubeMap(
				const C_BundleHeader& bundleHeader,
				const C_ResourceTextureCubeMap& resourceHeader);

			virtual void OnCreateTextureArray2D(
				const C_BundleHeader& bundleHeader,
				const C_ResourceTextureArray2D& resourceHeader);

			virtual void OnCreateTexture3D(
				const C_BundleHeader& bundleHeader,
				const C_ResourceTexture3D& resourceHeader);

			E_Format GetFormat(const U32 format);

			U32								m_Flags;
			S32								m_SystemID;
			U32								m_SysMemorySize;
			U8*								m_pSysMemory;
			U32								m_VidMemorySize;
			U8*								m_pVidMemory;
			Sys::C_FilePath				m_FileName;
			Ut::Vector<C_RefTexture>	m_Texture2D;
		};
	}
}; // namespace Display

#endif
