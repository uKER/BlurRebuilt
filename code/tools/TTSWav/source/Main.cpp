#include "stdafx.h"

#include "Options.h"
#include "TTSWav.h"

int main(int argc, char* argv[])
{
	TTSWav::C_TTSWav app(
		"TTSWav Command-line Tool",
		"Bizarre Creations Text-To-Speech Tool",
		"tools.ttswav",
		"",
		false);
		
	return app.Run(argc, argv);
}