#include "stdafx.h"


#include <maya/MFnEnumAttribute.h>

namespace MayaIo
{
	const int ACTOR_BOX_SHAPE		= 0;
	const int ACTOR_SPHERE_SHAPE	= 1;
	const int ACTOR_CYLINDER_SHAPE	= 2;
}

using namespace MayaIo;

Aftk::C_Status C_SceneCustomActorImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//Export the base AFTK scene node
	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	MDagPath shapeDagPath = dagPath;
	mStatus = shapeDagPath.extendToShape();
	MFnDagNode mFnShapeDagNode(shapeDagPath, &mStatus);

	MString XMLString;

	//==========================================
	// The actorString must contain the actor
	// name and category
	//==========================================

	MPlug actorTypePlug = mFnShapeDagNode.findPlug("actorType");
	MString actorType;
	actorTypePlug.getValue(actorType);

	MPlug actorCategoryPlug = mFnShapeDagNode.findPlug("actorCategory");
	MString actorCategory;
	actorCategoryPlug.getValue(actorCategory);

	XMLString += "<actor name=\"" + actorType + "\" category=\"" + actorCategory + "\">";

	//==========================================
	// Write the draw type info to the XML
	// string variable
	//==========================================
	MPlug dimensionsPlug = mFnShapeDagNode.findPlug("dimensions");
	MPlug radiusPlug	 = mFnShapeDagNode.findPlug("radius");
	MPlug namePlug		 = mFnShapeDagNode.findPlug("name");
	MPlug typePlug		 = mFnShapeDagNode.findPlug("type");
	MPlug positionPlug	 = mFnShapeDagNode.findPlug("drawPosition");

	MPlug drawCountPlug	= mFnShapeDagNode.findPlug("drawCount");
	int drawCount;
	drawCountPlug.getValue(drawCount);

	for (int i = 0; i < drawCount; i ++) {

		MPlug currentTypePlug = typePlug.elementByLogicalIndex(i);
		int typeVal;
		currentTypePlug.getValue(typeVal);

		MPlug currentNamePlug = namePlug.elementByLogicalIndex(i);
		MString nameStr;
		currentNamePlug.getValue(nameStr);

		MPlug currentPosPlug = positionPlug.elementByLogicalIndex(i);
		int drawPosVal;
		currentPosPlug.getValue(drawPosVal);

		switch (typeVal) {
			
			case ACTOR_BOX_SHAPE: {

				MPlug currentDimPlug = dimensionsPlug.elementByLogicalIndex(i);
				MPlug xDimPlug = currentDimPlug.child(0);
				MPlug yDimPlug = currentDimPlug.child(1);
				MPlug zDimPlug = currentDimPlug.child(2);

				float xDimVal, yDimVal, zDimVal;
				xDimPlug.getValue(xDimVal);
				yDimPlug.getValue(yDimVal);
				zDimPlug.getValue(zDimVal);

				MString drawPosStr = "Corner";
				if (drawPosVal == 0) drawPosStr = "Centre";
				if (drawPosVal == 1) drawPosStr = "Base";

				XMLString += "<box name=\"" + nameStr + "\"><float name=\"X\" default=\"" + xDimVal + "\"/><float name=\"Y\" default=\"" + yDimVal + "\"/><float name=\"Z\" default=\"" + zDimVal + "\"/><enum name=\"drawPosition\" values=\"drawPosition\" default_by_name=\"" + drawPosStr + "\"/></box>";
				break;
			}

			case ACTOR_SPHERE_SHAPE: {

				MPlug currentRadiusPlug = radiusPlug.elementByLogicalIndex(i);

				float radiusVal;
				currentRadiusPlug.getValue(radiusVal);

				XMLString += "<sphere name=\"" + nameStr + "\"><float name=\"radius\" default=\"" + radiusVal + "\"/></sphere>";
				break;

			}

			case ACTOR_CYLINDER_SHAPE: {

				MPlug currentDimPlug = dimensionsPlug.elementByLogicalIndex(i);
				MPlug currentRadiusPlug = radiusPlug.elementByLogicalIndex(i);
				MPlug yDimPlug = currentDimPlug.child(1);

				float radiusVal, heightVal;
				currentRadiusPlug.getValue(radiusVal);
				yDimPlug.getValue(heightVal);

				MString drawPosStr = "Centre";
				if (drawPosVal == 1) drawPosStr = "Base";

				XMLString += "<cylinder name=\"" + nameStr + "\"><float name=\"radius\" default=\"" + radiusVal + "\"/><float name=\"height\" default=\"" + heightVal + "\"/><enum name=\"drawPosition\" values=\"drawPosition\" default_by_name=\"" + drawPosStr + "\"/></cylinder>";
				break;

			}
		}
	}

	//==========================================
	// Write the dynamic attributes to the XML
	// string variable
	//==========================================
	int numAttributes = mFnShapeDagNode.attributeCount();
	for (int i = 0; i < numAttributes; i ++) {
		
		MObject currentAttr = mFnShapeDagNode.attribute(i);
		MFn::Type type = currentAttr.apiType();

		switch (type) {
		
			case MFn::kNumericAttribute:
			{
				MFnNumericAttribute attrFn(currentAttr);
				if (!attrFn.isDynamic()) continue;

				MString attributeName = attrFn.name();
				MFnNumericData::Type specificType = attrFn.unitType();
				MPlug numAttrPlug = mFnShapeDagNode.findPlug(attributeName);

				switch (specificType) {

					case MFnNumericData::kInt: 
					{
						int val;
						numAttrPlug.getValue(val);
						XMLString += "<int name=\"" + attributeName + "\" default=\"" + val + "\"/>";
						break;
					}
					case MFnNumericData::kFloat:
					{
						// This float may be part of a colour attribute. Therefore we need to do a test
						// to check if it has a parent or not
						if (numAttrPlug.isChild()) {

							// We have a child so get its parent
							MObject parentObj	= attrFn.parent();
							MFnAttribute parentFn(parentObj);
							attributeName		= parentFn.name();
							MPlug parentPlug	= numAttrPlug.parent();

							MObject data;
							float valR, valG, valB, valA = 0.0;
							parentPlug.getValue(data);
							MFnNumericData dataFn(data);
							mStatus = dataFn.getData(valR, valG, valB);
							if (!mStatus) valR = valG = valB = 0.0;

							i += 2;			// Skip the next 2 attributes because they will be the G and B attributes

							// Now this is a bit messy, the alpha attribute is separate from the rgb, but
							// we can almost certainly assume it is the next attribute due to how
							// the actor node is created. This may fail if users manually add attributes
							// to actor nodes (Hopefully will never happen :S) but I've put a simple test
							// in just to be sure
							MObject alphaAttr = mFnShapeDagNode.attribute(i);
							if (alphaAttr.apiType() == MFn::kNumericAttribute) {
								MPlug alphaPlug = mFnShapeDagNode.findPlug(attributeName + "Alpha", &mStatus);
								mStatus = alphaPlug.getValue(valA);
								if (!mStatus) valA = 0.0;
								i ++;		// Now skip this attribute too
							}

							XMLString += "<colour name=\"" + attributeName + "\" default=\"" + valR + " " + valG + " " + valB + " " + valA + "\"/>";
							break;

						} else {

							float val;
							numAttrPlug.getValue(val);
							XMLString += "<float name=\"" + attributeName + "\" default=\"" + val + "\"/>";
							break;
						}
					}
					case MFnNumericData::kBoolean:
					{
						bool val;
						numAttrPlug.getValue(val);
						XMLString += "<bool name=\"" + attributeName + "\" default=\"" + val + "\"/>";
						break;
					}
					case MFnNumericData::kDouble:
					{
						// Anoyingly we get each XYZ component of a vector attribute separately
						// so we make an assumption that the 3 components will be in order and
						// just grab the parent of the X, get the data and increament i to
						// skip the Y and Z components
						if (numAttrPlug.isChild()) {

							MObject parent = attrFn.parent();
							MFnAttribute parentFn(parent);
							attributeName = parentFn.name();
							MPlug parentPlug = numAttrPlug.parent();
							MObject data;
							double valX, valY, valZ;
							parentPlug.getValue(data);
							MFnNumericData dataFn(data);
							dataFn.getData(valX, valY, valZ);
							XMLString += "<vector name=\"" + attributeName + "\" default=\"" + valX + " " + valY + " " + valZ + "\"/>";
							i += 2;
							break;

						} else {

							double val;
							numAttrPlug.getValue(val);
							XMLString += "<float name=\"" + attributeName + "\" default=\"" + val + "\"/>";
							break;

						}
						
					}
				}
				break;
			}

			case MFn::kTypedAttribute:
			{
				MFnTypedAttribute attrFn(currentAttr);
				if (!attrFn.isDynamic()) continue;
				
				MString attributeName = attrFn.name();
				MPlug typedAttrPlug = mFnShapeDagNode.findPlug(attributeName);
				MString val;
				typedAttrPlug.getValue(val);
				XMLString += "<string name=\"" + attributeName + "\" default=\"" + val + "\"/>";

				break;
			}

			case MFn::kEnumAttribute:
			{
				MFnEnumAttribute attrFn(currentAttr);
				if (!attrFn.isDynamic()) continue;
				
				MString attributeName = attrFn.name();
				// The enum "values" data is stored as the attributes shortName
				MString enumValues    = attrFn.shortName();
				MPlug enumAttrPlug    = mFnShapeDagNode.findPlug(attributeName);
				int val;
				enumAttrPlug.getValue(val);
				MString fieldName	  = attrFn.fieldName(val);

				XMLString += "<enum name=\"" + attributeName + "\" values=\"" + enumValues + "\" default=\"" + fieldName + "\"/>";
				break;
			}
		}
	}

	//==========================================
	// Write the links to the XML string
	//==========================================
	MPlug linkCountPlug	= mFnShapeDagNode.findPlug("linkCount");
	int linkCount;
	linkCountPlug.getValue(linkCount);

	MPlug linkPlug	= mFnShapeDagNode.findPlug("link");
    
	for (int i = 0; i < linkCount; i ++) {

		MPlug currentLinkPlug = linkPlug.elementByLogicalIndex(i);
		MPlug currentLinkNamePlug  = currentLinkPlug.child(0);
		MPlug currentLinkValuePlug = currentLinkPlug.child(1);

        MString linkName, linkValue;

		currentLinkNamePlug.getValue(linkName);
		currentLinkValuePlug.getValue(linkValue);

		XMLString += "<link name=\"" + linkName + "\" default=\"" + linkValue + "\"/>";
	}
	
	XMLString += "</actor>";

	//Add actor data as dynamic attribute.
	Aftk::C_DynamicPlug* pDynamicPlug = new Aftk::C_DynamicPlug();
	pAftkSceneNode->AddPlugPtr(pDynamicPlug);
	pDynamicPlug->SetName("actorString2");
	pDynamicPlug->SetValue(std::string(XMLString.asChar()));
	
	return Aftk::C_Status::E_Status_Success;
}

