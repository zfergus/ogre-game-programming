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

#include "GameApplication.h"
#include <fstream>
#include <sstream>
#include <map> 

/*
 * Construct a new game with default values.
 */
GameApplication::GameApplication(void)
{
	///////////////////////////////////////////////////////////////////////////
	// HW 02: Walking
	this->agentList = new std::list<Agent*>();

	///////////////////////////////////////////////////////////////////////////
	// HW 03: Level Loading
	this->grid = NULL; // Init member data
	
	///////////////////////////////////////////////////////////////////////////
	// HW 05: Boids
	this->markers = new std::list<Ogre::SceneNode*>();
	this->testing = false;
	
	///////////////////////////////////////////////////////////////////////////
	// HW 06: Physics
	this->projectile  = NULL;
	this->target = NULL;
	this->readyToFire = true;
	this->trajectory  = INIT_ANGLE; // degrees
	this->speed = INIT_SPEED; // m/s
	this->score = 0;
}

/*
 * Destory the game, letting Ogre handle its own resources.
 */
GameApplication::~GameApplication(void)
{
	if (this->grid != NULL)  // clean up memory
		delete this->grid;

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

	if(this->markers != NULL)
		delete this->markers;
	
	///////////////////////////////////////////////////////////////////////////
	// HW 06: Physics
	// Delete the projectile
	if(this->projectile)
		delete this->projectile;
	// Don't need to delete target becuause OGRE handles it.
	// Don't need to delete GUI becuase OGRE handles it.
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

std::list<Agent*>* GameApplication::getAgents() const
{
	return this->agentList;
}
///////////////////////////////////////////////////////////////////////////////

/*
 * Create the scene with the default level file.
 */
void GameApplication::createScene(void)
{
	this->loadEnv(DEFAULT_LEVEL);
	this->setupEnv();
	this->loadObjects();
	this->loadCharacters();
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
	///////////////////////////////////////////////////////////////////////////
	// HW 04: Pathfinding
	this->currentLevel = levelFilename;
	this->testing = true;
	///////////////////////////////////////////////////////////////////////////

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
					// Use subclasses instead!
					Agent* agent = new Agent(this, getNewName(), 
						rent->filename, rent->posOffset.y, rent->scale,
						this->grid->getNode(i, j));
					this->agentList->push_back(agent);
					agent->setPosition(this->grid->getNode(i, j), 
						rent->posOffset.x, rent->posOffset.z);
				}
				///////////////////////////////////////////////////////////////
				// HW 06: Physics
				// Load the fish as a projectile
				else if(c == 'f')
				{
					if(this->projectile == NULL) // Can only have one projectile
					{
						Ogre::Vector3 pos = this->grid->getPosition(i, j) + 
							rent->posOffset;
						this->projectile = new Projectile(this, getNewName(), 
							rent->filename, pos, rent->scale);
						// Turn the fish around
						this->projectile->
							setOrientation(0, -90, -this->trajectory);

						// Attach the camera to the projectile.				
						Ogre::SceneNode* cn = 
							this->projectile->createChildSceneNode();
						cn->setPosition(0, 0, -20);
						cn->attachObject(this->mCamera);

						// Values used for moving the target later.
						this->max_z = this->projectile->getPosition().z - 
							NODESIZE;
						this->min_z = this->grid->getPosition(1, j).z;
					}
				}
				// Load the drum as the target
				else if(c == 'd')
				{
					if(this->target == NULL) // Only one target
					{
						this->target = this->getSceneManager()->
							getRootSceneNode()->createChildSceneNode();
						Ogre::Entity* targetEntity = this->getSceneManager()->
							createEntity(getNewName(), rent->filename);
						this->target->attachObject(targetEntity);

						Ogre::Vector3 pos = this->grid->getPosition(i, j) + 
							rent->posOffset;
						this->target->setPosition(pos);
						Ogre::Vector3 scale = rent->scale * 
							Ogre::Vector3::UNIT_SCALE;
						this->target->scale(scale);
						this->target->yaw(Ogre::Degree(rent->orient));
						this->target->pitch(Ogre::Degree(90));
						//this->target->showBoundingBox(true);
					}
				}
				///////////////////////////////////////////////////////////////
				// Load objects
				else 
				{
					this->grid->loadObject(getNewName(), rent->filename, i, j, 
						rent->posOffset, rent->orient, rent->scale);
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

					///////////////////////////////////////////////////////////
					// HW 05: Boids
					/* Hide the particles. */
					mNode->setVisible(false);
					this->markers->push_back(mNode);
					///////////////////////////////////////////////////////////
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
	//mCamera->setPosition(Ogre::Vector3(0,75,200));
	//mCamera->lookAt(Ogre::Vector3(0,0,-200));
	///////////////////////////////////////////////////////////////////////////
	// HW06: Physics
	mCamera->setPosition(Ogre::Vector3(-25, 5, 50));
	mCamera->setOrientation(Ogre::Quaternion(0.95, -0.02, -0.3, 0.0));
	///////////////////////////////////////////////////////////////////////////

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
	///////////////////////////////////////////////////////////////////////////
	// HW 05: BOIDS
#ifdef TEST_BOIDS
	int numRows = (int)(sqrt(INIT_BOID_DESTINATIONS));
	int numCols = (int)(INIT_BOID_DESTINATIONS/numRows);
	int deltaRow = (int)((this->grid->getRowCount()-2)/numRows);
	int deltaCol = (int)((this->grid->getColumnCount()-2)/numCols);
	for(int r = 1; r < this->grid->getRowCount()-1; r += deltaRow)
	{
		for(int c = 1; c < this->grid->getColumnCount()-1; c += deltaCol)
		{
#else
	/* Initialize the flock of agents. */
	for(int i = 0; i < INIT_BOID_DESTINATIONS; i++)
	{
		/* Random (row, col) coordinates in grid. */
		int r = rand() % (this->grid->getRowCount()-2) + 1;
		int c = rand() % (this->grid->getColumnCount()-2) + 1;
#endif
		GridNode* gn = this->grid->getNode(r, c);

		for(auto iter = this->agentList->begin(); 
			iter != this->agentList->end(); iter++)
		{
			(*iter)->addDestinationLocation(gn);
		}

		// Create a marker for the destionation

		// COMMENTED OUT FOR HW 06: PHYSICS
		//Ogre::ParticleSystem* ps = mSceneMgr->createParticleSystem(getNewName(),
		//	"Examples/PurpleFountain");
		//Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->
		//	createChildSceneNode();
		//mNode->attachObject(ps);
		//mNode->setPosition(this->grid->getPosition(gn).x, 0.0f, 
		//	this->grid->getPosition(gn).z);
		//mNode->setVisible(false);
		//this->markers->push_back(mNode);
	}
#ifdef TEST_BOIDS
	}
#endif
	// Make the first marker visible

	// COMMENTED OUT FOR HW 06: PHYSICS
	//Ogre::SceneNode* node = *(this->markers->begin());
	//node->setVisible(true);
	//////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
// HW 03: Level Loading

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
	
	this->markers->clear();

	this->mSceneMgr->clearScene();
	Ogre::MeshManager::getSingleton().remove("floor");

	///////////////////////////////////////////////////////////////////////////
	// HW 06: Physics
	delete this->projectile;
	this->projectile = NULL;
	this->target = NULL; // Deleted by resource manager
	this->readyToFire = true;
	this->trajectory = INIT_ANGLE;
	this->speedSlider->setValue(INIT_SPEED); // Set the speed
	// Reset score
	this->score = 0;
	this->scorePanel->setParamValue(0, std::to_string(this->score));
	// Reset target possible z's
	this->min_z = this->max_z = 0;
	///////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
// HW 04: Pathfinding

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
			(*iter)->addDestinationLocation(gn);
			//(*iter)->walkTo(gn);
		}
	}
	this->testing = false;
}

