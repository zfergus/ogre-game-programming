/*
 * Class for a simple physics based projectile.
 * Author: Zachary Ferguson
 */

#include "Projectile.h"

/* Construct a new projectile. Filename is the mesh file. */
Projectile::Projectile(GameApplication* game, std::string name,
	std::string filename, Ogre::Vector3 init_position, Ogre::Real scale)
{
	// Valid arguments?
	if (game == NULL || game->getSceneManager() == NULL || 
		game->getGrid() == NULL)
	{
		std::cout << "ERROR: No valid game in Agent constructor" << 
			std::endl;
		return;
	}

	this->game = game;
	
	// Create a body node and a rotation node
	this->bodyNode = game->getSceneManager()->getRootSceneNode()->
		createChildSceneNode();
	this->rotNode = this->bodyNode->createChildSceneNode();
	this->bodyEntity = game->getSceneManager()->createEntity(name, filename);
	this->rotNode->attachObject(this->bodyEntity);
	//this->rotNode->showBoundingBox(true);

	// Set the body node to the initial values.
	this->bodyNode->setPosition(init_position);
	this->bodyNode->scale(scale, scale, scale);

	// Store the initial values for later calculations.
	this->init_position = init_position;
	this->init_velocity = Ogre::Vector3::ZERO;
	this->timer = 0;

	// Has not been fired yet.
	this->isFired = false;
}

/* Nothing to delete. */
Projectile::~Projectile(){}

/* Creates a child scene node, useful for camera following. */
Ogre::SceneNode* Projectile::createChildSceneNode()
{
	/* Child will be expempt from setOrientation() calls. */
	return this->bodyNode->createChildSceneNode();
}

/* Sets the orientation given Euler Angles around each canonical axis. */
void Projectile::setOrientation(Ogre::Real x, Ogre::Real y, Ogre::Real z)
{
	Ogre::Matrix3 mat = Ogre::Matrix3();
	mat.FromEulerAnglesXYZ(Ogre::Degree(x), Ogre::Degree(y), Ogre::Degree(z));
	this->setOrientation(Ogre::Quaternion(mat));
}

/* Sets the orientation of the projectile. */
void Projectile::setOrientation(Ogre::Quaternion orien)
{
	/* Only rotate the projectile entity not children of bodyNode. */
	this->rotNode->setOrientation(orien);
}

/* Gets the position of the projectile in 3D space. */
Ogre::Vector3 Projectile::getPosition()
{
	return this->bodyNode->getPosition();
}

/* Move the projectile if it has been fired. */
void Projectile::update(Ogre::Real deltaTime)
{
	if(!(this->isFired) || (this->bodyNode->getPosition()).y <= 0)
	{
		return;
	}

	// Current time value
	this->timer += deltaTime;

	///////////////////////////////////////////////////////////////////////////
	// Projectile Motion Equations
	// a = g
	// v = a*t + v0
	// p = 0.5*a*t^2 + v0*t + p0
	Ogre::Vector3 position = 0.5f * GRAVITY * timer * timer + 
		init_velocity * timer + init_position;
	this->bodyNode->setPosition(position);

	///////////////////////////////////////////////////////////////////////////
	// Check for a collision with the target.
	if(this->checkCollision())
	{
		return;
	}

	///////////////////////////////////////////////////////////////////////////
	// The projectile has fallen through the floor plane
	if(this->bodyNode->getPosition().y <= 0)
	{
		this->miss();
	}
}

/* 
 * Check for a collision with the top of the target. Returns true iff hit 
 * something. 
 */
bool Projectile::checkCollision()
{
	// Get the whole targets AABB
	Ogre::AxisAlignedBox targetAABB = this->game->getTargetAABB();
	
	// Construct a top AABB for the target
	Ogre::AxisAlignedBox topAABB = targetAABB;
	topAABB.setMinimumY(topAABB.getMaximum().y - 0.1);
	// Is a height of one big enough?
	topAABB.setMaximumY(topAABB.getMaximum().y + 1.0);
	
	// Get the projectiles AABB
	this->bodyNode->_update(true, true);
	Ogre::AxisAlignedBox myAABB = this->rotNode->_getWorldAABB();
	
	// First check if the projectile is intersecting with the top of the 
	// target.
	if(topAABB.intersects(myAABB))
	{
		std::cout << "Scored a point" << std::endl;
		this->game->scorePoint();
		return true;
	}
	// Is the projectile intersecting the sides?
	else if(targetAABB.intersects(myAABB))
	{
		this->miss();
		return true;
	}

	return false;
}

/* 
 * Call this if the projectile missed. Plays a fail sound and resets through 
 * game.
 */
void Projectile::miss()
{
	std::string path = __FILE__; //gets the current cpp file's path
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename
	path+= "MissSound.wav";
	PlaySound(TEXT(path.c_str()), NULL, SND_FILENAME | SND_ASYNC);
		
	this->game->resetProjectile(); // Calls this->reset()
}

/* Fire this projectile, causing it to move during update. */
void Projectile::fire(Ogre::Vector3 init_velocity)
{
	this->init_velocity = init_velocity;
	this->isFired = true;
}

/* Reset the projectie to its initial settings. */
void Projectile::reset()
{
	this->isFired = false;
	this->bodyNode->setPosition(init_position);
	this->timer = 0;
}
