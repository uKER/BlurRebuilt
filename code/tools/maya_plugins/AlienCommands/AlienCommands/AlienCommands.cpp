#include "stdafx.h"

#include "AlienCommands.h"
#include <maya/MFnPlugin.h>

CNxNSmartIntegrator *g_pSmartIntegrator;
bool NXNInitialised = false;

//****************************************************************************
// Argument syntax
//
// -gl/getLatest:		Get latest from alienbrain (File or Folder)
// -s/synchronise:		Synchronise with alienbrain
// -co/checkOut:		Check the given file or folder out of alienbrain
//						with the given comment. Has the option of getting
//						a local copy or not getting alocal copy
// -ci/checkIn:			Check the given file or folder in to alienbrain
//						with the given comment. If no changes were made
//						undo the checkout
// -uco/undoCheckOut:	Undo the checkout on the given file without getting
//						a local copy or adding a comment
// -i/import:			Import the given file at its current location
//						into Alienbrain
// -ico/isCheckedOut:	Return whether or not the given file is checked
//						out or not
// -ex/exists:			Return Wether or not the given file/folder is imported
// -gc/getComment:		Return the comment on the given version of the
//						given file
// -dgl/dontGetLocal:	If set never get a local copy on any command flag
// -bfl/browseForFile	Open a file browser
// -bfo/browseForFolder Open a folder browser
//*****************************************************************************
const char *getFlag			 = "-gl",	*getFlagLong			= "-getLatest";
const char *syncFlag		 = "-s",	*syncFlagLong			= "-synchronise";
const char *outFlag			 = "-co",	*outFlagLong			= "-checkOut";
const char *inFlag			 = "-ci",	*inFlagLong				= "-checkIn";
const char *undoFlag		 = "-uco",	*undoFlagLong			= "-undoCheckOut";
const char *importFlag		 = "-i",	*importFlagLong			= "-import";
const char *isOutFlag		 = "-ico",	*isOutFlagLong			= "-isCheckedOut";
const char *existsFlag		 = "-ex",	*existsFlagLong			= "-exists";
const char *getComment		 = "-gc",	*getCommentLong			= "-getComment";
const char *noLocalFlag		 = "-dgl",	*noLocalFlagLong		= "-dontGetLocal";
const char *browseFileFlag	 = "-bfl",  *browseFileFlagLong		= "-browseFile";
const char *browseFolderFlag = "-bfo",  *browseFolderFlagLong	= "-browseFolder";

MSyntax AlienCommand::newSyntax() {

	MSyntax syntax;

	syntax.addFlag(getFlag,		getFlagLong,		MSyntax::kString);
	syntax.addFlag(syncFlag,	syncFlagLong,		MSyntax::kString);
	syntax.addFlag(outFlag,		outFlagLong,		MSyntax::kString,
													MSyntax::kString);
	syntax.addFlag(inFlag,		inFlagLong,			MSyntax::kString,
													MSyntax::kString);
	syntax.addFlag(undoFlag,	undoFlagLong,		MSyntax::kString);
	syntax.addFlag(importFlag,	importFlagLong,		MSyntax::kString,
													MSyntax::kString);
	syntax.addFlag(isOutFlag,	isOutFlagLong,		MSyntax::kString);
	syntax.addFlag(existsFlag,	existsFlagLong,		MSyntax::kString);
	syntax.addFlag(getComment,	getCommentLong,		MSyntax::kString,
													MSyntax::kLong);
	syntax.addFlag(noLocalFlag,	noLocalFlagLong);
	syntax.addFlag(browseFileFlag, browseFileFlagLong, MSyntax::kString);
	syntax.addFlag(browseFolderFlag, browseFolderFlagLong, MSyntax::kString);

	// Allow multiple calls to get latest
	syntax.makeFlagMultiUse(getFlag);
	syntax.makeFlagMultiUse(outFlag);
	syntax.makeFlagMultiUse(inFlag);
	syntax.makeFlagMultiUse(undoFlag);
	syntax.makeFlagMultiUse(importFlag);

	return syntax;
}

