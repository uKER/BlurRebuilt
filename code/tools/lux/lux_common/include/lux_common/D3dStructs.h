#ifndef D3DSTRUCTS_H
#define D3DSTRUCTS_H

struct TriVertex{
    F32 x, y, z;  // The transformed(screen space) position for the vertex.
    DWORD colour;   // The vertex colour.
};

#endif