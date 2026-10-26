set MAYA_LOCATION=C:\program files\Alias\Maya6.0
set MAYA_PLUG_IN_PATH=C:\MayaPlugins\6.0\Release;C:\MayaPlugins\6.0\Beta
nexus_cmd_maya6.0.exe -s "OpenScene('%BIZARRE_ROOT%\Gotham3/Art/Circuits/Tokyo/Scenes/tokyo_main_scene.mb', 'triangulate=false|game=pgr3|validatematerials=false');SaveScene('f:/160/tokyocmd.a3d');" -l "c:/nexus_cmd_line.log"