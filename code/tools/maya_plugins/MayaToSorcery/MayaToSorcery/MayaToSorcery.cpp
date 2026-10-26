///////////////////////////////////////////////////////////////////////////////
// Copyright © Bizarre Creations 2006
//
// File:        MayaToSorcery.cpp
//
// Created:     07/11/06
// Author:      Michael Bibby
///////////////////////////////////////////////////////////////////////////////

#include "MayaToSorcery.h"

XT_APPLICATION
XT_END_APPLICATION

MString MayaToSorcery::typeName = "mayaToSorcery";

//************************************************************
// Argument syntax
//
// -gdl/getDefinitionList: Return the list of defintions for the 
//						   requested type (xtField) (packOverride) (includeID)
//
// -ado/addDynamicObject:  Add the dynamic object to sorcery (recordName)
//
//************************************************************
const char *getDefListFlag    = "-gdl", *getDefListFlagLong    = "-getDefinitionList";
const char *addDynamicObjFlag = "-ado", *addDynamicObjFlagLong = "-addDynamicObject";

MSyntax MayaToSorcery::newSyntax() {

	MSyntax syntax;

	syntax.addFlag(getDefListFlag,		getDefListFlagLong,		MSyntax::kString,
																MSyntax::kString,
																MSyntax::kBoolean);
	syntax.addFlag(addDynamicObjFlag,	addDynamicObjFlagLong,  MSyntax::kString);

	return syntax;
}

//************************************************************
// Command functions
//************************************************************

//////////////////////////////////////////////////////////////
// Do it function
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::doIt(const MArgList &args) {

	MStatus stat;

	MString command = ";if (`window -exists MayaToSorceryPleaseWaitWindow`) deleteUI MayaToSorceryPleaseWaitWindow;";

	// Ensure the ID is set to 0
	m_id = 0;

	MArgDatabase argData( syntax(), args );

	// Grab the project name from the prefs
	MCommandResult result;
	MGlobal::executeCommand("optionVar -q biz_TM_currentProject", result, false, false);
	stat = result.getResult(m_currentProject);
	CHECKSTATUS(stat, "getting command result");

	// Set variables up based on project
	if (m_currentProject == "project_nightwing") {
	
		m_databasePath		= "z:\\nightwing\\sorcery3\\main";
		m_recordTypeName    = "ElementObjectRecord";
		m_damageLevelName   = "PhysicalObjectElementDamageLevel";
		m_elementName		= "PhysicalObjectElement";
		m_objectName        = "PhysicalObject";
		m_pack		        = "Physical_Objects";

	} else if (m_currentProject == "project_amax") {

		m_databasePath		= "z:\\amax\\sorcery3\\main";
		m_recordTypeName    = "ElementObjectRecord";
		m_damageLevelName   = "PhysicalObjectElementDamageLevel";
		m_elementName		= "PhysicalObjectElement";
		m_objectName        = "PhysicalObject";
		m_pack		        = "Amax";

	} else {

		MGlobal::displayError("Your current project is not supported");
		MGlobal::executeCommand(command);
		return MS::kFailure;

	}

	// Open the database
	stat = openDatabase();
	if (!stat) {
		MGlobal::executeCommand(command);
		return MS::kFailure;
	}

	///////////////////////////////////////
	// addDynamicObjFlag
	///////////////////////////////////////
	if (argData.isFlagSet(addDynamicObjFlag)) {

		MString recordName;

		// Get the flag arguments
		stat = argData.getFlagArgument(addDynamicObjFlag, 0, recordName);
		if (!stat || recordName == "") {
			MGlobal::displayError("No record name given");
			MGlobal::executeCommand(command);
			return MS::kFailure;	
		}

		// If the record already exists warn the user
		if (recordExists(recordName.asChar())) {

			command += "warning \"Record already exists! Currently editing of existing records is not implemented. Please delete the existing record and retry or manually edit it from within Sorcery\"";

		// If the record is not found, create a new one
		} else {

			stat = addPhysicalObject(recordName);
			if (!stat) 
				command += "error \"Problem adding dynamic object. See script window for details.\"";
			
			else {
				
				command += "confirmDialog -title \"Record Created!\" -b \"Close\" -ma \"center\" -message \"";
				command += recordName;
				command += " successfully created.\";";
			}
		}

	///////////////////////////////////////
	// getDefListFlag
	///////////////////////////////////////
	} else if (argData.isFlagSet(getDefListFlag)) {

		bool includeId;
		MString xtField;
		MString packOverride;
		MStringArray definitionList;

		// Get the flag arguments
		stat = argData.getFlagArgument(getDefListFlag, 0, xtField);
		if (!stat || xtField == "") {
			MGlobal::displayError("No xt field name given");
			MGlobal::executeCommand(command);
			return MS::kFailure;	
		}

		stat = argData.getFlagArgument(getDefListFlag, 1, packOverride);
		if (!stat) {
			MGlobal::executeCommand(command);
			return MS::kFailure;
		}

		// Set the global pack
		if (packOverride != "")
			m_pack = packOverride.asChar();

		stat = argData.getFlagArgument(getDefListFlag, 2, includeId);
		CHECKSTATUS(stat, "getting includeId");	

		// Get the list of definitions
		stat = getDefinitionList(&definitionList, xtField, includeId);
		CHECKSTATUS(stat, "getting definition list");

		setResult(definitionList);

	}

	closeDatabase();
	MGlobal::executeCommand(command);

	return MS::kSuccess;

}

