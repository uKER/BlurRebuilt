#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/Lights/PointLight.h"
#include "Lux/Lights/SpotLight.h"
#include "Lux/Lights/HemisphereLight.h"
#include "Lux/Lights/HemisphereLight2.h"
#include "Lux/Lights/BounceGatherLight.h"
#include "Lux/Lights/AreaLight.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/Lights/HDRLight.h"
#include "Lux/RenderChannel.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/Parameterization/TextureAtlas.h"
#include "lux_common/TexPreprocess.h"
#include "Lux/TextureTypes/DefaultTexture.h"
#include "Lux/RasterizedTriIterator.h"
#include "Lux/Sampler.h"
#include <algorithm>

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

LightingModelProperties::LightingModelProperties()
{
	Reset();
}

LightingModelProperties::LightingModelProperties(
	const LightingModelProperties& rhs)
{
	*this = rhs;
}

LightingModelProperties::~LightingModelProperties()
{
	Clear();
}

bool GetBoolValueFromString(char *pString)
{
	if (stricmp(pString, "Yes") == 0 ||
		stricmp(pString, "On") == 0 ||
		atoi(pString) != 0)
		return true;
	return false;
}

void LightingModelProperties::SetXMLParameter(char *pName, char *pValue)
{
	std::stringstream s;
	F32 f = 0;
	U32 unsignedInt = 0;
	s << pValue;

	// get gamma on/off
	if (stricmp(pName, "gamma") == 0)
	{
		if (stricmp(pValue, "on") == 0)
		{
			// set gamma on (default value)
			m_fInputGamma = m_fOutputGamma = 2.2f;
		}
		else if (stricmp(pValue, "off") == 0)
		{
			// set gamma off
			m_fInputGamma = m_fOutputGamma = 1.0f;
		}
		else
		{
			// set custom gamma
			s >> m_fInputGamma;
			m_fOutputGamma = m_fInputGamma;
		}
	}
	// get input gamma
	else if (stricmp(pName, "inputgamma") == 0)				{ s >> m_fInputGamma; }
	// get output gamma
	else if (stricmp(pName, "outputgamma") == 0)			{ s >> m_fOutputGamma; }
	// get unit scale
	else if (stricmp(pName, "metersize") == 0)				{ s >> m_fMeterSize; }
	// get brightness
	else if (stricmp(pName, "brightness") == 0)				{ s >> m_fBrightness; }						
	// get bounce lighting brightness
	else if (stricmp(pName, "BounceScale") == 0)			{ s >> m_fBounceScale; }
	// get raytracing epsilon
	else if (stricmp(pName, "Epsilon") == 0)				{ s >> m_fEpsilon; }
	// get area light global brightness
	else if (stricmp(pName, "AreaLightScale") == 0)			{ s >> m_fAreaLightScale; }
	// get max cluster size
	else if (stricmp(pName, "MaxClusterSize") == 0)			{ s >> m_fMaxClusterSize; }
	// get max cluster area
	else if (stricmp(pName, "MaxClusterArea") == 0)			{ s >> m_fMaxClusterArea; }
	// get texel density
	else if (stricmp(pName, "TexelDensity") == 0)			{ s >> m_fTexelDensity;	}
	// get alternative texel density (high)
	else if (stricmp(pName, "LightmapDensityHigh") == 0)	{ s >> m_fLightmapDensity[0]; }
	// get alternative texel density (normal)
	else if (stricmp(pName, "LightmapDensityNormal") == 0)	{ s >> m_fLightmapDensity[1]; }
	// get alternative texel density (low)
	else if (stricmp(pName, "LightmapDensityLow") == 0)		{ s >> m_fLightmapDensity[2]; }
	// get alternative texel density (very low)
	else if (stricmp(pName, "LightmapDensityVeryLow") == 0)	{ s >> m_fLightmapDensity[3]; }
	// get alternative texel density (ultra low)
	else if (stricmp(pName, "LightmapDensityUltraLow") == 0){ s >> m_fLightmapDensity[4]; }
	// get default lightmap size (atlas size)
	else if (stricmp(pName, "DefaultLightMapSize") == 0)	{ s >> m_defaultLightmapSize; }
	// get double sided polyflag bit
	else if (stricmp(pName, "DoubleSidedPolyflag") == 0)	{ s >> unsignedInt; m_iDoubleSidedPolyflag = (1<<unsignedInt); }
	// get emit light polyflag bit
	else if (stricmp(pName, "EmitLightPolyflag") == 0)		{ s >> unsignedInt; m_iEmitLightPolyflag = (1<<unsignedInt); }
	// get do not light polyflag bit
	else if (stricmp(pName, "DoNotLightPolyflag") == 0)		{ s >> unsignedInt; m_iDoNotLightPolyflag = (1<<unsignedInt); }
	// get do not cast shadows polyflag bit
	else if (stricmp(pName, "DoNotCastShadowsPolyflag") == 0)	{ s >> unsignedInt; m_iDoNotCastShadowsPolyflag = (1<<unsignedInt); }
	// get do not cast shadows polyflag bit
	else if (stricmp(pName, "LowResLightmapsPolyflag") == 0)	{ s >> unsignedInt; m_iLowResLightmapsPolyflag = unsignedInt; }
	// get number of bits in the lightmap res polyflag
	else if (stricmp(pName, "LowResLightmapsBits") == 0)	{ s >> unsignedInt; m_iLowResLightmapsBits = unsignedInt; }
	// get number of bounces
	else if (stricmp(pName, "Bounces") == 0)				{ s >> m_iMaxBounces; }
	// get attribute type
	else if (stricmp(pName, "DefaultBakeTarget") == 0)
	{
		if (stricmp(pValue, "vertex") == 0)
		{
			m_defaultBakeTarget = LUX_MODEL_BAKE_TARGET_VERTEX;
		}
		else if (stricmp(pValue, "lightmap") == 0)
		{
			m_defaultBakeTarget = LUX_MODEL_BAKE_TARGET_LIGHTMAP;
		}
	}
	else if (stricmp(pName, "AntiAliasing") == 0)
	{
		// get antialiasing settings (number of taps)
		s >> unsignedInt;
		m_iAntiAliasing = RoundUpPow2(unsignedInt);
		if (m_iAntiAliasing > 256)
			m_iAntiAliasing = 256;
	}
	else if (stricmp(pName, "AlphaScaling") == 0)
	{
		// scale the colour value using the alpha
		m_bAlphaScaling = GetBoolValueFromString(pValue);
	}
	else if (stricmp(pName, "IgnoreDirectionalLightsFromMaya") == 0)
	{
		// ignore all directional lights from maya
		m_bIgnoreDirectionalLightsFromMaya = GetBoolValueFromString(pValue);
	}
	else if (stricmp(pName, "TimeOfDay") == 0)
	{
		// get time of day setting
		if (stricmp(pValue, "Sunny") == 0)
		{
			m_iTimeOfDay = TIMEOFDAY_SUNNY;
		}
		else if (stricmp(pValue, "Overcast") == 0)
		{
			m_iTimeOfDay = TIMEOFDAY_OVERCAST;
		}
		else if (stricmp(pValue, "Night") == 0)
		{
			m_iTimeOfDay = TIMEOFDAY_NIGHT;
		}
	}
	else if (stricmp(pName,"OutputFormat") == 0)
	{
		// get output filetype
		if (stricmp(pValue, "A8L8") == 0)
		{
			m_iOutputFormat = TEXFMT_A8L8;
		}
		else if (stricmp(pValue, "DXT4") == 0)
		{
			m_iOutputFormat = TEXFMT_DXT4;
		}
		else if (stricmp(pValue, "RGBA32") == 0)
		{
			m_iOutputFormat = TEXFMT_A8R8G8B8;
		}
		else if (stricmp(pValue, "DXT1") == 0)
		{
			m_iOutputFormat = TEXFMT_DXT1;
		}
		else if (stricmp(pValue, "RGB16") == 0)
		{
			m_iOutputFormat = TEXFMT_R5G6B5;
		}
		else if (stricmp(pValue, "YUV") == 0)
		{
			m_iOutputFormat = TEXFMT_UYVY;
		}
	}
	else if (stricmp(pName,"OutputType") == 0)
	{
		// get output filetype
		if (stricmp(pValue, "RGB") == 0)
		{
			m_iOutputType = OUTPUT_TYPE_RGB;
		}
		else if (stricmp(pValue, "gotham_night") == 0)
		{
			m_iOutputType = OUTPUT_TYPE_GOTHAM_NIGHT;
		}
		else if (stricmp(pValue, "basisv2") == 0)
		{
			m_iOutputType = OUTPUT_TYPE_BASIS3V2;
		}
		else if (stricmp(pValue, "transfer") == 0)
		{
			m_iOutputType = OUTPUT_TYPE_TRANSFER;
		}
		else if (stricmp(pValue, "direction") == 0)
		{
			m_iOutputType = OUTPUT_TYPE_DIRECTION;
		}
	}
	else if (stricmp(pName,"ProgressiveBouncePasses") == 0)
	{
		s >> m_nProgressiveBouncePasses;
		m_nProgressiveBouncePasses--;
		if (m_nProgressiveBouncePasses < 0)
			m_nProgressiveBouncePasses = 0;
	}
	else if (stricmp(pName,"ColouredBounceLighting") == 0)
	{
		// get coloured bounce lighting on/off
		m_bColouredBounceLighting = GetBoolValueFromString(pValue);
	}
	else if (stricmp(pName,"PrescaleSH") == 0)
	{
		// get SH prescale on/off
		m_bPrescaleSH = GetBoolValueFromString(pValue);
	}
	else if (stricmp(pName,"TiledOutput") == 0)
	{
		// get output filetype
		m_bTiled = GetBoolValueFromString(pValue);;
	}
	else if (stricmp(pName, "AntiLightLeaking") == 0)
	{
		m_bAntiLightLeaking = GetBoolValueFromString(pValue);
	}
	else if (stricmp(pName, "SaveExtraSamplePointData") == 0)
	{
		m_bSaveExtraSamplePointData = GetBoolValueFromString(pValue);
	}
}

