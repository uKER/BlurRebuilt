#include "stdafx.h"
#include "xmlparser.h"
#include "resourcemanager.h"
#include "commandline.h"
#include "Fontmaker.h"
#include "SystemCommands.h"

ResourceManager gResourceManager;

void ResourceManager::RegisterTextureFormat(int texture,UI_TEXTURE_FORMAT format)
{
	for(int a=0;a<num_textures;a++)
	{
		if(textureid[a] == texture)
		{
			textureformat[a]=format;
			return;
		}
	}
}

int ResourceManager::RegisterTexture(char *name,char *file,UI_TEXTURE_TYPE type,int id)
{
	for(int a=0;a<num_textures;a++)
	{
		if(!strcmp(texturelist[a],file))
		{
			return textureid[a];
		}
	}

	// must be a new texture as we've made it to here - so put it next in the list
	strcpy(texturelist[num_textures],file);
	if(name)
	{
		strcpy(texturenamelist[num_textures],name);
	}
	else
	{
		char str[128];
		int c=0;

		while(file[c] != '.')
		{
			str[c]=file[c];
			c++;
		}
		str[c]=0;

		strcpy(texturenamelist[num_textures],str);
	}

	texturetype[num_textures]=type;
	if(id == -1)
	{
		id=mID++;
		textureid[num_textures]=id;
	}
	else
	{
		textureid[num_textures]=id;
	}

	num_textures++;
	mBuild=true;
	return id;
}

int ResourceManager::RegisterMovie(char *file)
{
	for(int a=0;a<num_movies;a++)
	{
		if(!strcmp(movielist[a],file))
		{
			return a;
		}
	}

	// must be a new texture as we've made it to here - so put it next in the list
	strcpy(movielist[num_movies],file);

	num_movies++;
	return num_movies-1;
}

int ResourceManager::RegisterModel(char *file)
{
	for(int a=0;a<num_models;a++)
	{
		if(!strcmp(modellist[a],file))
		{
			return a;
		}
	}

	// must be a new texture as we've made it to here - so put it next in the list
	strcpy(modellist[num_models],file);

	num_models++;
	return num_models-1;
}

