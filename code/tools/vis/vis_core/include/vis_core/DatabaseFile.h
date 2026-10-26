#pragma once

#define DATABASE_FILE_VERSION 1

struct DatabaseFileHeader
{
	int		iVersion;						// output file version number (1)
	int		nPrimitives;					// number of individual primitives that have their own bitfield
	int		iBitfieldSizeInBytes;			// size of each bitfield in bytes
};

struct DatabaseFilePrimitive
{
	int iNode;								// aftk index of the scene node
	int iTri;								// index of the polygon within the scene node
};
