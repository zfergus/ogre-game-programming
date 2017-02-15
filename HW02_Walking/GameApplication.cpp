#include "GameApplication.h"

//-------------------------------------------------------------------------------------
GameApplication::GameApplication(void)
{
	agent = NULL; // Init member data
}
//-------------------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
	if (agent != NULL)  // clean up memory
		delete agent; 
}

//-------------------------------------------------------------------------------------
void GameApplication::createScene(void)
{
    loadEnv();
	setupEnv();
	loadObjects();
	loadCharacters();
}

// Load the buildings or ground plane, etc
void GameApplication::loadEnv()
{
	using namespace Ogre;

	//create a floor mesh resource
	MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), 100, 100, 10, 10, true, 1, 10, 10, Vector3::UNIT_Z);

	//create a floor entity, give it material, and place it at the origin
	Entity* floor = mSceneMgr->createEntity("Floor", "floor");
	//floor->setMaterialName("Examples/Rockwall");
	floor->setMaterialName("Examples/WaterStream");  // moving water
	floor->setCastShadows(false);

	//attach floor to scene
	mSceneMgr->getRootSceneNode()->attachObject(floor);
}

// Set up lights, shadows, etc
void GameApplication::setupEnv()
{
	using namespace Ogre;

	//mSceneMgr->setSkyBox(true, "Examples/StormySkyBox");

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
}

// Load other props or objects
void GameApplication::loadObjects()
{
	using namespace Ogre;

}


// Load actors, agents, characters
void GameApplication::loadCharacters()
{
	agent = new Agent(this->mSceneMgr, "Sinbad", "Sinbad.mesh");
}

void GameApplication::addTime(Ogre::Real deltaTime)
{
	if (agent != NULL)
		agent->update(deltaTime); // at ever frame update the agent
}