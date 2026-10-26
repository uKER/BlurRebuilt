#ifndef RENDERDATACONVERSION_GRAPH_CLUSTER_H
#define RENDERDATACONVERSION_GRAPH_CLUSTER_H

#include "btl\containers\list.h"
#include "btl\containers\algorithm.h"
#include "btl\containers\map.h"
#include "btl\containers\multimap.h"

namespace RenderDataConversion {
namespace Clustering
{

// The graph contains a number of nodes.
// The nodes can be linked together and the length of the connections specified.
// The connections are sorted in order of length.
// Nodes can be merged, combining connections.
class Graph
{
public:
	class Node;
	typedef Btl::List<Node, Btl::ListParameters_External<> > NodeList;
	typedef NodeList::Iterator NodeIterator;
	
	struct Connection
	{
		Connection() {}
		Connection(NodeIterator i1, NodeIterator i2) : first(i1), second(i2) {}

		NodeIterator first;
		NodeIterator second;
	};

	friend bool operator < (const Graph::Connection& a, const Graph::Connection& b);

	class Node
	{
	public:
		Node() : weight(1.f) {}

		S32 GetNumConnections() const {return m_connections.Size();}
		NodeIterator GetConnection(int i) {return m_connections[i];}

	private:
		friend Graph;
		Btl::Vector<NodeIterator> m_connections;
		float weight;
	};
	
	///////////////////////////////////////////

	// Add a new node to the graph.
	NodeIterator AddNode();

	// Add a connection, or do nothing if it already exists.
	bool AddConnection(NodeIterator first, NodeIterator second, float length);

	// Returns true if the nodes are connected and gets the length of the connection.
	bool AreConnected(NodeIterator first, NodeIterator second, float* length);

	// Set the length of an existing connection.
	void SetConnectionLength(Graph::NodeIterator first, Graph::NodeIterator second, float length);

	// Get the shortest connection in the graph.
	Connection* GetShortestConnection(float* length = 0);

	// Merge nodes, which must be connected.
	NodeIterator MergeNodes(NodeIterator first, NodeIterator second);

	NodeIterator NodesBegin() {return m_nodes.Begin();}
	NodeIterator NodesEnd() {return m_nodes.End();}

	// Removes a connection and returns its length.
	float RemoveConnection(NodeIterator first, NodeIterator second);

	bool Empty() const {return m_nodes.Size() == 0;}
	S32 GetNumNodes() const {return m_nodes.Size();}

	S32 GetNumConnections() const {return m_connections.Size();}

	void EraseNode(NodeIterator node);

private:
	typedef Btl::MultiMap<float, Connection> SortedConnections;
	typedef Btl::Map<Connection, SortedConnections::Iterator> ConnectionMap;

	SortedConnections m_sortedConnections; // store connections sorted by length
	ConnectionMap m_connections; // store iterators to find items in the first map

	NodeList m_nodes;
};

}}

#endif