void ResourceManager::WriteOutResources()
{
	char command[256];

	int actual_number_of_textures[UI_TEXTURE_TYPE_MAX];
	
	actual_number_of_textures[UI_TEXTURE_TYPE_COMMON]=0;
	actual_number_of_textures[UI_TEXTURE_TYPE_GAME]=0;
	actual_number_of_textures[UI_TEXTURE_TYPE_UI]=0;
	actual_number_of_textures[UI_TEXTURE_TYPE_DLC_UI]=0;

	char dir[3];
	dir[0]=gUI.Config.UIMakerDir[0];
	dir[1]=gUI.Config.UIMakerDir[1];
	dir[2]=0;

	FILE *fp;

	if(mBuild)
	{
		for(int tt=0;tt<UI_TEXTURE_TYPE_MAX;tt++)
		{
			if(gXMLParser.mWriteOutJPNOnly)
			{
				sprintf(command,"%s\\JPN.xpr",gUI.Config.UIMakerTextureDir);
			}
			else if(gXMLParser.mWriteOutKOROnly)
			{
				sprintf(command,"%s\\KOR.xpr",gUI.Config.UIMakerTextureDir);
			}
			else if(gXMLParser.mWriteOutTCHOnly)
			{
				sprintf(command,"%s\\TCH.xpr",gUI.Config.UIMakerTextureDir);
			}
			else
			{
				if(tt == UI_TEXTURE_TYPE_UI) sprintf(command,"%s\\UI.xpr",gUI.Config.UIMakerTextureDir);
				else if(tt == UI_TEXTURE_TYPE_COMMON) sprintf(command,"%s\\COMMONUI.xpr",gUI.Config.UIMakerTextureDir);
				else if(tt == UI_TEXTURE_TYPE_GAME) sprintf(command,"%s\\GAMEUI.xpr",gUI.Config.UIMakerTextureDir);
				else if(tt == UI_TEXTURE_TYPE_DLC_UI) sprintf(command,"%s\\DLCUI.xpr",gUI.Config.UIMakerTextureDir);
			}
			fp=fopen(command,"w+");

			if(fp)
			{

#ifdef _XBOX
				if(gXMLParser.mWriteOutJPNOnly)
				{
					fprintf(fp,"out_header\t\t\tjpn_resource.h\n");
					fprintf(fp,"out_packedresource\tresource\\jpn_resource.xpr\n");
					fprintf(fp,"out_error\t\t\tjpn_resource.err\n\n");
				}
				else if(gXMLParser.mWriteOutKOROnly)
				{
					fprintf(fp,"out_header\t\t\tkor_resource.h\n");
					fprintf(fp,"out_packedresource\tresource\\kor_resource.xpr\n");
					fprintf(fp,"out_error\t\t\tkor_resource.err\n\n");
				}
				else if(gXMLParser.mWriteOutTCHOnly)
				{
					fprintf(fp,"out_header\t\t\ttch_resource.h\n");
					fprintf(fp,"out_packedresource\tresource\\tch_resource.xpr\n");
					fprintf(fp,"out_error\t\t\ttch_resource.err\n\n");
				}
				else
				{
					if(tt == UI_TEXTURE_TYPE_UI)
					{
						fprintf(fp,"out_header\t\t\tui_resource.h\n");
						fprintf(fp,"out_packedresource\tresource\\ui_resource.xpr\n");
						fprintf(fp,"out_error\t\t\tui_resource.err\n\n");
					}
					else if(tt == UI_TEXTURE_TYPE_GAME)
					{
						fprintf(fp,"out_header\t\t\tgameui_resource.h\n");
						fprintf(fp,"out_packedresource\tresource\\gameui_resource.xpr\n");
						fprintf(fp,"out_error\t\t\tgameui_resource.err\n\n");
					}
					else if(tt == UI_TEXTURE_TYPE_COMMON)
					{
						fprintf(fp,"out_header\t\t\tcommonui_resource.h\n");
						fprintf(fp,"out_packedresource\tresource\\commonui_resource.xpr\n");
						fprintf(fp,"out_error\t\t\tcommonui_resource.err\n\n");
					}
					else if(tt == UI_TEXTURE_TYPE_DLC_UI)
					{
						fprintf(fp,"out_header\t\t\tdlcui_resource.h\n");
						fprintf(fp,"out_packedresource\tresource\\dlcui_resource.xpr\n");
						fprintf(fp,"out_error\t\t\tdlcui_resource.err\n\n");
					}
				}


				for(int a=0;a<num_textures;a++)
				{
					if(texturetype[a] == tt || (gXMLParser.mWriteOutJPNOnly || gXMLParser.mWriteOutKOROnly || gXMLParser.mWriteOutTCHOnly))
					{
						actual_number_of_textures[tt]++;

						fprintf(fp,"Texture Texture%d\n",a);
						fprintf(fp,"{\n");
						fprintf(fp,"\tSource %s\n",texturelist[a]);

						if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED) fprintf(fp,"\tFormat D3DFMT_DXT3\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_32BIT) fprintf(fp,"\tFormat D3DFMT_A8R8G8B8\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_24BIT) fprintf(fp,"\tFormat D3DFMT_R5G6B5\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_16BIT) fprintf(fp,"\tFormat D3DFMT_A4R4G4B4\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_8BIT) fprintf(fp,"\tFormat D3DFMT_A8\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_LIN_32BIT) fprintf(fp,"\tFormat D3DFMT_LIN_A8R8G8B8\n");

						fprintf(fp,"\tLevels 1\n");
						fprintf(fp,"}\n\n");
					}
				}

				fclose(fp);

#else	//must be the Xbox360

				fprintf(fp,"<?xml version=\"1.0\" encoding=\"utf-8\" ?> \n\n");
				fprintf(fp,"<RDF Version=\"XPR2\">\n\n");


				for(int a=0;a<num_textures;a++)
				{
					if(texturetype[a] == tt || (gXMLParser.mWriteOutJPNOnly || gXMLParser.mWriteOutKOROnly || gXMLParser.mWriteOutTCHOnly))
					{
						actual_number_of_textures[tt]++;

						fprintf(fp,"<Texture\n");
						fprintf(fp,"\tName\t=\t\"%s\"\n",texturenamelist[a]);
						fprintf(fp,"\tSource\t=\t\"%s\"\n",texturelist[a]);

						if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED1) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT1\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED2) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT2\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED3) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT3\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED4) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT4\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED5) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT5\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_32BIT) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_A8R8G8B8\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_24BIT) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_R5G6B5\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_16BIT) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_A4R4G4B4\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_8BIT) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_A8\"\n");
						else if(textureformat[a] == UI_TEXTURE_FORMAT_LIN_32BIT) 
							fprintf(fp,"\tFormat\t=\t\"D3DFMT_LIN_A8R8G8B8\"\n");

						fprintf(fp,"\tLevels\t=\t\"1\"\n");
						fprintf(fp,"/>\n\n");
					}
				}

				fprintf(fp,"</RDF>\n");

				fclose(fp);