//////////////////////////////////////////////////////////////
// See if the given record exists in the database
//////////////////////////////////////////////////////////////
bool MayaToSorcery::recordExists(const char* name) {

	bool recordFound = false;
	MStatus stat;

	// Find main pack file
	Xt::DomPack* pack = findPack(m_pack, &stat);
	if (!stat) return false;

	// Iterate over every record in the pack
	for (Xt::DomPack::Iterator it(pack); it; ++it) {

		Xt::DomRecord* record = it;

		// Find the record type in the schema
		LarvaSchema* schema		= getSchema();
		LtStruct*	 recordType	= TypeOf(schema, record);

		// Only support references to structures
		if (!recordType) continue;

		if (recordType->name == m_recordTypeName) {

			if (record->name == name) {

				// Ask if the user wants to delete the record
				MString command = "confirmDialog -t \"Record Already Exists\" -m \"The record already exists in Sorcery.\\n\\nDo you want to overwrite the existing record?\"	\
								  -ma \"center\" -b \"Overwrite\" -b \"Cancel\"";

				MCommandResult result;
				MGlobal::executeCommand(command, result, false, false);
				MString resultStr;
				result.getResult(resultStr);
				
				// If they do want to delete it, store the id of
				// the existing record.
				if (resultStr == "Overwrite") {

					m_id = record->id;
					return false;

				} else
					return true;
			}
		}
	}

	return false;

}

//////////////////////////////////////////////////////////////
// Return the given record if it exists
//////////////////////////////////////////////////////////////
Xt::DomRecord* MayaToSorcery::getRecord(const char* name, const char* type) {

	MStatus stat;

	// Find main pack file
	Xt::DomPack* pack = findPack(m_pack, &stat);
	if (!stat) return 0;

	// Iterate over every record in the pack
	for (Xt::DomPack::Iterator it(pack); it; ++it) {

		Xt::DomRecord* record = it;

		// Find the record type in the schema
		LarvaSchema* schema		= getSchema();
		LtStruct*	 recordType	= TypeOf(schema, record);

		// Only support references to structures
		if (!recordType)
			continue;

		if (record->name == name && recordType->name == type) {
			return record;
		}
	}

	return 0;

}

//////////////////////////////////////////////////////////////
// Return the given record from the given ID if it exists
//////////////////////////////////////////////////////////////
Xt::DomRecord* MayaToSorcery::getRecord(XtId id) {

	MStatus stat;

	// Find main pack file
	Xt::DomPack* pack = findPack(m_pack, &stat);
	if (!stat) return 0;

	// Iterate over every record in the pack
	for (Xt::DomPack::Iterator it(pack); it; ++it) {

		Xt::DomRecord* record = it;

		// Find the record type in the schema
		LarvaSchema* schema		= getSchema();
		LtStruct*	 recordType	= TypeOf(schema, record);

		// Only support references to structures
		if (!recordType)
			continue;

		if (record->id == id) {
			return record;
		}
	}

	return 0;

}

//////////////////////////////////////////////////////////////
// Create a new record and return a pointer to it
//////////////////////////////////////////////////////////////
Xt::DomRecord* MayaToSorcery::createNewRecord(const char* name) {

	MStatus status;

	// Find main pack file
	Xt::DomPack* pack = findPack(m_pack, &status);

	// Create the new record
	if (m_id == 0)
	{
		Xt::DomRecord* newRecord = Xt::NewRecord(pack, name, m_recordTypeName);

		m_id = newRecord->id;

		return newRecord;
	}

	Xt::DomRecord* record = Xt::FindRecordById(&m_database, m_id);

	if (!record)
		return 0;

	Xt::Clear(&record->root.data);

	return record;
}

//////////////////////////////////////////////////////////////
// Add a field to the given block
//////////////////////////////////////////////////////////////
Xt::DomField* MayaToSorcery::addField(Xt::DomBlock* block, const char* key, const char* value) {

	const XtString xtKey = key;
	const XtString xtValue = value;
	Xt::DomField* returnField = Xt::Set(block, xtKey, xtValue);

	return returnField;

}

//////////////////////////////////////////////////////////////
// Add a component to the given array
//////////////////////////////////////////////////////////////
Xt::DomComponent* MayaToSorcery::addComponent(Xt::DomArray* domArray) {

	Xt::DomComponent* component = Xt::Append(domArray, new Xt::DomComponent());
	component->id = Xt::GenerateUniqueComponentId();

	return component;

}

//////////////////////////////////////////////////////////////
// Add a component to the given block with the given type
//////////////////////////////////////////////////////////////
Xt::DomComponent* MayaToSorcery::addComponent(Xt::DomArray* domArray, XtString type) {

	Xt::DomComponent* component = Xt::Append(domArray, new Xt::DomComponent());
	component->id   = Xt::GenerateUniqueComponentId();
	component->type = type;

	// TODO
	// Move this so that it doesn't do a look up each time...?
	LtStruct* xtType = AsStruct(FindTypeByName(m_database.schema, type));

	if (xtType)
	{
		SetDefaults(m_database.schema, xtType, component);
	}

	return component;

}

