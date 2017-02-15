#ifndef __GameApplication_h_
#define __GameApplication_h_

#include "BaseApplication.h"
#include "Agent.h"

class GameApplication : public BaseApplication
{
private:
	Agent* agent; // store a pointer to the character
	std::list<Agent*> agentList; // Lecture 5: now a list of agents
public:
    GameApplication(void);
    virtual ~GameApplication(void);

	void loadEnv();			// Load the buildings or ground plane, etc.
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

	//////////////////////////////////////////////////////////////////////////////////
	// Lecture 12
	bool bLMouseDown, bRMouseDown;		//true if mouse buttons are held down
	Ogre::SceneNode *mCurrentObject;	//pointer to our currently selected object
	/////////////////////////////////////////////////////////////////////////////////


protected:
    virtual void createScene(void);

	virtual void createGUI(void); // Lecture 12GUI
    virtual void buttonHit(OgreBites::Button* b); // Lecture 12GUI
	virtual void sliderMoved(OgreBites::Slider* s); // Lecture 12GUI
	OgreBites::ParamsPanel* mParamsPanel; // Lecture 12GUI
};

#endif // #ifndef __TutorialApplication_h_
