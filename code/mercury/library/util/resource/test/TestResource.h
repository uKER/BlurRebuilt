#ifndef UTIL_RESOURCE_TEST_H
#define UTIL_RESOURCE_TEST_H

#include <core/core.h>
#include <cxxtest/TestSuite.h>


//////////////////////////////////////////
// TEST CODE

#include <resource/Resource.h>
#include <resource/Manager.h>

namespace TestResource
{

// C_Model
class C_Model;

typedef Resource::C_Handle<C_Model> C_ModelHandle;

class C_Model : public Resource::C_Resource<C_Model>
{
public:
	bool LoadResourceData();
	bool FreeResourceData() { return true; }
	static Util::C_String StaticGetResourceTypeName() { return "DummyModel"; }
	virtual S32 GetEvictDelay() { return 1; }

private:
	U32 m_Data;
};

bool C_Model::LoadResourceData()
{
	Sys::I_File* pFile = OpenResourceFile();

	if( !pFile )
		return false;

	bool success = true;

	{
		Sys::BinaryDeserialiser< Ut::NoEndianSwap > ar(*pFile);
		ar >> m_Data;
	}

	SAFE_DELETE_PTR( pFile );

	return success;
}

// C_Skeleton
class C_Skeleton;

typedef Resource::C_Handle<C_Skeleton> C_SkeletonHandle;

class C_Skeleton : public Resource::C_Resource<C_Skeleton>
{
public:
	bool LoadResourceData();
	bool FreeResourceData() { return true; }
	static Util::C_String StaticGetResourceTypeName() { return "DummySkeleton"; }

private:
	U32 m_Data;
};

bool C_Skeleton::LoadResourceData()
{
	Sys::I_File* pFile = OpenResourceFile();

	if( !pFile )
		return false;

	bool success = true;
	
	{
		Sys::BinaryDeserialiser< Ut::NoEndianSwap > ar(*pFile);
		ar >> m_Data;
	}

	SAFE_DELETE_PTR( pFile );

	return success;
}

class C_Listener : public Resource::C_Listener
{
public:
	// inherited from Resource::C_Listener
	void ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
	{
		LOGINFO( Resource::g_Log, "Notify pResource=0x%08x, [%s] \"%s\" %d, 0x%08x\n", pResource, pResource->GetResourceTypeName().Chars(), pResource->GetResourceFilename().Chars(), result, pData );
	}
};

void CreateTestFile( const Util::C_String& filename, const U32 data )
{
	const Util::C_String directory = Util::ExtractDirectory( filename );

	Sys::GetDefaultFileSystem()->CreateDirectory( directory );

	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open( filename, Sys::E_FileAccess_Write );

	if( !pFile )
	{
		LOGWARN( Resource::g_Log, "Failed to create test file '%s'\n", filename.Chars() );
		return;
	}

	bool success = true;
	
	{
		Sys::BinarySerialiser< Ut::NoEndianSwap > ar(*pFile);
		ar << data;
	}

	SAFE_DELETE_PTR( pFile );
}

} // namespace TestResource

using namespace TestResource;

class MyTest : public CxxTest::TestSuite
{
	
public:

	void setUp() 
	{	
		CreateTestFile( "./skeletons/Test.skl", 1 );
		CreateTestFile( "./models/Test.mdl", 1 );
		CreateTestFile( "./models/Leopard.mdl", 1 );
		CreateTestFile( "./models/Aardvark.mdl", 1 );
		CreateTestFile( "./models/Zebra.mdl", 1 );
		CreateTestFile( "./models/lionskeleton.skl", 1 );
		CreateTestFile( "./models/wormskeleton.skl", 1 );
		CreateTestFile( "./models/lion.mdl", 1 );
		CreateTestFile( "./skeletons/mothskeleton.skl", 1 );
		CreateTestFile( "./models/aardVark.mdl", 1 );
		CreateTestFile( "./models/humanskeleton.skl", 1 );
		CreateTestFile( "./models/mouseskeleton.skl", 1 );

		m_pManager = new Resource::C_Manager();
	}

	void tearDown() 
	{
		SAFE_DELETE_PTR( m_pManager );
	}

	void testSimple1()
	{		
		Resource::C_Manager::GetInstance()->Dump();

		{
			C_SkeletonHandle skelHandle;
			skelHandle.Load( ".\\skeletons\\Test.skl" );

			C_Skeleton* pSkeleton = skelHandle;

			Resource::WaitForResourceData( pSkeleton );

			TS_ASSERT( pSkeleton->GetResourceFilename() == "./skeletons/test.skl" );
			TS_ASSERT( pSkeleton->GetResourceTypeId() == C_Skeleton::StaticGetResourceTypeId() );
			TS_ASSERT( pSkeleton->GetResourceTypeName() == C_Skeleton::StaticGetResourceTypeName() );
		}

		Resource::C_Manager::GetInstance()->Validate();

		Resource::C_Manager::GetInstance()->Update();
		MT::Thread::Sleep(100);

		Resource::C_Manager::GetInstance()->Validate();

		Resource::C_Manager::GetInstance()->Update();
		MT::Thread::Sleep(100);

		Resource::C_Manager::GetInstance()->Dump();
	}

