
SmokeTest command line

Racer2010SmokeTest.exe
	-config <filename>	- Specify config file to use
	-sorcery <path>		- Specify path to sorcery database (i.e. %BIZARRE_ROOT%\amax\sorcery3\main)
	-pnm <filename>		- Pick n Mix file (used for selecting certain Sorcery records to use)
	-kits <filename>	- Name of file to get list of kits from
	
	-reports <folder>	- Specify a path other than the output folder to put the reports
	
	-email <filename>	- Specify an email to send when the test is done

	-soc				- Stop on crash (If a kit crashes, the automation run stops so you can attach and debug)
	-soa				- Stop on assert (If a kit asserts, the automation run stops so you can attach and debug)
	
	-id					- Ignore dependency checking on tests
	
	-stability			- Run memory leak pass
	-vehicles			- Run vehicle pass
	-dash				- Run the 'dash' test of 20 random cars for 30 seconds on each route
	-load				- Run each event to completion
	-events				- Run all the RaceBook event races
	
	-js					- Job start - job to start processing on
	-je					- Job end - job to end processing on

e.g.


Racer2010SmokeTest -sorcery %BIZARRE_ROOT%\Racer2010\sorcery3\main -pnm %BIZARRE_ROOT%\amax\smoketest.pnm -kits e:\AutoReports\kits.xml -vehicles -config e:\AutoReports\config.xml

The vehicle test pass can check the following things:

	- Does the game crash/assert trying to load this vehicle?
	- Is there a model file (or is it using the default car)?
	- Is there a collision mesh (car will drop through world if not)
	- Is there a shadow cast mesh (it will use physics mesh as a fallback)
	- Is there a dynamic object mesh (simplified mesh for colliding with dynamic objects)




Example config file:

	<config>
	  <build XBOX_360="e:\Racer2010\Racer2010_Release.xex" />
	</config>

Example kit file:

	<kits>
		<kit Name="360thingy1" Type="360"/>
		<kit Name="360thingy2" Type="360" />
	</kits>
	
Example email file:

	<root>
	<to>user@example.com</to>
	<subject>%TITLE%: Test subject</subject>
	<body>The automation results are ready to view for the latest build in test.

	Results can be viewed here:

	%PATH%
	</body>
	</root>
