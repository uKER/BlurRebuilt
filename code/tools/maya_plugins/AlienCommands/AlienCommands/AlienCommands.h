#include <maya/MGlobal.h>
#include <NxNIntegratorSDK.h>
#include <NxNSmartIntegrator.h>
#include <NxNString.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>

void getLatestFiles(MStringArray paths);
void synchroniseFolder(CNxNString path);
CNxNFile   *launchFileBrowser(CNxNString path);
CNxNFolder *launchFolderBrowser(CNxNString Path);
void checkoutFiles(MStringArray paths, CNxNString comment, bool bDontGetLocal);
void checkinFiles(MStringArray paths, CNxNString comment);
void undoCheckOut(MStringArray paths);
void importFiles(MStringArray paths, CNxNString comment);
void ensureFolderIsConnected(CNxNFolder *pFolder);
void ensureFileIsConnected(CNxNFile *pFile);
void recurseDeleteFilesNotInAB(CNxNString folderName);
int  queryCheckedOut(CNxNString path);
bool queryFileFolderExists(CNxNString path);
MString ConvertAlienbrainStringToMayaString(CNxNString CNString);
MString getVersionComment(CNxNString path, long version);
MString getDirectoryFromPath(MString path);
MStatus inspectComment(MString &comment);
CNxNDbNodeList getDbNodeList(MStringArray paths);

//=======================================
// AlienCommand class
//=======================================
class AlienCommand : MPxCommand {

public:
	virtual MStatus		doIt ( const MArgList& );
	static void*		creator() { return new AlienCommand; }
	static MSyntax		newSyntax();

};