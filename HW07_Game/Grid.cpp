#include "Grid.h"
#include <iostream>
#include <fstream>
#include <climits>
#include <math.h>

////////////////////////////////////////////////////////////////
// create a node
GridNode::GridNode(int nID, int row, int column, bool isC)
{
	this->clear = isC;

	this->rCoord = row;
	this->cCoord = column;

	this->entity = NULL;

	if (isC)
		this->contains = '.';
	else
		this->contains = 'B';

	this->parent = NULL;
	this->F = this->G = this->H = INT_MAX;
}

// default constructor
GridNode::GridNode()
{
	nodeID = -999;			// mark these as currently invalid
	this->clear = true;
	this->contains = '.';
	this->parent = NULL;
	this->F = this->G = this->H = INT_MAX;
}

////////////////////////////////////////////////////////////////
// destroy a node
GridNode::~GridNode()
{}  // doesn't contain any pointers, so it is just empty

////////////////////////////////////////////////////////////////
// set the node id
void GridNode::setID(int id)
{
	this->nodeID = id;
}

////////////////////////////////////////////////////////////////
// set the x coordinate
void GridNode::setRow(int r)
{
	this->rCoord = r;
}

////////////////////////////////////////////////////////////////
// set the y coordinate
void GridNode::setColumn(int c)
{
	this->cCoord = c;
}

////////////////////////////////////////////////////////////////
// get the x and y coordinate of the node
int GridNode::getRow()
{
	return rCoord;
}

int GridNode::getColumn()
{
	return cCoord;
}

// return the position of this node given the number of rows and columns as parameters
Ogre::Vector3 GridNode::getPosition(int rows, int cols)
{
	Ogre::Vector3 t;
	t.z = (rCoord * NODESIZE) - (rows * NODESIZE)/2.0 + (NODESIZE/2.0);
	t.y = 0;
	t.x = (cCoord * NODESIZE) - (cols * NODESIZE)/2.0 + (NODESIZE/2.0);
	return t;
}

////////////////////////////////////////////////////////////////
// set the node as walkable
void GridNode::setClear()
{
	this->clear = true;
	this->contains = '.';
}

////////////////////////////////////////////////////////////////
// set the node as occupied
void GridNode::setOccupied()
{
	this->clear = false;
	this->contains = 'B';
}

////////////////////////////////////////////////////////////////
// is the node walkable
bool GridNode::isClear()
{
	return this->clear;
}

///////////////////////////////////////////////////////////////
// A-Star Methods
int GridNode::getG()
{
	return this->G;
}

int GridNode::getH()
{
	return this->H;
}

int GridNode::getF()
{
	return this->F;
}

void GridNode::setG(int G)
{
	this->G = G;
	this->F = this->G + this->H;
}

void GridNode::setH(int H)
{
	this->H = H;
	this->F = this->G + this->H;
}

void GridNode::setF(int G, int H)
{
	this->G = G;
	this->H = H;
	this->F = this->G + this->H;
}

GridNode* GridNode::getParent()
{
	return this->parent;
}

void GridNode::setParent(GridNode* parent)
{
	this->parent = parent;
}

bool GridNode::operator() (GridNode* lhs, GridNode* rhs) const
{
	if(lhs == rhs) return false;
	return lhs->F < rhs->F;
}

