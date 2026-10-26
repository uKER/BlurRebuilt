#include "stdafx.h"

#include "Vis_Core\vis.h"
#include "Vis_Core\D3DRenderer.h"
#include "Vis_Core\D3DMesh.h"

#include "Vis_Core\Database.h"
#include "Vis_Core\SamplePoints.h"

#include "Vis_Core\sampler.h"
#include "CServ/ClusterServHeaders.h"
#include "Vis_Core\ClientSide.h"
#include "Vis_Core\PVSResult.h"

#include "Vis_Core\Scene.h"
#include "vis_core\ProcessTextureTags.h"

int giDebugCell = -1;
bool gbDebugMode = false;
int gnTotalSamples;

Debug::Logger &g_Log = Debug::GetLogger("tools.vis");

extern double gnStats_NumObjectsRendered;
extern double gnStats_NumObjectsCulled;

void AddDummyNode(float x, float y, float z);

Vis::Vis()
{
	m_pDatabase = NULL;
	m_pLogCallbackFunc = NULL;
	m_pVS2Data = NULL;
	m_bGuiMode = false;
	m_bUseNetworkClients = false;
	m_iVisListMode = VISLIST_NOTHING;
	m_iTransparentPVSPolyFlagMask = 0;
	m_iClusterThreshold = 1;
	m_iPolygonThreshold = 1;
}

Vis::~Vis()
{
	if (m_pDatabase)
		delete m_pDatabase;
	if (m_pSampleAreas)
		delete m_pSampleAreas;
}

void Vis::LoadXML(const std::string filename)
{
	m_pSampleAreas = new SampleAreaList;

	// Read XML file using Xerces
	m_pSampleAreas->ReadBinaryInputFile( filename );

	if (!m_pSampleAreas->m_Areas.Size())
	{
		if (!gbDebugMode)
		{
			LOGERROR( g_Log, "Error: Failed to read any sample points from the specified XML file.\n");
			exit(-1);
		}
	}
}

void Vis::SetClusterThreshold(int i)
{
	m_iClusterThreshold = i;
}

void Vis::SetPolygonThreshold(int i)
{
	m_iPolygonThreshold = i;
}

// this function gets called by the scheduler when a job result has been received
void Vis::JobCompletedCallbackFunction(void *pThis, JobScheduler *pScheduler, JobScheduler::Job *pJob)
{
	Vis *p = (Vis*)pThis;
	PVSResult *pResult = (PVSResult*)pJob->m_pResult;
	assert(pResult);

	int iCell = pJob->m_iID;

	int nVisible = 0;

	// add the result of the job to our database
	int n = p->m_pDatabase->GetNumRefs();
	int iPolyRef = 0;
	int i=0;
	while (1)
	{
		// process this byte
		int iBitMask = 1;
		int iByte = pResult->m_pData[i];
		i++;
		do
		{
			// process each bit
			if (iByte & iBitMask)
			{
				p->m_pDatabase->Set(iPolyRef,iCell);
				nVisible++;
			}

			iBitMask<<=1;
			iPolyRef++;
			if (iPolyRef>=n)
			{
				switch (p->m_iVisListMode)
				{
				case VISLIST_CREATE:
					p->m_visList[iCell] = nVisible;
					break;
				case VISLIST_COMPARE:
					if (nVisible != p->m_visList[iCell])
					{
						LOGWARN( g_Log, "WARNING: mismatch cell %d is %d should be %d (???)\n",
							iCell, nVisible, p->m_visList[iCell]);
					}
					else
					{
						LOGINFO( g_Log, "ok\n");
					}
					break;
				}

				if (nVisible == 0)
				{
					LOGFATAL( g_Log, "Possible PVS Problem! Got entirely empty PVS cell from '%s'.\n", pScheduler->GetCurrentJobHostname() );
				}

				// finished
				delete pResult;
				return;
			}
		}
		while (iBitMask !=256);
	}
}

void AngleVectors (float3 &angles, float3 &out)
{
	float		angle;
	float		sr, sp, cr, cp;
	
	angle = (angles[1]-90) * (PI*2 / 360);
	sr = sinf(angle);
	cr = cosf(angle);
	angle = (angles[2]-90) * (PI*2 / 360);
	sp = sinf(angle);
	cp = cosf(angle);

	out[0] = cp*cr;
	out[2] = sp*cr;
	out[1] = -sr;
}

// put some debug code in here
void Vis::DebugCode()
{
}

