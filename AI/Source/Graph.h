#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include "Vector3.h"

struct Node; //forward declaration of Node
//an edge has a direction
//"from" being the starting point. and "to" being ending point
//why? imagine a graph of nodes(each node is a waypoint), there may be 
//an edge from a small hill to the ground below, but not vice-versa.
//it could be designed this way because we want the ai agent to be able to 
//jump off the hill to the ground below, but not have them super-leap their
//way to the top of the hill.
struct Edge
{
	Node* from{};
	Node* to{};
	//cost of traversal. the cost is usually just the length of the edge.
	//however, that may not always be the case, especially if different types of 
	//terrain can affect traversal speed
	float cost{};
};

struct Node
{
	Vector3 pos{};             //position of this waypoint
	std::vector<Edge> edges;   //stores all out-going edges
	int id{};                  //stores index of this node's position in the graph
};

//stores all nodes and edges
//a network of interconnected nodes is called a graph
class Graph
{
public:
	Graph();
	~Graph();

	void AddNode(Node* node);
	void AddEdge(Node* from, Node* to, float multiplier=1.f);
	void Generate(unsigned key, int count, Vector3 minPt, Vector3 maxPt, float minSeparation);

	int NearestNode(const Vector3& pos) const;

	//seriously, just use std::vectors
	std::vector<Node*> m_nodes;
	std::vector<Edge> m_edges;
};

#endif