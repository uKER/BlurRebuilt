#include "stdafx.h"

#include <EditorLib/Misc/Status.h>

#include <EditorLib/Core.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/CommandFactory.h>

#include <UserCmd/Conversion/ConvertDynamicPhysicalCmd.h>
#include <UserCmd/Drop/DropCmd.h>
#include <UserCmd/Nightwing/CreateLinkedCoverCmd.h>
#include <UserCmd/Poly/PolyAddPointsCmd.h>
#include <UserCmd/Poly/PolyInsertPointsCmd.h>
#include <UserCmd/Spline/AddPointsCmd.h>
#include <UserCmd/Spline/InsertPointsCmd.h>
#include <UserCmd/Spline/SmoothTangentsCmd.h>
#include <UserCmd/Spline/ReverseSplineCmd.h>
#include <UserCmd/Verification/Amax/VerifyPhysicalObjects.h>
#include <UserCmd/Verification/Common/VerifyDynamicPositions.h>
#include <UserCmd/Verification/Common/VerifyNull.h>
#include <UserCmd/Verification/Common/VerifyXtValue.h>
#include <UserCmd/Register.h>

namespace EditorUserCmd
{

C_Register::C_Register()
{
}

C_Register::~C_Register()
{
}

EditorLib::C_Status C_Register::Initialise(const Btl::String& projectName)
{
	EditorLib::C_Status status;
	
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_SplineAddPointsCmd>();
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_SplineInsertPointsCmd>();	
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_PolyAddPointsCmd>();
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_PolyInsertPointsCmd>();	
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_SmoothTangentsCmd>();	
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_ReverseSplineCmd>();
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_DropCmd>();
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_ConvertDynamicPhysicalCmd>();
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_VerifyDynamicPositionsCmd>();
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_VerifyNullCmd>();
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_VerifyXtValueCmd>();
	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_VerifyPhysicalObjectsCmd>();
	if (!status) return status;
	
	if (!stricmp(projectName.Chars(), "Nightwing"))
	{
		status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_CreateLinkedCoverCmd>();	
		if (!status) return status;
	}

	return EditorLib::E_Status_Success;
}

void C_Register::Shutdown()
{
}

}
