/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
Tutorial Framework (for Ogre 1.9)
http://www.ogre3d.org/wiki/
-----------------------------------------------------------------------------
*/

#include "TutorialApplication.h"

//---------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//---------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
}

//---------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
	
	Ogre::Entity* ogreEntity = mSceneMgr->createEntity("ogrehead.mesh");
	Ogre::SceneNode* ogreNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ogreNode->attachObject(ogreEntity);
	
	Ogre::Light* light = mSceneMgr->createLight("Mainlight");
	light->setPosition(20, 80, 50);

	mCamera->setPosition(0, 47, 222);

	Ogre::Entity* ogreEntity2 = mSceneMgr->createEntity("ogrehead.mesh");
	Ogre::SceneNode* ogreNode2 = ogreNode->createChildSceneNode(
		Ogre::Vector3(84, 48, 0));
	ogreNode2->attachObject(ogreEntity2);

	Ogre::Entity* ogreEntity3 = mSceneMgr->createEntity("ogrehead.mesh");
	Ogre::SceneNode* ogreNode3 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ogreNode3->setPosition(0, 104, 0);
	ogreNode3->setScale(2, 1.2, 1);
	ogreNode3->attachObject(ogreEntity3);

	Ogre::Entity* ogreEntity4 = mSceneMgr->createEntity("ogrehead.mesh");
	Ogre::SceneNode* ogreNode4 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ogreNode4->setPosition(-84, 48, 0);
	ogreNode4->roll(Ogre::Degree(-90));
	ogreNode4->attachObject(ogreEntity4);
}
//---------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    // Create application object
    TutorialApplication app;

    try {
        app.go();
    } catch(Ogre::Exception& e)  {
        MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
        std::cerr << "An exception has occurred: " <<
            e.getFullDescription().c_str() << std::endl;
    }

    return 0;
}

//---------------------------------------------------------------------------