//////////////////////////////////////////////////////////////
// Add a ref to the given array
//////////////////////////////////////////////////////////////
Xt::DomRef* MayaToSorcery::addRef(Xt::DomArray* domArray, XtId id) {

	Xt::DomRef* newRef = Xt::Append(domArray, new Xt::DomRef());
	newRef->value = id;

	return newRef;

}

//////////////////////////////////////////////////////////////
// Add a ref to the given block with the given name
//////////////////////////////////////////////////////////////
Xt::DomRef* MayaToSorcery::addRef(Xt::DomBlock* domBlock, const char* key, XtId id) {

	const XtString xtKey = key;
	Xt::DomRef* newRef = Xt::Set(domBlock, xtKey, id);

	return newRef;

}

//////////////////////////////////////////////////////////////
// Get definition list function
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::getDefinitionList(MStringArray *definitionList, MString xtField, bool includeId) {

	MStatus stat;

	// Find main pack file
	Xt::DomPack* pack = findPack(m_pack, &stat);
	if (!stat) {
		closeDatabase();
		return MS::kFailure;
	}

	// Iterate over every record in the pack
	for (Xt::DomPack::Iterator it(pack); it; ++it) {
		
		Xt::DomRecord* record = it;

		// Find the record type in the schema
		LarvaSchema* schema		= getSchema();
		LtStruct*	 recordType	= TypeOf(schema, record);

		// Only support references to structures
		if (!recordType)
			continue;

		if(recordType->name == xtField.asChar()) {

			XtString definitionXt = record->name;
			XtId id = record->id;

			// Add the record name
			stat = definitionList->append((const char*)definitionXt);
			if (!stat) {
				closeDatabase();
				return MS::kFailure;
			}

			// Add the record id if required
			if (includeId) {

				MString idStr = "";

				// Convert the int to hex before adding 
				idStr += Format("%08X", id);
				stat = definitionList->append(MString(idStr));
				if (!stat) {
					closeDatabase();
					return MS::kFailure;
				}

			}
		}
	}

	closeDatabase();

	return MS::kSuccess;
}

//////////////////////////////////////////////////////////////
// Add dynamic object function
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::addPhysicalObject(MString objectName) {

	MStatus stat;

	// Create a new record in the database
	Xt::DomRecord* newRecord = createNewRecord(objectName.asChar());
	
	// Now strip down the name to remove folder structure
	stat = stripObjectPath(&objectName);
	CHECKSTATUS(stat, "stripping object path");

	// Grab the scene root objects
	MDagPathArray rootObjects;
	MItDag dagIter(MItDag::kBreadthFirst, MFn::kTransform, &stat);
	CHECKSTATUS(stat, "creating iterator");
	for (; !dagIter.isDone(); dagIter.next()) {

		MDagPath thisObject;
		stat = dagIter.getPath(thisObject);
		CHECKSTATUS(stat, "getting object path");
		if (isObject(thisObject) && thisObject.pathCount() == 1)
			rootObjects.append(thisObject);
	}

	// Create the root arrays
	m_damageLevelsArray   = Xt::AcquireArray(&newRecord->root.data, "damageLevels");
	m_elementsArray		  = Xt::AcquireArray(&newRecord->root.data, "objectElements");
	m_objectsArray		  = Xt::AcquireArray(&newRecord->root.data, "objects");

	// Reset positions
	m_xPos = 0;
	m_yPos = 0;

	// Do this for every root object in the scene
	for (unsigned int i = 0; i < rootObjects.length(); i ++) {

		stat = recursePhysicalObject(rootObjects[i], objectName, NULL);
		CHECKSTATUS(stat, "recursing Physical Object");

		// NOTE: multiple root nodes currently aren't supported 
		// by the PhysicalObject system in game, hence breaking
		// after the first one...
		break;

	}

	// Set the first object as the root
	Xt::DomComponent* rootObjectComp = Xt::AcquireComponent(m_objectsArray, 0);
	addRef(&newRecord->root.data, "rootObject", rootObjectComp->id);

	// Check the first object for a model def ID
	MFnDagNode rootNodeFn(rootObjects[0]);
	MPlug modeDefPlug = rootNodeFn.findPlug("xt_ModelDef", &stat);
	if (stat) {

		MString modelDefStr = modeDefPlug.asString();
		addField(&newRecord->root.data, "Model", modelDefStr.asChar());

	}

	// Add any root object specific values to the root object element
	setNodeXtAttributes(rootObjects[0], rootObjectComp);

	// Store the new SorceryID on the first object as a dynamic attribute
	stat = saveDataOnObject(newRecord->id, "physicalObjectID", "poid", rootObjects[0]);
	CHECKSTATUS(stat, "adding sorcery ID to root object");

	// Finally send the new record to the game
	stat = sendRecordToGame(newRecord->id);
	CHECKSTATUS(stat, "sending record to game");

	return MS::kSuccess;

}

