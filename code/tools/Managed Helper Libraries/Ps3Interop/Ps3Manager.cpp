
#include "stdafx.h"

#include <windows.h>
#include "ps3tmapi.h"

#include "Ps3Console.h"

namespace Ps3Interop
{

	public ref class Ps3Manager
	{
	public:
		Ps3Manager()
		{
			//SNPS3InitTargetComms();
		}

		Ps3Console^ OpenConsole(String^ name)
		{
			Ps3Console^ console = gcnew Ps3Console;

			return console;
		}
	};

}