///////////////////////////////////////////////////////////////////////////////
// HW 05: Boids

/* Add a new location for the flock to head towards. */
void GameApplication::moveBoids()
{
	GridNode* gn;
	do
	{
		/* Random (row, col) coordinates in grid. */
		int r = rand() % (this->grid->getRowCount()    - 2) + 1;
		int c = rand() % (this->grid->getColumnCount() - 2) + 1;
		gn = this->grid->getNode(r, c);
	}while(gn == NULL || !(gn->isClear()));

	for(auto iter = this->agentList->begin(); iter != this->agentList->end(); 
		iter++)
	{
		if(*iter != NULL)
		{
			(*iter)->addDestinationLocation(gn);
		}
	}

	Ogre::ParticleSystem* ps = mSceneMgr->createParticleSystem(getNewName(), 
		"Examples/PurpleFountain");
	Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->
		createChildSceneNode();
	mNode->attachObject(ps);
	mNode->setPosition(this->grid->getPosition(gn).x, 0.0f, 
		this->grid->getPosition(gn).z);
	if(!this->markers->empty())
	{
		mNode->setVisible(false);
	}
	this->markers->push_back(mNode);

}

/* Switch the marker to the next one. */
void GameApplication::nextMarker()
{
	if(this->markers && !(this->markers->empty()))
	{
		this->markers->front()->setVisible(false);
		this->markers->pop_front();
		if(!(this->markers->empty()))
		{
			this->markers->front()->setVisible(true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// HW 06: Physics

/* Create the GUI overlay. */
void GameApplication::createGUI()
{
	if (mTrayMgr == NULL) 
	{
		return;
	}

	/* Slider to display the speed value. */
	this->speedSlider = mTrayMgr->createThickSlider(OgreBites::TL_TOPRIGHT, 
		"SpeedSplider", "Speed", 250, 80, MIN_SPEED, MAX_SPEED, 
		MAX_SPEED - MIN_SPEED);
	mTrayMgr->sliderMoved(this->speedSlider);
	this->speedSlider->setValue(INIT_SPEED);

	/* Textbox for the score. */
	Ogre::StringVector strVector;
	strVector.push_back("Score");
	this->scorePanel = this->mTrayMgr->createParamsPanel(
		OgreBites::TL_TOPLEFT, "ScorePanel", 100, strVector);
	this->scorePanel->setParamValue(0, std::to_string(this->score));

	/* Show Help/Directions */
	this->controlLabel = this->mTrayMgr->createLabel(
		OgreBites::TL_BOTTOMRIGHT, "HelpLabel", "Press H for Help", 250);
	strVector.clear();
	strVector.push_back("Increase Angle");
	strVector.push_back("Decrease Angle");
	strVector.push_back("Increase Speed");
	strVector.push_back("Decrease Speed");
	this->controlPanel = this->mTrayMgr->createParamsPanel(
		OgreBites::TL_NONE, "ControlPanel", 250, strVector);
	strVector.clear();
	strVector.push_back("Up");
	strVector.push_back("Down");
	strVector.push_back("Left");
	strVector.push_back("Right");
	this->controlPanel->setAllParamValues(strVector);
	this->controlPanel->hide();

	this->mTrayMgr->windowUpdate(); // Refresh the GUI
}

/* Callback for sliders. Updates this->speed. */
void GameApplication::sliderMoved(OgreBites::Slider* s)
{
	if (s == this->speedSlider)
	{
		this->speed = s->getValue();
		std::cout << "Speed: " << this->speed << std::endl;
		return;
	}
}

/* Get the target game object. */
Ogre::AxisAlignedBox GameApplication::getTargetAABB()
{
	if(this->target == NULL)
		return Ogre::AxisAlignedBox();

	this->target->_update(true, true);
	return this->target->_getWorldAABB();
}

/* Fire the projectile with the parameters set by the UI. */
void GameApplication::fireProjectile()
{
	if(this->projectile == NULL || !(this->readyToFire))
	{
		return;
	}

	/* Turn off particles to mark scoring point. */
	if(this->markers)
	{
		for(auto iter = this->markers->begin(); iter != this->markers->end(); 
			iter++)
		{
			(*iter)->setVisible(false);
		}
	}

	// Calculate the velocity from trajectory and speed.
	Ogre::Vector3 velocity = speed * Ogre::Vector3(0, 
		std::sin(degToRad(trajectory)), -std::cos(degToRad(trajectory)));
	this->projectile->fire(velocity);
	this->readyToFire = false; // No repeated firing
}

/* Call this if the target is hit, updates score and resets stuff. */
void GameApplication::scorePoint()
{
	// Play score sound
	std::string path = __FILE__; //gets the current cpp file's path
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename
	path+= "HitSound.wav";
	PlaySound(TEXT(path.c_str()), NULL, SND_FILENAME | SND_ASYNC);
	
	this->score++;
	std::cout << "Current Score: " << this->score << std::endl;
	this->scorePanel->setParamValue(0, std::to_string(this->score));

	/* Turn on particles to mark scoring point. */
	if(this->markers)
	{
		for(auto iter = this->markers->begin(); iter != this->markers->end(); 
			iter++)
		{
			(*iter)->setVisible(true);
		}
	}

	/* Reset the projectile to its initial state. */
	this->resetProjectile();

	/* Change the barrels location to up the challenge. */
	Ogre::Vector3 targetPos = this->target->getPosition();
	targetPos.z = (rand() % (int)(this->max_z - this->min_z)) + this->min_z;
	this->target->setPosition(targetPos);
}

/* Reset the projectile. */
void GameApplication::resetProjectile()
{
	if(this->projectile)
	{
		this->projectile->reset();
		this->readyToFire = true;
		this->projectile->setOrientation(0, -90, -this->trajectory);
	}
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

///////////////////////////////////////////////////////////////////////////////

void GameApplication::addTime(Ogre::Real deltaTime)
{
	// Iterate over the list of agents
	std::list<Agent*>::iterator iter;
	for (iter = this->agentList->begin(); iter != this->agentList->end(); 
		iter++)
	{
		if (*iter != NULL)
		{
			(*iter)->update(deltaTime);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// HW 06: Physics
	// Update the projectile
	if(this->projectile != NULL)
	{
		this->projectile->update(deltaTime);
	}
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
		/* HW 03/04 - Move the agents to new locations. */
		//this->moveAgents();
		/* HW 05 - Move the flock to a new target location. */
		//this->moveBoids();
		/* HW 06 - Fire the fish. */
		this->fireProjectile();
	}
	///////////////////////////////////////////////////////////////////////////
	// HW 03: Level Loading
#ifdef LEVEL01
	else if(arg.key == OIS::KC_1 || arg.key == OIS::KC_NUMPAD1)
	{
		/* Reload Level 01 if Level 01 already loaded. */
		this->loadLevel(LEVEL01);
	}
#endif
#ifdef LEVEL02
	else if(arg.key == OIS::KC_2 || arg.key == OIS::KC_NUMPAD2)
	{
		this->loadLevel(LEVEL02);
	}
#endif
#ifdef LEVEL03
	else if(arg.key == OIS::KC_3 || arg.key == OIS::KC_NUMPAD3)
	{
		this->loadLevel(LEVEL03);
	}
#endif
#ifdef LEVEL04
	else if(arg.key == OIS::KC_4 || arg.key == OIS::KC_NUMPAD4)
	{
		this->loadLevel(LEVEL04);
	}
#endif
#ifdef LEVEL05
	else if(arg.key == OIS::KC_5 || arg.key == OIS::KC_NUMPAD5)
	{
		this->loadLevel(LEVEL05);
	}
#endif
#ifdef LEVEL06
	else if(arg.key == OIS::KC_6 || arg.key == OIS::KC_NUMPAD6)
	{
		this->loadLevel(LEVEL06);
	}
#endif
	///////////////////////////////////////////////////////////////////////////
	// HW 06: Physics
	else if(arg.key == OIS::KC_UP || arg.key == OIS::KC_DOWN)
	{
		Ogre::Real delta = arg.key == OIS::KC_UP ? (1):(-1);
		this->trajectory = 
			clamp(this->trajectory + delta, MIN_ANGLE, MAX_ANGLE);
		this->projectile->setOrientation(0, -90, -this->trajectory);
		std::cout << "Trajectory: " << this->trajectory << std::endl;
	}
	else if(arg.key == OIS::KC_LEFT || arg.key == OIS::KC_RIGHT)
	{
		Ogre::Real delta = arg.key == OIS::KC_LEFT ? (-1):(1);
		this->speedSlider->setValue(this->speed + delta);
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
	///////////////////////////////////////////////////////////////////////////
	// COMMENTED OUT FOR HW 06: PHYSICS
    //mCameraMan->injectKeyDown(arg);

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