MString ConvertAlienbrainStringToMayaString(CNxNString CNString)
{
	std::string s((LPCTSTR)CNString);

	return MString(s.c_str());
}


MStatus AlienCommand::doIt( const MArgList& args ) {

	MStatus stat = MS::kSuccess;

	// Only initialise once
	if (NXNInitialised == false)
	{
		g_pSmartIntegrator = new CNxNSmartIntegrator();

		CNxNString Dummy;
		bool Value = g_pSmartIntegrator->InitInstance(true, Dummy);
		NXNInitialised = true;
	}

	MString path, comment;
	MArgDatabase argData( syntax(), args, &stat );
	if (!stat) return stat;

	// Query a file to see if its checked out already
	if (argData.isFlagSet(isOutFlag)) {
		argData.getFlagArgument(isOutFlag, 0, path);
		setResult(queryCheckedOut(path.asChar()));
		return MS::kSuccess;
	}

	// Query if the given file or folder exists in Alienbrain
	if (argData.isFlagSet(existsFlag)) {
		argData.getFlagArgument(existsFlag, 0, path);
		setResult(queryFileFolderExists(path.asChar()));
		return MS::kSuccess;
	}

	// Get Latest
	if (argData.isFlagSet(getFlag)) {

		MStringArray files;
		MArgList list;
		int numberOfFiles = argData.numberOfFlagUses(getFlag);

		for (int i = 0; i < numberOfFiles; i ++) {

			stat = argData.getFlagArgumentList(getFlag, i, list);
			files.append(list.asString(0));

		}
		getLatestFiles(files);
	}

	// Synchronise
	if (argData.isFlagSet(syncFlag)) {
		argData.getFlagArgument(syncFlag, 0, path);
		synchroniseFolder(path.asChar());
	}

	// Check out
	if (argData.isFlagSet(outFlag)) {

		bool bDontGetLocal = false;
		if (argData.isFlagSet(noLocalFlag))
			bDontGetLocal = true;

		argData.getFlagArgument(outFlag, 0, path);
		argData.getFlagArgument(outFlag, 1, comment);

		inspectComment(comment);

		MStringArray files;
		MArgList list;
		int numberOfFiles = argData.numberOfFlagUses(outFlag);

		for (int i = 0; i < numberOfFiles; i ++) {

			stat = argData.getFlagArgumentList(outFlag, i, list);
			files.append(list.asString(0));

		}
		checkoutFiles(files, comment.asChar(), bDontGetLocal);
	}

	// Check in
	if (argData.isFlagSet(inFlag)) {

		argData.getFlagArgument(inFlag, 0, path);
		argData.getFlagArgument(inFlag, 1, comment);

		inspectComment(comment);

		MStringArray files;
		MArgList list;
		int numberOfFiles = argData.numberOfFlagUses(inFlag);

		for (int i = 0; i < numberOfFiles; i ++) {

			stat = argData.getFlagArgumentList(inFlag, i, list);
			files.append(list.asString(0));

		}
		checkinFiles(files, comment.asChar());
	}

	// Undo Check Out
	if (argData.isFlagSet(undoFlag)) {

		argData.getFlagArgument(undoFlag, 0, path);

		MStringArray files;
		MArgList list;
		int numberOfFiles = argData.numberOfFlagUses(undoFlag);

		for (int i = 0; i < numberOfFiles; i ++) {

			stat = argData.getFlagArgumentList(undoFlag, i, list);
			files.append(list.asString(0));

		}
		undoCheckOut(files);

	}

	// Import
	if (argData.isFlagSet(importFlag)) {

		argData.getFlagArgument(importFlag, 0, path);
		argData.getFlagArgument(importFlag, 1, comment);

		inspectComment(comment);

		MStringArray files;
		MArgList list;
		int numberOfFiles = argData.numberOfFlagUses(importFlag);

		for (int i = 0; i < numberOfFiles; i ++) {

			stat = argData.getFlagArgumentList(importFlag, i, list);
			files.append(list.asString(0));

		}
		importFiles(files, comment.asChar());

	}


	// Get Comment
	if (argData.isFlagSet(getComment)) {

		int version = -1;
		argData.getFlagArgument(getComment, 0, path);
		argData.getFlagArgument(getComment, 1, version);

		comment = getVersionComment(path.asChar(), version);
		setResult(comment);
	}

	// Browse for file

	CNxNFile File;

	if (argData.isFlagSet(browseFileFlag) )
	{
		argData.getFlagArgument(browseFileFlag, 0, path);

		CNxNFile *pFile = launchFileBrowser(path.asChar() );

		MString Result;

		if (pFile)
		{
			Result = ConvertAlienbrainStringToMayaString(pFile->GetLocalPath());
		}
		else
		{
			Result = "No File";
		}

		setResult(Result);
	}

	// Browse for folder

	if (argData.isFlagSet(browseFolderFlag) )
	{
		argData.getFlagArgument(browseFolderFlag, 0, path);

		CNxNFolder *pFolder = launchFolderBrowser(path.asChar() );

		MString Result;

		if (pFolder)
		{
			Result = ConvertAlienbrainStringToMayaString(pFolder->GetLocalPath());
		}
		else
		{
			Result = "No Folder";
		}

		setResult(Result);
	}

	return stat;
}

