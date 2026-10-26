#include "stdafx.h"
#include "game/GameDef/GameData.h"
#include "GameApp.h"
#include "game/game.h"
#include "Game\GameFlow\GameflowMessages.h"

namespace Game
{

struct GameDefTest
{
	F32 m_MyFloat;
	S32 m_MyInt;
	XtSz8 m_MyString;
};

Debug::Bool g_debugCheckForSorceryUpdatesOnDisk("Game.CheckForSorceryDataExportsWhileRunning", false, Debug::Persist());

C_GameData *C_GameData::m_pGameData = NULL;

void XtPackOpenCallback(XtPackOpenStatus* status, void* context)
{
	if( status == NULL )
		return;

	LOGWARN( g_Log, "XtOpen : %s", status->message );
}

XtPack *C_GameData::GetMainPack()
{
	return m_pXtMainPack;
}

C_GameData::C_GameData()
{
	assert(m_pGameData == NULL);
	m_pGameData = this;
}

C_GameData::~C_GameData()
{
}

C_GameData* C_GameData::GetInstance()
{
	return m_pGameData;
}

void C_GameData::Init(XtDb* db, const Util::C_String& filename)
{
	m_Filename = filename;
	m_pXtMainPack = NULL;
	m_pXtDb = db;
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open( filename.Chars() );

	if ( pFile )
	{
		U32 length = (U32)pFile->Length();
		U8* pData = new U8[ length ];

		if ( pData )
		{
			if ( pFile->Read( pData, length ) )
			{
				XtSchema* pXtSchema = XtGetApplicationSchema();

				if ( pXtSchema )
				{
					XtPackOpenParams xtPackOpenParams;

					xtPackOpenParams.callback = XtPackOpenCallback;
					xtPackOpenParams.context = 0;

					m_pXtMainPack = XtPackOpen( db, pData, length, &xtPackOpenParams );
				}				
			}

			delete [] pData;
		}

		delete pFile;
	}
}

void C_GameData::Close()
{
	XtPackClose(m_pXtDb, m_pXtMainPack);
}

void C_GameData::Update()
{
	// not needed
	//static int checkXt = 0;

	//checkXt = (checkXt + 1) % 10;

	//if (!checkXt && g_debugCheckForSorceryUpdatesOnDisk)
	//{
	//	Sys::C_Fileinfo info;
	//	
	//	if (Sys::GetDefaultFileSystem()->FileInfo(m_Filename.Chars(),&info))
	//	{
	//		static Time::C_Time last;

	//		if ((U64&)last != (U64&)info.m_FileTime)
	//		{
	//			last = info.m_FileTime;

	//			Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open( m_Filename.Chars() );

	//			if ( pFile )
	//			{
	//				U32 length = (U32)pFile->Length();
	//				U8* pData = new U8[ length ];

	//				if ( pData )
	//				{
	//					if ( pFile->Read( pData, length ) )
	//					{
	//						XtSchema* pXtSchema = XtGetApplicationSchema();

	//						if ( pXtSchema )
	//						{
	//							XtPackOpenParams xtPackOpenParams;

	//							xtPackOpenParams.callback = XtPackOpenCallback;
	//							xtPackOpenParams.context = 0;

	//							m_pXtMainPack = XtPackOpen( C_GameApp::GetInstance()->GetXtDatabase(), pData, length, &xtPackOpenParams );

	//							//C_LocData::Instance().Delete();
	//							//C_LocData::Instance().Init();

	//							Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &Messaging::BuildSimpleMessage( Game::E_GameflowMessage_SorceryReloaded ) );
	//						}				
	//					}

	//					delete [] pData;
	//				}

	//				delete pFile;
	//			}
	//		}
	//	}
	//}
}


} // namespace Game


// GameDef Xt Lib

XT_TYPE(Game::GameDefTest,"GameDef_Test")
	XT_FIELD(m_MyFloat, "myf32")
	XT_FIELD(m_MyInt, "myi32")
	XT_FIELD(m_MyString, "mysz8")
XT_END_TYPE		

XT_LIBRARY(GameDefLib)
	XT_REGISTER_TYPE(Game::GameDefTest)
XT_END_LIBRARY




// Main Xt Application 

XT_APPLICATION
	XT_DEPENDS_ON(GameActors)

	XT_DEPENDS_ON(EnvironmentSettings)

	XT_DEPENDS_ON(RacingPhysics_Surfaces)
	XT_DEPENDS_ON(RacingPhysics_DynamicsParameters)
	XT_DEPENDS_ON(RacingPhysics_MotorbikeDynamics)
	XT_DEPENDS_ON(RacingPhysics_CarDynamics)

	// dynamic effect
	XT_DEPENDS_ON(GraphicsDynamic)
	XT_DEPENDS_ON(GraphicsDynamicLight)
	XT_DEPENDS_ON(GraphicsDynamicParticle)
	XT_DEPENDS_ON(GraphicsDynamicTracer)

	XT_DEPENDS_ON(GameDefLib)

	XT_DEPENDS_ON(ResourceUtils)

	XT_DEPENDS_ON(SpecialFX)

	XT_DEPENDS_ON(MGCam)

	XT_DEPENDS_ON(CrowdXtLib)
XT_END_APPLICATION
