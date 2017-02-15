///////////////////////////////////////////////////////////////////////////////
// CS 425: Game Programming
// HW 06: Physics
// Zachary Ferguson
//
//
// Controls:
//
// Up/Down Arrow - Change the trajectory of the fish. Reflected in the 
//		orientation of the fish.
// Right/Left Arrow - Change the initial speed of the fish. Reflected in the 
//		speed slider in the top right corner.
//	
// Directions:
//
// Press the space bar to fire the fish. Attempt to hit the barrel to score 
// points. After every point the barrel moves forward or backwards a small 
// amount.
///////////////////////////////////////////////////////////////////////////////

#ifndef __GameApplication_h_
#define __GameApplication_h_

#include <string>

#include "BaseApplication.h"
#include "Agent.h"
#include "Grid.h"
#include "Projectile.h"

////////////////////////////////////////////////////////////////////////////////
// HW 03: Level Loading
// Predefined filenames for the level files.
#define DEFAULT_LEVEL LEVEL01
#define LEVEL01 "level001.txt"
//#define LEVEL02 "level002.txt"
//#define LEVEL03 "level003.txt"
//#define LEVEL04 "level004.txt"
//#define LEVEL05 "level005.txt"
//#define LEVEL06 "level006.txt"

///////////////////////////////////////////////////////////////////////////////
// HW 04: Pathfinding
//#define TEST_PATHFINDING // Comment out this line to avoid test paths.

///////////////////////////////////////////////////////////////////////////////
// HW 05: Boids
//#define TEST_BOIDS       // Comment out this line to avoid testing boids.

/* Min number of boid destinations to initially generate. */
// CHANGED FOR HW06: PHYSICS
#define INIT_BOID_DESTINATIONS 0 //12

#define PARTICLE_TIMEOUT 0.5

///////////////////////////////////////////////////////////////////////////////
// HW 06: Physics
#define PI 3.1415926

// Predefined projectile values for the game
#define MIN_ANGLE   0.0
#define MAX_ANGLE  90.0
#define INIT_ANGLE 45.0
#define MIN_SPEED   0.0
#define MAX_SPEED  50.0
#define INIT_SPEED 10.0

// Helper functions
Ogre::Real clamp(Ogre::Real val, Ogre::Real min, Ogre::Real max);
Ogre::Real degToRad(Ogre::Real angle);

///////////////////////////////////////////////////////////////////////////////

class Agent;
class Grid;
class GridNode;
class Projectile;

class GameApplication : public BaseApplication
{
private:
	
	///////////////////////////////////////////////////////////////////////////
	// HW 02: Walking

	/* A list of agents in the game world. */
	std::list<Agent*>* agentList;
	
	/* Added a new destionation to each agents walk list. */
	void moveAgents();

	///////////////////////////////////////////////////////////////////////////
	// HW 03: Level Loading

	/* Pointer to the grid. */
	Grid* grid;

	/* Load a specified level, clearing any previously loaded data. */
	void loadLevel(std::string levelFilename);

	/* Destoys the current level agents, grid, etc. */
	void resetLevel();

	///////////////////////////////////////////////////////////////////////////
	// HW 04: Pathfinding
	std::string currentLevel;
	bool testing;

	///////////////////////////////////////////////////////////////////////////
	// HW 05: Boids

	/* List of particles marking the locations. */
	std::list<Ogre::SceneNode*>* markers;
	
	/* Add a new destionation to flock of agents. */
	void moveBoids();

	///////////////////////////////////////////////////////////////////////////
	// HW 06: Physics
	
	/* Projectile Object */
	Projectile* projectile;
	/* Barrel pointer. */
	Ogre::SceneNode* target;
	/* Has the fish not been fired? */
	bool readyToFire;
	/* Angle of the initial velocity.         */
	/* Should be bounded by 0 and 90 degrees. */
	Ogre::Real trajectory;
	/* Magnitude of the initial velocity vector. */
	Ogre::Real speed;
	/* Current score for the game. */
	unsigned int score;

	Ogre::Real min_z, max_z;

	/* GUI Elements */
	OgreBites::Slider* speedSlider;
	OgreBites::ParamsPanel* scorePanel;
	OgreBites::Label* controlLabel;
	OgreBites::ParamsPanel* controlPanel;

	/* Setup the GUI. */
	void createGUI();
	void sliderMoved(OgreBites::Slider* s);

	/* Fire the single projectile if not already fired. */
	void fireProjectile();

	///////////////////////////////////////////////////////////////////////////

public:
    GameApplication(void);
    virtual ~GameApplication(void);

	/* Accessor Methods: */
	Ogre::SceneManager* getSceneManager() const;
	Grid* getGrid() const;
	std::list<Agent*>* getAgents() const;

	void loadEnv(std::string levelFilename); // Load the level
	void setupEnv();		// Set up the lights, shadows, etc
	void loadObjects();		// Load other props or objects (e.g. furniture)
	void loadCharacters();	// Load actors, agents, characters

	void addTime(Ogre::Real deltaTime);		// update the game state
	
	///////////////////////////////////////////////////////////////////////////
	// HW 05: Boids
	/* Switch the marker to the next one. */
	void nextMarker();
	
	///////////////////////////////////////////////////////////////////////////
	// HW 06: Physics
	/* Get the target game object. */
	Ogre::AxisAlignedBox getTargetAABB();
	/* Call this if the target is hit, updates score and resets stuff. */
	void scorePoint();
	/* Reset the projectile. */
	void resetProjectile();

	///////////////////////////////////////////////////////////////////////////
	// I/O Methods for OIS
	// OIS::KeyListener
    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	///////////////////////////////////////////////////////////////////////////

protected:
    virtual void createScene(void);
};

#endif // #ifndef __TutorialApplication_h_
