#include "stdafx.h"
#include "textmanager.h"
#include "commandline.h"
#include "systemcommands.h"
#include "ui.h"

TextManager gTextManager;

void TextManager::AddText(char *define,char *string)
{
	int txt=FindText(define);

	LocText wltext;
	swprintf(wltext.string,L"%S",string);
	swprintf(wltext.define,L"%S",define);

	if(txt == -1)
	{
		wcscpy(loctext[num_loctext].define,wltext.define);
		wcscpy(loctext[num_loctext].string,wltext.string);
		num_loctext++;
	}
	else
	{
		wcscpy(loctext[txt].string,wltext.string);
	}
}

char *TextManager::GetText(int txt)
{
	sprintf(current_string,"%S",loctext[txt].string);
	return current_string;
}

int TextManager::FindText(char *name)
{
	int d=0;
	WCHAR wname[128];
	swprintf(wname,L"%S",name);

	for(int a=0;a<num_loctext;a++)
	{
		if(!wcscmp(wname,loctext[a].define))
		{
			return a;
		}
	}

	gCommandLine.AddLineArg(TC_RED,"Unable to find string %s",name);
	return -1;
}

void TextManager::LoadText()
{
	WCHAR wc;
	char filename[128];
	FILE *fp;
	int count=0;

	num_loctext=0;

	sprintf(filename,"%s\\English.ini",gUI.Config.UIMakerTextDir);

	fp=fopen(filename,"rb");

	if(fp)
	{
		// clear UNICODE header
		wc=fgetwc(fp);

		// Read first proper character
		wc=fgetwc(fp);

		bool end=false;
		while(!end)
		{
			// store the define
			count=0;
			while(wc != 0x0020)
			{
				loctext[num_loctext].define[count++]=wc;
				wc=fgetwc(fp);
			}
			loctext[num_loctext].define[count]=NULL;

			//clear = sign
			wc=fgetwc(fp);

			// clear trailing space if we have one
			wc=fgetwc(fp);
			
			// store the string
			count=0;

			// Read next character
			wc=fgetwc(fp);
			while(wc != 0x000d)
			{
				loctext[num_loctext].string[count++]=wc;
				wc=fgetwc(fp);
			}
			loctext[num_loctext].string[count]=NULL;

			// clear the line feed
			wc=fgetwc(fp);
			if(wc == 65535) end=true;
			
			num_loctext++;

			wc=fgetwc(fp);
			if(wc == 65535) end=true;
		}

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Reading English Text File",MB_OK|MB_ICONINFORMATION);
	}
}

void TextManager::SaveText()
{
	return;

	/*
	char filename[128];
	FILE *fp;

	sprintf(filename,"%s\\English.ini",gUI.Config.UIMakerTextDir);

	fp=fopen(filename,"wb");

	if(fp)
	{
		unsigned char uni_header=0xff;
		fwrite(&uni_header,sizeof(unsigned char),1,fp);
		uni_header=0xfe;
		fwrite(&uni_header,sizeof(unsigned char),1,fp);

		for(int a=0;a<num_loctext;a++)
		{
			WCHAR string[640+128];

			swprintf(string,L"%s = %s\r\n",loctext[a].define,loctext[a].string);

			int b=0;
			while(string[b])
			{
				fwrite(&string[b],sizeof(WCHAR),1,fp);
				b++;
			}
		}

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Saving English Text File (is it read-only)",MB_OK|MB_ICONINFORMATION);
	}
	*/
}

void TextManager::SaveLua()
{
	char filename[256];
	FILE *fp;

	sprintf(filename,"%s\\UIText.lua",gUI.Config.UIMakerTextDir);

	fp=fopen(filename,"w");

	if(fp)
	{
		fprintf(fp,"UIText =\n{\n");

		for(int a=0;a<num_loctext;a++)
		{
			fprintf(fp,"\t%S = %d,\n",loctext[a].define,a);
		}

		fprintf(fp,"}");

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Saving Lua Text File (is it read-only)",MB_OK|MB_ICONINFORMATION);
	}

	sprintf(filename,"copy /y \"%s\\UIText.lua\" %s\\UIText.lua",gUI.Config.UIMakerTextDir,gUI.Config.LuaDir);
	gSystemCommands.AddCommand(filename);

	sprintf(filename,"xbcp /y \"%s\\UIText.lua\" %s\\scripts\\UIText.lua",gUI.Config.UIMakerTextDir,gUI.Config.TargetDir);
	gSystemCommands.AddCommand(filename);

	if(gUI.Config.CopytoServer)
	{
		sprintf(filename,"copy /y \"%s\\UIText.lua\" %s\\scripts\\UIText.lua",gUI.Config.UIMakerTextDir,gUI.Config.ServerDir);
		gSystemCommands.AddCommand(filename);
	}


	//now sort out the header file for the game
	sprintf(filename,"%s\\UIText.h",gUI.Config.UIMakerTextDir);

	fp=fopen(filename,"w");

	if(fp)
	{
		for(int a=0;a<num_loctext;a++)
		{
			fprintf(fp,"#define %S %d\n",loctext[a].define,a);
		}

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Saving Lua Header File (is it read-only)",MB_OK|MB_ICONINFORMATION);
	}

}

void TextManager::SearchInit()
{
	mNumFoundStrings=0;
}