// 
void Vis::OverrideSamplePoints(const std::string &str)
{
	float f1, f2, f3;
	sscanf(str.c_str(), "%f,%f,%f", &f1, &f2, &f3);

	m_pSampleAreas = new SampleAreaList;

	SampleList *pSA = new SampleList;

	pSA->m_nSamples = 1;
	PvsPoint pt;
	pt.vPos.x = f1;
	pt.vPos.y = f2;
	pt.vPos.z = f3;
	pt.iType = PVS_TYPE_POSITION;

	LOGINFO( g_Log, "setting sample point to %5.2f, %5.2f, %5.2f\n", f1, f2, f3);

	pSA->m_vPoints.PushBack(pt);
	m_pSampleAreas->m_Areas.PushBack( pSA );	
}

// automatically generate sample points for PVS'ing a car
void Vis::AutoGenSamplePointsForCar()
{
	float fYawSize		= 22.5f;
	float fPitchSize	= 22.5f;
	float fDistance = 15.f;

	m_pSampleAreas = new SampleAreaList;

	LOGINFO( g_Log, "cell size (yaw):   %5.2f\n", fYawSize);
	LOGINFO( g_Log, "cell size (pitch): %5.2f\n", fPitchSize);

	for (float fPitch=0; fPitch<90.0f; fPitch+=fPitchSize)
	{
		for (float fYaw=0; fYaw<360.0f; fYaw+=fYawSize)
		{
			SampleList *pSA = new SampleList;

			int n = 16;
			pSA->m_nSamples = n*4;
			for (int i=0; i<n; i++)
			{
				float fAngle = fYaw + RandomFloat() * fYawSize;
				float fHeight = fPitch + RandomFloat() * fPitchSize;

				float3 vOut;
				AngleVectors(float3(0,90-fHeight,fAngle), vOut);
				vOut = vOut * fDistance;

				PvsPoint pt;
				pt.vPos = vOut;
				pt.iType = PVS_TYPE_POSITION;

				pSA->m_vPoints.PushBack(pt);
			}
			m_pSampleAreas->m_Areas.PushBack( pSA );
		}
	}

	gbCarMode = true;
}


void Vis::DisplayCell(int i)
{
	m_pDatabase->DisplayVisibilityInfo(i);
}

void Vis::DisplayUsage()
{
	m_pDatabase->DisplayUsageInfo();
}

// initialize vis processor with aftk scene
void Vis::SetScene(Aftk::C_Scene *pScene)
{
	// triangulate the scene
	LOGINFO( g_Log, "Triangulating scene...\n");

	//Traverse the entire scene.
	Aftk::C_Status status = Aftk::C_SceneNodeVisit::Traverse(
		pScene,
		Aftk::C_MeshTriangulate::C_TriangulateVisitor());	

	if (m_bGuiMode)
	{
		LOGINFO( g_Log, "Unsharing vertices...\n");
		//Traverse the entire scene.
		Aftk::C_Status status = Aftk::C_SceneNodeVisit::Traverse(
			pScene,
			Aftk::C_MeshExpandPolygonVertices::C_ExpandPolygonVerticesVisitor());
	}

	m_pScene = pScene;
}

// turn GUI mode on/off (default is off)
void Vis::SetGUIMode(bool bOn)
{
	m_bGuiMode = bOn;
}

// set the path where .fxb files are loaded from
void Vis::SetPaths(const char *pszTexCachePath, const char *pszFXBPath)
{
	m_sTextureCachePath = pszTexCachePath;
	m_sFXBPath = pszFXBPath;
}

// read xml config file
bool Vis::SetXMLConfigFile(const char *pszfilename)
{
	if (strlen(pszfilename) == 0)
		return true;

	return m_Config.Read(pszfilename);	
}

// set the path that the screensaver PCs get their exe from
void Vis::SetRemoteClientPath(const std::string &path)
{
	m_sNetworkClientsRemotePath = path;
	m_bUseNetworkClients = true;
}

// set the path that's used for the vis-list file
void Vis::SetVisListPath(const std::string &path, int iMode)
{
	m_visListPath = path;
	m_iVisListMode = iMode;
}

// set the transparent pvs polyflag mask
void Vis::SetTransparentPVSPolyFlagMask(U32 iMask)
{
	m_iTransparentPVSPolyFlagMask = iMask;
}