void GridNode::resetAStarParams()
{
	this->parent = NULL;
	this->F = this->G = this->H = INT_MAX;
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// create a grid
Grid::Grid(Ogre::SceneManager* mSceneMgr, int numRows, int numCols)
{
	this->mSceneMgr = mSceneMgr;

	assert(numRows > 0 && numCols > 0);
	this->nRows = numRows;
	this->nCols = numCols;

	data.resize(numCols, GridRow(numRows));

	// put the coordinates in each node
	int count = 0;
	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
		{
			GridNode *n = this->getNode(i,j);
			n->setRow(i);
			n->setColumn(j);
			n->setID(count);
			count++;
		}
}

/////////////////////////////////////////
// destroy a grid
Grid::~Grid(){};

////////////////////////////////////////////////////////////////
// get the node specified
GridNode* Grid::getNode(int r, int c)
{
	if (r >= nRows || c >= nCols || r < 0 || c < 0)
		return NULL;

	return &this->data[c].data[r];
}

GridNode* Grid::getNode(Ogre::Vector3 pos)
{
	// Closest Row Value
	int r = clamp(round((pos.z - NODESIZE/2.0 + (this->nRows * NODESIZE)/2.0) / 
		float(NODESIZE)), 0, this->nRows - 1);
	int c = clamp(round((pos.x - NODESIZE/2.0 + (this->nCols * NODESIZE)/2.0) /
		float(NODESIZE)), 0, this->nCols - 1);
	
	return this->getNode(r, c);
}

/* Getter methods for nRows and nColumns. */
int Grid::getRowCount()
{
	return this->nRows;
}

int Grid::getColumnCount()
{
	return this->nCols;
}

////////////////////////////////////////////////////////////////
// get adjacent nodes;

std::list<GridNode*>* Grid::getNeighbors(GridNode* n)
{
	std::list<GridNode*>* neighbors = new std::list<GridNode*>();

	neighbors->push_back(this->getNWNode(n));
	neighbors->push_back(this->getNorthNode(n));
	neighbors->push_back(this->getNENode(n));
	neighbors->push_back(this->getEastNode(n));
	neighbors->push_back(this->getSENode(n));
	neighbors->push_back(this->getSouthNode(n));
	neighbors->push_back(this->getSWNode(n));
	neighbors->push_back(this->getWestNode(n));

	return neighbors;
}

GridNode* Grid::getNorthNode(GridNode* n)
{
	if(n == NULL)
		return NULL;
	return this->getNode(n->getRow()-1, n->getColumn());
}

GridNode* Grid::getSouthNode(GridNode* n)
{
	if(n == NULL)
		return NULL;
	return this->getNode(n->getRow()+1, n->getColumn());
}

GridNode* Grid::getEastNode(GridNode* n)
{
	if(n == NULL)
		return NULL;
	return this->getNode(n->getRow(), n->getColumn()+1);
}

GridNode* Grid::getWestNode(GridNode* n)
{
	if(n == NULL)
		return NULL;
	return this->getNode(n->getRow(), n->getColumn()-1);
}

GridNode* Grid::getNENode(GridNode* n)
{
	return this->getEastNode(this->getNorthNode(n));
}

GridNode* Grid::getNWNode(GridNode* n)
{
	return this->getWestNode(this->getNorthNode(n));
}

GridNode* Grid::getSENode(GridNode* n)
{
	return this->getEastNode(this->getSouthNode(n));
}

GridNode* Grid::getSWNode(GridNode* n)
{
	return this->getWestNode(this->getSouthNode(n));
}

////////////////////////////////////////////////////////////////
// Get distance between between two nodes.
// Returns -1 if invalid node given.
int Grid::getDistance(GridNode* node1, GridNode* node2)
{
	if(node1 == NULL || node2 == NULL)
	{
		return -1;
	}

	/* Return the manhattan distance of the two nodes. */
	/*
	return 10 * (abs(node2->getRow() - node1->getRow()) +
		abs(node2->getColumn() - node1->getColumn()));
	*/

	/* Return the octile distance between two nodes. */
	int deltaX = abs(node2->getRow() - node1->getRow());
	int deltaY = abs(node2->getColumn() - node1->getColumn());
	return (int)(10 * (std::max(deltaX, deltaY) +
		0.41 * std::min(deltaX,deltaY)));
}

///////////////////////////////////////////////////////////////////////////////
// Print out the grid in ASCII
void Grid::printToFile(std::string filename)
{
	std::string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= filename; //if txt file is in the same directory as cpp file
	std::ofstream outFile;
	outFile.open(path);

	if (!outFile.is_open()) // oops. there was a problem opening the file
	{
		std::cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;
		return;
	}

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
		{
			outFile << this->getNode(i, j)->contains << " ";
		}
		outFile << std::endl;
	}
	outFile.close();
}

void Grid::resetPathChars()
{
	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
		{
			if(this->getNode(i, j)->contains != 'B')
				this->getNode(i, j)->contains = '.';
		}
	}
}

// load and place a model in a certain location.
void Grid::loadObject(std::string name, std::string filename, int row, int col,
					  Ogre::Vector3 posOffset, float orient, float scale)
{
	using namespace Ogre;

	if (row >= nRows || col >= nCols || row < 0 || col < 0)
		return;

	Entity *ent = mSceneMgr->createEntity(name, filename);
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode(name,
        Vector3::ZERO);
    node->attachObject(ent);
    node->setScale(scale, scale, scale);
	node->yaw(Ogre::Degree(orient));

	GridNode* gn = this->getNode(row, col);
	node->setPosition(this->getPosition(gn) + posOffset);
	gn->setOccupied();
	gn->entity = ent;
}

