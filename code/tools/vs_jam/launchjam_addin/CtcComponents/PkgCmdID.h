/***************************************************************************

Copyright (c) Microsoft Corporation. All rights reserved.
This code is licensed under the Visual Studio SDK license terms.
THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

***************************************************************************/

//
// Definition of the numeric part of the IDs for the CTC elements of this
// package.
//

/////////////////////////////////////////////////////////////////////
// Menus
//
#define MyToolbar           0x101
#define MyToolbar2          0x102

/////////////////////////////////////////////////////////////////////
// Menu Groups
//
#define MyMenuGroup          0x1010
#define SolutionToolbarGroup 0x1011
#define MyMainToolbarGroup   0x1012
#define MyEditorCtxGroup     0x1013
#define JamToolbarGroup		 0x1014

/////////////////////////////////////////////////////////////////////
// Commands
//
#define cmdidBuildXWithJam				0x2001
#define cmdidRebuildXWithJam			0x2002
#define cmdidBuildSolutionWithIBJam     0x2003
#define cmdidRebuildSolutionWithIBJam   0x2004
#define cmdidGenerateNewVcproj			0x2005
#define cmdidCancelJAMBuild				0x2006
#define cmdidBuildWithJamCtx			0x2007
#define cmdidJamProperties				0x2008
#define cmdidBuildAndRunUnitTest		0x2009
#define cmdidBuildDocumentation			0x200a
#define cmdidPerforceCheckoutOrAdd		0x200b

/////////////////////////////////////////////////////////////////////
// Bitmaps
//
#define bmpBuild	6
#define bmpRebuild	9
#define bmpClean	7
#define bmpCancel	2
#define bmpBuildSel	14
#define bmpCheckout 29
