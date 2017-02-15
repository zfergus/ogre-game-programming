///////////////////////////////////////////////////////////////////////////////
// CS 425: Game Programming
// HW 07: Game
// Zachary Ferguson
//
// See README.txt for controls and extra.
///////////////////////////////////////////////////////////////////////////////

#include "GameApplication.h"
#include <fstream>
#include <sstream>
#include <map> 

#include "Guard.h"
#include "Player.h"

/*
 * Construct a new game with default values.
 */
GameApplication::GameApplication(void)
{
	///////////////////////////////////////////////////////////////////////////
	// HW 03: Level Loading
	this->grid = NULL; // Init member data
	
	///////////////////////////////////////////////////////////////////////////
	// HW 07: Game
	this->guards = new std::list<Guard*>();

	this->player = NULL;
	this->drone = NULL;
}

/*
 * Destory the game, letting Ogre handle its own resources.
 */
GameApplication::~GameApplication(void)
{
	if (this->grid != NULL)  // clean up memory
		delete this->grid;

	if(this->guards != NULL)
	{
		std::list<Guard*>::iterator iter;
		for (iter = this->guards->begin(); iter != this->guards->end(); iter++)
		{
			if (*iter != NULL)
			{
				delete (*iter);
			}
		}
		delete (this->guards);
	}

	if(this->player)
	{
		delete this->player;	
	}

	if(this->drone)
	{
		delete this->drone;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Accessor Methods:
//
Ogre::SceneManager* GameApplication::getSceneManager() const
{
	return this->mSceneMgr;
}

Grid* GameApplication::getGrid() const
{
	return this->grid;
}

std::list<Guard*>* GameApplication::getGuards() const
{
	return this->guards;
}

Player* GameApplication::getPlayer() const
{
	return this->player;
}

Ogre::Camera* GameApplication::getCamera() const
{
	return this->mCamera;
}
///////////////////////////////////////////////////////////////////////////////

/*
 * Create the initial screen which is the main menu.
 */
void GameApplication::createScene(void)
{
	this->currentLevel = GameLevel::MAIN_MENU;
	this->loadNextLevelFlag = true;
}

/* 
 * Returns a unique name for loaded objects and agents
 */
std::string getNewName() // return a unique name 
{
	static int count = 0;	// keep counting the number of objects

	std::string s;
	std::stringstream out;	// a stream for outputing to a string
	out << count++;			// make the current count into a string
	s = out.str();

	return "object_" + s;	// append the current count onto the string
}

/*
 * Load level from file! Loads the buildings or ground plane, etc.
 */
void GameApplication::loadEnv(std::string levelFilename)
{
	using namespace Ogre;	// use both namespaces
	using namespace std;

	class readEntity // need a structure for holding entities
	{
	public:
		string filename;
		Ogre::Vector3 posOffset;
		float scale;
		float orient;
		bool agent;
	};

	ifstream inputfile;		// Holds a pointer into the file

	string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= levelFilename; //if txt file is in the same directory as cpp file
	inputfile.open(path);

	if (!inputfile.is_open()) // oops. there was a problem opening the file
	{
		cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;
		return;
	}

	// the file is open
	int x,z;
	inputfile >> x >> z;  // read in the dimensions of the grid
	string matName;
	inputfile >> matName; // read in the material name

	// create floor mesh using the dimension read
	MeshManager::getSingleton().createPlane("floor", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), x*NODESIZE, z*NODESIZE, x, z, true, 1, x, z,
		Vector3::UNIT_Z);
	
	//create a floor entity, give it material, and place it at the origin
	Entity* floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName(matName);
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode(
		"Floor", Ogre::Vector3(0,0,0))->attachObject(floor);

	// Set up the grid-> z is rows, x is columns
	this->grid = new Grid(mSceneMgr, z, x); 
	
	string buf;
	inputfile >> buf;	// Start looking for the Objects section
	while  (buf != "Objects")
		inputfile >> buf;
	if (buf != "Objects")	// Oops, the file must not be formated correctly
	{
		cout << "ERROR: Level file error" << endl;
		return;
	}

	// read in the objects
	readEntity *rent = new readEntity();	// hold info for one object
	float x_offset, y_offset, z_offset;
	std::map<string,readEntity*> objs;		// hold all object and agent types;
	// read through until you find the Characters section
	while (!inputfile.eof() && buf != "Characters") 
	{ 
		inputfile >> buf;			// read in the char
		if (buf != "Characters")
		{
			inputfile >> rent->filename >> x_offset >> y_offset >> z_offset >>
				rent->orient >> rent->scale;  // read the rest of the line
			rent->posOffset = Ogre::Vector3(x_offset, y_offset, z_offset);
			rent->agent = false; // these are objects
			objs[buf] = rent; // store this object in the map
			
			// create a new instance to store the next object
			rent = new readEntity(); 
		}
	}

	// Useless check
	while  (buf != "Characters")	// get through any junk
		inputfile >> buf;
	
	// Read in the characters, reads through until the world section.
	while (!inputfile.eof() && buf != "World")
	{
		inputfile >> buf;		// read in the char
		if (buf != "World")
		{
			inputfile >> rent->filename >> x_offset >> y_offset >> z_offset >>
				rent->scale;  // read the rest of the line
			rent->posOffset = Ogre::Vector3(x_offset, y_offset, z_offset);
			rent->agent = true;			// this is an agent
			objs[buf] = rent;			// store the agent in the map
			
			// create a new instance to store the next object
			rent = new readEntity();	
		}
	}
	delete rent; // we didn't need the last one

	// read through the placement map
	char c;
	for (int i = 0; i < z; i++)			// down (row)
		for (int j = 0; j < x; j++)		// across (column)
		{
			inputfile >> c;			// read one char at a time
			buf = c + '\0';			// convert char to string
			rent = objs[buf];		// find cooresponding object or agent
			if (rent != NULL)		// it might not be an agent or object
				if (rent->agent)	// if it is an agent...
				{
					if(c == 'p')
					{
						this->player = new Player(this, getNewName(), 
							rent->filename, rent->posOffset.y, rent->scale, 
							this->grid->getNode(i, j));
						this->player->setPosition(this->grid->getNode(i, j), 
							rent->posOffset.x, rent->posOffset.z);

						// Attach the camera to the player.
						Ogre::SceneNode* sn = this->player->getBodyNode();
						sn->setOrientation(Ogre::Quaternion(Ogre::Degree(180), 
							Ogre::Vector3::UNIT_Y));
						Ogre::SceneNode* cn = sn->createChildSceneNode();
						cn->setPosition(Ogre::Vector3::ZERO);
						cn->attachObject(this->mCamera);
						cn->setOrientation(Ogre::Quaternion(Ogre::Degree(180), 
							Ogre::Vector3::UNIT_Y));
						this->mCamera->setPosition(0, 15, 30);
						this->mCamera->lookAt(this->grid->getPosition(
							this->player->getPosition()) + 2*rent->posOffset);
						this->mCamera->setNearClipDistance(25);
					}
					else
					{
						// Use subclasses instead!
						Guard* guard = new Guard(this, getNewName(), 
							rent->filename, rent->posOffset.y, rent->scale,
							this->grid->getNode(i, j));
						this->guards->push_back(guard);
						guard->setPosition(this->grid->getNode(i, j), 
							rent->posOffset.x, rent->posOffset.z);
					}
				}
				///////////////////////////////////////////////////////////////
				// Load objects
				else if(c == DRONE_CHAR)
				{
					this->drone = new Drone(this, getNewName(), rent->filename,
						this->grid->getNode(i, j), rent->posOffset, 
						rent->orient, rent->scale);
				}
				else 
				{
					this->grid->loadObject(getNewName(), rent->filename, i, j, 
						rent->posOffset, rent->orient, rent->scale);
					if(c == EXIT_CHAR)
					{
						GridNode* gn = this->grid->getNode(i, j);
						std::list<GridNode*>* neighbors = 
							this->grid->getNeighbors(this->grid->getNode(i, j));
						for(auto iter = neighbors->begin(); 
							iter != neighbors->end(); iter++)
						{
							if(*(iter) != NULL)
							{
								(*iter)->setOccupied();
								(*iter)->contains = c;
							}
						}
						delete neighbors;
					}

				}
			else // not an object or agent
			{
				if (c == 'w') // create a wall
				{
					Entity* ent = mSceneMgr->createEntity(getNewName(), 
						Ogre::SceneManager::PT_CUBE);
					ent->setMaterialName("Examples/RustySteel");
					Ogre::SceneNode* mNode = 
						mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ent);
					mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
					// indicate that agents can't pass through
					this->grid->getNode(i,j)->setOccupied();  
					mNode->setPosition(this->grid->getPosition(i,j).x, 10.0f, 
						this->grid->getPosition(i,j).z);
				}
				else if (c == 'e')
				{
					ParticleSystem* ps = mSceneMgr->createParticleSystem(
						getNewName(), "Examples/PurpleFountain");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->
						createChildSceneNode();
					mNode->attachObject(ps);
					mNode->setPosition(this->grid->getPosition(i,j).x, 0.0f, 
						this->grid->getPosition(i,j).z);
				}
			}
		}
	
	// delete all of the readEntities in the objs map
	rent = objs["s"]; // just so we can see what is going on in memory (delete this later)
	
	std::map<string,readEntity*>::iterator it;
	for (it = objs.begin(); it != objs.end(); it++) // iterate through the map
	{
		delete (*it).second; // delete each readEntity
	}
	objs.clear(); // calls their destructors if there are any. (not good enough)
	
	inputfile.close();
	this->grid->printToFile(); // see what the initial grid looks like.
}

