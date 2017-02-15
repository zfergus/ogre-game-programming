///////////////////////////////////////////////////////////////////////////////
// CS 425: Game Programming
// HW 07: Game
// Zachary Ferguson
//
// See README.txt for controls and extra.
///////////////////////////////////////////////////////////////////////////////

#ifndef __GameApplication_h_
#define __GameApplication_h_

#include <string>

#include "BaseApplication.h"
#include "Grid.h"
#include "Drone.h"

// Predefined filenames for the level files.
#define LEVEL01_FNAME "level001.txt"
#define LEVEL02_FNAME "level002.txt"
#define LEVEL03_FNAME "level003.txt"

// Constants for the game's name and a description of the game.
#define GAME_NAME "Stealth Ogre"
#define GAME_DESCRIPTION \
"Sneak to the house without being detected by the robot guards. Find the \
drone hidden in the world to get a bird's eye view."

// Number of levels including menu/win/lose screen
#define NUM_GAME_LEVELS 6

// Character for the exit marker in the level file
#define EXIT_CHAR 't'

// Time before particle timeout
#define PARTICLE_TIMEOUT 0.5

// Constants for Degree to Radian
#define PI 3.1415926
#define PI_OVER_2 1.5708

// Helper functions
/* Clamp a given value to the range [min, max]. */
Ogre::Real clamp(Ogre::Real val, Ogre::Real min, Ogre::Real max);
/* Convert from degrees to radians. */
Ogre::Real degToRad(Ogre::Real angle);
/* Round the given number to the closest integer. */
Ogre::Real round(Ogre::Real number);

class Guard;
class Player;
class Grid;
class GridNode;

class GameApplication : public BaseApplication
{
protected:
	
	/* An enumeration of all of the levels in the game. */
	enum GameLevel
	{
		WIN_SCREEN  = 0, // Main Menu - 2
		LOSE_SCREEN = 1, // Main Menu - 1
		MAIN_MENU   = 2,
		LEVEL01     = 3,
		LEVEL02     = 4,
		LEVEL03     = 5
		// Could add more levels here
	};

	///////////////////////////////////////////////////////////////////////////
	// HW 03: Level Loading

	/* Pointer to the grid. */
	Grid* grid;

	/* Load a specified level, clearing any previously loaded data. */
	void loadLevel(std::string levelFilename);

	/* Destoys the current level agents, grid, etc. */
	void resetLevel();

	/* Create the initial screen. */
	virtual void createScene(void);

	///////////////////////////////////////////////////////////////////////////
	// HW 07: Game

	/* A list of Guards in the game world. */
	std::list<Guard*>* guards;
	/* The player character. */
	Player* player;
	/* Drone to fly up and give top down view. */
	Drone* drone;

	/* Current Level Number */
	GameLevel currentLevel;
	bool loadNextLevelFlag;

	/* Load the main menu. */
	void loadMainMenu();
	/* Show the end screen. */
	void loadEndScreen();

	///////////////////////////////////////////////////////////////////////////
	// GUI Elements
	
	/* Panel for showing drone timer. */
	OgreBites::ParamsPanel* timerPanel;
	/* Help label for the controls. */
	OgreBites::Label* controlLabel;
	/* Help panel for the controls. */
	OgreBites::ParamsPanel* controlPanel;
	/* Title and messages. */
	OgreBites::Label* centerLabel;
	/* Next level button. */
	OgreBites::Button* centerBtn;
	/* Textbox for the game description. */
	OgreBites::TextBox* gameDescription;


	/* Setup the GUI, including the main menu and all the win/lose buttons. */
	void createGUI();
	/* Call back function for when a button is pressed. */
	virtual void buttonHit(OgreBites::Button* b);

public:
    
	/* Default constructor. */
	GameApplication(void);
	/* Default destructor. */
    virtual ~GameApplication(void);

	/* Accessor Methods: */
	Ogre::SceneManager* getSceneManager() const;
	Grid* getGrid() const;
	std::list<Guard*>* getGuards() const;
	Player* getPlayer() const;
	Ogre::Camera* getCamera() const;

	/* Load the level file. */
	void loadEnv(std::string levelFilename);
	/* Set up the lights, shadows, etc. */
	void setupEnv();
	/* Load other props or objects (e.g. furniture). */
	void loadObjects();
	/* Load actors, agents, characters. */
	void loadCharacters();

	/* Calls the appropriate update functions. */
	void addTime(Ogre::Real deltaTime);		// update the game state

	/* Load the next level of the game. */
	void nextLevel();
	/* Reset the game through the game over screen. */
	void gameOver();

	/*
	 * Activate the drone, moving the camera to above the player and starting 
	 * the timer.
	 */
	void activateDrone();
	
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
};

#endif // #ifndef __TutorialApplication_h_