	void testSimple2()
	{	
		Resource::C_Manager::GetInstance()->Dump();

		{
			C_ModelHandle modelHandle;
			modelHandle.Load( ".\\mOdels\\test.mdl" );

			C_Model* pModel = modelHandle;			

			Resource::WaitForResourceData( pModel );

			TS_ASSERT( pModel->GetResourceFilename() == "./models/test.mdl" );
			TS_ASSERT( pModel->GetResourceTypeId() == C_Model::StaticGetResourceTypeId() );
			TS_ASSERT( pModel->GetResourceTypeName() == C_Model::StaticGetResourceTypeName() );

		}

		Resource::C_Manager::GetInstance()->Validate();

		Resource::C_Manager::GetInstance()->Update();
		MT::Thread::Sleep(1000);

		Resource::C_Manager::GetInstance()->Validate();

		Resource::C_Manager::GetInstance()->Update();
		MT::Thread::Sleep(1000);

		Resource::C_Manager::GetInstance()->Dump();
	}

	void testListIteration()
	{	
		C_ModelHandle modelHandle[5];
		C_SkeletonHandle skeletonHandle[5];

		modelHandle[0].Load(	".\\models\\Leopard.mdl" );
		modelHandle[1].Load(	".\\models\\Aardvark.mdl" );
		modelHandle[2].Load(	".\\models\\Zebra.mdl" );
		skeletonHandle[0].Load( ".\\models\\lionskeleton.skl" );
		skeletonHandle[1].Load( ".\\models\\wormskeleton.skl" );
		modelHandle[3].Load(	".\\models\\lion.mdl" );
		skeletonHandle[2].Load( ".\\skeletons/mothskeleton.skl" );
		modelHandle[4].Load(	".\\models/aardVark.mdl" );
		skeletonHandle[3].Load( ".\\models\\humanskeleton.skl" );
		skeletonHandle[4].Load( ".\\models\\mouseskeleton.skl" );

		Resource::WaitForResourceData( (Resource::C_ResourceBase*)modelHandle[0] );
		Resource::WaitForResourceData( (Resource::C_ResourceBase*)modelHandle[1] );
		Resource::WaitForResourceData( (Resource::C_ResourceBase*)modelHandle[2] );
		Resource::WaitForResourceData( (Resource::C_ResourceBase*)modelHandle[3] );
		Resource::WaitForResourceData( (Resource::C_ResourceBase*)modelHandle[4] );

		Resource::WaitForResourceData( (Resource::C_ResourceBase*)skeletonHandle[0] );
		Resource::WaitForResourceData( (Resource::C_ResourceBase*)skeletonHandle[1] );
		Resource::WaitForResourceData( (Resource::C_ResourceBase*)skeletonHandle[2] );
		Resource::WaitForResourceData( (Resource::C_ResourceBase*)skeletonHandle[3] );
		Resource::WaitForResourceData( (Resource::C_ResourceBase*)skeletonHandle[4] );

		Resource::C_HandleList handleList;

		Resource::C_Manager::GetInstance()->GetHandleList( handleList, -1, Resource::E_State_Unknown );

		TS_ASSERT( handleList.Size() == 9 );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[0])->GetResourceFilename() == "./models/aardvark.mdl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[1])->GetResourceFilename() == "./models/humanskeleton.skl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[2])->GetResourceFilename() == "./models/leopard.mdl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[3])->GetResourceFilename() == "./models/lion.mdl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[4])->GetResourceFilename() == "./models/lionskeleton.skl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[5])->GetResourceFilename() == "./models/mouseskeleton.skl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[6])->GetResourceFilename() == "./models/wormskeleton.skl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[7])->GetResourceFilename() == "./models/zebra.mdl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[8])->GetResourceFilename() == "./skeletons/mothskeleton.skl" );

		for(S32 i=0; i<handleList.Size(); i++)
		{
			Resource::C_ResourceBase* pResource = handleList[i];
			LOGINFO( Resource::g_Log, "All Resources[%d]: %s\n", i, pResource->GetResourceFilename().Chars() );
		}

		Resource::C_Manager::GetInstance()->GetHandleList( handleList, C_Model::StaticGetResourceTypeId(), Resource::E_State_Unknown );

		TS_ASSERT( handleList.Size() == 4 );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[0])->GetResourceFilename() == "./models/aardvark.mdl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[1])->GetResourceFilename() == "./models/leopard.mdl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[2])->GetResourceFilename() == "./models/lion.mdl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[3])->GetResourceFilename() == "./models/zebra.mdl" );

		for(S32 i=0; i<handleList.Size(); i++)
		{
			Resource::C_ResourceBase* pResource = handleList[i];
			LOGINFO( Resource::g_Log, "All Models[%d]: %s\n", i, pResource->GetResourceFilename().Chars() );
		}

		Resource::C_Manager::GetInstance()->GetHandleList( handleList, C_Skeleton::StaticGetResourceTypeId(), Resource::E_State_Unknown );

		TS_ASSERT( handleList.Size() == 5 );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[0])->GetResourceFilename() == "./models/humanskeleton.skl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[1])->GetResourceFilename() == "./models/lionskeleton.skl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[2])->GetResourceFilename() == "./models/mouseskeleton.skl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[3])->GetResourceFilename() == "./models/wormskeleton.skl" );
		TS_ASSERT( ((Resource::C_ResourceBase*)handleList[4])->GetResourceFilename() == "./skeletons/mothskeleton.skl" );

		for(S32 i=0; i<handleList.Size(); i++)
		{
			Resource::C_ResourceBase* pResource = handleList[i];
			LOGINFO( Resource::g_Log, "All Skeletons[%d]: %s\n", i, pResource->GetResourceFilename().Chars() );
		}


		Resource::C_Manager::GetInstance()->GetHandleList( handleList, -1, Resource::E_State_Ready );
		for(S32 i=0; i<handleList.Size(); i++)
		{
			Resource::C_ResourceBase* pResource = handleList[i];
			LOGINFO( Resource::g_Log, "All Ready Resources[%d]: %s\n", i, pResource->GetResourceFilename().Chars() );
		}
	}

	void testTemporaryCode()
	{		
		{
			C_Listener listener1;

			C_ModelHandle resHandle;
			resHandle.Load( "./models/Test.dat" );

			Resource::C_Manager::GetInstance()->Validate();

			//Resource::C_Manager::GetInstance()->DumpLoadQueue();

			C_ModelHandle resHandle2;
			resHandle2.Load( "./models\\Test.dat" );

			C_Model* pDummy = resHandle;
			C_Model* pDummy2 = resHandle2;

			C_Model* pRes3;
			{
				C_Listener listener2;

				C_ModelHandle resHandle3;
				resHandle3.Load( "./models\\Resource.dat", &listener2, (void*)1 );

				pRes3 = resHandle3;
				pRes3->DumpResourceInfo();

				pRes3->AddListener( &listener1, (void*)2 );

				{
					C_ModelHandle resHandle4 = resHandle3;
					//pRes3->DumpResourceInfo();

					Resource::C_Manager::GetInstance()->Update();
					MT::Thread::Sleep(1000);

				}

				//pRes3->DumpResourceInfo();

				//Resource::C_Manager::GetInstance()->Dump();

				Resource::C_Manager::GetInstance()->Dump();
			}

			Resource::C_Manager::GetInstance()->Validate();
			//Resource::C_Manager::GetInstance()->Dump();

			C_ModelHandle resHandleLots001;
			resHandleLots001.Load( "A\\Resource1.dat" );

			//Resource::C_Manager::GetInstance()->Dump();

			C_ModelHandle resHandleLots000;
			resHandleLots000.Load( "A\\Resource0.dat" );

			//Resource::C_Manager::GetInstance()->Dump();

			C_ModelHandle resHandleLots002;
			resHandleLots002.Load( "A\\Resource2.dat" );

			Resource::C_Manager::GetInstance()->Update();
			MT::Thread::Sleep(1000);

			Resource::C_Manager::GetInstance()->Update();
			MT::Thread::Sleep(1000);
			Resource::C_Manager::GetInstance()->Validate();

			{
				C_ModelHandle handleF;
				handleF.Load( "F" );
				C_ModelHandle handleC;
				handleC.Load( "C" );
				C_ModelHandle handleD;
				handleD.Load( "D" );
				C_ModelHandle handleE;
				handleE.Load( "E" );

				MT::Thread::Sleep(1000);

				{
					Resource::C_HandleList handleList;
					Resource::C_Manager::GetInstance()->GetHandleList( handleList, -1, Resource::E_State_Unknown );

					for(S32 i=0; i<handleList.Size(); i++)
					{
						Resource::C_ResourceBase* pResource = handleList[i];
						LOGINFO( Resource::g_Log, "Res: %s\n", pResource->GetResourceFilename().Chars() );
					}
				}
			}
		}

		Resource::C_Manager::GetInstance()->Update();
		MT::Thread::Sleep(1000);

		Resource::C_Manager::GetInstance()->Validate();
		Resource::C_Manager::GetInstance()->Dump();
	}

private:
	Resource::C_Manager* m_pManager;
};




#endif
