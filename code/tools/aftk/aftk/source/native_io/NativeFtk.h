#ifndef NATIVE_FTK_H
#define NATIVE_FTK_H

#ifdef _MSC_VER_
#pragma once
#endif

extern Ut::SSSMultiMemoryPool* g_pNativeFtkMemoryPool;
#include "NativeAllocators.h"
#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeLegacyShaderConnection.h"

namespace Aftk
{
	class C_NativeFtk : public C_Ftk
	{
	public:
		C_NativeFtk();

		~C_NativeFtk();
		
		void Clear();

		static U32 GetLowestSupportedFileVersion();

		static U32 GetHighestSupportedFileVersion();

		static C_Status GetFileVersion(
			const std::string& fileName, 
			U32& version);
		
		C_Status Read(
			const std::string& fileName, 
			C_Scene *pScene,
			const char* pOptions = 0,
			I_Progress* pProgressCallback = 0);

		C_Status Write(
			const std::string& fileName, 
			U32 version, 
			const C_Scene *pScene,
			const char* pOptions = 0,
			I_Progress* pProgressCallback = 0);

		C_Scene*			GetScenePtr();
		const C_Scene*		GetScenePtr() const;
		///
		///Build a mapping table that converts integer into pointers.
		bool				AddToIdPtrMapping(S32 id, void* pObject);

		///
		void*				GetPtrFromId(S32 id);
		///
		S32					GetIdFromPtr(const void* p);

		void				RebuildSceneNodeArray();

		const Ut::UtVector< Ut::DynamicVector<S32> >& GetObjectsToResolve();

		U32					m_nextSceneNodeUniqueId;
	private:
		C_Scene*			m_pScene;
		const C_Scene*		m_pSceneConst;

		C_NativeDiskWriter	m_diskWriter;
		C_NativeDiskReader	m_diskReader;

		Ut::UtMap<const void*, S32>* m_pPtrSerialisationMapping;
		Ut::UtVector<Ut::DynamicVector<void*> >* m_pPtrDeserialisationMapping;

		Ut::UtVector< Ut::DynamicVector<S32> >	m_objectsToResolve;

		Ut::UtVector< Ut::DynamicVector<const C_BlindDataResource*> > m_blindDataMappingArray;

		Ut::UtMap<const C_BlindDataResource*, U32>* 
			m_pBlindDataResToIthIndexMapping;


		friend class C_NativeScene;
		friend class C_NativeSceneNode;
		friend class C_NativeSceneMeshRemappableNode;
		friend class C_NativePolygon;
	};
}
#endif
