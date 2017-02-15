/*
 * Class for a drone that controls the camera and moves it up.
 * Author: Zachary Ferguson
 */

#ifndef DRONE_H
#define DRONE_H

/* Drone character in the level file. */
#define DRONE_CHAR 'd'
/* Timer values */
#define TAKE_OFF_TIME 3.0
#define FLIGHT_TIME 25.0 // seconds
/* Max height for take off. */
#define FLYING_HEIGHT 70.0

class GameApplication;
class GridNode;

class Drone
{
protected:

	/* States the drone can be in. */
	enum DroneState
	{
		ON_GROUND,
		TAKING_OFF,
		FLYING
	};

	GameApplication* game;

	Ogre::SceneNode* bodyNode;
	Ogre::Entity*    bodyEntity;

	/* Timer for take off and flight. */
	float timer;
	/* The drone's current state. */
	DroneState state;

	/* The inital position of the camera. */
	Ogre::Vector3 camOriginalPos;

	/* The position node for the landed drone. */
	GridNode* posNode;

public:

	Drone(GameApplication* game, std::string name, std::string filename, 
		GridNode* posNode, Ogre::Vector3 posOffset, float orient, float scale);
	~Drone();

	/* Get remaining flight time. */
	float getRemainingFlightTime();

	/* Starts the drone. */
	void activate();

	/* Shutdown the drone. */
	void deactivate();

	/* Update the timer/battery. */
	void update(Ogre::Real deltaTime);
};

#endif