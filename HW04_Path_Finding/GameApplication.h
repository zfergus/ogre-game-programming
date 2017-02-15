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
	
	/* Added a new destionation to each agents walk list. */
	void moveAgents();

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
	Ogre::SceneManager* getSceneManager();
	Grid* getGrid();

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


protected:
    virtual void createScene(void);
};

#endif // #ifndef __TutorialApplication_h_