// Set up lights, shadows, etc
void GameApplication::setupEnv()
{
	using namespace Ogre;

	// set shadow properties
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	// disable default camera control so the character can do its own thing
	mCameraMan->setStyle(OgreBites::CS_FREELOOK); // CS_FREELOOK, CS_ORBIT, CS_MANUAL

	// use small amount of ambient lighting
	mSceneMgr->setAmbientLight(ColourValue(0.3f, 0.3f, 0.3f));

	// add a bright light above the scene
	Light* light = mSceneMgr->createLight();
	light->setType(Light::LT_POINT);
	light->setPosition(-10, 40, 20);
	light->setSpecularColour(ColourValue::White);

	/* Create a sky box */
	//mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
	
	/* Reset the cameras position and direction. */
	// TODO: Camera follow player
	//mCamera->setPosition(Ogre::Vector3(0,75,200));
	//mCamera->lookAt(Ogre::Vector3(0,0,-200));

	// set nonvisible timeout
	ParticleSystem::setDefaultNonVisibleUpdateTimeout(PARTICLE_TIMEOUT);  
}

// Load other props or objects
void GameApplication::loadObjects()
{
}

// Load actors, agents, characters
void GameApplication::loadCharacters()
{
}

///////////////////////////////////////////////////////////////////////////////
// HW 03: Level Loading