void getLatestFiles(MStringArray paths) {

	CNxNString NameSpacePath;
	CNxNDbNodeList list;

	for (unsigned int i = 0; i < paths.length(); i ++) {

		MString currentPath = paths[i];
		CNxNString currentNxNPath = currentPath.asChar();
		if (!g_pSmartIntegrator->MapManagedPath(currentNxNPath, NameSpacePath))
		{
			MGlobal::displayError("Path not found: " + currentPath);
			continue;
		}

		// If this isn't a file, test if its a folder
		CNxNFile *pFile = (CNxNFile*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFile::GetType());

		if (!pFile)
		{
			CNxNFolder *pFolder = (CNxNFolder*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFolder::GetType());

			if (!pFolder)
			{
				MGlobal::displayWarning("Non file or folder: " + currentPath);
				continue;
			}

			// Add the folder
			list.Add(pFolder);

			continue;
		}

		// Add the file
		list.Add(pFile);
	}

	// Get latest
	list.GetLatest(false, false, false, false, false, "", true, false,
					NXN_GETLATEST_OVERWRITE_WRITABLE_SKIP,
					NXN_GETLATEST_OVERWRITE_CHECKEDOUT_SKIP);

}



void synchroniseFolder(CNxNString Path) {

	CNxNString NameSpacePath;

	if (!g_pSmartIntegrator->MapManagedPath(Path, NameSpacePath) )
	{
		return; // can't sync latest on local file
	}

	CNxNFolder *pFolder = (CNxNFolder*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFolder::GetType() );

	if (!pFolder)
	{
		//we're only interested in folders for syncing

		return;
	}

	//now, recurse every file in this folder, and see if it exists in the database.
	//If not, delete it.

	//flip slashes for windows!

	char buffer[260];

	strncpy_s(buffer, Path, 260);

	char *cp = buffer;

	while (*cp)
	{
		if (*cp == '/')
		{
			*cp = '\\';
		}
		cp++;
	}

	//add trailing slash if not present

	size_t length = strlen(buffer);

	if (buffer[length-1] == '\\')
	{
		recurseDeleteFilesNotInAB(CNxNString(buffer));
	}
	else
	{
		recurseDeleteFilesNotInAB(CNxNString(buffer) + "\\");
	}

	//finally, do a get latest on the folder

	pFolder->GetLatest(false, false, false, false, false, "", true, false);

	delete pFolder;
}


