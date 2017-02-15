#include "GameApplication.h"
#include <fstream>
#include <sstream>
#include <map> 

/* Predefined filenames for the level files. */
#define DEFAULT_LEVEL LEVEL01
#define LEVEL01 "level001.txt"
#define LEVEL02 "level002.txt"
#define LEVEL03 "level003.txt"
#define LEVEL04 "level004.txt"
#define LEVEL05 "level005.txt"
#define LEVEL06 "level006.txt"

#define TEST_PATHFINDING // Comment out this line to avoid test paths.

//-----------------------------------------------------------------------------
GameApplication::GameApplication(void)
{
	this->grid = NULL; // Init member data
	this->agentList = new std::list<Agent*>();
	this->testing = false;
}

//-----------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
	if (this->grid != NULL)  // clean up memory
	{
		delete this->grid;
	}

	if(this->agentList != NULL)
	{
		std::list<Agent*>::iterator iter;
		for (iter = this->agentList->begin(); iter != this->agentList->end(); iter++)
		{
			if (*iter != NULL)
			{
				delete (*iter);
			}
		}
		delete (this->agentList);
	}
}

/* Accessor Methods: */
Ogre::SceneManager* GameApplication::getSceneManager()
{
	return this->mSceneMgr;
}

Grid* GameApplication::getGrid()
{
	return this->grid;
}