// load walter's VS2 BSP file
void Vis::SetVS2(const std::string filename)
{
	// load the data and pass it to clients as a binary buffer
	FILE *f = fopen(filename.c_str(), "rb");

	if (!f)
	{
		printf("Could not open file '%s' for reading.\n", filename.c_str());
		return;
	}

	fseek(f,0,SEEK_END);
	int file_length = ftell(f);
	rewind(f);

	m_pVS2Data = new Ut::Vector<BYTE>;

	m_pVS2Data->Resize(file_length);

	fread(&m_pVS2Data->At(0), 1, file_length, f);
}


// main processing function
bool Vis::Process(Progress::I_ProgressBar *pProgress)
{
	int i;
	char buffer[512];

	assert(m_pScene);

	LOGINFO( g_Log, "Preparing to start server...\n");
	// create scene mesh in serializable format
	Scene scene;
	scene.SetClusterThreshold(m_iClusterThreshold);
	scene.SetPolygonThreshold(m_iPolygonThreshold);

	scene.SetPaths(m_sTextureCachePath.c_str(), m_sFXBPath.c_str());

	// generate pvs pass information from config data
	if (m_Config.m_Passes.Size() > 0)
	{
		for (int iPass=0; iPass<m_Config.m_Passes.Size(); iPass++)
		{
			PVSPass pass;
			LightingPass *pPass = m_Config.m_Passes[iPass];
			for (int iLayer=0; iLayer<m_pScene->GetNumLayers(); iLayer++)
			{
				Aftk::C_LayerNode *pLayer = m_pScene->GetLayerPtr(iLayer);
				bool bPVS = pPass->DoesLayerPVS(pLayer->GetName().c_str());
				bool bOcc = pPass->DoesLayerOcclude(pLayer->GetName().c_str());
				if (bPVS && bOcc)
					pass.m_BothLayers.PushBack(iLayer);
				else if (bPVS)
					pass.m_PVSLayers.PushBack(iLayer);
				else if (bOcc)
					pass.m_OcclusionLayers.PushBack(iLayer);
			}
			if (pPass->m_bLightAllLayersByDefault && pPass->m_bCastShadowsOnAllLayersByDefault)
				pass.m_BothLayers.PushBack(-1);
			else if (pPass->m_bLightAllLayersByDefault)
				pass.m_PVSLayers.PushBack(-1);
			else if (pPass->m_bCastShadowsOnAllLayersByDefault)
				pass.m_OcclusionLayers.PushBack(-1);

			scene.AddPVSPass(pass);
		}
	}
	else
	{
		PVSPass pass;
		pass.m_BothLayers.PushBack(-1);
		for (int iLayer=0; iLayer<m_pScene->GetNumLayers(); iLayer++)
			pass.m_BothLayers.PushBack(iLayer);
		scene.AddPVSPass(pass);
	}

	scene.LoadAftkScene(m_pScene);

	// create initial update containing the mesh in serializable form
	ClientSide *pInitialUpdate = new ClientSide;
	pInitialUpdate->SetScene(&scene);
	if (m_pVS2Data)
	{
		pInitialUpdate->SetVS2(m_pVS2Data);
		m_pVS2Data = NULL;
	}

	// create server
	JobServer *pServer = CreateServer(pInitialUpdate);
	pServer->GetJobSchedulerPtr()->SetJobCompletedCallbackFunction( &JobCompletedCallbackFunction, this );
	delete pInitialUpdate;

	//
	int n = m_pSampleAreas->m_Areas.Size();
	m_pDatabase = new BitfieldDatabase( m_pScene, scene.GetPolyRefs(), n);

	m_visList.Resize(n);

	switch (m_iVisListMode)
	{
	case VISLIST_CREATE:
		// initialize a new vislist
		memset(&m_visList[0], 0, sizeof(int)*n);
		break;
	case VISLIST_COMPARE:
		// read a previously calculated one to compare with
		FILE *f = fopen(m_visListPath.c_str(), "rb");
		fread(&m_visList[0], n, sizeof(int), f);
		fclose(f);
		break;
	}

	scene.TakeOwnerShipOfPolyRefs();

	int nTotalSamples = 0;
	for (i=0; i<n; i++)
		nTotalSamples += m_pSampleAreas->m_Areas[i]->GetNumSamples();

	LOGINFO( g_Log, "Cluster threshold: %d, Polygon threshold: %d \n", m_iClusterThreshold, m_iPolygonThreshold);
	LOGINFO( g_Log, "%d samples in %d cells\n", nTotalSamples, n);
	LOGINFO( g_Log, "%d primitives\n", scene.GetNumIds());
	LOGINFO( g_Log, "%d clusters\n", scene.GetNumClusterIds());
	LOGINFO( g_Log, "%d triangles\n", scene.GetNumTriangles());
	LOGINFO( g_Log, "%d instances\n", scene.GetNumInstances());

	gnTotalSamples = 0;

	// generate a list of jobs for the server jobscheduler
	if (giDebugCell != -1)
	{
		i = giDebugCell;
		n = giDebugCell+1;
	}
	else
		i = 0;

	ost::Thread *pClient = 0;
/*
	if (gbDebugMode)
	{
		LoadDatabase("c:\\temp\\pvsoutput.dat");
	}
	else
*/
	{
		LOGINFO( g_Log, "Creating %d jobs...\n", n);

		// add jobs to server
		for (; i<n; i++)
		{
			if ( m_pSampleAreas->m_Areas[i] )
				pServer->GetJobSchedulerPtr()->AddJob( i, 1, m_pSampleAreas->m_Areas[i] );
		}

		// if we have no GUI, we're ok to delete the aftk
		if (!m_bGuiMode)
			delete m_pScene;

		// optionally send job to farm - makes screensaver PCs connect to us
		if (m_bUseNetworkClients)
		{
			SOCKADDR_IN sa;
			char hostname[512];
			gethostname(hostname, 512);

			char pszSaville[MAX_PATH]; GetTempPathA(MAX_PATH, pszSaville); // [Blur2PC] local temp folder; the farm share is gone

			// copy the file to a temporary folder on the network
			char pszTmp[MAX_PATH];
			if (GetTempFileName(pszSaville, "vis", 0, pszTmp) != 0)
			{
				strcpy(buffer, pszTmp);

				if (CopyFile(m_sNetworkClientsRemotePath.c_str(), buffer, false))
				{
					char bufferManifest[1024];
					strcpy(bufferManifest, buffer);
					strcat(bufferManifest, ".manifest");
					std::string sManifestDest = m_sNetworkClientsRemotePath + std::string(".manifest");
					// copy manifest file if it exists. we run the remote command either way.
					CopyFile(sManifestDest.c_str(), bufferManifest, false);

					char buffer2[512];
					sprintf(buffer2, "%s /worker %s", buffer, hostname);

					pServer->AddFarmClientCommandline( buffer2, CLUSTERSERV_FLAG_WINDOWS);
				}
				else
				{
					LOGWARN( g_Log, "Failed to copy Vis temp file to %s\n", buffer);
				}
			}
		}

		// start the server
		pServer->start();

		// start a client
		pClient = StartClient();

		LOGINFO( g_Log, "Rendering...\n");

		// wait until all jobs have been done
		pServer->WaitForAllJobsCompleted(pProgress);

		LOGINFO( g_Log, "%5.2f%% of polygons culled\n", (float)(gnStats_NumObjectsCulled * 100.0 / gnStats_NumObjectsRendered));
	}
/*
	if (gbDebugMode)
	{
		DisplayCell(847);

		LOGINFO( g_Log, "Writing out debug .a3d file.\n");

		m_pScene->Write("c:\\temp\\testout.a3d", AFTK_FILE_TYPE_BEST_GUESS);
	}
*/

	// note: database will have been update by the callback function
	pServer->Exit();

	if (pClient)
	{
		LOGINFO( g_Log, "Waiting for client to finish...\n");
		pClient->join();	// TODO: wait for one minute and then timeout
	}

	pServer->join();
	delete pServer;
	delete pClient;

	// delete the temporary exe file
	if (m_bUseNetworkClients)
	{
		DeleteFile(buffer);
		strcat(buffer, ".manifest");
		DeleteFile(buffer);
	}

	if (m_iVisListMode == VISLIST_CREATE)
	{
		// write out a vislist
		FILE *f = fopen(m_visListPath.c_str(), "wb");
		fwrite(&m_visList[0], n, sizeof(int), f);
		fclose(f);
	}


	return true;
}

void Vis::Save(const char *psz)
{
	if (!m_pDatabase->Save(psz))
		printf("WARNING: Could not open file '%s' for writing.\n",psz);
}

// load a previously calculated database
void Vis::LoadDatabase(const std::string filename)
{
	// load the database
	m_pDatabase = new BitfieldDatabase( m_pScene, filename );
}