//////////////////////////////////////////////////////////////
// Recursively create a phsyicalObject type in the Xt database
//////////////////////////////////////////////////////////////
MStatus	MayaToSorcery::recursePhysicalObject(MDagPath object, MString objectName, Xt::DomArray* parentArray) {

	MStatus stat;

	// Create New object component in the objects array
	Xt::DomComponent* objectComponent = addComponent(m_objectsArray, m_objectName);

	// Also create a new element component
	Xt::DomComponent* elementComponent = addComponent(m_elementsArray, m_elementName);

	// Add a component to the damage levels arary
	Xt::DomComponent* damageLevelsComponent = addComponent(m_damageLevelsArray, m_damageLevelName);

	// Add the name of the object and element
	MFnDagNode nodeFn(object);
	addField(&objectComponent->data,		"Name",				nodeFn.name().asChar());
	addField(&elementComponent->data,		"Name",				nodeFn.name().asChar());
	addField(&damageLevelsComponent->data,	"ModelElementName", nodeFn.name().asChar());

	// Attempt to position the parts of the object
	setNodeGraphPosition(objectComponent, true, false);
	setNodeGraphPosition(elementComponent, true);
	setNodeGraphPosition(damageLevelsComponent, true, true);

	// Add a new array to the object component called Elements
	Xt::DomArray* elementsArray = Xt::AcquireArray(&objectComponent->data, "Elements");

	// Add arrays to the element component
	Xt::DomArray* damageLevelsArray = Xt::AcquireArray(&elementComponent->data, "DamageLevels");
	Xt::DomArray* childrenArray		= Xt::AcquireArray(&elementComponent->data, "Children");

	// Add arrays to the damage level
	Xt::DomArray* debrisArray  = Xt::AcquireArray(&damageLevelsComponent->data, "DebrisParts");
	Xt::DomArray* shatterArray = Xt::AcquireArray(&damageLevelsComponent->data, "ShatterReplacements");

	// Link the object with the element
	addRef(elementsArray, elementComponent->id);

	// Link the damage level to the element
	addRef(damageLevelsArray, damageLevelsComponent->id);

	// If the parentComponent is not null, link this object back to it
	if (parentArray != NULL)
		addRef(parentArray, objectComponent->id);

	float currentX = m_xPos - WIDTH_INCREMENT;

	// Check for any xt_ attributes and add them as required
	stat = setNodeXtAttributes(object, objectComponent);
	CHECKSTATUS(stat, "adding xt_attributes to physical object");
	stat = setNodeXtAttributes(object, damageLevelsComponent);
	CHECKSTATUS(stat, "adding xt_attributes to damage level");

	// Add all additional damage levels
	stat = recurseDamageLevel(object, damageLevelsArray);
	CHECKSTATUS(stat, "recursing damage levels");

	// Set it back for the children to fall in line with the damage levels
	m_xPos = currentX;

	// Add all children
	stat = recurseChildren(object, childrenArray);
	CHECKSTATUS(stat, "recursing children");

	return MS::kSuccess;
}

//////////////////////////////////////////////////////////////
// Recursively create a damage level
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::recurseDamageLevel(MDagPath object, Xt::DomArray* damageLevelsArray) {

	MStatus stat;

	Xt::DomArray* debrisArray  = 0;
	Xt::DomArray* shatterArray = 0;
	Xt::DomComponent* damageLevelsComponent = 0;
	float currentY = m_yPos - HEIGHT_INCREMENT;
	float currentX = m_xPos;

	// Grab the ref to the damage level component
	Xt::DomRef* damageLevelsRef = Xt::AcquireRef(damageLevelsArray, 0);
	
	// Search for the component in the global damage levels array
	damageLevelsComponent = findComponentMatchingID(damageLevelsRef->value, m_damageLevelsArray);

	// If not found, abort this damage level
	if (damageLevelsComponent == NULL) {

		MGlobal::displayError("Could not find damage level for " + object.fullPathName());
		return MS::kFailure;

	// Otherwise grab the array pointers required
	} else {

		debrisArray  = Xt::AcquireArray(&damageLevelsComponent->data, "DebrisParts");
		shatterArray = Xt::AcquireArray(&damageLevelsComponent->data, "ShatterReplacements");

	}

	// Look for replacements
	MDagPathArray replacementsArray;
	stat = getReplacements(object, &replacementsArray);
	CHECKSTATUS(stat, "getting replacements");

	// For every replacement found, consider whether it is 
	// a damage level, debris or replacement part.
	if (replacementsArray.length() > 0) {

		for (unsigned int i = 0; i < replacementsArray.length(); i++) {

			MFnDagNode currentNodeFn(replacementsArray[i]);

			// Check if its a damage level
			if (isDamageLevel(replacementsArray[i])) {
			
				// Add a component to the global damage levels arary
				Xt::DomComponent* newDamageLevelsComponent = addComponent(m_damageLevelsArray, m_damageLevelName);
	
				// Name the damage level
				addField(&newDamageLevelsComponent->data,	"ModelElementName", currentNodeFn.name().asChar());

				// Link the damage level to the element
				addRef(damageLevelsArray, newDamageLevelsComponent->id);

				// Set the position of the damage level
				m_xPos -= WIDTH_INCREMENT;
				setNodeGraphPosition(newDamageLevelsComponent, true, true);

				// Check for any xt_ attributes and add them as required
				stat = setNodeXtAttributes(replacementsArray[i], newDamageLevelsComponent);
				CHECKSTATUS(stat, "adding xt_attributes");

				// Recurse this damage level
				stat = recurseDamageLevel(replacementsArray[i], damageLevelsArray);
				CHECKSTATUS(stat, "recursing damage level");

			// Check if its debris
			} else if (isDebris(replacementsArray[i]) && m_currentProject == "project_nightwing") {

				// Add a new component to the debris parts array
				Xt::DomComponent* currentDebrisPart = addComponent(debrisArray);
				addField(&currentDebrisPart->data, "ModelElementName", currentNodeFn.name().asChar());

			// Otherwise it must be a replacement
			} else  {

				//recursePhysicalObject(replacementsArray[i], "", shatterArray);
				//Xt::DomArray* shatterArray2 = Xt::AcquireArray(&damageLevelsComponent->data, "ShatterReplacements");

				//MGlobal::displayInfo("OK I'm here!!! " + replacementsArray[i].fullPathName());

				// Reset the y pos if this is the first replacement
				// otherwise reset the x
				size_t size = Xt::Count(shatterArray);
				if (size == 0)
					m_yPos = currentY;
				else
					m_xPos = currentX;

				// Recurse the physical object
				recursePhysicalObject(replacementsArray[i], "", shatterArray);
			}
		}
	}

	return MS::kSuccess;

}

