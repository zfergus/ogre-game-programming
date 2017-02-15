#include "AStar.h"

std::list<GridNode*> findPath(Grid* grid, GridNode* start, GridNode* end)
{
	AStarPriorityQueue open   = AStarPriorityQueue();
	AStarPriorityQueue closed = AStarPriorityQueue();

	AStarGridNode* startANode = new AStarGridNode(start, NULL, 0, 
		grid->getDistance(start, end));
	open.insert(startANode);

	AStarGridNode* currentNode = startANode;
	while (!(open.empty()))
	{
		currentNode = *(open.begin()); // Get the first sorted node
		open.erase(open.begin()); // Pop the from of the queue

		/* If the current node is the destination we are done. */
		if(currentNode->node == end)
			break;
	
		/* Construct an array of the neighboring nodes. */
		GridNode *neighbors[] = {
			grid->getNorthNode(currentNode->node), 
			grid->getSouthNode(currentNode->node), 
			grid->getEastNode(currentNode->node),
			grid->getWestNode(currentNode->node),
			grid->getNENode(currentNode->node), 
			grid->getNWNode(currentNode->node), 
			grid->getSENode(currentNode->node),
			grid->getSWNode(currentNode->node)
		};

		for(int i = 0; i < 8; i++)
		{
			AStarGridNode* neighbor = new AStarGridNode(neighbors[i], 
				currentNode, currentNode->G + (i<4) ? (10):(14), 
					grid->getDistance(neighbors[i], end));

			/* If the neighbor node is clear and has not already been */
			/* explored.                                              */
			if(neighbors[i] != NULL && neighbors[i]->isClear() &&
			  (closed.empty() || findGridNode(closed, *neighbor) == closed.end()))
			{
				auto tmp = findGridNode(open, *currentNode);
				if(tmp == open.end())
				{
					open.insert(neighbor);
				}
				else if((*tmp)->G < neighbor->G)
				{
					delete *tmp;
					open.erase(tmp);
					open.insert(neighbor);
				}
				else // Otherwise the new neighbor memory is not needed.
				{
					delete neighbor;
				}
			}
		}

		closed.insert(currentNode);
	}

	if(currentNode->node != end)
		return std::list<GridNode*>();

	std::list<GridNode*> path;
	while(currentNode->parent != NULL)
	{
		path.push_front(currentNode->node);
		currentNode = currentNode->parent;
	}

	/* Delete the memory allocated for the AStarNodes. */
	deleteAStarNodes(open);
	deleteAStarNodes(closed);

	return path;
}

AStarPriorityQueue::iterator findGridNode(AStarPriorityQueue& pq, 
										  AStarGridNode& gn)
{
    auto first = pq.begin();
    auto last = pq.end();
    while (first!=last) {
        if (**first == gn) return first;
        ++first;
    }
    return last;
}

void deleteAStarNodes(AStarPriorityQueue& nodes)
{
	for(auto iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		delete (*iter);
	}
}
