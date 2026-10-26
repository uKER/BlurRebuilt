//=============================================================================
// Graph.h
// 30/11/09
//=============================================================================
#ifndef _GRAPH_H__
#define _GRAPH_H__

//=============================================================================
// Includes
//=============================================================================

//=============================================================================
// Forward Declarations
//=============================================================================
//=============================================================================
// Structs
//=============================================================================


//=============================================================================
// Class Definition
//=============================================================================
namespace Kanzen
{
	//-------------------------------------------------------------------------
	// GraphData
	//-------------------------------------------------------------------------
	class GraphData
	{	
	public:
		static const int s_graphDataElements = 200;

		GraphData();
		void SetValue(F32 val);
		int Size();

		F32 GetValue(int index);

	
		F32 m_data[s_graphDataElements];
		int m_index;
		bool m_visible;

		
	};

	
	//-------------------------------------------------------------------------
	// Graph
	//-------------------------------------------------------------------------
	class Graph
	{
	public:

		F32 m_xPos;
		F32 m_yPos;

		F32 m_width;
		F32 m_height;

		F32 m_yAxisMax;
		F32 m_yAxisMin;
		F32 GetStartX() { return m_xPos; }
		F32 GetStartY() { return m_yPos; }
		F32 GetEndX() { return m_xPos + m_width; }
		F32 GetEndY() { return m_yPos + m_height; }
	};

	//-------------------------------------------------------------------------
	// GraphRenderer
	//-------------------------------------------------------------------------
	class GraphRenderer
	{
	public:
		void RenderYAxisLine( Graph& graph, F32 yPos );
		void RenderXAxisLine( Graph& graph, F32 xPos );
		
		void RenderAxis( Graph& graph );

		void RenderData( Graph& graph, GraphData& data, Maths::RGBA& col );
	};

} // End of namespace Kanzen

#endif  //_GRAPH_H__