////////////////////////////////////////////////////////////////////////////
// Added this method and changed GridNode version to account for varying floor
// plane dimensions. Assumes each grid is centered at the origin.
// It returns the center of each square.
Ogre::Vector3 Grid::getPosition(GridNode* node)
{
	return this->getPosition(node->getRow(), node->getColumn());
}

Ogre::Vector3 Grid::getPosition(int r, int c)
{
	Ogre::Vector3 t;
	t.z = (r * NODESIZE) - (this->nRows * NODESIZE)/2.0 + NODESIZE/2.0;
	t.y = 0;
	t.x = (c * NODESIZE) - (this->nCols * NODESIZE)/2.0 + NODESIZE/2.0;
	return t;
}

//////////////////////////////////////////////////////////////////////////////
// A-Star Path Finding
//
std::list<GridNode*> Grid::findPath(GridNode* start, GridNode* end)
{
	if(start == NULL || end == NULL)
		return std::list<GridNode*>();

	AStarPriorityQueue open   = AStarPriorityQueue();
	AStarPriorityQueue closed = AStarPriorityQueue();

	start->setParent(NULL);
	start->setF(0, this->getDistance(start, end));

	GridNode* currentNode = start;
	while (currentNode == start || !(open.empty()))
	{
		if(!(open.empty()))
		{
			currentNode = *(open.begin()); // Get the first sorted node
			open.erase(open.begin()); // Pop the from of the queue
		}

		/* If the current node is the destination we are done. */
		if(currentNode == end)
			break;

		/* 
		 * Construct an array of the neighboring nodes. 
		 * Go around in circle clockwise.
		 */
		GridNode *neighbors[] = {
			this->getNWNode(currentNode),
			this->getNorthNode(currentNode),
			this->getNENode(currentNode),
			this->getEastNode(currentNode),
			this->getSENode(currentNode),
			this->getSouthNode(currentNode),
			this->getSWNode(currentNode),
			this->getWestNode(currentNode)
		};

		for(int i = 0; i < 8; i++)
		{
			GridNode* neighbor = neighbors[i];

			/* Check that the neighbor exists.*/
			if(neighbor == NULL)
				continue;

			/* If the neighbor is not clear. */
			if(!(neighbor->isClear()))
				continue;

			/* Do not cut corners if ortho is blocked. */
			if(i%2 == 0 && (!(neighbors[(i+7)%8]->isClear()) ||
				!(neighbors[(i+1)%8]->isClear())))
				continue;

			/* Calculate a new G value through the current node. */
			int newG = currentNode->getG() + ((i%2) ? (10):(14));

			/* If the node has already been explored. */
			auto closedNeighbor = findGridNode(closed, neighbor);
			if(closedNeighbor != closed.end())
			{
				/* Update closed with the new path. */
				if(newG < (*closedNeighbor)->getG())
				{
					closed.erase(closedNeighbor);
					neighbor->setF(newG, this->getDistance(neighbor, end));
					neighbor->setParent(currentNode);
					closed.insert(neighbor);
				}
				continue;
			}

			auto prevNeighbor = findGridNode(open, neighbor);
			// If neighbor not in open or node in open's G is greater.
			if(prevNeighbor == open.end() || newG < (*prevNeighbor)->getG())
			{
				if(prevNeighbor != open.end())
				{
					open.erase(prevNeighbor);
				}
				neighbor->setF(newG, this->getDistance(neighbor, end));
				neighbor->setParent(currentNode);
				open.insert(neighbor);
			}
		}

		closed.insert(currentNode);
	}

	if(currentNode != end)
		return std::list<GridNode*>();

	std::list<GridNode*> path;
	while(currentNode != NULL)
	{
		path.push_front(currentNode);
		currentNode = currentNode->getParent();
	}

	/* Reset all the nodes we used during the search. */
	for(auto iter = open.begin(); iter != open.end(); iter++)
	{
		(*iter)->resetAStarParams();
	}
	for(auto iter = closed.begin(); iter != closed.end(); iter++)
	{
		(*iter)->resetAStarParams();
	}

	return path;
}

AStarPriorityQueue::iterator findGridNode(AStarPriorityQueue& pq,
										  GridNode* gn)
{
    auto first = pq.begin();
    auto last = pq.end();
    while (first!=last) {
        if (*first == gn) return first;
        ++first;
    }
    return last;
}