//////////////////////////////////////////////////////////////
// Recursively create a child
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::recurseChildren(MDagPath object, Xt::DomArray* currentArray) {

	MStatus stat;

	// Look for child objects
	MDagPathArray objectsArray;
	stat = getObjects(object, &objectsArray);
	CHECKSTATUS(stat, "getting child objects");

	// Store the initial X
	float currentX = m_xPos;

	for (unsigned int i = 0; i < objectsArray.length(); i++) {
		
		// reset the x pos
		m_xPos = currentX;

		// Recurse the child
		recursePhysicalObject(objectsArray[i], "", currentArray);

	}

	return MS::kSuccess;
}

//////////////////////////////////////////////////////////////
// Filthy Filthy
//////////////////////////////////////////////////////////////
volatile bool gCanContinue = false;

void ConProc(ArteryConnection con, void* context)
{
	MayaToSorcery* self = (MayaToSorcery*)context;

	Xt::DomRecord* record = self->getRecord(self->m_networkSendId);

	MString out = "Sending to game: ";
	out += record->name;
	MGlobal::displayInfo(out);

	self->SendNetworkSub(con, &self->m_database, &record, 1);

	gCanContinue = true;
	ArteryWaitContext ctx;
	while(con.IsConnected())
	{
		con.Wait(ctx);
	}
}

//////////////////////////////////////////////////////////////
// Update the give record on the devkit via an artery connection
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::sendRecordToGame(XtId recordId) {

	MStatus stat;

	m_networkSendId = recordId;

	// Get the record of the new settings and obtain its ID
	//Xt::DomRecord* newSettings = getRecord(newSettingsName, "LightingSettings");
	//char idstr[32] = {0};
	//sprintf(idstr, "%x", recordId);

	// Check that the level record has been obtained
	//if (!m_level) return MS::kFailure;

	// Now set the level's lighting settings
	//Xt::DomField* settingsField = Xt::AcquireField(&m_level->root, "LightingSettings");
	//Xt::Set(settingsField, idstr);

	if (ArteryStartup(false, "MayaToSorcery"))
	{
		// to get targetaddr
		DWORD dwAddr;
		if (FAILED(DmGetAltAddress(&dwAddr)))
			return MS::kFailure;
		
		WsSockAddr targetAddr = WsSockAddr(dwAddr, 6317);

		ArteryConnection con;

		gCanContinue = false;
		int iCounter = 0;

		if (ArteryConnect(con, targetAddr, "Sorcery", ConProc, this))
		{
			while(!gCanContinue)
			{
				Sleep(1);
				iCounter ++;
				if (iCounter >= 50) {
					MGlobal::displayError("Unable to connect to devkit. Operation Aborted");
					ArteryCleanup();
					return MS::kFailure;
				}
			}
		}

		ArteryCleanup();
	}

	return MS::kSuccess;

}

struct MtsBuildParams : public XtPackBuildParams
{
	virtual void Output(XtSeverity severity, const char* text)
	{
		XtString text2 = XtString("Build Error: ") + text;

		MGlobal::displayInfo(text2.Text());
	}
};

void MayaToSorcery::SendNetworkSub(ArteryConnection con, Xt::DomDb* db, Xt::DomRecord** records, int count) {

	XtUt::Block data;

	XtTargetProfile p;
	MtsBuildParams params;

	p.endian = kXtBigEndian;

	if (!XtPackBuild(&p, db, records, count, &data, &params))
	{
		MGlobal::displayInfo("Failed to build pack!");
		return;
	}
	
	ArteryMessageBuilder b;

	if (!b.Write(data.start, (int)Capacity(&data)))
	{
		MGlobal::displayInfo("Failed to write pack!");
		return;
	}

	ArteryMessage msg;

	if (!b.Build(msg))
	{
		MGlobal::displayInfo("Failed to build message!");
		return;
	}

	if (!con.Send(msg))
	{
		MGlobal::displayInfo("Failed to send message!");
	}
}

//************************************************************
// Helper functions
//************************************************************


//////////////////////////////////////////////////////////////
// Get the current scehema
//////////////////////////////////////////////////////////////
LarvaSchema* MayaToSorcery::getSchema() {
	return m_database.schema;
}

