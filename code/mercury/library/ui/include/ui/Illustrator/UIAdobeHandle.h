
#ifndef UI_ADOBEHANDLE_H
#define UI_ADOBEHANDLE_H

#include "ui/Illustrator/UIAdobeScene.h"

namespace UI
{

// Adobe handle
class C_AdobeHandle
{
public:
	C_AdobeHandle( C_AdobeScene* pScene, const char* pFileName )
	:	m_pSceneHandle( pScene ), 
		m_Dirty( false )
	{
		strcpy_s( m_Filepath, m_MaxSceneNameLen, pFileName );
	}

	const C_AdobeScene* 	ScenePtr() const
							{
								return m_pSceneHandle;
							}

	const bool				IsDirty() const
							{
								return m_Dirty;
							}

	const char*				Filepath() const
							{
								return m_Filepath;
							}

	void					Destroy()
							{
								SAFE_DELETE_PTR( m_pSceneHandle );
							}

private:
	static const U32		m_MaxSceneNameLen = 128;			///< @a
	char					m_Filepath[m_MaxSceneNameLen];			///< Name of this scene.

	C_AdobeScene*			m_pSceneHandle;
	bool					m_Dirty;

	// Make friend so it can remap the scene handle pointer
	friend class C_AdobeSceneManager;
};


}; // namespace

#endif // UI_ADOBEHANDLE_H
