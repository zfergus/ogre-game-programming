/*
 * Class for a simple physics based projectile.
 * Author: Zachary Ferguson
 */

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "GameApplication.h"

// Predefine value for gravity
#define GRAVITY (Ogre::Vector3(0, -9.81, 0))

class GridNode;
class GameApplication;

class Projectile
{
private:
	GameApplication* game; // Pointer to the game.

	Ogre::SceneNode* bodyNode;
	Ogre::SceneNode* rotNode; // Child node only for the projectiles rotation.
	Ogre::Entity* bodyEntity;
	
	// Values to be used for projectile motion calculations
	Ogre::Vector3 init_position;
	Ogre::Vector3 init_velocity;
	Ogre::Real timer;

	// Has the projectile been fired.
	bool isFired;

	/* 
	 * Check for a collision with the top of the target. Returns true iff hit 
	 * something. 
	 */
	bool checkCollision();

	/* 
	 * Call this if the projectile missed. Plays a fail sound and resets through 
	 * game.
	 */
	void miss();

public:
	Projectile(GameApplication* game, std::string name, std::string filename, 
		Ogre::Vector3 init_position, Ogre::Real scale);
	~Projectile();

	/* Abstract the complex SceneNode structure. */
	Ogre::SceneNode* createChildSceneNode();
	void setOrientation(Ogre::Real x, Ogre::Real y, Ogre::Real z); // Euler
	void setOrientation(Ogre::Quaternion orien); // Quaternion
	Ogre::Vector3 Projectile::getPosition();

	/* Update motion if fired. */
	void update(Ogre::Real deltaTime);

	/* Fire this projectile. */
	void fire(Ogre::Vector3 init_velocity);

	/* Reset the projectile to its original position and state. */
	void reset();
};

#endif