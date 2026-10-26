#ifndef _SCENE_CONVERTER_ROUTE_NO_H_
#define _SCENE_CONVERTER_ROUTE_NO_H_

#ifdef _MSC_VER
#pragma once
#endif

class C_DataConversionOptions;
namespace Aftk
{
	class C_SceneNode;
}

S32 GetRouteNo(const C_DataConversionOptions* pOptions, const Util::C_String& layerName);
S32 GetRouteNo( const C_DataConversionOptions* pOptions, const Aftk::C_SceneNode* pAftkNode );

#endif