/*
 * Load a new level. Clears out the previous level data and entities.
 */
void GameApplication::loadLevel(std::string levelFilename)
{
	this->mTrayMgr->hideCursor(); // Hide the cursor in game.
	this->centerBtn->hide();
	this->centerLabel->hide();
	this->mTrayMgr->moveWidgetToTray(this->centerBtn, OgreBites::TL_NONE);
	this->mTrayMgr->moveWidgetToTray(this->centerLabel, OgreBites::TL_NONE);
	
	this->timerPanel->hide();
	this->mTrayMgr->moveWidgetToTray(this->timerPanel, OgreBites::TL_NONE);

	this->gameDescription->hide();
	this->mTrayMgr->moveWidgetToTray(this->gameDescription, OgreBites::TL_NONE);

	this->resetLevel();
	this->loadEnv(levelFilename);
	this->setupEnv();
	this->loadObjects();
	this->loadCharacters();
}

/*
 * Reset the dynamic allocated memory (grid and agents) and clear the scene.
 */
void GameApplication::resetLevel()
{
	if (this->grid != NULL)  // clean up memory
	{
		delete this->grid;
		this->grid = NULL;
	}

	if(this->guards != NULL)
	{
		for (auto iter = this->guards->begin(); iter != this->guards->end(); 
			iter++)
		{
			if (*iter != NULL)
			{
				delete (*iter);
			}
		}
		this->guards->clear();
	}
	
	if(this->player)
	{
		delete this->player;
		this->player = NULL;
	}

	if(this->drone)
	{
		delete this->drone;
		this->drone = NULL;
	}
	this->mSceneMgr->clearScene();
	Ogre::MeshManager::getSingleton().remove("floor");
}

