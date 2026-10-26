#include "XSIA3DExportPCH.h"

//*****************************************************************************
/*!	\file importmesh_command.cpp
	\brief Implementation of the DemoImportMesh command, which imports 
	mesh geometry into XSI.
	
	(c) Copyright 1996 - 2005 Softimage/Avid Corporation	
*/
//*****************************************************************************

#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_status.h>
#include <xsi_string.h>
#include <xsi_command.h>
#include <xsi_model.h>
#include <xsi_parameter.h>
#include <xsi_x3dobject.h>
#include <xsi_selection.h>
#include <xsi_primitive.h>
#include <xsi_polygonmesh.h>
#include <xsi_geometryaccessor.h>
#include <xsi_floatarray.h>
#include <xsi_doublearray.h>
#include <xsi_customproperty.h>
#include <xsi_ppglayout.h>
#include <xsi_meshbuilder.h>
#include "helper.h"
#include <xsi_progressbar.h>
#include <xsi_uitoolkit.h>

extern XSI::CustomProperty GetImportExportProp();
extern XSI::Parameter GetImportExportOption( const XSI::CString& in_strName );

//*****************************************************************************
/*!	Helper function for importing polygons from a file exported by the 
	DemoExportMesh command.
	
	\param in_strSection Section of the file that contains the polygon data
	\param in_mfr CMeshFileReader object used to parse the exported file. 
	\param in_meshBuilder XSI::CMeshBuilder object used to build the mesh. 
	\param in_nChunks Maximum number of polygons to read at a time. This controls 
	the amount of memory allocated for an array of polygons when building a 
	polygon mesh.
	\param in_bar Progress bar used to show the import status.
 */
//*****************************************************************************
XSI::CStatus ImportPolygons
( 
	const XSI::CString&	in_strSection, 
	CMeshFileReader&	in_mfr, 
	XSI::CMeshBuilder&	in_meshBuilder, 
	LONG				in_nChunks,
	XSI::ProgressBar&	in_bar	
)
{
	if (in_mfr.GotoSection(in_strSection)!=XSI::CStatus::OK)
	{
		return XSI::CStatus::Fail;
	}

	LONG* pPolyVertexCounts = NULL;
	LONG* pVtxIndices = NULL;
	bool bEndSection = false;

	// parse the whole section until all polygons are built
	while(!in_mfr.EndSection())
	{	
		if (in_bar.IsCancelPressed()) return XSI::CStatus::False;
	
		// creates the polygon array in chunks
		LONG vIdx = 0;
		LONG nVertexSize = 0;
        LONG i = 0;
		for ( ; i<in_nChunks && !(bEndSection=in_mfr.EndSection()); i++)
		{
			// get nb vertices per poly			
			LONG* pVals = NULL;
			LONG nVertexCount = in_mfr.GetLongValues(&pVals);
							
			nVertexSize += nVertexCount;
			pPolyVertexCounts = (LONG*)realloc(pPolyVertexCounts,sizeof(LONG)*(i+1));
			pVtxIndices = (LONG*)realloc(pVtxIndices,sizeof(LONG)*nVertexSize);
		
			// set the polygon vertex count for polygon i
			pPolyVertexCounts[i] = nVertexCount;
			
			// set the vertex indices for polygon i
			for ( LONG j=0; j<nVertexCount; j++)
			{								
				pVtxIndices[vIdx++] = pVals[j];
			}			
		}

		LONG nChunks = in_nChunks;
		if ( i < nChunks )
		{
			// nb chunks is larger than nb of available polygons
			nChunks = i;
		}
		
		// add polygons to mesh
		XSI::CStatus st = in_meshBuilder.AddPolygons( nChunks, pPolyVertexCounts, pVtxIndices ); 
		st.AssertSucceeded( L"ImportPolygons" );
		
		// cleanup arrays
		free(pPolyVertexCounts);
		free(pVtxIndices);
		pPolyVertexCounts = NULL;
		pVtxIndices = NULL;

		in_bar.Increment();
		
		if (bEndSection)
		{
			return XSI::CStatus::OK;
		}
	}
	return XSI::CStatus::OK;
}

//*****************************************************************************
/*!	Helper function for importing polygon vertices from a file exported by the 
	DemoExportMesh command.
	
	\param in_mfr CMeshFileReader object used to parse the export file. 
	\param in_meshBuilder XSI::CMeshBuilder object used to build the mesh. 
	\param in_nChunks Maximum number of vertices to read at a time. This 
	controls the amount of memory allocated for an array of vertices when 
	setting the positions of the polygon mesh vertices.  
	\param in_bar Progress bar to show the import status.
 */
