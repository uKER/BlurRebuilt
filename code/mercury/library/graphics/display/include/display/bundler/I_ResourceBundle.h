#ifndef DISPLAY_I_RESOURCE_BUNDLE_H_
#define DISPLAY_I_RESOURCE_BUNDLE_H_

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	namespace ResourceBundler
	{
		struct C_BundleHeader;
		struct C_ResourceTexture2D;
		struct C_ResourceTextureCubeMap;
		struct C_ResourceTextureArray2D;
		struct C_ResourceTexture3D;
		class C_ResourceBundleImpl1;
		class C_ResourceBundleImpl2;

		class I_ResourceBundle : public I_ResourceFactory
		{
		public:
			virtual ~I_ResourceBundle() {}

			virtual bool Create(const Sys::C_FilePath& fileName) = 0;
			virtual void Destroy() = 0;

			virtual const S32 GetTexture2DCount() { return 0; }
			virtual C_RefTexture* GetTexture2D(const S32 textureIndex) { return NULL; }
			virtual C_RefTexture* GetTexture2D(const Util::C_String& textureName) { return NULL; }

			virtual const U32 ResourceCount() const  { return NULL; }
			virtual const char* GetResourceName(const S32 index) { return NULL; }

		//[HACK] Remove this, we ought to be using a cross-platform texture abstraction.

			virtual U32 CalculateVideoMemorySize() = 0;

		protected:
			virtual bool RequiresResourceCount() = 0;

			virtual void OnCreateTexture2D(
				const C_BundleHeader& bundleHeader,
				const C_ResourceTexture2D& resourceHeader) = 0;

			virtual void OnCreateTextureCubeMap(
				const C_BundleHeader& bundleHeader,
				const C_ResourceTextureCubeMap& resourceHeader) = 0;

			virtual void OnCreateTextureArray2D(
				const C_BundleHeader& bundleHeader,
				const C_ResourceTextureArray2D& resourceHeader) = 0;

			virtual void OnCreateTexture3D(
				const C_BundleHeader& bundleHeader,
				const C_ResourceTexture3D& resourceHeader) = 0;
		};
	}
}; // namespace Display

#endif
