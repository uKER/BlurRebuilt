#pragma once

#include "uiDefines.h"

#define MAX_TEXTURES 256
#define MAX_MOVIES 64
#define MAX_MODELS 64
#define MAX_VARS 256

enum UI_TEXTURE_FORMAT
{
	UI_TEXTURE_FORMAT_COMPRESSED1,
	UI_TEXTURE_FORMAT_COMPRESSED2,
	UI_TEXTURE_FORMAT_COMPRESSED3,
	UI_TEXTURE_FORMAT_COMPRESSED4,
	UI_TEXTURE_FORMAT_COMPRESSED5,
	UI_TEXTURE_FORMAT_32BIT,
	UI_TEXTURE_FORMAT_24BIT,
	UI_TEXTURE_FORMAT_16BIT,
	UI_TEXTURE_FORMAT_8BIT,

	UI_TEXTURE_FORMAT_LIN_32BIT,

	UI_TEXTURE_FORMAT_MAX,
};

class ResourceManager
{
public:
	ResourceManager(){num_textures=0;num_movies=0;num_models=0;mBuild=false;};

	void Init(){num_textures=0;num_movies=0;num_models=0;};

	void Start(){mBuild=false;mID=9000;};

	int RegisterTexture(char *name,char *file,UI_TEXTURE_TYPE type,int id);
	void RegisterTextureFormat(int texture,UI_TEXTURE_FORMAT format);

	int RegisterMovie(char *file);
	int RegisterModel(char *file);

	void WriteOutResources();

	void SetBuild(){mBuild=true;};

	void SetFileName(char *f){strcpy(mFileName,f);};
	char *GetFileName(){return mFileName;};

private:

	bool mBuild;

	char texturelist[MAX_TEXTURES][128];
	char texturenamelist[MAX_TEXTURES][128];
	UI_TEXTURE_TYPE texturetype[MAX_TEXTURES];
	UI_TEXTURE_FORMAT textureformat[MAX_TEXTURES];
	int textureid[MAX_TEXTURES];
	int textureoffsets[MAX_TEXTURES];
	int num_textures;

	char movielist[MAX_MOVIES][32];
	int num_movies;

	char modellist[MAX_MODELS][32];
	int num_models;

	char mFileName[64];

	int mID;
};

extern ResourceManager gResourceManager;