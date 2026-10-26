// Turn on the shader optimisation
set RUNSHADERITERATOR=1
// Only allow XGE to use 1 CPU per PC (stops us hammering PC's)
set SHADERITERATORLIMITCPU=1
// Total number of CPU's we can be allocated
set SHADERITERATORMAXCPU=100
// How many iterations to perform (more is slower)
rem set RANDOM_SHADER_ITERATIONS=100000
set RANDOM_SHADER_ITERATIONS=1000
// How many jobs to split the number of iterations up into
set RANDOM_SHADER_WORKERS=90

// Run the job... PS3 Speed only (-a forces all shaders to be rebuilt - won't need to rehash if hash already exists though)
rem jam -sTARGET=PS3 -sBUILD=Speed -j1 -a pixelshaders
jam -sTARGET=PS3 -sBUILD=Speed -j1 -a pixelshaders
pause
