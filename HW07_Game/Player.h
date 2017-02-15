/*
 * Class for the player character. Includes movement and exit collision 
 * detection.
 * Author: Zachary Ferguson
 */

#ifndef PLAYER_H
#define PLAYER_H

// Player character movement keybindings.
#define FORWARD_KEY   OIS::KC_W
#define BACKWARD_KEY  OIS::KC_S
#define LEFTWARD_KEY  OIS::KC_A
#define RIGHTWARD_KEY OIS::KC_D
#define RUN_KEY       OIS::KC_LSHIFT

#define PLAYER_RUN_SPEED  45
#define PLAYER_WALK_SPEED 20
#define PLAYER_ROT_SPEED  90

#include "Agent.h"

class GameApplication;
class GridNode;

class Player : public Agent
{
protected:

	bool goingForward, goingBack, turningLeft, turningRight;

	Ogre::Real mRotSpeed;

	void updateLocomote(Ogre::Real deltaTime);

public:

	/* Create a player character. */
	Player(GameApplication* game, std::string name, std::string filename, 
		float height, float scale, GridNode* posNode);

	~Player();

	/* Get the axis aligned bounding box around the player. */
	Ogre::AxisAlignedBox getAABB() const;

	/* Get the body node of this player. */
	Ogre::SceneNode* getBodyNode() const;

	/* Returns the grid node this agent is in. */
	GridNode* getPosition() const;
	/* Returns the absolute position of the player. */
	Ogre::Vector3 getAbsolutePosition() const;

	/* Process key presses. */
	bool injectKeyDown( const OIS::KeyEvent &arg );
	bool injectKeyUp( const OIS::KeyEvent &arg );
};

#endif