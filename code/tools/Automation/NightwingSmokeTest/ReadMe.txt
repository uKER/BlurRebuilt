
SmokeTest command line

NightwingSmokeTest.exe
	-config <filename>	- Specify config file to use
	-sorcery <path>		- Specify path to sorcery database (i.e. %BIZARRE_ROOT%\amax\sorcery3\main)
	-pnm <filename>		- Pick n Mix file (used for selecting certain Sorcery records to use)
	-kits <filename>	- Name of file to get list of kits from
	
	-email <filename>	- Specify an email to send when the test is done

	-soc				- Stop on crash (If a kit crashes, the automation run stops so you can attach and debug)
	-soa				- Stop on assert (If a kit asserts, the automation run stops so you can attach and debug)
	
	-js					- Job start - specify which job to start processing on
	-je					- Job end - specify which job to end processing on
	
	-id					- Ignore dependency checking on tests

e.g.

NightwingSmokeTest -sorcery %BIZARRE_ROOT%\nightwing\sorcery3\main -kits e:\AutoReports\kits.xml -config e:\AutoReports\config.xml


Example config file:

	<config>
	  <build XBOX_360="e:\Nightwing\Bond09_Release.xex" />
	</config>

Example kit file:

	<kits>
		<kit Name="360thingy1" Type="360"/>
		<kit Name="360thingy2" Type="360" />
	</kits>