//-----------------------------------------------------------------------------
void GameApplication::createScene(void)
{
	loadEnv(DEFAULT_LEVEL);
	setupEnv();
	loadObjects();
	loadCharacters();
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
	this->currentLevel = levelFilename;
	this->testing = true;

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

	//inputfile.open("D:/CS425-2012/Lecture 8/GameEngine-loadLevel/level001.txt"); // bad explicit path!!!
	if (!inputfile.is_open()) // oops. there was a problem opening the file
	{
		cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	// Hmm. No output?
		return;
	}

	// the file is open
	int x,z;
	inputfile >> x >> z;	// read in the dimensions of the grid
	string matName;
	inputfile >> matName;	// read in the material name

	// create floor mesh using the dimension read
	MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), x*NODESIZE, z*NODESIZE, x, z, true, 1, x, z, Vector3::UNIT_Z);
	
	//create a floor entity, give it material, and place it at the origin
	Entity* floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName(matName);
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->
		createChildSceneNode("Floor", Ogre::Vector3(0,0,0))->attachObject(floor);

	this->grid = new Grid(mSceneMgr, z, x); // Set up the grid-> z is rows, x is columns
	
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
	while (!inputfile.eof() && buf != "Characters") // read through until you find the Characters section
	{ 
		inputfile >> buf;			// read in the char
		if (buf != "Characters")
		{
			inputfile >> rent->filename >> x_offset >> y_offset >> z_offset >>
				rent->orient >> rent->scale;  // read the rest of the line
			rent->posOffset = Ogre::Vector3(x_offset, y_offset, z_offset);
			rent->agent = false;		// these are objects
			objs[buf] = rent;			// store this object in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}

	// Useless check
	while  (buf != "Characters")	// get through any junk
		inputfile >> buf;
	
	// Read in the characters
	while (!inputfile.eof() && buf != "World") // Read through until the world section
	{
		inputfile >> buf;		// read in the char
		if (buf != "World")
		{
			inputfile >> rent->filename >> x_offset >> y_offset >> z_offset >>
				rent->scale;  // read the rest of the line
			rent->posOffset = Ogre::Vector3(x_offset, y_offset, z_offset);
			rent->agent = true;			// this is an agent
			objs[buf] = rent;			// store the agent in the map
			rent = new readEntity();	// create a new instance to store the next object
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
					// Use subclasses instead!
					Agent* agent = new Agent(this, getNewName(), 
						rent->filename, rent->posOffset.y, rent->scale,
						this->grid->getNode(i, j));
					this->agentList->push_back(agent);
					agent->setPosition(this->grid->getNode(i, j), 
						rent->posOffset.x, rent->posOffset.z);
					/*
					agent->setPosition(this->grid->getPosition(i,j).x + 
						rent->posOffset.x, 0, this->grid->getPosition(i,j).z + 
						rent->posOffset.z);
					*/
					// If we were using different characters, we'd have to deal with 
					// different animation clips. 
				}
				else	// Load objects
				{
					this->grid->loadObject(getNewName(), rent->filename, i, j, 
						rent->posOffset, rent->orient, rent->scale);
				}
			else // not an object or agent
			{
				if (c == 'w') // create a wall
				{
					Entity* ent = mSceneMgr->createEntity(getNewName(), Ogre::SceneManager::PT_CUBE);
					ent->setMaterialName("Examples/RustySteel");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ent);
					mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
					this->grid->getNode(i,j)->setOccupied();  // indicate that agents can't pass through
					mNode->setPosition(this->grid->getPosition(i,j).x, 10.0f, this->grid->getPosition(i,j).z);
				}
				else if (c == 'e')
				{
					ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);  // set nonvisible timeout
					ParticleSystem* ps = mSceneMgr->createParticleSystem(getNewName(), "Examples/PurpleFountain");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ps);
					mNode->setPosition(this->grid->getPosition(i,j).x, 0.0f, this->grid->getPosition(i,j).z);
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

	// disable default camera control so the character can do its own 
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
	mCamera->setPosition(Ogre::Vector3(0,75,200));
	mCamera->lookAt(Ogre::Vector3(0,0,-200));
}

// Load other props or objects
void GameApplication::loadObjects()
{
}

// Load actors, agents, characters
void GameApplication::loadCharacters()
{
}

/*
 * Load a new level. Clears out the previous level data and entities.
 */
void GameApplication::loadLevel(std::string levelFilename)
{
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

	if(this->agentList != NULL)
	{
		std::list<Agent*>::iterator iter;
		for (iter = this->agentList->begin(); iter != this->agentList->end(); 
			iter++)
		{
			if (*iter != NULL)
			{
				delete (*iter);
			}
		}
		delete (this->agentList);
	}
	this->agentList = new std::list<Agent*>();
	
	this->mSceneMgr->clearScene();
	Ogre::MeshManager::getSingleton().remove("floor");
}

/*
 * Send all agents a new destionation node. This node has to be a clear node on 
 * the grid.
 */
void GameApplication::moveAgents()
{
	// Iterate over the list of agents
	std::list<Agent*>::iterator iter;
	int i = -2;
	for (iter = this->agentList->begin(); iter != this->agentList->end(); 
		iter++)
	{
		if (*iter != NULL)
		{
			GridNode *gn;
#ifdef TEST_PATHFINDING
			/* If Level 01 is loaded, send all agents to the center. */
			if(this->testing && this->currentLevel == LEVEL01)
			{
				gn = this->grid->getNode(7+(i++), 9);
			}
			else if(this->testing && this->currentLevel == LEVEL02)
			{
				gn = this->grid->getNode(24, 20+(i++));
			}
			/* If Level 03 is loaded, the maze level. */
			else if(this->currentLevel == LEVEL03)
			{
				/* Walk back and forth through the maze. */
				GridNode* posNode = (*iter)->getPosition();
				if(posNode->getRow() == 22 && posNode->getColumn() == 2)
				{
					gn = this->grid->getNode(0, 70);
				}
				else
				{
					gn = this->grid->getNode(22, 2);
				}
			}
			else if(this->currentLevel == LEVEL04)
			{
				gn = this->grid->getNode(9-(*iter)->getPosition()->getRow(), 
					(*iter)->getPosition()->getColumn());
			}
			/* Test the boundary conditions. */
			else if(this->currentLevel == LEVEL05)
			{
				/* Walk from corner to corner. */
				if((*iter)->getPosition()->getColumn() == 3)
				{
					gn = this->grid->getNode(0, 0);
				}
				else
				{
					gn = this->grid->getNode(3, 3);
				}
			}
			/* Different length paths. */
			else if(this->currentLevel == LEVEL06)
			{
				static int pathLength = 0;
				if(pathLength == 0)
					gn = (*iter)->getPosition();
				else if(pathLength == 1)
					gn = this->grid->getNode(0, 
						abs((*iter)->getPosition()->getColumn() - 1));
				else
					gn = this->grid->getNode(2, 2); // Inaccessable

				pathLength = (pathLength + 1) % 3;
			}
			else
			{
#endif
				/* Loop until a clear node is found. This loop is guaranteed to
				 * terminate iff an agent exists because an agent is always 
				 * spawned on a clear node. 
				 */
				do
				{
					/* Random (row, col) coordinates in grid. */
					int r = rand() % (this->grid->getRowCount());
					int c = rand() % (this->grid->getColumnCount());
					gn = this->grid->getNode(r, c);
				}while(gn == NULL || !(gn->isClear()));
#ifdef TEST_PATHFINDING
			}
#endif
			/* Add the grid node to the list of destinations. */
			//(*iter)->addDestinationLocation(gn);
			(*iter)->walkTo(gn);
		}
	}
	this->testing = false;
}

void GameApplication::addTime(Ogre::Real deltaTime)
{
	// Iterate over the list of agents
	std::list<Agent*>::iterator iter;
	for (iter = this->agentList->begin(); iter != this->agentList->end(); iter++)
		if (*iter != NULL)
			(*iter)->update(deltaTime);
}

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
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
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
	else if (arg.key == OIS::KC_SPACE)
	{
		this->moveAgents();
	}
	else if(arg.key == OIS::KC_1 || arg.key == OIS::KC_NUMPAD1)
	{
		this->loadLevel(LEVEL01);
	}
	else if(arg.key == OIS::KC_2 || arg.key == OIS::KC_NUMPAD2)
	{
		this->loadLevel(LEVEL02);
	}
	else if(arg.key == OIS::KC_3 || arg.key == OIS::KC_NUMPAD3)
	{
		this->loadLevel(LEVEL03);
	}
	else if(arg.key == OIS::KC_4 || arg.key == OIS::KC_NUMPAD4)
	{
		this->loadLevel(LEVEL04);
	}
	else if(arg.key == OIS::KC_5 || arg.key == OIS::KC_NUMPAD5)
	{
		this->loadLevel(LEVEL05);
	}
	else if(arg.key == OIS::KC_6 || arg.key == OIS::KC_NUMPAD6)
	{
		this->loadLevel(LEVEL06);
	}
   
    mCameraMan->injectKeyDown(arg);
    return true;
}

bool GameApplication::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool GameApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
    mCameraMan->injectMouseMove(arg);
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