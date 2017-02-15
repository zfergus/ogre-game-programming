#ifndef __GameApplication_h_
#define __GameApplication_h_

#include "BaseApplication.h"
#include "Agent.h"

class GameApplication : public BaseApplication
{
private:
	Agent* agent; // store a pointer to the character

public:
    GameApplication(void);
    virtual ~GameApplication(void);

	void loadEnv();			// Load the buildings or ground plane, etc.
	void setupEnv();		// Set up the lights, shadows, etc
	void loadObjects();		// Load other props or objects (e.g. furniture)
	void loadCharacters();	// Load actors, agents, characters

	void addTime(Ogre::Real deltaTime);		// update the game state

protected:
    virtual void createScene(void);
};

#endif // #ifndef __TutorialApplication_h_
