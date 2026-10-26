#ifndef LOGICALFS_OVERLAY_LAYER_H
#define LOGICALFS_OVERLAY_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Utility/String.h>

namespace LogicalFs
{
	class C_OverlayLayer
	{
	public:
		enum E_Type
		{
			E_Type_Unknown	= 0,
			E_Type_Overlay	= 1,	//This layer accessed first, then any layers below upon failure.
			E_Type_Opaque	= 2,	//This layer completely replaces layers below.
			E_Type_Newer	= 3		//This layer replaces those below when file is newer.
		};
		
		struct C_Params
		{
			C_Params()
			{
				flags.data = 0;
			}

			union C_Flags
			{
				U32 data;
				struct
				{
					U32 layerType : 4;
					U32 writable : 1;
				};
			};

			C_Flags flags;
		};

		C_OverlayLayer();

		C_OverlayLayer(
			const Sys::C_FilePath& overlayPath,		//Where to mount overlay.
			Sys::I_FileSystem* pFs,					//File system for overlay.
			const Sys::C_FilePath& fsPath,			//Path within above file system to use for our overlay.
			C_Params params,						//Overlay properties.
			const Btl::String& name);				//Optional name for overlay.
		C_OverlayLayer(const C_OverlayLayer& rhs);
		~C_OverlayLayer();

		const C_OverlayLayer& operator=(const C_OverlayLayer& rhs);

		const Sys::C_FilePath& GetOverlayPath() const;

		Sys::C_FilePath& GetOverlayPath();

		Sys::I_FileSystem* GetFs();

		const Sys::I_FileSystem* GetFs() const;

		const Sys::C_FilePath& GetFsPath() const;

		Sys::C_FilePath& GetFsPath();

		const Btl::String& GetName() const;

		C_Params& GetParams();

		const C_Params& GetParams() const;

	private:
		Sys::C_FilePath		m_overlayPath;
		Sys::I_FileSystem*	m_pFs;
		Sys::C_FilePath		m_fsPath;
		Btl::String			m_name;
		C_Params	 		m_params;
	};
}

#endif