void LightingModelProperties::ReadXMLNode(DOMNode *pNode)
{
    DOMNode *child;

    if (pNode) 
	{
        if (pNode->getNodeType() == DOMNode::ELEMENT_NODE)
		{
            char *pElementName = XMLString::transcode(pNode->getNodeName());
			if (stricmp(pElementName, "modelproperties") == 0)
			{
				if(pNode->hasAttributes())
				{
					// get all the attributes of the node
					DOMNamedNodeMap *pAttributes = pNode->getAttributes();
					int nSize = pAttributes->getLength();

					for(int i = 0; i < nSize; ++i) 
					{
						DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
						// get attribute name
						char *pName = XMLString::transcode(pAttributeNode->getName());

						char* pValue = XMLString::transcode(pAttributeNode->getValue());
						SetXMLParameter(pName, pValue);
						XMLString::release(&pValue);
						XMLString::release(&pName);
					}
				}
			}
			else if (stricmp(pElementName, "Channel") == 0)
			{
				// read renderchannel info
				RenderChannel *pChannel = new RenderChannel( true );
				pChannel->ReadXMLNode( pNode );
				AddRenderChannel( pChannel );
			}
			else if (stricmp(pElementName, "Light") == 0)
			{
				Light* pLight = 0;
				if(pNode->hasAttributes())
				{
					// get all the attributes of the node
					DOMNamedNodeMap *pAttributes = pNode->getAttributes();
					int nSize = pAttributes->getLength();

					// two passes, first one creates the light, second one initializes it
					for (int iPass=0; iPass<2; iPass++)
					{
						for(int i = 0; i < nSize; ++i) 
						{
							DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
							// get attribute name
							char *pName = XMLString::transcode(pAttributeNode->getName());
							char* pValue = XMLString::transcode(pAttributeNode->getValue());

							switch (iPass)
							{
							case 0: // create light
								if (stricmp(pName, "Type") == 0)
								{
									// get attribute type
									if (stricmp(pValue, "Directional") == 0)
									{
										pLight = new DirectionalLight;
									}
									else if (stricmp(pValue, "PointLight") == 0)
									{
										pLight = new PointLight;
									}
									else if (stricmp(pValue, "Hemisphere") == 0)
									{
										pLight = new HemisphereLight;
									}
									else if (stricmp(pValue, "Hemisphere2") == 0)
									{
										pLight = new HemisphereLight2;
									}
									else if (stricmp(pValue, "BounceGather") == 0)
									{
										pLight = new BounceGatherLight;
									}
									else if (stricmp(pValue, "HDR") == 0)
									{
										pLight = new HDRLight;
									}

									if (pLight != 0)
									{
										//Escape for-loop.
										break;
									}
								}
								break;
							case 1: // initialize light parameters
								pLight->SetXMLParameter(pName, pValue);
								if (pLight->GetType() == LUX_LIGHT_TYPE_BOUNCEGATHER &&
									!stricmp(pName, "bouncepasses"))
								{
									m_nProgressiveBouncePasses = atoi(pValue) -1;
									if (m_nProgressiveBouncePasses < 0)
										m_nProgressiveBouncePasses = 0;	
								}
								break;
							}

							XMLString::release(&pValue);
							XMLString::release(&pName);
						}
					}
				}
				if (pLight)
				{
					//Memory deallocation for the light will be handled by the destructor of this class.
					AddLight(pLight);
				}
			}
            XMLString::release(&pElementName);
		}
        for (child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
		{
			LightingModelProperties::ReadXMLNode(child);
		}
    }
}

void LightingModelProperties::DeleteAllLights()
{
	for (U32 i = 0; i < (U32)m_lights.Size(); i++)
	{
		if (m_lights[i] != 0)
		{
			delete m_lights[i];
		}
	}
	m_lights.Clear();

	for (int i=0; i<m_Channels.Size(); i++)
		delete m_Channels[i];
	m_Channels.Destroy();
}

void LightingModelProperties::Clear()
{
	Reset();
	DeleteAllLights();
}

void LightingModelProperties::AddLight(Light* pLight)
{
	m_lights.PushBack(pLight);

	// get renderchannel index
	int i = GetRenderChannel(pLight->m_channel.c_str());
	if (i == -1) i = 0;
	pLight->m_iChannel = i;
}

// add a renderchannel to the list - checks for duplicates by name
void LightingModelProperties::AddRenderChannel(RenderChannel *pChannel)
{
	// if a channel with this name already exists, quit
	int i = GetRenderChannel( pChannel->m_name.c_str() );
	if (i != -1)
	{
		// already exists. any channel in the .xml config file with the same name overrides the default one
		delete pChannel;
		return;
	}

	if ( pChannel->m_target == RenderChannel::Alpha )
		m_bWhiteAlpha = false;

	// doesn't exist yet - create a new channel
	m_Channels.PushBack();
	m_Channels[m_Channels.Size()-1] = pChannel;
}

// find renderchannel by name
int LightingModelProperties::GetRenderChannel(const char *szName)
{
	// if a channel with this name already exists, overwrite it
	for (int i=0; i<m_Channels.Size(); i++)
	{
		if (stricmp(m_Channels[i]->m_name.c_str(), szName) == 0)
			return i;
	}
	return -1; // not found - return default
}

int LightingModelProperties::AddDefaultRenderChannel(const char *szName)
{
	RenderChannel *pNewChannel = new RenderChannel();

	// check for animated[n]on/off
	char buffer[512];
	memcpy(buffer, szName, 8);
	buffer[8] = 0;
	if (!stricmp(buffer,"animated"))
	{
		pNewChannel->m_iSubChannel = szName[8] - '1';
		pNewChannel->m_target = RenderChannel::AnimatedOn;
		if (!stricmp(szName+9, "off"))
			pNewChannel->m_target = RenderChannel::AnimatedOff;
	}
	else if (!stricmp(szName,"sun"))
		pNewChannel->m_target = RenderChannel::Sun;
	else if (!stricmp(szName,"sky"))
		pNewChannel->m_target = RenderChannel::Sky;
	else
		pNewChannel->m_target = RenderChannel::RGB;

	pNewChannel->m_name = szName;

	AddRenderChannel(pNewChannel);

	return m_Channels.Size()-1;
}

void LightingModelProperties::SetDefaultBakeTarget(LuxModelBakeTargetEnum bakeTarget)
{
	m_defaultBakeTarget = bakeTarget;
}

void LightingModelProperties::SetDefaultLightmapSize(S32 size)
{
	m_defaultLightmapSize = size;
}

S32 LightingModelProperties::GetNumLights() const
{
	return (S32)m_lights.Size();
}

int LightingModelProperties::GetMaxNumBounces() const
{
	return m_iMaxBounces;
}

const Light* LightingModelProperties::GetLightPtr(S32 instance) const
{
	return m_lights[instance];
}

TRenderChannelVector *LightingModelProperties::GetRenderChannelsPtr()
{
	return &m_Channels;
}

Light* LightingModelProperties::GetLightPtr(S32 instance)
{
	return m_lights[instance];
}

LuxModelBakeTargetEnum LightingModelProperties::GetDefaultBakeTarget() const
{
	return m_defaultBakeTarget;
}

S32 LightingModelProperties::GetDefaultLightmapSize() const
{
	return m_defaultLightmapSize;
}

TextureFormat LightingModelProperties::GetOutputFormat() const
{
	return m_iOutputFormat;
}

int LightingModelProperties::GetOutputType() const
{
	return m_iOutputType;
}

bool LightingModelProperties::GetTiled() const
{
	return m_bTiled;
}

bool LightingModelProperties::GetWhiteAlpha() const
{
	if (m_bAlphaScaling)
		return false;
	if (m_iOutputType == OUTPUT_TYPE_BASIS3 || m_iOutputType == OUTPUT_TYPE_DIRECTION)
		return false;

	return m_bWhiteAlpha;
}


F32 LightingModelProperties::GetTexelDensity() const
{
	return m_fTexelDensity / m_fMeterSize;
}

F32 LightingModelProperties::GetEpsilon() const
{
	return m_fEpsilon;
}

F32 LightingModelProperties::GetTexelDensityRatio(int i) const
{
	return m_fLightmapDensity[i];
}

bool LightingModelProperties::GetAntiLightLeaking() const
{
	return m_bAntiLightLeaking;
}

bool LightingModelProperties::GetSaveExtraSamplePointData() const
{
	return m_bSaveExtraSamplePointData;
}

U32 LightingModelProperties::GetNumAntiAliasingTaps() const
{
	return m_iAntiAliasing;
}

float LightingModelProperties::GetBounceScaleFactor() const
{
	return m_fBounceScale;
}

float LightingModelProperties::GetAreaLightScaleFactor() const
{
	return m_fAreaLightScale;
}

void LightingModelProperties::Reset()
{
	m_defaultLightmapSize = 256;
	m_fTexelDensity = 1;
	m_fLightmapDensity[0] = 2.f;
	m_fLightmapDensity[1] = 1.f;
	m_fLightmapDensity[2] = 0.25f;
	m_fLightmapDensity[3] = 1.f/16.f;
	m_fLightmapDensity[4] = 1.f/64.f;
	m_fBounceScale = 1.0f;
	m_fEpsilon = 0.0001f;
	m_fAreaLightScale = 1.0f;
	m_iAntiAliasing = 8;
	m_defaultBakeTarget = LUX_MODEL_BAKE_TARGET_LIGHTMAP;
	DeleteAllLights();
	m_bTiled = false;
	m_bPrescaleSH = false;
	m_bWhiteAlpha = true;
	m_fMeterSize = 1;
	m_bAlphaScaling = false;
	m_bIgnoreDirectionalLightsFromMaya = false;
	m_nProgressiveBouncePasses = 0;
	m_bColouredBounceLighting = false;
	m_fMaxClusterSize = 250.0f;
	m_fMaxClusterArea = 40.0f;
	m_fInputGamma = 1.0f;
	m_fOutputGamma = 1.0f;
	m_iMaxBounces = 2;
	m_bAntiLightLeaking = 0;
	m_bSaveExtraSamplePointData = false;

	// gotham default polyflags
	m_iDoubleSidedPolyflag = (1<<8);
	m_iEmitLightPolyflag = (1<<20);
	m_iDoNotLightPolyflag = (1<<11);
	m_iDoNotCastShadowsPolyflag = (1<<10);
	m_iLowResLightmapsPolyflag = 12;
	m_iLowResLightmapsBits = 1;

	m_Channels.PushBack(new RenderChannel(true));

	m_iOutputFormat = OUTPUT_FORMAT_DONTCARE;
	m_iOutputType = OUTPUT_TYPE_RGB;
	m_iTimeOfDay = TIMEOFDAY_ALL;
}

// find an existing directional light or creates a new one
void LightingModelProperties::DeleteExistingDirectionalLights()
{
	// if there are any maya lights in the scene, remove all lights added from the xml file
	for (int i=0; i<m_lights.Size(); i++)
	{
		if (m_lights[i]->IsGlobalLight() && m_lights[i]->IsDeltaLight())
		{
			// delete it
			delete m_lights[i];
			m_lights.EraseByIndex(i);
		}
	}
}

// check time of day setting of a light and compare it to our time of day
bool LightingModelProperties::CheckLightTimeOfDaySetting(Aftk::C_SceneNode *pNode)
{
	if (m_iTimeOfDay == TIMEOFDAY_ALL)
		return true;

	AftkUt::C_NodeFlagParser parser(*pNode);

	std::string text = parser.GetFlagGroupCategory();
	std::transform(text.begin(), text.end(), text.begin(), tolower);
	if (text == "system")
	{
		text = parser.GetFlagGroupName();
		std::transform(text.begin(), text.end(), text.begin(), tolower);
		if (text != "light")
			return true;
	}
	else
	{
		if (text != "light")
			return true;
	}

	std::string result;
	parser.GetParameterValue("Conditions", result);
	int iLightTimeOfDay = atoi(result.c_str());

	if (m_iTimeOfDay == TIMEOFDAY_ALL || iLightTimeOfDay == TIMEOFDAY_ALL ||
		m_iTimeOfDay == iLightTimeOfDay)
		return true;

	return false;
}

// read lights from the maya scene and add them to the lighting model properties
bool LightingModelProperties::AddLightsFromScene(Aftk::C_Scene *pScene)
{
	// iterate through the scene graph, find the lights, add them to our list
	for (Aftk::C_SceneNodeConstIterator it(pScene); it; it++)
	{
		const Aftk::C_SceneNode* pNode = it;

		// check light isn't a child of a non-exported node
		if (!DoesBaseInstanceNeedExporting((Aftk::C_SceneNode*)pNode))
			continue;

		Light *pLight;
		Aftk::Matrix44 worldMatrix = pNode->GetWorldMatrix();
		const Aftk::C_SceneNode *pINode = pNode;

		Aftk::E_ObjType type = pNode->GetType();
		Aftk::Matrix44 mat = pNode->GetWorldMatrix();

		if (type == Aftk::E_ObjType_SceneInstance)
		{
			pNode = ((Aftk::C_SceneInstanceNode*)pNode)->GetSourceNodePtr();
			if (!pNode)
				continue;
			type = pNode->GetType();
		}

		Aftk::C_SceneNonAmbientLightNode *pMayaLight = (Aftk::C_SceneNonAmbientLightNode*)pNode;

		// reject certain lights depending on the time of day
		if (!CheckLightTimeOfDaySetting(pMayaLight))
			continue;

		switch (type)
		{
			// skip anything we don't recognize
		default:
			continue;
		case Aftk::E_ObjType_SceneDirectionalLight:	// read in directional light specific parameters
			if (m_bIgnoreDirectionalLightsFromMaya)
				continue;
			DeleteExistingDirectionalLights();
			pLight = new DirectionalLight((Aftk::C_SceneDirectionalLightNode*)pMayaLight, mat);
			break;
		case Aftk::E_ObjType_SceneSpotLight:			// read in spotlight specific parameters
			pLight = new SpotLight((Aftk::C_SceneSpotLightNode*)pMayaLight, mat);
			break;
		case Aftk::E_ObjType_ScenePointLight:			// read in pointlight specific parameters
			pLight = new PointLight((Aftk::C_ScenePointLightNode*)pMayaLight, mat);
			break;
		case Aftk::E_ObjType_SceneAreaLight:			// read in area light
			pLight = new AreaLight((Aftk::C_SceneAreaLightNode*)pMayaLight, mat);
			break;
		case Aftk::E_ObjType_SceneHemisphereLight:
			pLight = new HemisphereLight((Aftk::C_SceneHemisphereLightNode*)pMayaLight, mat);
			break;
		}

		// check whether this light has specific channel settings from Maya
		Aftk::I_Plug *pPlug = pMayaLight->GetPlugPtr("lightmapChannel");
		if (pPlug && pPlug->GetDataType() == Aftk::E_GenericAttributeType_String)
		{
			std::string value;
			pPlug->GetValue(value);
			pLight->m_channel = value;
			U32 iChannel = GetRenderChannel(value.c_str());
			// create this channel using the default target for it's name
			if (iChannel == -1)
				AddDefaultRenderChannel(value.c_str());
		}

		AddLight( pLight );
	}

	// print out a warning if there are multiple directional lights in the scene
	float3 vDirectionalLightDir;
	int nDirectionalLights = 0;
	for (int i = 0; i <GetNumLights(); i++)
	{
		const LUX::Light* pLight = GetLightPtr(i);
		if (pLight->GetType() == LUX_LIGHT_TYPE_DIRECTIONAL)
		{
			vDirectionalLightDir = ((DirectionalLight*)pLight)->GetDirection();
			nDirectionalLights++;
		}
	}

	// if we have a hemispherelight2, copy the sun direction into that
	if (nDirectionalLights)
	{
		for (int i = 0; i <GetNumLights(); i++)
		{
			const LUX::Light* pLight = GetLightPtr(i);
			if (pLight->GetType() == LUX_LIGHT_TYPE_HEMISPHERE2)
			{
				((HemisphereLight2*)pLight)->SetSunDirection(vDirectionalLightDir);
			}
		}
	}

	return (nDirectionalLights<=1);
}

bool DoesNodeHaveAreaLightActorProperties(Aftk::C_SceneNode *pNode, float *pfIntensity)
{
	AftkUt::C_NodeFlagParser parser(*pNode);

	std::string strClass, strName;
	strClass = parser.GetFlagGroupCategory();
	std::transform(strClass.begin(), strClass.end(), strClass.begin(), tolower);

	strName = parser.GetFlagGroupName();
	std::transform(strName.begin(), strName.end(), strName.begin(), tolower);
	if (!(strClass == "light" && strName == "area") &&
		!(strClass == "system" && strName == "arealight"))
		return false;

	// yes it has an area light actor properties

	// get the intensity value
	if (pfIntensity)
	{
		std::string result;
		parser.GetParameterValue("Intensity", result);
		*pfIntensity = atof(result.c_str());
	}

	return true;
}

// create area lights for sets of incandescent polygons
void LightingModelProperties::AddAreaLightsFromScene(Aftk::C_Scene *pScene, TChartList *pChartList)
{
	bool bDefaultRightTimeOfDay = true;
	float fIntensity, fDefaultIntensity = 100;
	int i,n;

	// get texture preprocessor
	TexturePreprocessor texpp;

	// find the default area light settings
	Aftk::C_SceneNodeIterator it(pScene);
	while (it)
	{
		Aftk::C_SceneNode *pNode = it;

		// does this node have area light actor properties?
		if (!DoesNodeHaveAreaLightActorProperties(pNode, &fDefaultIntensity))
		{
			it++;
			continue;
		}

		bDefaultRightTimeOfDay = CheckLightTimeOfDaySetting(pNode);
		break;
	}

	// check each chart for emissive polygons
	n = pChartList->Size();
	for (i=0; i<n; i++)
	{
		// get the material associated with this chart
		CTextureChart *pChart = pChartList->At(i);
		if (!pChart)
			continue;

		Aftk::I_SceneMeshNode *m_pMesh = pChart->m_pMesh;

		// check time of day setting
		if (DoesNodeHaveAreaLightActorProperties(m_pMesh, &fIntensity))
		{
			if (!CheckLightTimeOfDaySetting(m_pMesh))
				continue;
		}
		else
		{
			if (!bDefaultRightTimeOfDay)
				continue;

			fIntensity = fDefaultIntensity;
		}

		fIntensity *= GetAreaLightScaleFactor();

		// does this chart have any triangles on it with their 'emit light' polyflag set?
		AreaLight *pLight = new AreaLight();

		Aftk::Matrix44 matrix = pChart->m_pMatrixNode->GetWorldMatrix();

		Aftk::Matrix44 worldmatrix = m_pMesh->GetWorldMatrix();

		// and add this chart's polygons into it
		int iFace = pChart->m_iFace;
		int nTrianglesAdded = 0;
		while (iFace != -1)
		{
			const Aftk::C_Polygon *pPoly = &m_pMesh->GetPolygon(iFace);

			if (GetPolyFlags(
				pPoly,
				0,
				0,
				0,
				m_iEmitLightPolyflag,
				0,
				0).emitLight)
			{
				float3 v[3];
				for (int iv=0; iv<3; iv++)
					v[iv] = LPVS::ApplyMatrix( matrix, 
						m_pMesh->GetEvaluatedPolygonVertexPosition( pPoly->GetPolygonVertexIndex( iv ) ) );

				// get material
				float3 vEmit = float3(1,1,1);

				const Aftk::C_MaterialResource *pRes = GetMaterialResourcePtr(&m_pMesh->GetPolygon(iFace));
				if (!pRes)
				{
					iFace = POLY_DATA(iFace)->iChartLink;
					continue;
				}

				const Aftk::C_MaterialNode *pMat = &pRes->GetContent();

				// get texture
				if (pMat->GetNumShaderImageNodes())
				{
					const Aftk::C_ShaderImageNode *pSIN = pMat->GetShaderImageNodePtr(0);
					std::string texname;
					pSIN->GetFilePlug().GetValue(texname);

					// get texture data
					TextureData *pTexData = texpp.LookupTexture(texname);
					if (pTexData)
					{
						// get UVs
						float2 uv[3];
						int m_iLMapUVSet = 0;
						for (int iPV=0; iPV<3; iPV++)
						{
							int iPolyVert = pPoly->GetPolygonVertexIndex(iPV);
							Aftk::TextureCoordinate4 newuv = m_pMesh->GetPolygonVertexTextureCoordinate( m_iLMapUVSet, iPolyVert );
							uv[iPV].x = newuv.x;
							uv[iPV].y = newuv.y;

						}

						float f_y=0,f_u=0,f_v=0;
						int R,G,B;
						float Y,U,V;

						// sample the texture a few times
						// TODO: generate a proper sampling pattern and use that
						int iSample;
						for (iSample=0; iSample<32; iSample++)
						{
							// get a random position on the triangle in UV space
							float r1,r2;
							UniformSampleTriangle(genrand_real1(), genrand_real1(), &r1, &r2);

							float r3 = 1.0f - (r1+r2);

							float fx = r1*uv[0].x + r2*uv[1].x + r3*uv[2].x;
							float fy = r1*uv[0].y + r2*uv[1].y + r3*uv[2].y;

							fx = fx - ceil(fx) + 1.0f;
							fy = fy - ceil(fy) + 1.0f;

							int x = fx * pTexData->m_cX;
							int y = fy * pTexData->m_cY;

							// protect against floating point inaccuracies
							if (x >= pTexData->m_cX)
								x = 0;
							if (y >= pTexData->m_cY)
								y = 0;

                            U32 col = pTexData->GetTexel(x,y);
							U8 *p = (U8*)&col;

							R = p[0];
							G = p[1];
							B = p[2];

							Y =  (0.257 * R) + (0.504 * G) + (0.098 * B) + 16;
							V =  (0.439 * R) - (0.368 * G) - (0.071 * B) + 128;
							U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128;

							f_y += Y, f_u += U, f_v += V;
						}

						Y=f_y/(float)iSample; U=f_u/(float)iSample; V=f_v/(float)iSample;
						Y *= 0.5f;

						B = 1.164*(Y - 16) + 2.018*(U - 128);
						G = 1.164*(Y - 16) - 0.813*(V - 128) - 0.391*(U - 128);
						R = 1.164*(Y - 16) + 1.596*(V - 128);

						vEmit.x = (float)R/255.0f;
						vEmit.y = (float)G/255.0f;
						vEmit.z = (float)B/255.0f;

						if (vEmit.x < 0) vEmit.x = 0;
						if (vEmit.y < 0) vEmit.y = 0;
						if (vEmit.z < 0) vEmit.z = 0;
					}
				}

				// apply inverse gamma correction
				vEmit.x = powf(vEmit.x*2.0f, InputGamma()) * 0.5f;
				vEmit.y = powf(vEmit.y*2.0f, InputGamma()) * 0.5f;
				vEmit.z = powf(vEmit.z*2.0f, InputGamma()) * 0.5f;

				vEmit *= fIntensity;

				pLight->AddTriangle( v[0], v[1], v[2], vEmit );
				nTrianglesAdded++;
			}
            
			iFace = POLY_DATA(iFace)->iChartLink;
		}

		if (nTrianglesAdded)
		{
			// add the light to the scene
			AddLight( pLight );

			if (nTrianglesAdded>24)
			{
				LOGWARN( g_Log, "Performance warning: '%s' has '%d' light emitting triangles. Are you sure this is correct?",
					m_pMesh->GetPath().c_str(), nTrianglesAdded);
			}
		}
		else
			delete pLight;
	}
}

bool LightingModelProperties::NeedTangentSpace() const
{
	if (m_iOutputType == OUTPUT_TYPE_BASIS3 ||
		m_iOutputType == OUTPUT_TYPE_BASIS3V2)
		return true;

	return false;
}
