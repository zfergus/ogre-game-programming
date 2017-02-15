/*
 * Child class of the Agent class. Class for a guard that roams and searches 
 * for the player.
 * Author: Zachary Ferguson
 */

#ifndef GUARD_H
#define GUARD_H

#include "Agent.h"

#define GUARD_RUN_SPEED  (PLAYER_RUN_SPEED + 10)
#define GUARD_WALK_SPEED PLAYER_WALK_SPEED

class Agent;
class GameApplication;
class GridNode;

class Guard : public Agent
{
protected:

	/* Enumeration of the guards possible states. */
	enum GuardState { ROAMING, SEARCHING };
	
	/* This guard's current state. */
	enum GuardState state;

	/* Siren sound variables. */
	/* Filename of the siren wav file. */
	static std::string sirenFName;
	/* How many guards are chasing the player. */
	static int chasingPlayer;

	/* Load this character's animations */
	virtual void setupAnimations();
	
	/* Get the next location to go to. */
	virtual bool nextLocation();

	/* Check if the player is in the linesight of the guard. */
	void checkForPlayer();

	/* Check for a collision with the Player. */
	void collisionDetection();

public:

	/* Static function for reseting static variables. */
	static void resetSiren()
	{
		PlaySound(NULL, NULL, 0);
		Guard::chasingPlayer = 0;
	};

	/* Constructor for a new guard. */
	Guard(GameApplication* game, std::string name, std::string filename, 
		float height, float scale, GridNode* posNode);

	/* Destructor. */
	virtual ~Guard();

	/* Update the agent's animation and locomotion. */
	virtual void update(Ogre::Real deltaTime);
};

#endif