#include <stdafx.h>
#include <RouteNo.h>
#include <data/ConversionOptions.h>
#include <converter_shared/helpers/AftkHelpers.h>

// helper functions for routes
S32 GetRouteNo(const C_DataConversionOptions* pOptions, const Util::C_String& layerName)
{
	if( pOptions->IgnoreRoutes() )
	{
		return -1;
	}

	const Util::C_String matchString = pOptions->GetToolConfig().GetRouteLayerNameBase();
	const S32 matchStringLength = matchString.Length();
	if(matchStringLength == 0)
	{
		return -1;
	}

	const S32 routePosStart = layerName.ISearch( matchString );
	if(routePosStart < 0)
	{
		return -1;
	}

	S32 routeDigitsStart = routePosStart + matchStringLength;

	S32 layerNameLength = layerName.Length();
	if( routeDigitsStart >= layerNameLength )
	{
		return -1;
	}

	// allow an optional '_' or ' '
	const char nextChar = layerName[routeDigitsStart];
	if((nextChar == ' ') || (nextChar == '_'))
	{
		routeDigitsStart++;
	}

	S32 routeDigitsEnd = layerNameLength;

	Util::C_String routeDigits = layerName( routeDigitsStart, routeDigitsEnd );

	S32 routeNo = atoi( routeDigits.Chars() );
	
	assert( routeNo < 32 );

	//LOGINFO( Tools::g_log, "%s %s %d", layerName.Chars(), routeDigits.Chars(),  routeNo );

	return routeNo;
}

S32 GetRouteNo( const C_DataConversionOptions* pOptions, const Aftk::C_SceneNode* pAftkNode )
{
	const Util::C_String layerName = AftkHelpers::GetLayerName( pAftkNode );
	return GetRouteNo( pOptions, layerName );
}
