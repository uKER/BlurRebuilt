// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>

#ifdef BD_PLATFORM_PS2

#include <bdGraphics/bdMesh.h>
#include <bdGraphics/bdGfx.h>

#include <libvu0.h>
#include <shellCore.h>

#define BD_LOG_LEVEL "mesh-ps2"

void bdTransformVertices(sceVu0FVECTOR *outputVerts,
						 sceVu0FVECTOR *inputVerts,
						 sceVu0FMATRIX projectionMatrix,
						 int nVertices)
{
	asm __volatile__("

		lqc2 			vf4, 0x00(%2)     # load projection matrix
		lqc2 			vf5, 0x10(%2)
		lqc2 			vf6, 0x20(%2)
		lqc2 			vf7, 0x30(%2)

# REALLY SIMPLE SLOW ROT-TRANS-PERS ON VERTICES AND TEXTURE COORD

		lqc2			vf8,0x00(%1)		# load 1st vertex

_RotTransPersN_loop:
		vmulaw.xyzw     ACC, vf7,vf0		# apply matrix (result in vf12)
		vmadday.xyzw    ACC, vf5,vf8
		vmaddaz.xyzw    ACC, vf6,vf8
		vmaddx.xyzw     vf12,vf4,vf8    

		vdiv Q,         vf0w, vf12w			# Perspective divide
#		lqc2            vf30, 0x00(%1)		# load STQ
		vnop
		vwaitq

		vmulq.xyz		vf12,vf12,Q			# Multiply XYZ by Q
#		vmulq.xyz       vf30,vf30,Q			# Multiply STQ by Q
		vnop

		vftoi4.xyzw		vf12, vf12			# Float to int on XYZ
		vmulw.w			vf12, vf8, vf0		# copy back original ADC
#		sqc2            vf30,0x00(%0)		# Store STQ
		vnop
		addi       		%1,0x10				# InputAddr  += 1        
		lqc2     		vf8,0x00(%1)		# Load next XYZ
		addi	        %0,0x10				# outputAddr += 1
		addi	        %3,-1				# nVertices--
		sqc2      		vf12,-0x10(%0)		# Store XYZ and ADC

		bne	        	$0,%3,_RotTransPersN_loop
		vnop
		": : "r" (outputVerts) ,"r" (inputVerts), "r"(projectionMatrix), "r" (nVertices));
}

inline void bdPackVertex(u_long *outputVerts,
					   sceVu0FVECTOR *inputVerts,
					   sceVu0FMATRIX projectionMatrix,
					   int nVertices)
{
	u_long result = 0L;

	u_short x;
	u_short y;
	u_int z;

	sceVu0IVECTOR out[3];

//	bdTransformVertices(out, inputVerts, projectionMatrix, nVertices);

	for (bdInt i = 0; i < nVertices; i++)
	{
		x = static_cast<u_short>(out[i][0] & 0x0000ffff);
		y = static_cast<u_short>(out[i][1] & 0x0000ffff);
		z = static_cast<u_long>(out[i][2]);

		result = static_cast<u_long>(x);
		result |= static_cast<u_long>(y) << 16;
		result |= static_cast<u_long>(z) << 32;

		outputVerts[i] = *reinterpret_cast<u_long*>(&result);
	}
}

void bdMesh::draw() const
{
	if (m_visible)
	{
		// The type of primitive that we want the GS to draw. Makes sense of the vertex data
		bdUInt primType;

		// Each GS primitive contains of multiple of this number of vertices. E.g. For a triangle list, it would be 3
		bdUInt vertsPerLoop;

		switch (m_meshType)
		{
		case BD_MESH_LINE_LIST:
			primType = SCE_GS_PRIM_LINE;
			vertsPerLoop = 2;
			break;
		case BD_MESH_TRIANGLE_LIST:
			primType = SCE_GS_PRIM_TRI;
			vertsPerLoop = 3;
			break;
		case BD_MESH_TRIANGLE_STRIP:
			primType = SCE_GS_PRIM_TRISTRIP;
			vertsPerLoop = 1;
			break;
		case BD_MESH_TRIANGLE_FAN:
			primType = SCE_GS_PRIM_TRIFAN;
			vertsPerLoop = 1;
			break;
		default:
			BD_ERR(BD_LOG_LEVEL, "m_meshType must be one of the bdMeshType enum values");

			// Initialize variable to stop non-initialized variable compiler warning
			primType = SCE_GS_PRIM_LINE;
			vertsPerLoop = 2;
		}

		// To draw the mesh we need to build a GS Packet, which is sent to the GIF using a DMA transfer.
		// The GS Packet consists of several consecutive GS Primitives. Each GS Primitive consists of a GIF Tag
		// at the beginning which describes its payload and a data payload, containing the drawing data for the
		// GS.
		// Our GS Packet starts off with a GS Primitive which sets the color of the mesh.
		// The remaining GS Primitives give the vertex data for the mesh.
		// The GIF Tags for the vertex data only sets 
		const bdUInt BD_PS2_QUADWORD_SIZE = 128 / 8;
		const bdUInt BD_MAX_NLOOPS = (1 << 15);
		const bdUInt BD_MAX_VERTS_PER_GS_PRIM = vertsPerLoop * BD_MAX_NLOOPS;

		ShellMatrix worldMatrix;
		ShellVector position;
		ShellMatrix transformationMatrix;
		bdUInt pktIndex = 0;
		u_long *packet;

		// Calculate the number of GS Primitives we'll need to create for the packet
		bdUInt numGSPrims = m_numVertices / BD_MAX_VERTS_PER_GS_PRIM;
		if ((m_numVertices % BD_MAX_VERTS_PER_GS_PRIM) != 0)
		{
			numGSPrims++;
		}

		const bdUInt BD_VERTS_IN_LAST_GS_PRIM = (m_numVertices - ((numGSPrims - 1) * BD_MAX_VERTS_PER_GS_PRIM));

		// Calculate the size of the GS Packet
		const bdUInt BD_MESH_PACKET_SIZE = BD_PS2_QUADWORD_SIZE *
			(5 + BD_VERTS_IN_LAST_GS_PRIM + ((numGSPrims - 1) * (BD_MAX_VERTS_PER_GS_PRIM + 1)));

		// Allocate the memory for the packet, aligned to 128 bytes. Adjust the pointer so that it bypasses the cache
/*		packet = reinterpret_cast<u_long*>(bdGfxImpl::UncachePointer(
			bdMemory::alignedAllocate(BD_MESH_PACKET_SIZE, 128)));
*/
		packet = bdGfx::getInstance()->m_packetMem;

		// Copy the position to a local variable
		shellVU0MathVectorSet(	&position,
								m_position.getX(),
								m_position.getY(),
								m_position.getZ(),
								m_position.getW());

		// Save the transformation matrix
		shellVU0MathMatrixStore1i(&transformationMatrix);

		// Create the local-to-world matrix
		sceVu0UnitMatrix(worldMatrix.m);
		sceVu0TransMatrix(worldMatrix.m, worldMatrix.m, position.v);

		// Create the final transformation matrix by multiplying the old one with the local-to-world matrix
		sceVu0MulMatrix(transformationMatrix.m, transformationMatrix.m, worldMatrix.m);

		// Create the GIF Tag that we use to for the GS Primitive that sets the color for the rest of the mesh
		// This contains the per-mesh data.	
		packet[pktIndex++] = SCE_GIF_SET_TAG(1,				// NLOOP
											0,				// EOP
											0,				// PRE
											0,				// PRIM
											SCE_GIF_REGLIST,// FLG
											1);				// NREG
		packet[pktIndex++] = 0x0000000000000001L;			// Setting the color reg
		packet[pktIndex++] = SCE_GS_SET_RGBAQ(	m_color.getRed(),	// Set the red component
												m_color.getGreen(),	// Set the blue component
												m_color.getBlue(),	// Set the green component
												m_color.getAlpha(),	// Set the alpha component
												0);					// Q: Unused
		packet[pktIndex++] = 0L;	// The next 64-bits are unused.


		{
			packet[pktIndex++] = SCE_GIF_SET_TAG(1, 0, 0, 0, SCE_GIF_PACKED, 1);
			packet[pktIndex++] = 0xe;
			packet[pktIndex++] = SCE_GS_SET_TEST(0, 1, 0, 0, 0, 0, 1, 1);
			packet[pktIndex++] = 0x47;
		}


		if (m_indexBuffer == 0)
		{
			bdUInt vertexIndex = 0;

			for (bdUInt gifTagIndex = 0; gifTagIndex < numGSPrims; gifTagIndex++)
			{
				u_long NLOOP;
				u_long EOP;
				u_long PRE = 1;

				if (gifTagIndex == numGSPrims - 1)
				{
					EOP = 1;
					NLOOP = BD_VERTS_IN_LAST_GS_PRIM / vertsPerLoop;
					BD_ASSERT((BD_VERTS_IN_LAST_GS_PRIM % vertsPerLoop) == 0,
						"VERTS_IN LAST_GS_PRIM is not a multiple of NREG");
				}
				else
				{
					// This is not the last GS Primitive in the GS Packet, stuff it with vertices
					NLOOP = BD_MAX_NLOOPS - (BD_MAX_NLOOPS % vertsPerLoop);
					EOP = 0;
				}
				
				// Insert the GIF tag for the vertex data into the packet
				packet[pktIndex++] = SCE_GIF_SET_TAG(NLOOP,
													EOP,
													PRE,
													SCE_GS_SET_PRIM(primType, 0, 0, 0, 0, 0, 0, 0, 0),
													SCE_GIF_PACKED,
													vertsPerLoop);
				packet[pktIndex++] = 0x5555555555555555L;

				// Create a buffer for the vertices
				ShellVector inputVertex[vertsPerLoop * NLOOP];
				for (bdUInt i = 0; i < NLOOP * vertsPerLoop; i++, vertexIndex++)
				{
					bdUInt adc = ((i % vertsPerLoop) == (vertsPerLoop - 1)) ? 0 : ~0;
					shellVU0MathVectorSet(&inputVertex[i],
										m_vertexBuffer[vertexIndex].m_x,
										m_vertexBuffer[vertexIndex].m_y,
										m_vertexBuffer[vertexIndex].m_z,
										0);

					*reinterpret_cast<bdUInt*>(&inputVertex[i].v[3]) = adc;
				}

				// Transform the vertices and pack them into the packet
				bdTransformVertices(reinterpret_cast<sceVu0FVECTOR*>(&packet[pktIndex]),
					&inputVertex[0].v,
					transformationMatrix.m,
					vertsPerLoop * NLOOP);
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Indexed meshes currently unimplemented for PS2");
		}

		FlushCache(0);

		// Send the GS Packet off to the GIF
		shellDmaStart(SHELLDMA_CHANNEL_GIF, static_cast<void *>(packet), BD_MESH_PACKET_SIZE / 16);
		shellDmaSync(SHELLDMA_CHANNEL_GIF, 0);

		// Free the memory for the packet
//		bdMemory::alignedDeallocate(bdGfxImpl::CachePointer(packet));
	}
}

#endif // BD_PLATFORM_PS2