CNxNFile *launchFileBrowser(CNxNString Path)
{
	CNxNString NameSpacePath;

	if (!g_pSmartIntegrator->MapManagedPath(Path, NameSpacePath) )
	{
		return 0;
	}

	CNxNFolder *pStartFolder = (CNxNFolder*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFolder::GetType() );

	if (!pStartFolder)
	{
		return 0;
	}

	CNxNBrowseFilter bfFilter;

	bfFilter.Hide("");												// Hide all
	bfFilter.Show(CNxNFile::GetType());								// show files
    bfFilter.Show(CNxNFolder::GetType());                           // show folders
	bfFilter.NoDrag("");                                            // disable dragging
    bfFilter.NoDrop("");                                            // disable dropping
    bfFilter.NoLabelEdit("");                                       // disable label editing
    bfFilter.DisableCommand("");                                    // disable all context menu commands
    bfFilter.Accept(CNxNFile::GetType());							// user can only select files
    bfFilter.SetRoot(NameSpacePath);								// the root to display

	CNxNFile *pFile = (CNxNFile *)pStartFolder->BrowseForObject(bfFilter, "Select File");

	return pFile;
}



CNxNFolder *launchFolderBrowser(CNxNString Path)
{
	CNxNString NameSpacePath;

	if (!g_pSmartIntegrator->MapManagedPath(Path, NameSpacePath) )
	{
		return 0;
	}

	CNxNFolder *pStartFolder = (CNxNFolder*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFolder::GetType() );

	if (!pStartFolder)
	{
		return 0;
	}

	CNxNBrowseFilter bfFilter;

	bfFilter.Hide("");												// Hide all
	bfFilter.Show(CNxNFolder::GetType());                           // show folders
    bfFilter.Hide(CNxNFile::GetType());                             // hide files
	bfFilter.NoDrag("");                                            // disable dragging
    bfFilter.NoDrop("");                                            // disable dropping
    bfFilter.NoLabelEdit("");                                       // disable label editing
    bfFilter.DisableCommand("");                                    // disable all context menu commands
    bfFilter.Accept(CNxNFolder::GetType());                         // user can only select folders
    bfFilter.SetRoot(NameSpacePath);								// the root to display

	CNxNFolder *pFolder = 0;
	pFolder = (CNxNFolder *)pStartFolder->BrowseForObject(bfFilter, "Select Folder");

	return pFolder;
}


