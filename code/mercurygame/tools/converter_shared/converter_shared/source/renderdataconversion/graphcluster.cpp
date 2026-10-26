#include "stdafx.h"
#include "converter_shared/renderdataconversion/graphcluster.h"

//#pragma optimize("", off)

namespace RenderDataConversion {
namespace Clustering
{

// comparison operator for map
bool operator < (const Graph::Connection& a, const Graph::Connection& b)
{
	// list iterators don't have comparison operators, so use addresses
	if (&*a.first != &*b.first)
		return &*a.first < &*b.first;

	return &*a.second < &*b.second;
}

Graph::NodeIterator Graph::AddNode() 
{
	m_nodes.PushBack(Node()); 
	NodeList::ReverseIterator r = m_nodes.ReverseBegin();
	return r.Iterator();
}

// add connection, or do nothing if it already exists
bool Graph::AddConnection(Graph::NodeIterator first, Graph::NodeIterator second, float length) 
{
	if (&*first > &*second) 
		Swap(first, second);

	if (first == second)
		return false;

	if (length == 0.f)
		int breakhere = 0;

	if (Btl::Find(first->m_connections, second) == first->m_connections.End())
	{
		// add to the maps
		Connection connection(first, second);
		SortedConnections::Iterator result = m_sortedConnections.Insert(SortedConnections::DataType(length, connection));
		m_connections.Insert(ConnectionMap::DataType(connection, result));

		// add to the nodes
		first->m_connections.PushBack(second);
		second->m_connections.PushBack(first);

		return true;
	}

	return false;
}

// return true if the nodes are connected and get the length of the connection
bool Graph::AreConnected(Graph::NodeIterator first, Graph::NodeIterator second, float* length)
{
	if (&*first > &*second) 
		Swap(first, second);

	assert(first != second);
	
	Connection connection(first, second);

	ConnectionMap::Iterator result = m_connections.Find(connection);

	if (result != m_connections.End())
	{
		if (length) *length = result->GetValue()->GetKey();
		return true;
	}
	else
		return false;
}

void Graph::SetConnectionLength(Graph::NodeIterator first, Graph::NodeIterator second, float length)
{
	if (&*first > &*second) 
		Swap(first, second);

	assert(first != second);
	
	Connection connection(first, second);

	// erase from both maps
	{
		ConnectionMap::Iterator result = m_connections.Find(connection);
		assert(result != m_connections.End());
		m_sortedConnections.Erase(result->GetValue());
		m_connections.Erase(connection);
	}

	// add to the maps
	{
		SortedConnections::Iterator result = m_sortedConnections.Insert(SortedConnections::DataType(length, connection));
		m_connections.Insert(ConnectionMap::DataType(connection, result));
	}
}

// remove a connection and return its length
float Graph::RemoveConnection(Graph::NodeIterator first, Graph::NodeIterator second)
{
	if (&*first > &*second) 
		Swap(first, second);

	assert(first != second);

	// see if nodes are connected
	Connection connection(first, second);

	// erase from both maps
	ConnectionMap::Iterator result = m_connections.Find(connection);
	assert(result != m_connections.End());
	float length = result->GetValue()->GetKey();
	m_sortedConnections.Erase(result->GetValue());
	m_connections.Erase(connection);

	// erase from nodes (find, move to the end, pop back)
	Swap(*Find(first->m_connections, second), first->m_connections.Back());
	first->m_connections.PopBack();

	Swap(*Find(second->m_connections, first), second->m_connections.Back());
	second->m_connections.PopBack();

	return length;
}

// Get the shortest connection
Graph::Connection* Graph::GetShortestConnection(float* length)
{
	if (!m_sortedConnections.Empty())
	{
		if (length)
			*length = m_sortedConnections.Begin()->GetKey();

		return &(m_sortedConnections.Begin()->GetValue());
	}
	else
		return 0;
}

void Graph::EraseNode(NodeIterator node)
{
	Node* pNode = &*node;

	// copy connections from both nodes
	while (!node->m_connections.Empty())
	{
		NodeIterator connected = node->m_connections.Front();
		RemoveConnection(connected, node);
	}

	m_nodes.Erase(node);
}

// merge nodes, which must be connected
Graph::NodeIterator Graph::MergeNodes(Graph::NodeIterator first, Graph::NodeIterator second)
{
	if (&*first > &*second) 
		Swap(first, second);

	assert(first != second);

	float l = RemoveConnection(first, second);

	// make new node
	NodeIterator newNode = AddNode();
	newNode->weight = first->weight + second->weight;

	// copy items to new node
	//for (int i = 0; i < first->m_elements.Size(); i++)
	//	newNode->m_elements.PushBack(first->m_elements[i]);

	//for (int i = 0; i <  second->m_elements.Size(); i++)
	//	newNode->m_elements.PushBack(second->m_elements[i]);

	// copy connections from both nodes
	while (!first->m_connections.Empty())
	{
		NodeIterator node = first->m_connections.Front();

		// Get the length of the connection to the new node.
		// Use weighted average of connections to original nodes.
		float l1 = RemoveConnection(node, first);
		float l2 = l1 + l; AreConnected(node, second, &l2);
		float newLength = (l1 * first->weight + l2 * second->weight)
						/ newNode->weight;

		AddConnection(node, newNode, newLength);
	}

	while (!second->m_connections.Empty())
	{
		NodeIterator node = second->m_connections.Front();

		// As above, but if the node was connected to first,
		// it already has a connection of the correct length.
		float l2 = RemoveConnection(node, second);
		float l1 = l2 + l;
		float newLength = (l1 * first->weight + l2 * second->weight)
						/ newNode->weight;

		AddConnection(node, newNode, newLength);
	}

	// delete nodes
	m_nodes.Erase(first);
	m_nodes.Erase(second);

	return newNode;
}

}}