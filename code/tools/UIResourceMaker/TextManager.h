#pragma once

#define MAX_TEXT_STRINGS 4096

typedef struct
{
	WCHAR define[128];
	WCHAR string[640];
} LocText;

class TextManager
{
public:
	void LoadText();
	void SaveText();
	void SaveLua();
	int FindText(char *define);
	char *GetText(int txt);
	void AddText(char *define, char *string);

	void SetFileName(char *file){strcpy(mFileName,file);};
	char *GetFileName(){return mFileName;};

	void SetDLCFileName(char *file){strcpy(mDLCFileName,file);};
	char *GetDLCFileName(){return mDLCFileName;};

	void CleanUp();

	void SearchInit();
	void SearchFiles(const char *dir);
	void FoundFile(FILE *fp);
	void SearchEnd();

private:
	char mFileName[128];
	char mDLCFileName[128];

	LocText loctext[MAX_TEXT_STRINGS];
	int num_loctext;
	char current_string[640];

	char mFoundStrings[MAX_TEXT_STRINGS][128];
	int mNumFoundStrings;
};

extern TextManager gTextManager;