/* Create the GUI overlay. */
void GameApplication::createGUI()
{
	if (mTrayMgr == NULL) 
	{
		return;
	}

	/* Textbox for the score. */
	Ogre::StringVector strVector;
	strVector.push_back("Remaining Battery");
	this->timerPanel = this->mTrayMgr->createParamsPanel(
		OgreBites::TL_NONE, "ScorePanel", 200, strVector);
	this->timerPanel->setParamValue(0, std::to_string((int)FLIGHT_TIME));
	this->timerPanel->hide();

	/* Show Help/Directions */
	this->controlLabel = this->mTrayMgr->createLabel(
		OgreBites::TL_BOTTOMRIGHT, "HelpLabel", "Press H for Help", 250);
	strVector.clear();
	strVector.push_back("Move Forward");
	strVector.push_back("Turn Left");
	strVector.push_back("Move Backward");
	strVector.push_back("Turn Right");
	this->controlPanel = this->mTrayMgr->createParamsPanel(
		OgreBites::TL_NONE, "ControlPanel", 250, strVector);
	strVector.clear();
	strVector.push_back("W");
	strVector.push_back("A");
	strVector.push_back("S");
	strVector.push_back("D");
	this->controlPanel->setAllParamValues(strVector);
	this->controlPanel->hide();

	this->centerLabel = this->mTrayMgr->createLabel(OgreBites::TL_CENTER,
		"CenterLabel", GAME_NAME, 200);
	this->centerBtn = this->mTrayMgr->createButton(OgreBites::TL_CENTER, 
		"CenterButton", "Play Game", 100);
	
	this->gameDescription = this->mTrayMgr->createTextBox(OgreBites::TL_BOTTOM,
		"GameDescription", "Instructions:", 300, 150);
	this->gameDescription->setText(GAME_DESCRIPTION);

	this->mTrayMgr->windowUpdate(); // Refresh the GUI
}

/* Load the main menu. */
void GameApplication::loadMainMenu()
{
	this->gameDescription->show();
	this->mTrayMgr->moveWidgetToTray(this->gameDescription, OgreBites::TL_BOTTOM);

	this->mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

	this->mCamera->setPosition(0, 0, -20);
	this->mCamera->lookAt(0, 0, 0);

	// Nothing should be loaded at this point.
	this->centerLabel->show();
	this->centerLabel->setCaption(GAME_NAME);
	this->centerBtn->show();
	this->centerBtn->setCaption("Play Game");
}

/* Show the end screen. */
void GameApplication::loadEndScreen()
{
	this->resetLevel();

	this->centerBtn->show();
	this->centerLabel->show();
	this->mTrayMgr->moveWidgetToTray(this->centerLabel, OgreBites::TL_CENTER);
	this->mTrayMgr->moveWidgetToTray(this->centerBtn, OgreBites::TL_CENTER);


	this->mTrayMgr->showCursor();
	if(this->currentLevel == GameLevel::WIN_SCREEN)
	{
		this->centerLabel->setCaption("You Win!");
	}
	else if(this->currentLevel == GameLevel::LOSE_SCREEN)
	{
		this->centerLabel->setCaption("Game Over");
	}
	this->centerBtn->setCaption("Main Menu");
}

/* Clamp the value so it is in the interval [min, max]. */
Ogre::Real clamp(Ogre::Real val, Ogre::Real min, Ogre::Real max)
{
	return std::max(min, std::min(max, val));
}

/* Convert degrees to radians. */
Ogre::Real degToRad(Ogre::Real angle)
{
	return angle/180.0 * PI;
}

