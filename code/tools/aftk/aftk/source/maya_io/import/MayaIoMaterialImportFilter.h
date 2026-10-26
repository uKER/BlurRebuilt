#ifndef MAYA_IO_MATERIAL_IMPORT_FILTER_H
#define MAYA_IO_MATERIAL_IMPORT_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace MayaIo
{
	class C_MaterialImportFilter
	{
	public:

		C_MaterialImportFilter()
		{
			//perform filter
			m_filterSet.Insert(std::string("normals"));
			m_filterSet.Insert(std::string("colors"));
			m_filterSet.Insert(std::string("uvpt"));
			m_filterSet.Insert(std::string("edge"));
			m_filterSet.Insert(std::string("vrts"));
			m_filterSet.Insert(std::string("controlPoints"));
			m_filterSet.Insert(std::string("message"));
			m_filterSet.Insert(std::string("caching"));
			m_filterSet.Insert(std::string("isHistoricallyInteresting"));
			m_filterSet.Insert(std::string("nodeState"));
			m_filterSet.Insert(std::string("binMembership"));
			m_filterSet.Insert(std::string("objectId"));
			m_filterSet.Insert(std::string("primitiveId"));
			m_filterSet.Insert(std::string("raySampler"));
			m_filterSet.Insert(std::string("rayDepth"));
			m_filterSet.Insert(std::string("rayInstance"));
			m_filterSet.Insert(std::string("refractionLimit"));
			m_filterSet.Insert(std::string("refractiveIndex"));
			m_filterSet.Insert(std::string("mediumRefractiveIndex"));
			m_filterSet.Insert(std::string("refractions"));
			m_filterSet.Insert(std::string("diffuse"));
			m_filterSet.Insert(std::string("rayDirection"));
			m_filterSet.Insert(std::string("rayDirectionX"));
			m_filterSet.Insert(std::string("rayDirectionY"));
			m_filterSet.Insert(std::string("rayDirectionZ"));
			m_filterSet.Insert(std::string("opacityDepth"));
			m_filterSet.Insert(std::string("hideSource"));
			m_filterSet.Insert(std::string("surfaceThickness"));
			m_filterSet.Insert(std::string("shadowAttenuation"));
			m_filterSet.Insert(std::string("transparencyDepth"));
			m_filterSet.Insert(std::string("lightAbsorbance"));
			m_filterSet.Insert(std::string("chromaticAberration"));
			m_filterSet.Insert(std::string("outGlowColor"));
			m_filterSet.Insert(std::string("outGlowColorR"));
			m_filterSet.Insert(std::string("outGlowColorG"));
			m_filterSet.Insert(std::string("outGlowColorB"));
			m_filterSet.Insert(std::string("pointCamera"));
			m_filterSet.Insert(std::string("pointCameraX"));
			m_filterSet.Insert(std::string("pointCameraY"));
			m_filterSet.Insert(std::string("pointCameraZ"));
			m_filterSet.Insert(std::string("normalCamera"));
			m_filterSet.Insert(std::string("normalCameraX"));
			m_filterSet.Insert(std::string("normalCameraY"));
			m_filterSet.Insert(std::string("normalCameraZ"));
			m_filterSet.Insert(std::string("lightDataArray"));
			m_filterSet.Insert(std::string("matteOpacityMode"));
			m_filterSet.Insert(std::string("matteOpacity"));
			m_filterSet.Insert(std::string("outMatteOpacity"));
			m_filterSet.Insert(std::string("outMatteOpacityR"));
			m_filterSet.Insert(std::string("outMatteOpacityG"));
			m_filterSet.Insert(std::string("outMatteOpacityB"));
			m_filterSet.Insert(std::string("hardwareShader"));
			m_filterSet.Insert(std::string("hardwareShaderR"));
			m_filterSet.Insert(std::string("hardwareShaderG"));
			m_filterSet.Insert(std::string("hardwareShaderB"));
			m_filterSet.Insert(std::string("miRefractionBlur"));
			m_filterSet.Insert(std::string("miRefractionRays"));
			m_filterSet.Insert(std::string("miIrradiance"));
			m_filterSet.Insert(std::string("miIrradianceR"));
			m_filterSet.Insert(std::string("miIrradianceG"));
			m_filterSet.Insert(std::string("miIrradianceB"));
			m_filterSet.Insert(std::string("miIrradianceColor"));
			m_filterSet.Insert(std::string("miIrradianceColorR"));
			m_filterSet.Insert(std::string("miIrradianceColorG"));
			m_filterSet.Insert(std::string("miIrradianceColorB"));
			m_filterSet.Insert(std::string("miDeriveFromMaya"));
			m_filterSet.Insert(std::string("miRefractiveIndex"));
			m_filterSet.Insert(std::string("miRefractions"));
			m_filterSet.Insert(std::string("miAbsorbs"));
			m_filterSet.Insert(std::string("miDiffuse"));
			m_filterSet.Insert(std::string("miColor"));
			m_filterSet.Insert(std::string("miColorR"));
			m_filterSet.Insert(std::string("miColorG"));
			m_filterSet.Insert(std::string("miColorB"));
			m_filterSet.Insert(std::string("miTransparency"));
			m_filterSet.Insert(std::string("miTransparencyR"));
			m_filterSet.Insert(std::string("miTransparencyG"));
			m_filterSet.Insert(std::string("miTransparencyB"));
			m_filterSet.Insert(std::string("miTranslucence"));
			m_filterSet.Insert(std::string("miTranslucenceFocus"));
			m_filterSet.Insert(std::string("miNormalCamera"));
			m_filterSet.Insert(std::string("miNormalCamera"));
			m_filterSet.Insert(std::string("miNormalCameraX"));
			m_filterSet.Insert(std::string("miNormalCameraY"));
			m_filterSet.Insert(std::string("miNormalCameraZ"));
			m_filterSet.Insert(std::string("miShinyness"));
			m_filterSet.Insert(std::string("miAngle"));
			m_filterSet.Insert(std::string("miSpreadX"));
			m_filterSet.Insert(std::string("miSpreadY"));
			m_filterSet.Insert(std::string("miWhiteness"));
			m_filterSet.Insert(std::string("miWhitenessR"));
			m_filterSet.Insert(std::string("miWhitenessG"));
			m_filterSet.Insert(std::string("miWhitenessB"));
			m_filterSet.Insert(std::string("miSpecularColor"));
			m_filterSet.Insert(std::string("miSpecularColorR"));
			m_filterSet.Insert(std::string("miSpecularColorG"));
			m_filterSet.Insert(std::string("miSpecularColorB"));
			m_filterSet.Insert(std::string("miReflectivity"));
			m_filterSet.Insert(std::string("miReflectionBlur"));
			m_filterSet.Insert(std::string("miReflectionRays"));
			m_filterSet.Insert(std::string("uvCoord"));
			m_filterSet.Insert(std::string("uCoord"));
			m_filterSet.Insert(std::string("vCoord"));
			m_filterSet.Insert(std::string("uvFilterSize"));
			m_filterSet.Insert(std::string("uvFilterSizeX"));
			m_filterSet.Insert(std::string("uvFilterSizeY"));
			m_filterSet.Insert(std::string("disableFileLoad"));
			m_filterSet.Insert(std::string("useFrameExtension"));
			m_filterSet.Insert(std::string("frameExtension"));
			m_filterSet.Insert(std::string("frameOffset"));
			m_filterSet.Insert(std::string("useHardwareTextureCycling"));
			m_filterSet.Insert(std::string("startCycleExtension"));
			m_filterSet.Insert(std::string("endCycleExtension"));
			m_filterSet.Insert(std::string("byCycleIncrement"));
			m_filterSet.Insert(std::string("filterType"));
			m_filterSet.Insert(std::string("filterWidth"));
			m_filterSet.Insert(std::string("useCache"));
			m_filterSet.Insert(std::string("useMaximumRes"));
			m_filterSet.Insert(std::string("coverage"));
			m_filterSet.Insert(std::string("coverageU"));
			m_filterSet.Insert(std::string("coverageV"));
			m_filterSet.Insert(std::string("doTransform"));
			m_filterSet.Insert(std::string("stagger"));
			m_filterSet.Insert(std::string("noiseUV"));
			m_filterSet.Insert(std::string("noiseU"));
			m_filterSet.Insert(std::string("noiseV"));
			m_filterSet.Insert(std::string("blurPixelation"));
			m_filterSet.Insert(std::string("vertexCameraOne"));
			m_filterSet.Insert(std::string("vertexCameraOneX"));
			m_filterSet.Insert(std::string("vertexCameraOneY"));
			m_filterSet.Insert(std::string("vertexCameraOneZ"));
			m_filterSet.Insert(std::string("vertexCameraTwo"));
			m_filterSet.Insert(std::string("vertexCameraTwoX"));
			m_filterSet.Insert(std::string("vertexCameraTwoY"));
			m_filterSet.Insert(std::string("vertexCameraTwoZ"));
			m_filterSet.Insert(std::string("vertexCameraThree"));
			m_filterSet.Insert(std::string("vertexCameraThreeX"));
			m_filterSet.Insert(std::string("vertexCameraThreeY"));
			m_filterSet.Insert(std::string("vertexCameraThreeZ"));
			m_filterSet.Insert(std::string("vertexUvOne"));
			m_filterSet.Insert(std::string("vertexUvOneU"));
			m_filterSet.Insert(std::string("vertexUvOneV"));
			m_filterSet.Insert(std::string("vertexUvTwo"));
			m_filterSet.Insert(std::string("vertexUvTwoU"));
			m_filterSet.Insert(std::string("vertexUvTwoV"));
			m_filterSet.Insert(std::string("vertexUvThree"));
			m_filterSet.Insert(std::string("vertexUvThreeU"));
			m_filterSet.Insert(std::string("vertexUvThreeV"));
			m_filterSet.Insert(std::string("objectType"));
			m_filterSet.Insert(std::string("rayDepth"));
			m_filterSet.Insert(std::string("primitiveId"));
			m_filterSet.Insert(std::string("pixelCenter"));
			m_filterSet.Insert(std::string("pixelCenterX"));
			m_filterSet.Insert(std::string("pixelCenterY"));
			m_filterSet.Insert(std::string("preFilter"));
			m_filterSet.Insert(std::string("preFilterRadius"));
			m_filterSet.Insert(std::string("infoBits"));
			m_filterSet.Insert(std::string("fast"));
			m_filterSet.Insert(std::string("outUvFilterSize"));
			m_filterSet.Insert(std::string("outUvFilterSizeX"));
			m_filterSet.Insert(std::string("outUvFilterSizeY"));
			m_filterSet.Insert(std::string("doTransform"));
			m_filterSet.Insert(std::string("lightDataArray[-1]"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightDirection"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightDirectionX"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightDirectionY"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightDirectionZ"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightIntensity"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightIntensityR"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightIntensityG"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightIntensityB"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightDiffuse"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightAmbient"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightSpecular"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightShadowFraction"));
			m_filterSet.Insert(std::string("lightDataArray[-1].preShadowIntensity"));
			m_filterSet.Insert(std::string("lightDataArray[-1].lightBlindData"));
			m_filterSet.Insert(std::string("fileHasAlpha"));
			m_filterSet.Insert(std::string("alphaIsLuminance"));
		}

		bool Find(const char* str) const
		{
			Ut::UtSet<std::string>::ConstIterator it;
			it = m_filterSet.Find(str);

			if (it == m_filterSet.End())
			{
				// Not found;
				return false;
			}

			return true;
		}

		private:
			Ut::UtSet<std::string> m_filterSet;

	};
}

#endif