#endif
			}
			else
			{
				AfxMessageBox("Error Writing Texture XPR File",MB_OK|MB_ICONINFORMATION);
			}
			// run the batch file to build the buggers
			if(gXMLParser.mWriteOutJPNOnly)
			{
				sprintf(command,"call \"%s\\BundleJPNTextures\"",gUI.Config.UIMakerDir);
			}
			else if(gXMLParser.mWriteOutKOROnly)
			{
				sprintf(command,"call \"%s\\BundleKORTextures\"",gUI.Config.UIMakerDir);
			}
			else if(gXMLParser.mWriteOutTCHOnly)
			{
				sprintf(command,"call \"%s\\BundleTCHTextures\"",gUI.Config.UIMakerDir);
			}
			else
			{
				if(tt == UI_TEXTURE_TYPE_UI)
				{
					sprintf(command,"call \"%s\\BundleUITextures\"",gUI.Config.UIMakerDir);
				}
				else if(tt == UI_TEXTURE_TYPE_GAME)
				{
					sprintf(command,"call \"%s\\BundleGameUITextures\"",gUI.Config.UIMakerDir);
				}
				else if(tt == UI_TEXTURE_TYPE_COMMON)
				{
					sprintf(command,"call \"%s\\BundleCommonUITextures\"",gUI.Config.UIMakerDir);
				}
				else if(tt == UI_TEXTURE_TYPE_DLC_UI)
				{
					sprintf(command,"call \"%s\\BundleDLCUITextures\"",gUI.Config.UIMakerDir);
				}
			}
			
			system(dir);
			system(command);

			if(gXMLParser.mWriteOutJPNOnly)
			{
				sprintf(command,"xbcp /y \"%s\\resource\\jpn_resource.xpr\" %s\\textures\\jpn_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
				gSystemCommands.AddCommand(command);

				if(gUI.Config.CopytoServer)
				{
					sprintf(command,"copy /y \"%s\\resource\\jpn_resource.xpr\" %s\\textures\\jpn_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
					gSystemCommands.AddCommand(command);
				}
				return;		// East Asian fonts don't need the rest of this ...
			}
			else if(gXMLParser.mWriteOutKOROnly)
			{
				sprintf(command,"xbcp /y \"%s\\resource\\kor_resource.xpr\" %s\\textures\\kor_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
				gSystemCommands.AddCommand(command);

				if(gUI.Config.CopytoServer)
				{
					sprintf(command,"copy /y \"%s\\resource\\kor_resource.xpr\" %s\\textures\\kor_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
					gSystemCommands.AddCommand(command);
				}
				return;		// East Asian fonts don't need the rest of this ...
			}
			else if(gXMLParser.mWriteOutTCHOnly)
			{
				sprintf(command,"xbcp /y \"%s\\resource\\tch_resource.xpr\" %s\\textures\\tch_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
				gSystemCommands.AddCommand(command);

				if(gUI.Config.CopytoServer)
				{
					sprintf(command,"copy /y \"%s\\resource\\tch_resource.xpr\" %s\\textures\\tch_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
					gSystemCommands.AddCommand(command);
				}
				return;		// East Asian fonts don't need the rest of this ...
			}


			

#ifdef _XBOX
			//read the .h file back in and parse so we don't have to include anything into the main project - and we can re-load textures during the game
			if(tt == UI_TEXTURE_TYPE_UI)
			{
				sprintf(command,"%s\\ui_resource.h",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_GAME)
			{
				sprintf(command,"%s\\gameui_resource.h",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_COMMON)
			{
				sprintf(command,"%s\\commonui_resource.h",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_DLC_UI)
			{
				sprintf(command,"%s\\dlcui_resource.h",gUI.Config.UIMakerTextureDir);
			}
			fp=fopen(command,"r");

			if(fp)
			{
				char c;

				c=fgetc(fp);
				// find number of resources and skip
				while(c != '#')
				{
					c=fgetc(fp);
				}

				c=fgetc(fp);
				// grab the offsets for each texture
				bool line_start=false;
				int space=0;
				char number[8];
				int texture_number=0;

				while(c != EOF)
				{
					c=fgetc(fp);

					if(c == '#')
					{
						line_start=true;

					}

					if(line_start && c == 0x20) space++;

					if(line_start && space == 2)
					{
						line_start=false;
						space=0;
						int count=0;

						while(c != 'U')
						{
							number[count]=c;
							count++;
							c=fgetc(fp);
						}

						number[count]='\0';

						textureoffsets[texture_number]=atoi(number);
						texture_number++;
					}
				}

				fclose(fp);
			}
			else
			{
				AfxMessageBox("Error Reading Texture Header File",MB_OK|MB_ICONINFORMATION);
			}

			if(tt == UI_TEXTURE_TYPE_UI)
			{
				sprintf(command,"%s\\resource\\ui_resource.dat",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_GAME)
			{
				sprintf(command,"%s\\resource\\gameui_resource.dat",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_COMMON)
			{
				sprintf(command,"%s\\resource\\commonui_resource.dat",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_DLC_UI)
			{
				sprintf(command,"%s\\resource\\dlcui_resource.dat",gUI.Config.UIMakerTextureDir);
			}
			fp=fopen(command,"wb+");

			if(fp)
			{
				fwrite(&actual_number_of_textures[tt],sizeof(int),1,fp);

				int b=0;
				for(int a=0;a<num_textures;a++)
				{
					if(texturetype[a] == tt)
					{
						fwrite(&textureoffsets[b],sizeof(int),1,fp);
						fwrite(texturelist[a],sizeof(char),128,fp);
						fwrite(&textureid[a],sizeof(int),1,fp);
						b++;
					}
				}

				fclose(fp);
			}
			else
			{
				AfxMessageBox("Error Writing Texture Data File",MB_OK|MB_ICONINFORMATION);
			}
#else	//must be xbox360

			if(tt == UI_TEXTURE_TYPE_UI)
			{
				sprintf(command,"%s\\resource\\ui_resource.dat",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_GAME)
			{
				sprintf(command,"%s\\resource\\gameui_resource.dat",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_COMMON)
			{
				sprintf(command,"%s\\resource\\commonui_resource.dat",gUI.Config.UIMakerTextureDir);
			}
			else if(tt == UI_TEXTURE_TYPE_DLC_UI)
			{
				sprintf(command,"%s\\resource\\dlcui_resource.dat",gUI.Config.UIMakerTextureDir);
			}
			fp=fopen(command,"wb+");

			if(fp)
			{
				fwrite(&actual_number_of_textures[tt],sizeof(int),1,fp);

				int b=0;
				for(int a=0;a<num_textures;a++)
				{
					if(texturetype[a] == tt)
					{
						fwrite(texturenamelist[a],sizeof(char),128,fp);
						fwrite(texturelist[a],sizeof(char),128,fp);
						fwrite(&textureid[a],sizeof(int),1,fp);
						b++;
					}
				}

				fclose(fp);
			}
			else
			{
				AfxMessageBox("Error Writing Texture Data File",MB_OK|MB_ICONINFORMATION);
			}
#endif

			if(gUI.Config.Export == UIEXPORT_FULL)
			{
				if(tt == UI_TEXTURE_TYPE_UI)
				{
					sprintf(command,"xbcp /y \"%s\\resource\\ui_resource.xpr\" %s\\textures\\ui_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
					gSystemCommands.AddCommand(command);

					if(gUI.Config.CopytoServer)
					{
						sprintf(command,"copy /y \"%s\\resource\\ui_resource.xpr\" %s\\textures\\ui_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
						gSystemCommands.AddCommand(command);
					}

					sprintf(command,"xbcp /y \"%s\\resource\\ui_resource.dat\" %s\\data\\ui_resource.dat",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
					gSystemCommands.AddCommand(command);

					if(gUI.Config.CopytoServer)
					{
						sprintf(command,"copy /y \"%s\\resource\\ui_resource.dat\" %s\\data\\ui_resource.dat",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
						gSystemCommands.AddCommand(command);
					}
				}
				else if(tt == UI_TEXTURE_TYPE_GAME)
				{
					sprintf(command,"xbcp /y \"%s\\resource\\gameui_resource.xpr\" %s\\textures\\gameui_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
					gSystemCommands.AddCommand(command);

					if(gUI.Config.CopytoServer)
					{
						sprintf(command,"copy /y \"%s\\resource\\gameui_resource.xpr\" %s\\textures\\gameui_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
						gSystemCommands.AddCommand(command);
					}

					sprintf(command,"xbcp /y \"%s\\resource\\gameui_resource.dat\" %s\\data\\gameui_resource.dat",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
					gSystemCommands.AddCommand(command);

					if(gUI.Config.CopytoServer)
					{
						sprintf(command,"copy /y \"%s\\resource\\gameui_resource.dat\" %s\\data\\gameui_resource.dat",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
						gSystemCommands.AddCommand(command);
					}
				}
				else if(tt == UI_TEXTURE_TYPE_COMMON)
				{
					sprintf(command,"xbcp /y \"%s\\resource\\commonUI_resource.xpr\" %s\\textures\\commonUI_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
					gSystemCommands.AddCommand(command);

					if(gUI.Config.CopytoServer)
					{
						sprintf(command,"copy /y \"%s\\resource\\commonUI_resource.xpr\" %s\\textures\\commonUI_resource.xpr",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
						gSystemCommands.AddCommand(command);
					}

					sprintf(command,"xbcp /y \"%s\\resource\\commonUI_resource.dat\" %s\\data\\commonUI_resource.dat",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
					gSystemCommands.AddCommand(command);

					if(gUI.Config.CopytoServer)
					{
						sprintf(command,"copy /y \"%s\\resource\\commonUI_resource.dat\" %s\\data\\commonUI_resource.dat",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
						gSystemCommands.AddCommand(command);
					}
				}
				else if(tt == UI_TEXTURE_TYPE_DLC_UI)
				{
					//no need to do any of this as it goes in the pack
				}
			}
		}
	}

	if(gUI.Config.Export == UIEXPORT_EAFONTS) return;

	//do Lua script now
	
	char filename[256];

	sprintf(filename,"%s\\UITexture.lua",gUI.Config.UIMakerTextureDir);

	fp=fopen(filename,"w");

	char str[128];
	int c=0;

	if(fp)
	{
		fprintf(fp,"UITexture =\n{\n");
		
			fprintf(fp,"\tTextures =\n\t{\n");

			for(int a=0;a<this->num_textures;a++)
			{
				if(!strlen(texturenamelist[a]))
				{
					c=0;
					while(texturelist[a][c] != '.')
					{
						str[c]=texturelist[a][c];
						c++;
					}
					str[c]=0;
				}
				else
				{
					strcpy(str,texturenamelist[a]);
				}

				fprintf(fp,"\t\t%s = %d,\n",str,textureid[a]);
			}

			fprintf(fp,"\t},\n\n");

		fprintf(fp,"}");

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Saving Lua Text File (is it read-only)",MB_OK|MB_ICONINFORMATION);
	}

	sprintf(command,"copy /y \"%s\\UITexture.lua\" %s\\UITexture.lua",gUI.Config.UIMakerTextureDir,gUI.Config.LuaDir);
	gSystemCommands.AddCommand(command);

	sprintf(command,"xbcp /y \"%s\\UITexture.lua\" %s\\scripts\\UITexture.lua",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
	gSystemCommands.AddCommand(command);

	if(gUI.Config.CopytoServer)
	{
		sprintf(filename,"copy /y \"%s\\UITexture.lua\" %s\\scripts\\UITexture.lua",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
		gSystemCommands.AddCommand(filename);
	}

}