Ogre::Real round(Ogre::Real number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

void GameApplication::addTime(Ogre::Real deltaTime)
{
	if(this->loadNextLevelFlag)
	{
		Guard::resetSiren();
		switch (this->currentLevel)
		{
		case GameLevel::LEVEL01:
			this->loadLevel(LEVEL01_FNAME);
			break;
		case GameLevel::LEVEL02:
			this->loadLevel(LEVEL02_FNAME);
			break;
		case GameLevel::LEVEL03:
			this->loadLevel(LEVEL03_FNAME);
			break;
		case GameLevel::WIN_SCREEN:
		case GameLevel::LOSE_SCREEN:
			this->loadEndScreen();
			break;
		case GameLevel::MAIN_MENU:
		default:
			this->loadMainMenu();
		}
		this->loadNextLevelFlag = false;
	}

	// Iterate over the list of agents
	for (auto iter = this->guards->begin(); iter != this->guards->end(); iter++)
	{
		if (*iter != NULL)
		{
			(*iter)->update(deltaTime);
		}
	}

	if(this->player)
		this->player->update(deltaTime);

	if(this->drone)
	{
		this->drone->update(deltaTime);
		this->timerPanel->setParamValue(0, 
			std::to_string((int)this->drone->getRemainingFlightTime()));
	}
}

/* Reset the game through the game over screen. */
void GameApplication::gameOver()
{
	this->currentLevel = GameLevel::LOSE_SCREEN;
	this->loadNextLevelFlag = true;
}

/* Load the next level based on the current level. */
void GameApplication::nextLevel()
{
	// Increment to the next level.
	this->currentLevel = static_cast<GameLevel>((this->currentLevel + 1) % 
		NUM_GAME_LEVELS);
	this->loadNextLevelFlag = true;
}

/*
 * Activate the drone, moving the camera to above the player and starting the 
 * timer.
 */
void GameApplication::activateDrone()
{
	if(this->drone)
	{
		this->timerPanel->show();
		this->mTrayMgr->moveWidgetToTray(this->timerPanel, OgreBites::TL_TOPLEFT);
		this->drone->activate();
	}
}

///////////////////////////////////////////////////////////////////////////////
// I/O Methods for OIS
// OIS::KeyListener
bool GameApplication::keyPressed( const OIS::KeyEvent &arg ) // Moved from BaseApplication
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_BOTTOMLEFT);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
		if(this->player)
		{
			GridNode* gn = this->player->getPosition();
			if(gn != NULL && gn->isClear())
			{
				this->player->setPosition(gn);
			}
		}
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
	else if(arg.key == OIS::KC_H)
	{
		if(this->controlPanel->getTrayLocation() == OgreBites::TL_NONE)
		{
			this->mTrayMgr->moveWidgetToTray(this->controlPanel, 
				OgreBites::TL_BOTTOMRIGHT);
			this->controlPanel->show();
			this->controlLabel->setCaption("Controls");
		}
		else
		{
			this->mTrayMgr->removeWidgetFromTray(this->controlPanel);
			this->controlPanel->hide();
			this->controlLabel->setCaption("Press H for Help");
		}
	}
	else if(this->player)
	{
		if (this->player->injectKeyDown(arg))
			return true;
	}
	//mCameraMan->injectKeyDown(arg);

    return true;
}

bool GameApplication::keyReleased( const OIS::KeyEvent &arg )
{
	if(this->player)
		this->player->injectKeyUp(arg);
	//mCameraMan->injectKeyUp(arg);
    return true;
}

bool GameApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
	if(this->currentLevel > GameLevel::MAIN_MENU)
	{
		mCameraMan->injectMouseMove(arg);
	}
    return true;
}

bool GameApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool GameApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

void GameApplication::buttonHit(OgreBites::Button* b)
{
	if (b->getName() == "CenterButton")
	{
		this->currentLevel = static_cast<GameLevel>((this->currentLevel + 1) % 
			NUM_GAME_LEVELS);
		if(this->currentLevel == GameLevel::LOSE_SCREEN)
		{
			this->currentLevel = static_cast<GameLevel>(
				(this->currentLevel + 1) % NUM_GAME_LEVELS);
		}
		this->loadNextLevelFlag = true;

		return;
	}
}
///////////////////////////////////////////////////////////////////////////////