//////////////////////////////////////////////////////////////
// Find the given pack file
//////////////////////////////////////////////////////////////
Xt::DomPack* MayaToSorcery::findPack(const char* name, MStatus *stat) {

	Xt::DomPack* pack = 0;
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&m_database.packs); it; ++it) {

		if (_stricmp(it->name, name) == 0) {
			pack = it;
			break;
		}
	}
	
	if (!pack) *stat = MS::kFailure;
	else *stat = MS::kSuccess;

	return pack;
}

//////////////////////////////////////////////////////////////
// Open the database
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::openDatabase() {
	
	MtsStatus sc;
	Xt::OpenResult openResult = Xt::Open(&m_database, m_databasePath.asChar(), &m_storage, &sc);

	if (openResult == Xt::kOpenLocked) {
		MGlobal::displayError("Sorcery database is locked. Close Sorcery and ActorStudio if they are already running");
		return MS::kFailure;
	}

	if (openResult == Xt::kOpenSuccess) {
		MGlobal::displayInfo("Successfully connected to database: " + m_databasePath);
		return MS::kSuccess;
	}

	// Something must have gone wrong other than a locked database
	MGlobal::displayError("Failed to connect to sorcery database: " + m_databasePath);
	return MS::kFailure;
}

//////////////////////////////////////////////////////////////
// Close the database
//////////////////////////////////////////////////////////////
void MayaToSorcery::closeDatabase() {
	Xt::StatusCallback sc;
	Xt::Close(&m_database, &m_storage, &sc);
}

//////////////////////////////////////////////////////////////
// Recurse the object and return a list of child objects
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::getObjects(MDagPath object, MDagPathArray* objects) {

	MStatus stat = MS::kSuccess;

	int childCount = object.childCount(&stat);
	if (!stat) return MS::kFailure;

	for (int i = 0; i < childCount; i ++) {

		MObject child = object.child(i, &stat);
		if (!stat) return MS::kFailure;

		// Get a dag path to this object
		MFnDagNode childFn(child);
		MDagPath childPath;
		stat = childFn.getPath(childPath);
		if (!stat) return MS::kFailure;

		// Only allow transforms through, skipping shapes
		if (childFn.object().apiType() != MFn::kTransform) continue;

		// Check if this transform is an element
		if (isObject(childPath)) {

			stat = objects->append(childPath);
			if (!stat) return MS::kFailure;

		// Otherwise recurse the transform in case it has elements as children
		} else if (!isReplacement(childPath))
			getObjects(childPath, objects);

	}

	return MS::kSuccess;

}

//////////////////////////////////////////////////////////////
// Return a list of replacement objects
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::getReplacements(MDagPath object, MDagPathArray* replacements) {

	MStatus stat = MS::kSuccess;

	int childCount = object.childCount(&stat);
	if (!stat) return MS::kFailure;

	for (int i = 0; i < childCount; i ++) {

		MObject child = object.child(i, &stat);
		if (!stat) return MS::kFailure;

		// Get a dag path to this object
		MFnDagNode childFn(child);
		MDagPath childPath;
		stat = childFn.getPath(childPath);
		if (!stat) return MS::kFailure;

		// Only allow transforms through, skipping shapes
		if (childFn.object().apiType() != MFn::kTransform) continue;

		// Check if this transform is an element
		if (isReplacement(childPath)) {

			stat = replacements->append(childPath);
			if (!stat) return MS::kFailure;

		// Otherwise if its not an object recurse the 
		// transform in case it has replacements as children
		} else if (!isObject(childPath))
			getReplacements(childPath, replacements);
	}

	return MS::kSuccess;

}

//////////////////////////////////////////////////////////////
// Return a list of effect objects
//////////////////////////////////////////////////////////////
MStatus	MayaToSorcery::getEffects(MDagPath object, MDagPathArray* effects) {

	MStatus stat = MS::kSuccess;

	int childCount = object.childCount(&stat);
	if (!stat) return MS::kFailure;

	for (int i = 0; i < childCount; i ++) {

		MObject child = object.child(i, &stat);
		if (!stat) return MS::kFailure;

		// Get a dag path to this object
		MFnDagNode childFn(child);
		MDagPath childPath;
		stat = childFn.getPath(childPath);
		if (!stat) return MS::kFailure;

		// Only allow transforms through, skipping shapes
		if (childFn.object().apiType() != MFn::kTransform) continue;

		// Check if this transform is an element
		if (isEffect(childPath)) {

			stat = effects->append(childPath);
			if (!stat) return MS::kFailure;

		// Otherwise if its not an object recurse the 
		// transform in case it has effects as children
		} else if (!isObject(childPath) && !isReplacement(childPath))
			getEffects(childPath, effects);
	}

	return MS::kSuccess;

}

//////////////////////////////////////////////////////////////
// Check if the object is a game object
//////////////////////////////////////////////////////////////
bool MayaToSorcery::isObject(MDagPath object) {

	MFnDependencyNode nodeFn(object.node());
	MString name = nodeFn.name();
	name = name.substring(0, 1);
	name = name.toLowerCase();
	if (name == "o_") return true;
	else return false;

}

//////////////////////////////////////////////////////////////
// Check if the object is a replacement object
//////////////////////////////////////////////////////////////
bool MayaToSorcery::isReplacement(MDagPath object) {

	MFnDependencyNode nodeFn(object.node());
	MString name = nodeFn.name();
	name = name.substring(0, 1);
	name = name.toLowerCase();
	if (name == "s_") return true;
	else return false;

}