void TextManager::SearchEnd()
{
	FILE *new_fp,*old_fp;
	char filename[256];
	WCHAR line[1024];

	gCommandLine.AddLineArg(TC_RED,"Found %d Unique Strings",mNumFoundStrings);

	sprintf(filename,"%s\\English.old",gUI.Config.UIMakerTextDir);
	old_fp=fopen(filename,"rb");

	sprintf(filename,"%s\\English.ini",gUI.Config.UIMakerTextDir);
	new_fp=fopen(filename,"wb");

	if(old_fp)
	{
		WCHAR wc;
		//write out the unicode header
		unsigned char uni_header=0xff;
		fwrite(&uni_header,sizeof(unsigned char),1,new_fp);
		uni_header=0xfe;
		fwrite(&uni_header,sizeof(unsigned char),1,new_fp);

		// clear UNICODE header
		wc=fgetwc(old_fp);

		// Read first proper character
		wc=fgetwc(old_fp);

		bool end=false;
		while(!end)
		{
			// store the define
			int count=0;
			while(wc != 0x0020)
			{
				line[count++]=wc;
				wc=fgetwc(old_fp);
			}
			line[count]=NULL;


			bool write_out_line=false;
			for(int a=0;a<mNumFoundStrings;a++)
			{
				//convert ascii define into wchar define
				WCHAR tempwc[128];
				swprintf(tempwc,L"%S",mFoundStrings[a]);

				if(!wcscmp(tempwc,line))
				{
					write_out_line=true;
				}
			}

			//add the last character we got from teh above while loop
			line[count++]=wc;

			// Read the rest of the line
			wc=fgetwc(old_fp);
			while(wc != 0x000d)
			{
				line[count++]=wc;
				wc=fgetwc(old_fp);
			}
			line[count]=NULL;


			if(write_out_line)
			{
				WCHAR string[640+128];

				swprintf(string,L"%s\r\n",line);

				int b=0;
				while(string[b])
				{
					fwrite(&string[b],sizeof(WCHAR),1,new_fp);
					b++;
				}
			}

			// clear the line feed
			wc=fgetwc(old_fp);
			if(wc == 65535) end=true;
			
			wc=fgetwc(old_fp);
			if(wc == 65535) end=true;
		}

		fclose(new_fp);
		fclose(old_fp);
	}
}

void TextManager::FoundFile(FILE *fp)
{
	char c;
	int found=0;

	c=fgetc(fp);
	while(c != EOF)
	{
		if(c == 'U' && found == 0) found++;
		else if(c == 'I' && found == 1) found++;
		else if(c == 'T' && found == 2) found++;
		else if(c == 'e' && found == 3) found++;
		else if(c == 'x' && found == 4) found++;
		else if(c == 't' && found == 5) found++;
		else if(c == '.' && found == 6) found++;
		else found=0;

		if(found == 7)
		{
			char w;
			char ids_value[128];
			int ptr=0;

			w=fgetc(fp);
			while(w != ',' && w != 0x20 && w != 0x0d && w!= 0x0a)
			{
				ids_value[ptr++]=w;
				w=fgetc(fp);
			}
			ids_value[ptr]=NULL;
			found=0;

			bool already_found=false;

			for(int a=0;a<mNumFoundStrings;a++)
			{
				if(!strcmp(mFoundStrings[a],ids_value))
				{
					already_found=true;
				}
			}

			if(already_found == false)
			{
				strcpy(mFoundStrings[mNumFoundStrings],ids_value);
				mNumFoundStrings++;
			}
						
			gCommandLine.AddLineArg(TC_RED,"Found %s",ids_value);
		}
		c=fgetc(fp);
	}
}

void TextManager::SearchFiles(const char *dir)
{
	struct _finddata_t lua_file;
    long hFile;
	char name[256];
	char filename[256];

	sprintf(name,"%s\\*",dir);

	if( (hFile = _findfirst( name, &lua_file )) != -1)
	{
		if(strcmp(lua_file.name,".") && strcmp(lua_file.name,"..") && !(lua_file.attrib&_A_HIDDEN))
		{
			if(lua_file.attrib&_A_SUBDIR)
			{
				sprintf(name,"%s\\%s",dir,lua_file.name);
				this->SearchFiles(name);
			}
			else
			{
				//search file
				FILE *fp;
				sprintf(filename,"%s\\%s",dir,lua_file.name);
				fp=fopen(filename,"r");
				if(fp)
				{
					gCommandLine.AddLineArg(TC_BLUE,"Searching %s",lua_file.name);
					FoundFile(fp);
					fclose(fp);
				}
			}
		}

		while( _findnext( hFile, &lua_file ) == 0 )
		{
			if(strcmp(lua_file.name,".") && strcmp(lua_file.name,"..") && !(lua_file.attrib&_A_HIDDEN))
			{
				if(lua_file.attrib&_A_SUBDIR)
				{
					sprintf(name,"%s\\%s",dir,lua_file.name);
					this->SearchFiles(name);
				}
				else
				{
					//search file
					FILE *fp;
					sprintf(filename,"%s\\%s",dir,lua_file.name);
					fp=fopen(filename,"r");
					if(fp)
					{
						gCommandLine.AddLineArg(TC_BLUE,"Searching %s",lua_file.name);
						FoundFile(fp);
						fclose(fp);
					}
				}
			}
		}
	}

	_findclose(hFile);
}

void TextManager::CleanUp()
{
	char filename[128];

	//backup the file
	gSystemCommands.Flush();
	sprintf(filename,"copy /y \"%s\\English.ini\" %s\\English.old",gUI.Config.UIMakerTextDir,gUI.Config.UIMakerTextDir);
	gSystemCommands.AddCommand(filename);
	gSystemCommands.Execute();

	SearchInit();
	//parse all lua files and make a big list of all the text
	SearchFiles(gUI.Config.LuaDir);

	SearchEnd();
}




