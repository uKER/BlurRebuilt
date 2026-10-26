// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>

#ifdef BD_PLATFORM_WII

#include <bdGraphics/bdMesh.h>
#include <bdGraphics/bdGfx.h>


#define BD_LOG_LEVEL "mesh-wii"

void bdMesh::draw() const
{
	if (m_visible)
	{
		// The type of primitive that we want GX to draw. Makes sense of the vertex data
		GXPrimitive primType;
		bdUInt vertexCount;

		// The vertex count is either the number of indices or vertices depending on whether or not the mesh is indexed
		vertexCount = (m_numIndices != 0) ? m_numIndices : m_numVertices;

  		switch (m_meshType)
		{
			case BD_MESH_LINE_LIST:
				primType = GX_LINES;
				break;
			case BD_MESH_TRIANGLE_LIST:
				primType = GX_TRIANGLES;
				break;
			case BD_MESH_TRIANGLE_STRIP:
				primType = GX_TRIANGLESTRIP;
				break;							
			case BD_MESH_TRIANGLE_FAN:
				primType = GX_TRIANGLEFAN;
				break;							
			default:
				BD_ERR(BD_LOG_LEVEL, "m_meshType must be one of the bdMeshType enum values");
				primType = GX_LINES;
		}

        GXSetNumTexGens( 0 );
    	GXSetNumTevStages( 1 );
    	GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
		
		GXClearVtxDesc();
		// This sets the input mode as "direct" for indexed, use: GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
		GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
		
		
		// Position has 3 elements (x,y,z), each of type float32 GX_F32,
		// no fractional bits (integers)
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		
		// Color 0 has 4 components (r, g, b, a), each component is 8b.
    	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
		
		// Initialize lighting, texgen, and tev parameters
		GXSetNumChans(1); // default, color = vertex color
		GXSetNumTexGens(0); // no texture in this demo
    	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
  /*
	//----- TEV
	GXSetNumTevStages( 1 ) ;
  GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL ) ;
	GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO ) ;
	GXSetTevColorOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV ) ;
	GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO ) ;
	GXSetTevAlphaOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV ) ;
	GXSetAlphaCompare( GX_ALWAYS,0, GX_AOP_OR, GX_ALWAYS,0 ) ;

					GXColor		orange_clr   = { 245,125, 32, 255 } ;


    GXSetTevColor( GX_TEVREG0, orange_clr  ) ;
*/	 
 	     GXBegin(primType, GX_VTXFMT0, vertexCount); 
		for(int i=0;i<vertexCount;i++)
		{
			GXPosition3f32(m_vertexBuffer[i].m_x + m_position[0], m_vertexBuffer[i].m_y + m_position[1], m_vertexBuffer[i].m_z);
			GXColor4u8(m_color.getRed(), m_color.getGreen(), m_color.getBlue(), m_color.getAlpha());
			// 	printf("GXPosition3f32(%.3f,%.3f,%.3f);\n",m_vertexBuffer[i].m_x, m_vertexBuffer[i].m_y, m_vertexBuffer[i].m_z);
		}                  
        GXEnd();  
	}
}

#endif // BD_PLATFORM_WII
