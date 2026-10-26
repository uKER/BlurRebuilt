#ifndef DISPLAY_I_RESOURCE_FACTORY_H_
#define DISPLAY_I_RESOURCE_FACTORY_H_

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

		class I_ResourceFactory
		{
		public:
			virtual ~I_ResourceFactory() {}

			virtual U8* AllocateVideoMemory(size_t bytes) = 0;

			virtual U8* AllocateSystemMemory(size_t bytes) = 0;

			virtual void FreeSystemMemory(U8* p) = 0;

			virtual void OnCreateHeader(
				const C_BundleHeader& bundleHeader,
				const U8* pResourceHeaders,
				U32 numTexture2D,
				U32 numCubeMap,
				U32 numTextureArray2D,
				U32 numTexture3D) = 0;

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