//////////////////////////////////////////////////////////////
// Check if the object is a effect object
//////////////////////////////////////////////////////////////
bool MayaToSorcery::isEffect(MDagPath object) {

	MFnDependencyNode nodeFn(object.node());
	MString name = nodeFn.name();
	name = name.substring(0, 2);
	name = name.toLowerCase();
	if (name == "fx_") return true;
	else return false;

}

//////////////////////////////////////////////////////////////
// Check if the object is a damage level
//////////////////////////////////////////////////////////////
bool MayaToSorcery::isDamageLevel(MDagPath object) {

	MStatus stat;

	// First check to see that this is in fact a replacement part
	if (!isReplacement(object)) return false;

	// Check to see if this replacement is the first child of its parent
	// which is the condition required for it to be a damage level
	MString path = object.fullPathName();
	stat = object.pop();
	if (!stat) {
		MGlobal::displayError("Popping path");	
		return false;
	}
	
	// Keep popping until the actual parent is found
	while (!isReplacement(object) && !isObject(object) && object.length() > 0)
		object.pop();

	MDagPathArray children, replacements;
	stat = getReplacements(object, &replacements);
	if (!stat) {
		MGlobal::displayError("Getting replacements");	
		return false;
	}

	// If the object is the first child it is a damage level
	if (path == replacements[0].fullPathName())
		return true;

	return false;
}

//////////////////////////////////////////////////////////////
// Check if the object is a debris element
//////////////////////////////////////////////////////////////
bool MayaToSorcery::isDebris(MDagPath object) {

	MStatus stat;

	// First check to see that this is in fact a replacement part
	if (!isReplacement(object)) return false;

	// Now check for any replacement models
	MDagPathArray replacements;
	stat = getReplacements(object, &replacements);
	if (!stat) {
		MGlobal::displayError("Getting replacements");	
		return false;
	}

	if (replacements.length())
		return false;

	// The last check is to count the number of sibling replacements and 
	// see if this is the first replacement part of an object.
	if (isDamageLevel(object)) return false;

	return true;
}

//////////////////////////////////////////////////////////////
// Check if the given string shoul dbe treated as an ID
//////////////////////////////////////////////////////////////
bool MayaToSorcery::treatAsHexID(MString value, XtId &id) {

	char* end = 0;
	id = strtol(value.asChar(), &end, 16);

	int length = end - value.asChar();
	if (length == value.length()) return true;

	return false;
}

//////////////////////////////////////////////////////////////
// Set the node graph position parameters of the given node
//////////////////////////////////////////////////////////////
void MayaToSorcery::setNodeGraphPosition(Xt::DomComponent* component, bool incrementX, bool incrementY) {

	MString xPos, yPos;
	xPos += m_xPos;
	yPos += m_yPos;
	addField(&component->data, "__x", xPos.asChar());
	addField(&component->data, "__y",yPos.asChar());

	// Increment positions if required
	if (incrementX) m_xPos += WIDTH_INCREMENT;
	if (incrementY) m_yPos += HEIGHT_INCREMENT;

}

//////////////////////////////////////////////////////////////
// Save the given data on the given object
//////////////////////////////////////////////////////////////
MStatus	MayaToSorcery::saveDataOnObject(XtId id, MString longName, MString shortName, MDagPath object) {

	MStatus stat = MS::kSuccess;

	MFnDagNode nodeFn(object);
	
	MFnTypedAttribute tAttr;
	
	// First look for the given attribute on the object
	MPlug attributePlug = nodeFn.findPlug(longName, &stat);
	if (!stat) {;

		// Create the attribute
		MObject newAttr = tAttr.create(longName, shortName, MFnData::kString, &stat);
		if (!stat) return stat;

		stat = nodeFn.addAttribute(newAttr);
		if (!stat) return stat;

		attributePlug = nodeFn.findPlug(newAttr, &stat);
		if (!stat) return stat;
	
	}

	// Set the ID
	MString value = "";
	value += Format("%08X", id);
	stat = attributePlug.setString(value);
	if (!stat) return stat;

	return stat;
}

//////////////////////////////////////////////////////////////
// Remove all parts of a record name before the final item name
// i.e. Objects.Furniture.Manor2.piano becomes piano
//////////////////////////////////////////////////////////////
MStatus	MayaToSorcery::stripObjectPath(MString* objectName) {

	MStatus stat;
	MStringArray tokens;

	// Split the name by '.' delimeters
	stat = objectName->split('.', tokens);
	if (!stat) return MS::kFailure;

	*objectName = tokens[tokens.length() - 1];
	return MS::kSuccess;

}

//////////////////////////////////////////////////////////////
// Find the component in the array matching the given id
//////////////////////////////////////////////////////////////
Xt::DomComponent* MayaToSorcery::findComponentMatchingID(XtId id, Xt::DomArray* theArray) {

	size_t size = Xt::Count(theArray);
	for (size_t i = 0; i < size; i ++) {

		Xt::DomComponent* currentComponent = Xt::AcquireComponent(theArray, i);
		if (currentComponent->id == id)
			return currentComponent;
	}
	
	return 0;

}

