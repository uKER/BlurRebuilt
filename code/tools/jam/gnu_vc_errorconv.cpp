#define _CRT_SECURE_NO_DEPRECATE

#define _MAX_LINE_LEN (256*16)
#define _SEPS ":()"
#define _FROM " from "

#include <stdio.h>
#include <string.h>
#include <string>

#ifndef MAX_PATH
#define MAX_PATH          260
#endif

static char _tmp[_MAX_LINE_LEN];

bool gbConvertCygdrivePath = false;

int _isNum (char *x_str)
{
	size_t l_i;

	// ~FOR 
	for (l_i=0;l_i<strlen(x_str);l_i++)
	{
		//  ~IF 
		if ( !((x_str[l_i] >= '0') && (x_str[l_i] <= '9')) )
		{ //  ~THEN 
			return 0;
		} // ~ENDIF 
	} // ~ENDFOR 

	return 1;
}

void ConvertPath(char *pOut, char *pIn)
{
	if (gbConvertCygdrivePath)
	{
		// does the path start with 'cygdrive' ?
		if (strncmp(pIn, "/cygdrive/", 10) == 0)
		{
			// yes, replace with DOS-style path
			pOut[0] = pIn[10]; // copy drive letter
			pOut[1] = ':';
			pOut[2] = 0;
			strcat(pOut, pIn+11);
			return;
		}
	}

	// it's a relative path - convert it to absolute
	if (!_fullpath(pOut, pIn, MAX_PATH))
	{
		// error
		strcpy(pOut, pIn);
	}
}

std::string gnu_vc_errorconv(char *_line)
{
	char *l_file;
	char l_fullpath[MAX_PATH];
	char *l_num;
	char *l_end;
	char *l_from;
	int l_ret;

	std::string originalLine(_line);

	char filebuf[1024];

	l_ret = 0;
	{
		//printf ("*%s", _line);
		strcpy (_tmp, _line);
		l_file = strtok( _tmp, _SEPS);	// read filename
		if (!l_file)
			return originalLine;
		
		// handle the case of absolute paths (e.g. c:\)
		if (strlen(l_file)<3)
		{
			strcpy(filebuf, l_file);
			l_file = strtok( NULL, _SEPS);	// read filename
			if (l_file != NULL)
			{
				strcat(filebuf, ":");
				strcat(filebuf, l_file);
				l_file = filebuf;
			}
		}
			

		if ( l_file != NULL )
		{
			//printf ("*file: %s\n", l_file);
			l_num = strtok( NULL, _SEPS);	// read linenumber
			
			if ( l_num != NULL )
			{ 
				l_end = strtok(NULL, "");
				//printf ("*num: %s\n", l_num);
				l_num = strtok(l_num, " ");	// read the rest of the error string
				if (l_num == NULL)
					return originalLine;
				//  
				if (_isNum (l_num) == 1)
				{
					//printf ("*is num\n");
					l_from = strstr (l_file, _FROM);	// read error
					//
					if ( l_from == NULL )
					{
						ConvertPath(l_fullpath, l_file);

						strcpy (_line, l_fullpath);

						strcat (_line, "(");
						strcat (_line, l_num);
						strcat (_line, "): ");
						//
						if (l_end != NULL)
						{
							strcat (_line, l_end);
						}
						//
						if ( strstr (_line, "warning:") == NULL )
						{
							l_ret = 1;
						}
					}
				}
			}
		}

		return std::string(_line);
	}

	return originalLine;
}