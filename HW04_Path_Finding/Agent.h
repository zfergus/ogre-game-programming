#ifndef AGENT_H
#define AGENT_H

#include <deque>
#include <queue>

#include "GameApplication.h"

class Grid;
class GridNode;
class GameApplication;

class Agent
{
private:
	GameApplication* game; // Pointer to the game.

	Ogre::SceneNode* mBodyNode;			
	Ogre::Entity* mBodyEntity;
	float height; // height the character should be moved up
	float scale;  // scale of character from original model


	// all of the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together
	enum AnimID
	{
		ANIM_IDLE_BASE,
		ANIM_IDLE_TOP,
		ANIM_RUN_BASE,
		ANIM_RUN_TOP,
		ANIM_HANDS_CLOSED,
		ANIM_HANDS_RELAXED,
		ANIM_DRAW_SWORDS,
		ANIM_SLICE_VERTICAL,
		ANIM_SLICE_HORIZONTAL,
		ANIM_DANCE,
		ANIM_JUMP_START,
		ANIM_JUMP_LOOP,
		ANIM_JUMP_END,
		ANIM_NONE
	};

	Ogre::AnimationState* mAnims[13]; // master animation list
	AnimID mBaseAnimID;	// current base (full- or lower-body) animation
	AnimID mTopAnimID; // current top (upper-body) animation
	bool mFadingIn[13]; // which animations are fading in
	bool mFadingOut[13];// which animations are fading out
	Ogre::Real mTimer;// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity; // for jumping

	void setupAnimations(); // load this character's animations
	void fadeAnimations(Ogre::Real deltaTime); // blend from one animation to another
	void updateAnimations(Ogre::Real deltaTime); // update the animation frame

	// for locomotion
	Ogre::Real mDistance; // The distance the agent has left to travel
	Ogre::Vector3 mDirection; // The direction the object is moving
	Ogre::Vector3 mDestination; // The destination the object is moving towards
	//std::deque<Ogre::Vector3> mWalkList; // The list of points we are walking to
	Ogre::Real mWalkSpeed; // The speed at which the object is moving
	bool nextLocation(); // Is there another destination?
	void updateLocomote(Ogre::Real deltaTime); // update the character's walking

	/* Current position of this agent on the grid. */
	GridNode* positionNode;
	/* Path to follow in updateLocomote()/nextLocation(). */
	std::list<GridNode*>* path;

	/* Returns a unique name for loaded objects and agents */
	void printPath(std::list<GridNode*>& pathToPrint);

	/* Moves the agent to <x, y+height, z>. */
	void setPosition(float x, float y, float z);

public:
	Agent(GameApplication* game, std::string name, 
		std::string filename, float height, float scale, GridNode* posNode);
	~Agent();
	
	void setPosition(GridNode* gn, float xOffset = 0, float zOffset = 0);

	/* Returns the grid node this agent is in. */
	GridNode* Agent::getPosition();

	/* Update the agent's animation and locomotion. */
	void update(Ogre::Real deltaTime);
	
	/* Set the animation to display. */
	void setBaseAnimation(AnimID id, bool reset = false);
	void setTopAnimation(AnimID id, bool reset = false);

	/* Add a destionation to the WalkList. */
	// void addDestinationLocation(GridNode* node);

	/* A* Path Finding from the current node of the agent to the given */
	/* destination.                                                    */
	void walkTo(GridNode* node);
};

#endif