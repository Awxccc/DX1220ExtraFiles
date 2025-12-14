#include <random>
#include <algorithm>
#include "Graph.h"
#include "MyMath.h"

Graph::Graph()
	: m_nodes{}
{
}

Graph::~Graph()
{
	for (Node* n : m_nodes)
		delete n;
}

void Graph::AddNode(Node* node)
{
	node->id = static_cast<int>(m_nodes.size());
	m_nodes.push_back(node);
}

//multiplier can be used for special terrains to increase or decrease cost.
//for example, if agents walk slower on sand, we can give the corresponding edge
//a higher multiplier(> 1)
//note that multiplier should never be negative!
void Graph::AddEdge(Node* from, Node* to, float multiplier)
{
	float cost = multiplier * (from->pos - to->pos).Length();
	//let's just connect the 2 nodes both ways
	from->edges.push_back(Edge{ from, to, cost });
	to->edges.push_back(Edge{ to, from, cost });

	//using this for collision test and rendering only
	m_edges.push_back(Edge{ from, to, multiplier*(from->pos - to->pos).Length()});
}

static bool IntersectLine(const Vector3& p0, const Vector3& p1,
						  const Vector3& p2, const Vector3& p3)
{
	Vector3 v{ p1 - p0 };
	Vector3 w{ p3 - p2 };

	//check if lines are parallel
	float determinant = w.y * v.x - w.x * v.y;
	if (fabs(determinant) < 0.0001f)
		return false;

	float dx = p2.x - p0.x;
	float dy = p2.y - p0.y;
	//find time of collision along v and w. each has its own unit of time.
	float t = (w.y * dx - w.x * dy) / determinant; //time along v
	float s = (v.y * dx - v.x * dy) / determinant; //time along w

	if (t <= 0 || t >= 1 || s <= 0 || s >= 1)
		return false;

	return true;
}

//there is no right or wrong way to implement this function
//just make a graph of nodes here, preferably without overlapping edges
//how this function is implemented is NOT the focus of this practical
//this algorithm is written on a whim and will not give super nice results.
//for a nicer looking result, look up "delaunay triangulation"
void Graph::Generate(unsigned key, int count, Vector3 minPt, Vector3 maxPt, float minSeparation)
{
	m_nodes.clear();
	m_edges.clear();

	//instantiate an RNG just for this function
	//if you like, rand() works too
	std::mt19937 mt{key}; //key is the seed
	std::uniform_real_distribution<float> distrX{ minPt.x, std::nextafterf(maxPt.x, FLT_MAX) };
	std::uniform_real_distribution<float> distrY{ minPt.y, std::nextafterf(maxPt.y, FLT_MAX) };

	//instantiate the specified number of nodes
	Vector3 proposedPos{};
	std::vector<Node*> filteredNodes;
	for (int i = 0; i < count; ++i)
	{
		int iterations{};
		do
		{
			//generate random position
			proposedPos.x = distrX(mt);
			proposedPos.y = distrY(mt);

			//let's make use of the standard template library to ensure new node
			//doesn't collide with existing nodes
			//iterations < 10 is just to ensure there isn't any possibilities of an infinite loop
		} while (iterations < 10 && std::find_if(m_nodes.begin(), m_nodes.end(), [proposedPos, minSeparation](const Node* n)
			{
				//returns true if position is too close to an existing node
				return (n->pos - proposedPos).LengthSquared() <= minSeparation*minSeparation;
			}) != m_nodes.end());

		//make node with desired position
		Node* node = new Node{ proposedPos };

		//try connecting this node to other nodes
		//copy node from graph to temporary array if they are close enough
		std::copy_if(m_nodes.begin(), m_nodes.end(), std::back_inserter(filteredNodes), [proposedPos, minSeparation](const Node* n)
			{
				float dist = minSeparation * minSeparation;
				return (proposedPos - n->pos).LengthSquared() <= dist * dist;
			});

		//connect new node to other nodes
		for (Node* fnode : filteredNodes)
		{
			//check if proposed edge intersects any other edges
			auto it = 
			std::find_if(m_edges.begin(), m_edges.end(), [node, fnode](Edge& e) {
				return IntersectLine(node->pos, fnode->pos, e.from->pos, e.to->pos);
			});

			//add edge if that is no intersection with other edges
			if (it == m_edges.end())
				AddEdge(node, fnode);
		}

		//add node into graph
		AddNode(node);

		filteredNodes.clear();
	}
}

//returns index to nearest node
int Graph::NearestNode(const Vector3& pos) const
{
	//let's opt to use more of the STL functions
	//we pass in a function as the 3rd argument specifying how the elements should be ordered when
	//determining which is the smallest element.
	//if this looks scary to you, you can always just use a for-loop
	float posX = pos.x;
	float posY = pos.y;
	std::vector<Node*>::const_iterator it =
	std::min_element(m_nodes.begin(), m_nodes.end(), [posX, posY](const Node* lhs, const Node* rhs) {
		float dx1 = lhs->pos.x - posX;
		float dy1 = lhs->pos.y - posY;
		float dx2 = rhs->pos.x - posX;
		float dy2 = rhs->pos.y - posY;
		//comparison via squared distance
		return (dx1 * dx1 + dy1 * dy1) < (dx2 * dx2 + dy2 * dy2);
		});

	//return -1 if no node is found
	if (it == m_nodes.end())
		return -1;

	//return index to found node
	return it - m_nodes.begin();
}
