// AftkToXml.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdlib.h"

using namespace std;

class AftkToXML
{
private:

	AftkScene* m_aftk;

public:

	AftkToXML(AftkScene* aftk)
	{
		m_aftk = aftk;
	}

	//Exporting members

	//each is responsible for opening and closing its own file, which 
	//will have a unique name

	void ExportOccludersAsXML(AftkStatus& stat, const string& filename);
	void ExportTreesAsXML(AftkStatus& stat, const string& filename);

	string CreateFileName(const string& oldFileName, const string& appendDataType);

};


int _tmain(int argc, _TCHAR* argv[])
{
	//Get filename from args 

	if (argc != 2)
	{
		cout << "You need a filename e.g. AftkToXml c:\\data.a3d " << endl;
		return 0;
	}

	string filename = argv[1];

	//create new AFTK instance

	AftkScene* aftk = new AftkScene();
	
	VALIDATE_ALLOC_RET_INT(aftk);

	//Load aftk scene from file

	if (aftk->Read(filename) != AftkStatus::SUCCESS)
	{
		cout << "Failed to read AFTK scene, quitting" << endl;
		return 0;
	}

	//export relevant info as XML

	AftkStatus stat;
	string newFileName;
	AftkToXML aftkToXml(aftk);

	//Export occluders

	newFileName = aftkToXml.CreateFileName(filename, "AudioData");

	aftkToXml.ExportOccludersAsXML(stat, newFileName);
	
	if (stat != AftkStatus::SUCCESS)
	{
		cout << "Failed to export occluders" << endl;
	}
/*
	newFileName = aftkToXml.CreateFileName(filename, "Trees");

	aftkToXml.ExportTreesAsXML(stat, newFileName);
	
	//Export trees

	if (stat != AftkStatus::SUCCESS)
	{
		cout << "Failed to export trees" << endl;
	}
*/
	if (aftk)
	{
		delete aftk;
		aftk = 0;
	}

	return 0;
}

void AftkToXML::ExportOccludersAsXML(AftkStatus& stat, const string& filename)
{
	FILE* fp = 0;

	fp = fopen(filename.c_str(), "w");

	if (!fp)
	{
		cout << "Failed to open file: " << filename << endl;
		stat = AftkStatus::FAILURE;
		return;
	}
	fprintf(fp, "<Triangles>\n");

	//Iterrate heirachy, and then triangles, spitting out as we go...

	AftkSceneNodeIterator it(m_aftk);
	const AftkPolygonVertex* ppVerts[3];

    while(it)
    {
		AftkSceneNode* pNode = it;
		if (pNode->GetType() == AFTK_NODE_TYPE_SCENE_MESH)
		{         
			//if (strstr(pNode->GetName().c_str(), "Occluder"))
			//{
				AftkSceneMeshNode* pMesh = (AftkSceneMeshNode*)pNode;

				U32 numPolygons = pMesh->GetNumPolygons(0);
				for (U32 i = 0; i < numPolygons; i++)
				{
					AftkPolygon& pl = pMesh->GetPolygon(i);
					U32 numTriangles = pl.GetNumTriangles();
					for (U32 k=0; k<numTriangles; k++)
					{
						if (pl.GetTrianglePolygonVertices(k, ppVerts) == AftkStatus::SUCCESS)
						{
							//export triangle

							/*<Triangle
								<Position x = "-10" y = "0" z = "0"/>
								<Position x = "0" y = "15" z = "0"/>
								<Position x = "10" y = "0" z = "0"/>
							</Triangle>*/

							fprintf(fp, "   <Triangle>\n");

							for (int j=0; j<3; j++)
							{
								const AftkPolygonVertex* pPv = ppVerts[j];
								int posIndex = pPv->GetPositionIndex();
								AftkPosition3 pos = pMesh->GetVertexPosition(posIndex);

								AftkMatrix44 mat = pNode->GetWorldMatrix();

								pos = AftkApplyMatrix(mat, pos);

								fprintf(fp, "      <Position x = \"%f\" y = \"%f\" z = \"%f\" />\n", pos[0], pos[1], pos[2]);
							}

							fprintf(fp, "   </Triangle>\n\n");
						
						}
					}
				}
			//}

		}
		it++;
	}

 	fprintf(fp, "</Triangles>\n");



	fclose(fp);
}

void AftkToXML::ExportTreesAsXML(AftkStatus& stat, const string& filename)
{
	FILE* fp = 0;

	fp = fopen(filename.c_str(), "w");

	if (!fp)
	{
		cout << "Failed to open file: " << filename << endl;
		stat = AftkStatus::FAILURE;
		return;
	}

	fprintf(fp, "<Columns>\n");

	//Iterrate heirachy...

	AftkSceneNodeIterator it((AftkScene*)m_aftk);

    while(it)
    {
		AftkSceneNode* pNode = it;
		const AftkBlindDataResource* bd = pNode->GetBlindDataResourcePtr();
		
		if (bd)
		{
			const AftkBlindDataNode& bdn = bd->GetContent();

			U32 numAttr = bdn.GetNumAttributes();

			for (U32 i=0; i<numAttr; i++)
			{
				const AftkGenericAttribute& attr = bdn.GetAttribute(i);
				if (attr.GetName() == "SPTData")
				{
					//Ladies and gentleman, we have a tree
					/*<Column
						radius = "1"
						height = "15">
						<Position x = "0" y = "0" z = "0"/>
					</Column>*/

					float radius = 3.0f;
					float height = 8.0f;

					fprintf(fp, "   <Column>\n");
					fprintf(fp, "      radius = \"%f\"\n", radius);
					fprintf(fp, "      height = \"%f\"\n", height);

					AftkMatrix44 mat = pNode->GetWorldMatrix();
					fprintf(fp, "      <Position x = \"%f\" y = \"%f\" z = \"%f\" />\n", mat[3][0], mat[3][1], mat[3][2]);

					fprintf(fp, "   </Column>\n");
					break;
				}
			}
		}
		it++;
	}

	fprintf(fp, "</Columns>\n");

	fclose(fp);
}

string AftkToXML::CreateFileName(const string& oldFileName, const string& appendDataType)
{
	//using the original filename, append the datatype name, and swap the 
	//extension to xml.

	char pathBuffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char newFileName[_MAX_FNAME];
	char ext[_MAX_EXT];

	strncpy(pathBuffer, oldFileName.c_str(), _MAX_PATH);

	_splitpath(pathBuffer, drive, dir, fname, ext);

	sprintf(newFileName, "%s_%s", fname, appendDataType.c_str());

	_makepath(pathBuffer, drive, dir, newFileName, "xml");

	return pathBuffer;
}


















