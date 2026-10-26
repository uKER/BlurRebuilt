#ifndef AFTK_XREF_SCENE_H
#define AFTK_XREF_SCENE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_XRefScene
	{
	public:
		C_XRefScene(C_Scene* pRootScene);

		C_XRefScene(const C_XRefScene& rhs);

		~C_XRefScene();

		const C_XRefScene& operator=(const C_XRefScene& rhs);

		C_Status SetSourceScene(const std::string& fileName);

		C_Status SetSourceScene(const std::string& fileName, U32 crc);

		C_Scene* GetScenePtr();
		
		const C_Scene* GetScenePtr() const;

		U32 GetCRC() const;

		const std::string& GetFileName() const;

		bool IsLoaded() const;

		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void OneTimeInit(C_Scene* pRootScene);

		C_Status Load();

		void Unload();

		C_Scene*	m_pRootScene;
		C_Scene*	m_pScene;
		std::string m_fileName;
		//The CRC value that identifies the scene.
		U32			m_crc;

		friend class C_Scene;
		friend class C_NativeXRefScene;
	#endif
	};

	typedef Ut::UtVector< Ut::DynamicVector<C_XRefScene*> > XRefScenePtrArray;
}

#endif
