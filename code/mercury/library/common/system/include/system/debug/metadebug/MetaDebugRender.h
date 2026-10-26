#ifndef __SYSTEM_DEBUG_METADEBUGRENDER_H__
#define __SYSTEM_DEBUG_METADEBUGRENDER_H__


#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Utility/string.h>
#include <btl/Containers/Vector.h>
#include <system/debug/debugrender/DebugRenderItem.h>
#include <system/Debug/metadebug/MetaDebug.h>

namespace MetaDebug
{
	// C_MetaDebugRender
	class C_DebugRender : public C_MetaNode, public DbUtil::C_ItemDebugRenderer
	{
		public:
			C_DebugRender();
			C_DebugRender( const Util::C_String& name, const DebugLevel::Level debugLevel );
			~C_DebugRender();

			E_Type GetType() const OVERRIDE;

			void DebugRender( DbUtil::I_DebugRender* pDebugRender, Maths::HMat43 worldMatrix );

			// callback from C_ItemDebugRender
			void OnRender( DbUtil::C_DebugRenderItem* pDebugRenderItem ) OVERRIDE;

			void ReadData(  Sys::I_File* pFile ) OVERRIDE;
			void WriteData(  Sys::I_File* pFile ) const OVERRIDE;
			void DumpData(const Util::C_String& prefix ) const OVERRIDE;

		private:
			Ut::Vector<DbUtil::C_DebugRenderItem> m_RenderItemList;
	};

} // namespace MetaDebug


#endif