//////////////////////////////////////////////////////////////
// Find all attrbutes on the given node which match the xt_ 
// convension and store their type as well
//////////////////////////////////////////////////////////////
MStatus MayaToSorcery::setNodeXtAttributes(MDagPath node, Xt::DomComponent* component) {

	MStatus stat;

	MFnDagNode nodeFn(node);
	int numAttr = nodeFn.attributeCount(&stat);
	if (!stat) return stat;

	// Look for any fx nodes and add their parameters to the
	// object if its type is "PhysicalObject"
	if (!isEffect(node)) {

		if (component->type == "PhysicalObject") {

			MDagPathArray effects;
			stat = getEffects(node, &effects);
			if (!stat) return stat;

			Xt::DomArray* particleArray = 0;

			// Create the particle array
			if (effects.length())
				particleArray = Xt::AcquireArray(&component->data, "BreakParticleEvents");

			// Loop over each effect in turn and add its parameters recursively
			for (unsigned int i = 0; i < effects.length(); i ++) {

				// Get the locator matrix in order to populate its 
				// position and orientation parameters
				MFnTransform transFn(effects[i].node());
				MMatrix mat = transFn.transformationMatrix(&stat);
				if (!stat) return stat;

				// Add a component for the current effect
				Xt::DomComponent* currentComp = addComponent(particleArray);

				// Add the direction and position blocks
				Xt::DomBlock* positionBlock  = Xt::AcquireBlock(&currentComp->data, "position");
				Xt::DomBlock* directionBlock = Xt::AcquireBlock(&currentComp->data, "direction");

				double matrix[4][4];
				stat = mat.get(matrix);
				if (!stat) return stat;

				// Add the position
				MString strVal;
				strVal = matrix[3][0];
				addField(positionBlock, "vx", strVal.asChar());
				strVal = matrix[3][1];
				addField(positionBlock, "vy", strVal.asChar());
				strVal = -matrix[3][2];
				addField(positionBlock, "vz", strVal.asChar());

				// Add the direction
				strVal = matrix[1][0];
				addField(directionBlock, "vx", strVal.asChar());
				strVal = matrix[1][1];
				addField(directionBlock, "vy", strVal.asChar());
				strVal = -matrix[1][2];
				addField(directionBlock, "vz", strVal.asChar());

				// Recurse the object's parameters
				setNodeXtAttributes(effects[i], currentComp);

			}
		}
	}

	// Go over each attribute and find xt_attributes
	for (int i = 0; i < numAttr; i ++) {

		MObject attribute = nodeFn.attribute(i, &stat);
		if (!stat) return stat;

		MFnAttribute attrFn(attribute);
		MString attrName = attrFn.name();
		
		MPlug valuePlug = nodeFn.findPlug(attrFn.name(), &stat);
		if (!stat) return stat;

		// Ignore the model def xt attribute if found as its used
		// elsewhere by the system
		if (attrName.toLowerCase() == "xt_modeldef") continue;
		
		// Need to filter certain types of data for certain component types
		if (!isEffect(node)) {
			
			if (component->type != "PhysicalObjectElementDamageLevel") {

				if (attrName.toLowerCase() == "xt_strength" ||
					attrName.toLowerCase() == "xt_damageammountthreshold" ||
					attrName.toLowerCase() == "xt_mass") {				
					continue;
				}
			}
		}

		// Any others simply get added to the array
		if (attrName.substring(0, 2).toLowerCase() == "xt_") {
			
			// Currently this system is set up to support 
			// attributes of the following types:
			//
			// Int
			// Float
			// Bool
			// String
			MFnNumericAttribute numericFn(attribute);
			MString valueStr = "";

			// Check for Int
			if (numericFn.unitType() == MFnNumericData::kLong ||
				numericFn.unitType() == MFnNumericData::kInt) {

				int valueInt = valuePlug.asInt();
				valueStr += valueInt;

			// Check for Float
			} else if (numericFn.unitType() == MFnNumericData::kDouble ||
					   numericFn.unitType() == MFnNumericData::kFloat) {

				float valueFloat = valuePlug.asFloat();
				valueStr += valueFloat;

			// Check for Bool
			} else if (numericFn.unitType() == MFnNumericData::kBoolean) {
			
				bool valueBool = valuePlug.asBool();
				valueStr += valueBool;

			// Check for String
			} else if (numericFn.unitType() == MFnNumericData::kInvalid) {

				MFnTypedAttribute typedFn(attribute);
				if (typedFn.attrType() == MFnData::kString) {

					MString valueString = valuePlug.asString();
					valueStr = valueString;
				}

			// Unknown type
			} else {
				MGlobal::displayError("Unrecognised Attribute Type: " + attrName);
				continue;
			}

			// Get rid of the xt_ bit
			MString actualName = attrFn.name().substring(3, attrFn.name().length());
			addField(&component->data, actualName.asChar(), valueStr.asChar());

		}
	}

	return stat;
}

MayaToSorcery::MayaToSorcery() {}
MayaToSorcery::~MayaToSorcery() {}

//////////////////////////////////////////////////////////////
// Debug override
//////////////////////////////////////////////////////////////
/*void DebugLn(const char* text, ...) {

	char buffer[512];
	va_list vl;

	va_start(vl, text);
	vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, text, vl);
	va_end(vl);

	OutputDebugString(buffer);
	OutputDebugString("\r\n");
}*/