//*****************************************************************************
XSI::CStatus ImportVertices
( 
	CMeshFileReader&	in_mfr, 
	XSI::CMeshBuilder&	in_meshBuilder, 
	LONG				in_nChunks,
	XSI::ProgressBar&	in_bar
)
{
	// build the polygon connection array in chunks
	if (in_mfr.GotoSection(L"Vertex Positions")!=XSI::CStatus::OK)
	{
		return XSI::CStatus::Fail;
	}

	double* pVertices = NULL;
	bool bEndSection = false;
	while(!in_mfr.EndSection())
	{	
		if (in_bar.IsCancelPressed()) return XSI::CStatus::False;

		LONG vIdx = 0;
		LONG nSize = 0;
		LONG i = 0;
		for ( ; i<in_nChunks && !(bEndSection=in_mfr.EndSection()); i++)
		{	
			double* pVals = NULL;
			LONG nCount = in_mfr.GetDoubleValues(&pVals);
		
			nSize += nCount;
			pVertices = (double*)realloc(pVertices, sizeof(double)*nSize);

			for ( LONG j=0; j<nCount; j++)
			{								
				pVertices[vIdx++] = pVals[j];
			}			
		}
		
		LONG nChunks = in_nChunks;
		if ( i < nChunks )
		{
			// nb chunks is larger than nb of available vertices
			nChunks = i;
		}
		
		// add vertices to mesh
		XSI::CStatus st = in_meshBuilder.AddVertices( nChunks, pVertices ); 
		st.AssertSucceeded( L"ImportVertices" );
		
		free( pVertices );
		pVertices = NULL;

		in_bar.Increment();
		
		if (bEndSection)
		{
			return XSI::CStatus::OK;
		}		
	}
	return XSI::CStatus::OK;
}

//*****************************************************************************
/*!	Helper function for importing a polygon mesh from a file exported by the 
	DemoExportMesh command.
	
	\param in_strSection Section of the file that contains the polygon data.
	\param in_strCaption Caption for the ProgressBar.
 */
//*****************************************************************************
XSI::CStatus ImportMesh
( 
	const XSI::CString& in_strSection,
	const XSI::CString& in_strCaption 
)
{
	XSI::Application app;
	XSI::Model root = app.GetActiveSceneRoot();
	
	// use empty mesh to create the imported mesh
	XSI::X3DObject xobj;
	root.AddPrimitive( L"EmptyPolygonMesh", L"MyImportedMesh", xobj );
	
	// get a mesh builder from the newly created geometry
	XSI::Primitive prim = xobj.GetActivePrimitive();
	XSI::PolygonMesh mesh = prim.GetGeometry();
	if (!mesh.IsValid()) return XSI::CStatus::False;

	XSI::CMeshBuilder meshBuilder = mesh.GetMeshBuilder();

	// prepare the export text file reader
	XSI::CString strOut = GetImportExportOption( L"ImportFile" ).GetValue();		
	LONG nChunks = GetImportExportOption( L"MeshChunkSize" ).GetValue();		

	CMeshFileReader mfr;
	XSI::CStatus st = mfr.Init( strOut );
	
	if (st != XSI::CStatus::OK)
	{
		return st;
	}	

	// set up a progress bar	
	XSI::UIToolkit kit = app.GetUIToolkit();
	XSI::ProgressBar bar = kit.GetProgressBar();

	bar.PutMaximum( 50000 );
	bar.PutStep( 100 );
	bar.PutVisible( true );
	bar.PutCaption( in_strCaption );
	
	// creates the vertex components
	ImportVertices( mfr, meshBuilder, nChunks, bar );
	
	// creates the polygon components
	ImportPolygons( in_strSection, mfr, meshBuilder, nChunks, bar );	
	
	XSI::CMeshBuilder::CErrorDescriptor err = meshBuilder.Build(false /*no undo*/);
	if (err != XSI::CStatus::OK)
	{
		app.LogMessage( L"Error building the mesh: " + err.GetDescription() );
	}
	return err;
}

//*****************************************************************************
/*!	Implementation of the DemoImportPolygonMesh command. This command uses the 
	CMeshBuilder class to import a mesh of quad polygons into XSI.
	\param in_ctxt The context that encapsulates the command to execute.
 */
//*****************************************************************************
XSIPLUGINCALLBACK XSI::CStatus A3DImport_Execute( XSI::CRef& in_ctxt )
{
	return ImportMesh( L"Polygon Vertex Indices", L"Importing polygon mesh..." );
}