void recurseDeleteFilesNotInAB(CNxNString folderName)
{
	WIN32_FIND_DATA fileData;

	HANDLE handle = FindFirstFile(folderName + "*.*", &fileData);

	if (handle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	bool bGoing=true;

	while (bGoing)
	{
		CNxNString fileAndPath = folderName + fileData.cFileName;

		bool bDirectory = ((GetFileAttributes(fileAndPath) & FILE_ATTRIBUTE_DIRECTORY) != 0);

		if (bDirectory)
		{
			if(strcmp(".", fileData.cFileName) && strcmp("..", fileData.cFileName))
			{
				recurseDeleteFilesNotInAB(fileAndPath + "\\");
			}
		}
		else
		{
			//can we map the file in AB? If not, make sure its not readonly and delete

			CNxNString nxnpath;

			g_pSmartIntegrator->MapManagedPath(fileAndPath, nxnpath);

			CNxNFile *pFile = (CNxNFile*)g_pSmartIntegrator->GetManagedNode(nxnpath, CNxNFile::GetType());

			if (!pFile)
			{
				//is it read only?

				DWORD attr = GetFileAttributes(fileAndPath);

				if (attr&FILE_ATTRIBUTE_READONLY)
				{
					SetFileAttributes(fileAndPath, attr&=~FILE_ATTRIBUTE_READONLY);
				}

				//delete it

				DeleteFile(fileAndPath);
			}
			else
			{
				delete pFile;
			}
		}

		(FindNextFile(handle, &fileData)==TRUE)?(bGoing = true):(bGoing = false);
	}
}



void checkoutFiles(MStringArray paths, CNxNString comment, bool bDontGetLocal) {

	// Convert the list of strings to a list of database nodes
	CNxNDbNodeList list = getDbNodeList(paths);

	// Do the check out
	list.CheckOut(false, comment, bDontGetLocal, false, false, "", true, false);
}



void checkinFiles(MStringArray paths, CNxNString comment) {

	// Convert the list of strings to a list of database nodes
	CNxNDbNodeList list = getDbNodeList(paths);

	// Do the check in
	list.CheckIn(false, "", comment, false, false, false, false, NXN_CHECKIN_CREATE_VERSION_DEFAULT, true, false);
}


void undoCheckOut(MStringArray paths) {

	// Convert the list of strings to a list of database nodes
	CNxNDbNodeList list = getDbNodeList(paths);

	// Undo the check out
	list.UndoCheckOut(false, true);

}

void importFiles(MStringArray paths, CNxNString comment) {

	CNxNString NameSpacePath;
	CNxNDbNodeList list;

	for (unsigned int i = 0; i < paths.length(); i ++) {

		MString currentPath = paths[i];
		CNxNString currentNxNFile = currentPath.asChar();

		// Strip the name off this object to retrieve
		// the folder structure only
		MString currentParent		= getDirectoryFromPath(currentPath);
		CNxNString currentNxNParent = currentParent.asChar();
		MGlobal::displayInfo(currentParent);

		if (!g_pSmartIntegrator->MapManagedPath(currentNxNParent, NameSpacePath)) {
			MGlobal::displayError("Path not in alienbrain: " + currentParent);
			continue;
		}

		// Grab the folder object
		CNxNFolder *pFolder = (CNxNFolder*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFolder::GetType());

		if (!pFolder) {
			MGlobal::displayError("Could not find folder: " + currentParent);
			continue;
		}

		pFolder->ImportDbNode(currentNxNFile, false, comment, "", false, true, false, "", false, true, false);
	}
}

///////////////////////////////////////////
// return val:
// 1:  File already checked out
// 0:  File not already checked out
// -1: File not found in alienbrain
///////////////////////////////////////////
int queryCheckedOut(CNxNString path) {

	CNxNString NameSpacePath;
	if (!g_pSmartIntegrator->MapManagedPath(path, NameSpacePath))
		return -1;

	CNxNFile *pFile = (CNxNFile*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFile::GetType());

	// Should only work on files in alienbrain
	if (!pFile) return -1;

	return pFile->IsCheckedOut();

}

///////////////////////////////////////////
// Return wether or not the given file
// or folder in imported into Alienbrain
///////////////////////////////////////////
bool queryFileFolderExists(CNxNString path) {

	CNxNString NameSpacePath;
	//if (!g_pSmartIntegrator->MapManagedPath(path, NameSpacePath))
	//	return 0;

	return g_pSmartIntegrator->ExistsInDatabase(path);
}

///////////////////////////////////////////
// Return the comment of the given version
// of the given file. If the version given
// is -1 get the CheckOutComment if there
// is one
///////////////////////////////////////////
MString getVersionComment(CNxNString path, long version) {

	CNxNString NameSpacePath;
	if (!g_pSmartIntegrator->MapManagedPath(path, NameSpacePath))
		return "";

	CNxNFile *pFile = (CNxNFile*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFile::GetType());

	// Should only work on files in alienbrain
	if (!pFile) return "";

	std::string comment = "";

	// Get the check-out comment
	if (version == -1)
		comment = pFile->GetStringProperty(NXN_PROPERTY_CHECKOUT_COMMENT);

	// Scan the history of this object to find the
	// given version and get its comment
	else {

		// Get the history object
		CNxNHistory *pHistory = pFile->GetHistory();
		long versionCount = pHistory->GetVersionCount();

		if (version > versionCount) {
			MGlobal::displayError("Version number given was too high");
			return "";
		}

		// Iterate the history
		TNxNNodeIterator it;
		CNxNHistoryVersion *pVersion = pHistory->GetFirstVersion(&it);

		// Enumerate all children until the end has been reached
		long i = 0;
		while (!it.IsEnd()) {

			i++;
			if (i == version) break;

			// Retrieve the next history node
			pVersion = pHistory->GetNextVersion(&it);
		}

		 comment = pVersion->GetComment();

	}

	return MString(comment.c_str());
}

///////////////////////////////////////////
// Strip the filename from the given path
///////////////////////////////////////////
MString getDirectoryFromPath(MString path) {

	MStringArray tokens;
	MString finalPath = "";
	path.split('/', tokens);
	for (unsigned int i = 0; i < tokens.length() - 1; i ++) {
		finalPath += tokens[i];
		finalPath += "/";
	}

	return finalPath;

}

void ensureFolderIsConnected(CNxNFolder *pFolder)
{
	CNxNProject *pProject = pFolder->GetProject();
	CNxNString Host = pProject->GetHostName();

	if (!pProject->IsConnected())		// Need to connect
	{
		pProject->Connect("davida", "davida", Host, false);
	}
}



void ensureFileIsConnected(CNxNFile *File)
{
	CNxNProject *Project = File->GetProject();
	CNxNString Host = Project->GetHostName();

	if (!Project->IsConnected())		// Need to connect
	{
		Project->Connect("davida", "davida", Host, false);
	}
}

///////////////////////////////////////////
// Ensure the comment is not blank
///////////////////////////////////////////
MStatus inspectComment(MString &comment) {

	if (comment == "") {
		MGlobal::displayWarning("No appropriate comment was given for this check out. default will be used");
		comment = "< Automatically checked out by Maya->Alienbrain plugin >";
	}

	return MS::kSuccess;
}

///////////////////////////////////////////
// Convert the list of string to an array
// of database nodes
///////////////////////////////////////////
CNxNDbNodeList getDbNodeList(MStringArray paths) {

	CNxNString NameSpacePath;
	CNxNDbNodeList list;

	for (unsigned int i = 0; i < paths.length(); i ++) {

		MString currentPath = paths[i];
		CNxNString currentNxNPath = currentPath.asChar();
		if (!g_pSmartIntegrator->MapManagedPath(currentNxNPath, NameSpacePath))
		{
			MGlobal::displayError("Path not found: " + currentPath);
			continue;
		}

		// If this isn't a file, test if its a folder
		CNxNFile *pFile = (CNxNFile*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFile::GetType());

		if (!pFile)
		{
			CNxNFolder *pFolder = (CNxNFolder*)g_pSmartIntegrator->GetManagedNode(NameSpacePath, CNxNFolder::GetType());

			if (!pFolder)
			{
				MGlobal::displayWarning("Non file or folder: " + currentPath);
				continue;
			}

			// Add the folder
			list.Add(pFolder);

			continue;
		}

		// Add the file
		list.Add(pFile);
	}

	return list;

}

MStatus initializePlugin( MObject obj ) {

	MStatus   status;
	MFnPlugin plugin( obj, "Michael Bibby", "", "Any");

	status = plugin.registerCommand( "alienCommand",
									 AlienCommand::creator,
									 AlienCommand::newSyntax);
	if (!status) {
		status.perror("Registering command: AlienCommand");
		return status;
	}

	NXNInitialised = false;

	return status;
}

MStatus uninitializePlugin( MObject obj) {

	MStatus   status;
	MFnPlugin plugin( obj );

	// Exit the NXN smart integrator instance
	if (NXNInitialised == true)
		g_pSmartIntegrator->ExitInstance();

	status = plugin.deregisterCommand( "alienCommand" );
	if (!status) {
		status.perror("De-registering command: AlienCommand");
		return status;
	}

	return status;
}

