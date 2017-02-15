////////////////////////////////////////////////////////
// Class to hold the grid layout of our environment
// Used for navigation and AI, not graphics

#ifndef GRID_H
#define GRID_H
#include <iostream>
#include <vector>
#include <assert.h>
#include "GameApplication.h"

#define NODESIZE 10.0

class GridNode {
protected:
	int nodeID;			// identify for the node
	int rCoord;			// row coordinate
	int cCoord;			// column coordinate
	bool clear;			// is the node walkable?
	
	/* A* additional data members. */
	GridNode* parent;
	int G, H, F;

public:
	Ogre::Entity *entity; // A pointer to the entity in this node
	
	/* For printing: B = blocked, S = start, G = goal, numbers = path */
	char contains;
	GridNode();	// default constructor
	GridNode(int nID, int row, int column, bool isC = true); // Create a node
	~GridNode(); // destroy a node

	void setID(int id);			 // set the node id
	int getID(){return nodeID;}; // get the node ID
	void setRow(int r);			 // set the row coordinate
	void setColumn(int c);		 // set the column coordinate
	
	/* Get the row and column coordinate of the node */
	int getRow();
	int getColumn();
	
	/* Return the position of this node given the number of rows and columns */
	Ogre::Vector3 getPosition(int rows, int cols);
	void setClear();		// set the node as walkable
	void setOccupied();		// set the node as occupied
	bool isClear();			// is the node walkable

	void resetAStarParams();

	int getG();
	int getH();
	int getF();
	void setG(int G);
	void setH(int H);
	void setF(int G, int H);

	GridNode* getParent();
	void setParent(GridNode* parent);

	bool operator() (GridNode* lhs, GridNode* rhs) const;
};

class GridRow {  // helper class
public:
	std::vector<GridNode> data;
	GridRow(int size) {data.resize(size);};
	~GridRow(){};
};

class Grid {
private:
	Ogre::SceneManager* mSceneMgr;		// pointer to scene graph
	std::vector<GridRow> data;  // actually hold the grid data
	int nRows;					// number of rows
	int nCols;					// number of columns
public:
	Grid(Ogre::SceneManager* mSceneMgr, int numRows, int numCols);	// create a grid
	~Grid();					// destroy a grid

	int getRowCount();
	int getColumnCount();

	GridNode* getNode(int r, int c);  // get the node specified 


	std::list<GridNode*>* getNeighbors(GridNode* n);
	GridNode* getNorthNode(GridNode* n);		  // get adjacent nodes;
	GridNode* getSouthNode(GridNode* n);
	GridNode* getEastNode(GridNode* n);
	GridNode* getWestNode(GridNode* n);
	GridNode* getNENode(GridNode* n);
	GridNode* getNWNode(GridNode* n);
	GridNode* getSENode(GridNode* n);
	GridNode* getSWNode(GridNode* n);

	int getDistance(GridNode* node1, GridNode* node2);  // get Manhattan distance between between two nodes
	
	void printToFile(std::string filename = "Grid.txt"); // Print a grid to a file.  Good for debugging
	
	void resetPathChars();

	/* load and place a model in a certain location. */
	void loadObject(std::string name, std::string filename, int row, int col, 
		Ogre::Vector3 posOffset, float orient, float scale = 1);
	
	/*Returns the position of the node in 3D-space.*/
	Ogre::Vector3 getPosition(GridNode* node);
	Ogre::Vector3 getPosition(int r, int c); 

	/* Returns the closest node on the grid. */
	GridNode* getNode(Ogre::Vector3 pos);

	std::list<GridNode*> findPath(GridNode* start, GridNode* end);
};

typedef std::multiset<GridNode*, GridNode> AStarPriorityQueue;
AStarPriorityQueue::iterator findGridNode(AStarPriorityQueue& pq, GridNode* gn);

#endif
