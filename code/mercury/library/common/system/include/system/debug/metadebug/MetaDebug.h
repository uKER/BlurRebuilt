#ifndef __SYSTEM_DEBUG_METADEBUG_H__
#define __SYSTEM_DEBUG_METADEBUG_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/Vector.h>

namespace Sys
{
	class I_File;
}

// generic storage for debug metadata
//		used to transfer debug data from tools to game
//		stored as attributes(S32, F32, string, etc), debuglog and debugrender types

// requirements
// size of data ingame comapact + number of allocations kept to a minimum (we could possibly use pooled allocator stored on root node object)
// allow filtering of stored node data during load (use user defined reader class to filter)

namespace MetaDebug
{
	class C_MetaNode;
	class C_AttributeList;
	class C_Log;
	class C_DebugRender;

	extern Debug::Logger &g_Log;

	enum E_Type
	{
		E_Type_Node = 0,		
		E_Type_AttributeList,
		E_Type_Log,
		E_Type_DebugRender,

		E_Type_Count
	};

	// I_MetaDebugNotify
	class I_MetaDebugNotify
	{
		public:
			// callback to do something with debug data. returns whether node should be stored or not
			virtual bool OnLoad(C_MetaNode* pNode) = 0;
	};

	// C_MetaNode
	class C_MetaNode
	{
		friend class C_MetaDebug;

		public:
			// craete a new root type meta node
			C_MetaNode( const Util::C_String& name );
			virtual ~C_MetaNode();
			
			S32 GetChildCount() const;
			C_MetaNode* GetParent() const;
			C_MetaNode* GetNextSibling() const;
			C_MetaNode* GetFirstChild() const;

			C_MetaNode* GetChildByIndex(S32 index) const;
			C_MetaNode* GetChildByName(const Util::C_String& name) const;

			const Util::C_String& GetName() const;
			Util::C_String GetFullName() const;
			virtual E_Type GetType() const;
			DebugLevel::Level GetDebugLevel() const;

			C_MetaNode* AddNode( const Util::C_String& name, DebugLevel::Level debugLevel = DebugLevel::Info );
			C_AttributeList* AddAttributeList( const Util::C_String& name, DebugLevel::Level debugLevel = DebugLevel::Info );
			C_Log* AddLog( const Util::C_String& name, DebugLevel::Level debugLevel = DebugLevel::Info );
			C_DebugRender* AddDebugRender( const Util::C_String& name, DebugLevel::Level debugLevel = DebugLevel::Info );

			void Dump() const;
			void Read( Sys::I_File* pFile, I_MetaDebugNotify* pDebugNotify = NULL );
			void Write( Sys::I_File* pFile ) const;

		protected:
			C_MetaNode();
			C_MetaNode( const Util::C_String& name, DebugLevel::Level debugLevel );
			void AddChild( C_MetaNode* pChild );
			void ReadNode( Sys::I_File* pFile, I_MetaDebugNotify* pDebugNotify = NULL );
			void WriteNode( Sys::I_File* pFile ) const;
			virtual void ReadData(  Sys::I_File* pFile );
			virtual void WriteData(  Sys::I_File* pFile ) const;

			void Dump(const Util::C_String& prefix ) const;
			virtual void DumpData(const Util::C_String& prefix ) const;

		private:
			C_MetaNode* m_pParent;
			C_MetaNode* m_pNextSibling;
			C_MetaNode* m_pFirstChild;
			Util::C_String m_Name;
			U8 m_DebugLevel;
	};

} // namespace MetaDebug

void MetaDebugTest();


#endif
