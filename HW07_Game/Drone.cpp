#include "GameApplication.h"
#include "Drone.h"
#include "Player.h"

Drone::Drone(GameApplication* game, std::string name, std::string filename, 
		GridNode* posNode, Ogre::Vector3 posOffset, float orient, float scale)
{
	if (game == NULL || game->getSceneManager() == NULL || 
		game->getGrid() == NULL || posNode == NULL)
	{
		std::cout << "ERROR: No valid game in Agent constructor" << 
			std::endl;
		return;
	}

	this->game = game;

	this->bodyEntity = this->game->getSceneManager()->createEntity(name, 
		filename);
	this->bodyNode = this->game->getSceneManager()->getRootSceneNode()->
		createChildSceneNode(name, Ogre::Vector3::ZERO);
	this->bodyNode->attachObject(this->bodyEntity);
	this->bodyNode->setScale(scale, scale, scale);
	this->bodyNode->yaw(Ogre::Degree(orient));

	this->posNode = posNode;
	this->bodyNode->setPosition(this->game->getGrid()->getPosition(
		this->posNode) + posOffset);
	this->posNode->setOccupied();
	this->posNode->entity = this->bodyEntity;
	this->posNode->contains = DRONE_CHAR;

	this->timer = 0;
	this->state = DroneState::ON_GROUND;

	this->camOriginalPos = Ogre::Vector3::ZERO;
}

Drone::~Drone(){}

/* Get remaining flight time. */
float Drone::getRemainingFlightTime()
{
	switch (this->state)
	{
	case DroneState::ON_GROUND:
		return 0;
	case DroneState::FLYING:
		return this->timer;
	default:
		return FLIGHT_TIME;
	}
}

void Drone::activate()
{
	if(this->state == DroneState::ON_GROUND)
	{
		this->timer = TAKE_OFF_TIME;
		this->state = DroneState::TAKING_OFF;
		// Save the current position.
		this->camOriginalPos = this->game->getCamera()->getPosition();
		this->game->getCamera()->setPosition(Ogre::Vector3::ZERO);

		this->bodyNode->setVisible(false);
		this->posNode->setClear();
		this->posNode->entity = NULL;
		this->posNode->contains = '.';
	}
}

void Drone::deactivate()
{
	this->state = DroneState::ON_GROUND;
	this->timer = 0.0;
	this->game->getCamera()->setPosition(this->camOriginalPos);
	this->game->getCamera()->lookAt(
		this->game->getPlayer()->getAbsolutePosition());
}

void Drone::update(Ogre::Real deltaTime)
{
	if(this->state == DroneState::TAKING_OFF)
	{
		this->timer -= deltaTime;
		// Change the height of the camera
		if(this->timer > 0)
		{
			float height = (TAKE_OFF_TIME - this->timer)/TAKE_OFF_TIME * 
				FLYING_HEIGHT;
			this->game->getCamera()->setPosition(height * 
				Ogre::Vector3::UNIT_Y);
		}
		else
		{
			this->state = DroneState::FLYING;
			this->timer = FLIGHT_TIME;
			this->game->getCamera()->setPosition(FLYING_HEIGHT * 
				Ogre::Vector3::UNIT_Y);
		}
	}
	else if(this->state == DroneState::FLYING)
	{
		this->timer -= deltaTime;
		if(this->timer <= 0)
		{
			this->deactivate();
		}
	}
}