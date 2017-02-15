#ifndef ASTAR_H
#define ASTAR_H

#include <queue>

#include "Grid.h"

class GridNode;
class Grid;

class AStarGridNode
{
public:
	GridNode* node;
	AStarGridNode* parent;
	int G, F;

	AStarGridNode()
	{
		node = NULL;
		parent = NULL;
		G = F = 0;
	}

	AStarGridNode(GridNode* n, AStarGridNode* p, int G, int H)
	{
		this->node = n;
		this->parent = p;
		this->G = G;
		this->F = G + H;
	}

	bool operator() (AStarGridNode* lhs, AStarGridNode* rhs) const
	{
		if(lhs->node == rhs->node) return false;
		return lhs->F < rhs->F;
	}

	bool operator== (AStarGridNode rhs) const
	{
		return this->node == rhs.node;
	}
};

typedef std::multiset<AStarGridNode*, AStarGridNode> AStarPriorityQueue;

AStarPriorityQueue::iterator findGridNode(AStarPriorityQueue& pq, AStarGridNode& gn);

std::list<GridNode*> findPath(Grid* grid, GridNode* start, GridNode* end);

void deleteAStarNodes(AStarPriorityQueue& nodes);

#endif
