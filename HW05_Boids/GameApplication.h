#ifndef __GameApplication_h_
#define __GameApplication_h_

#include <string>

#include "BaseApplication.h"
#include "Agent.h"
#include "Grid.h"

class Agent;
class Grid;
class GridNode;

class GameApplication : public BaseApplication
{
private:
	/* Pointer to the grid. */
	Grid* grid;

	/* A list of agents in the game world. */
	std::list<Agent*>* agentList;
	/* List of particles marking the locations. */
	std::list<Ogre::SceneNode*>* markers;
	
	/* Added a new destionation to each agents walk list. */
	void moveAgents();

	/* Add a new destionation to flock of agents. */
	void moveBoids();

	/* Load a specified level, clearing any previously loaded data. */
	void loadLevel(std::string levelFilename);

	/* Destoys the current level agents, grid, etc. */
	void resetLevel();

	std::string currentLevel;
	bool testing;

public:
    GameApplication(void);
    virtual ~GameApplication(void);

	/* Accessor Methods: */
	Ogre::SceneManager* getSceneManager() const;
	Grid* getGrid() const;
	std::list<Agent*>* getAgents() const;

	void loadEnv(std::string levelFilename); // Load the buildings or ground plane, etc.
	void setupEnv();		// Set up the lights, shadows, etc
	void loadObjects();		// Load other props or objects (e.g. furniture)
	void loadCharacters();	// Load actors, agents, characters

	void addTime(Ogre::Real deltaTime);		// update the game state

	//////////////////////////////////////////////////////////////////////////
	// Lecture 4: keyboard interaction
	// moved from base application
	// OIS::KeyListener
    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	////////////////////////////////////////////////////////////////////////////
	
	/* Switch the marker to the next one. */
	void nextMarker();

protected:
    virtual void createScene(void);
};

#endif // #ifndef __TutorialApplication_h_
