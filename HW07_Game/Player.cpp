/*
 * Class for the player character. Includes movement and exit collision 
 * detection.
 * Author: Zachary Ferguson
 */

#include "Player.h"

/* Create a player character. */
Player::Player(GameApplication* game, std::string name, 
		std::string filename, float height, float scale, GridNode* posNode)
		: Agent(game, name, filename, height, scale, posNode)
{
	setupAnimations(); // load the animation for this character

	this->goingForward = false;
	this->goingBack = false;
	this->turningLeft = false; 
	this->turningRight = false;

	this->mWalkSpeed = PLAYER_WALK_SPEED;
	this->mRotSpeed = PLAYER_ROT_SPEED;
}

/* Delete the player. */
Player::~Player(){}

/* Get the axis aligned bounding box around the player. */
Ogre::AxisAlignedBox Player::getAABB() const
{
	//this->mBodyNode->showBoundingBox(true);
	this->mBodyNode->_update(true, true);
	return this->mBodyNode->_getWorldAABB();
}

/* Get the body node of this player. */
Ogre::SceneNode* Player::getBodyNode() const
{
	return this->mBodyNode;
}

/* Returns the grid node this agent is in. */
GridNode* Player::getPosition() const
{
	return this->game->getGrid()->getNode(this->mBodyNode->getPosition());
}

/* Returns the absolute position of the player. */
Ogre::Vector3 Player::getAbsolutePosition() const
{
	return this->mBodyNode->getPosition();
}

/* 
 * Moves the agent to the next location.
 */
void Player::updateLocomote(Ogre::Real deltaTime)
{
	if(this->goingForward || this->goingBack)
	{
		// Start Animations
		if(this->mTopAnimID != AnimID::ANIM_RUN_TOP)
		{
			setTopAnimation(AnimID::ANIM_RUN_TOP, true);
		}
		if(this->mBaseAnimID != AnimID::ANIM_RUN_BASE)
		{
			setBaseAnimation(AnimID::ANIM_RUN_BASE, true);
		}
		
		Ogre::Real move = mWalkSpeed * deltaTime; 
		Ogre::Vector3 pos = this->mBodyNode->getPosition();
		Ogre::Vector3 tmp = this->mBodyNode->getOrientation() * 
			Ogre::Vector3::UNIT_Z;
		tmp *= this->goingForward ? (move):(-move);

		GridNode* gn = this->game->getGrid()->getNode(pos + 10*tmp);
		if(gn->isClear())
		{
			this->mBodyNode->setPosition(pos + tmp);
		}
		else if(gn->contains == EXIT_CHAR)
		{
			this->game->nextLevel();
		}
		else if(gn->contains == DRONE_CHAR)
		{
			this->game->activateDrone();
		}
	}
	else
	{
		// Stop Animation
		if(this->mTopAnimID != AnimID::ANIM_IDLE_TOP)
		{
			setTopAnimation(AnimID::ANIM_IDLE_TOP, true);
		}
		if(this->mBaseAnimID != AnimID::ANIM_IDLE_TOP)
		{
			setBaseAnimation(AnimID::ANIM_IDLE_BASE, true);
		}
	}

	if(this->turningLeft || this->turningRight)
	{
		Ogre::Degree angle = Ogre::Degree((this->turningLeft ? (1):(-1)) * 
			deltaTime * this->mRotSpeed);
		this->mBodyNode->yaw(angle);
	}
}

/* Process directional movement in the grid. */
bool Player::injectKeyDown( const OIS::KeyEvent &arg)
{
	if(arg.key == FORWARD_KEY)
	{
		this->goingForward = true;
		this->goingBack = false;
		return true;
	}
	else if (arg.key == BACKWARD_KEY)
	{
		this->goingForward = false;
		this->goingBack = true;
		return true;
	}
	else if (arg.key == LEFTWARD_KEY)
	{
		this->turningLeft = true;
		this->turningRight = false;
		return true;
	}
	else if (arg.key == RIGHTWARD_KEY)
	{
		this->turningLeft = false;
		this->turningRight = true;
		return true;
	}
	else if (arg.key == RUN_KEY)
	{
		this->mWalkSpeed = PLAYER_RUN_SPEED;
		return true;
	}
	return false;
}

/* Process directional movement in the grid. */
bool Player::injectKeyUp( const OIS::KeyEvent &arg)
{
	if(arg.key == FORWARD_KEY)
	{
		this->goingForward = false;
		return true;
	}
	else if (arg.key == BACKWARD_KEY)
	{
		this->goingBack = false;
		return true;
	}
	else if (arg.key == LEFTWARD_KEY)
	{
		this->turningLeft = false;
		return true;
	}
	else if (arg.key == RIGHTWARD_KEY)
	{
		this->turningRight = false;
		return true;
	}
	else if (arg.key == RUN_KEY)
	{
		this->mWalkSpeed = PLAYER_WALK_SPEED;
		return true;
	}
	return false;
}