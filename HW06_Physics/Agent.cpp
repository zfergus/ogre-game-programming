/*
 * Implementation of the agent class which moves around the world, point to 
 * point.
 * Created by: Zachary Ferguson
 */

#include "Agent.h"

///////////////////////////////////////////////////////////////////////////////
// Boid constants for weighting.
/*
 * The neighborhood radius squared. This is interms global units and multiplied 
 * by the size of a grid node. 
 */
#define NEIGHBORHOOD_RADIUS_SQ (25.0 * NODESIZE * NODESIZE) // = 5 grid nodes
/* 
 * The following coefficents are relative to each other.
 * Alignment and destination pull the agents together when they approach the 
 * goal. Cohesion always pushes boids together. Therefore speration should be 
 * weighted larger than any other component.
 */
#define ALIGNMENT_COEFF   0.50
#define COHESION_COEFF    1.00
#define SEPARATION_COEFF  1.50
#define DESTINATION_COEFF 1.25
/* 
 * Just a weight for each agent with out any distiction. 
 * What other individual weighting could we use? Distance? Scale of agent? 
 * We can just assume that the boids have equal weighting on each other.
 */
#define UNIFORM_WEIGHT 1
///////////////////////////////////////////////////////////////////////////////


/* Create a new Agent. */
Agent::Agent(GameApplication* game, std::string name,  
			 std::string filename, float height, float scale, 
			 GridNode* posNode)
{
	if (game == NULL || game->getSceneManager() == NULL || 
		game->getGrid() == NULL || posNode == NULL)
	{
		std::cout << "ERROR: No valid game in Agent constructor" << 
			std::endl;
		return;
	}
	this->game = game;

	this->height = height;
	this->scale = scale;

	mBodyNode = game->getSceneManager()->getRootSceneNode()->
		createChildSceneNode(); // create a new scene node
	mBodyEntity = game->getSceneManager()->
		createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity); // attach the model to the scene node

	mBodyNode->translate(0,height,0); // make the Ogre stand on the plane
	mBodyNode->scale(scale,scale,scale); // Scale the figure

	setupAnimations(); // load the animation for this character

	// configure walking parameters
	mWalkSpeed = 35.0f;
	mDirection = Ogre::Vector3::ZERO;
	mDestination = Ogre::Vector3::ZERO;

	this->positionNode = posNode;
	this->path = new std::list<GridNode*>();
}

/* Delete this agent. */
Agent::~Agent()
{
	if(this->path != NULL)
		delete this->path;
}

/*
 * Set the position this agents position to <x, y+height, z>.
 */
void Agent::setPosition(float x, float y, float z)
{
	this->mBodyNode->setPosition(x, y + height, z);
}

/*
 * Set the agent's position to the given grid node.
 */
void Agent::setPosition(GridNode* gn, float xOffset, float zOffset)
{
	/* Invalid gridnode for the given offsets. */
	if(abs(xOffset) > NODESIZE/2.0 || abs(zOffset) > NODESIZE/2.0)
		return;
	this->positionNode = gn;
	Ogre::Vector3 pos = this->game->getGrid()->getPosition(gn);
	this->setPosition(pos.x + xOffset, 0, pos.z + zOffset);
}

/*
 * Returns the grid node this agent is in.
 */
GridNode* Agent::getPosition()
{
	return this->positionNode;
}

/* 
 * Update is called at every frame from GameApplication::addTime.
 */
void Agent::update(Ogre::Real deltaTime)
{
	this->updateAnimations(deltaTime);	// Update animation playback
	this->updateLocomote(deltaTime);	// Update Locomotion
}

void Agent::setupAnimations()
{
	this->mTimer = 0;	// Start from the beginning
	this->mVerticalVelocity = 0;	// Not jumping

	// this is very important due to the nature of the exported animations
	mBodyEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

	// Name of the animations for this character
	Ogre::String animNames[] =
		{"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", 
		"HandsRelaxed", "DrawSwords", "SliceVertical", "SliceHorizontal", 
		"Dance", "JumpStart", "JumpLoop", "JumpEnd"};

	// populate our animation list
	for (int i = 0; i < 13; i++)
	{
		mAnims[i] = mBodyEntity->getAnimationState(animNames[i]);
		mAnims[i]->setLoop(true);
		mFadingIn[i] = false;
		mFadingOut[i] = false;
	}

	// start off in the idle state (top and bottom together)
	setBaseAnimation(ANIM_IDLE_BASE);
	setTopAnimation(ANIM_IDLE_TOP);

	// relax the hands since we're not holding anything
	mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
}

void Agent::setBaseAnimation(AnimID id, bool reset)
{
	if (mBaseAnimID >= 0 && mBaseAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mBaseAnimID] = false;
		mFadingOut[mBaseAnimID] = true;
	}

	mBaseAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

void Agent::setTopAnimation(AnimID id, bool reset)
{
	if (mTopAnimID >= 0 && mTopAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mTopAnimID] = false;
		mFadingOut[mTopAnimID] = true;
	}

	mTopAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

void Agent::updateAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	Real baseAnimSpeed = 1;
	Real topAnimSpeed = 1;

	mTimer += deltaTime; // how much time has passed since the last update

	// increment the current base and top animation times
	if (mBaseAnimID != ANIM_NONE)
	{
		mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
	}
	if (mTopAnimID != ANIM_NONE)
	{
		mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);
	}

	// apply smooth transitioning between our animations
	fadeAnimations(deltaTime);
}

void Agent::fadeAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	for (int i = 0; i < 13; i++)
	{
		if (mFadingIn[i])
		{
			// slowly fade this animation in until it has full weight
			Real newWeight = mAnims[i]->getWeight() + deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight >= 1) mFadingIn[i] = false;
		}
		else if (mFadingOut[i])
		{
			// slowly fade this animation out until it has no weight, and then disable it
			Real newWeight = mAnims[i]->getWeight() - deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight <= 0)
			{
				mAnims[i]->setEnabled(false);
				mFadingOut[i] = false;
			}
		}
	}
}

/*
 * Returns true if there is a location left, and sets the destination vars.
 */
bool Agent::nextLocation()
{
	//if(this->path->empty())
	//	return false;

	//this->positionNode = this->path->front();
	//this->path->pop_front();

	//mDestination = this->game->getGrid()->getPosition(this->positionNode);

	if(this->mWalkList.empty())
		return false;

	mDestination = mWalkList.front();
	mWalkList.pop_front();

	mDestination.y = this->height;
	mDirection = mDestination - mBodyNode->getPosition();
	mDistance = mDirection.normalise();
	return true;
}

/* 
 * Moves the agent to the next location.
 */
void Agent::updateLocomote(Ogre::Real deltaTime)
{
	// If no current destination
	if(mDirection == Ogre::Vector3::ZERO && 
		mDestination != mBodyNode->getPosition())
	{
		// Is there a next destination
		if(nextLocation())
		{
			if(this->mBaseAnimID != AnimID::ANIM_RUN_BASE)
			{
				// Start Animations
				setBaseAnimation(AnimID::ANIM_RUN_BASE, true);
			}
			if(this->mTopAnimID != AnimID::ANIM_RUN_TOP)
			{
				setTopAnimation(AnimID::ANIM_RUN_TOP, true);
			}

			// Turn towards the next location
			Ogre::Vector3 src =
				mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;
			if(1.0 + src.dotProduct(mDirection) < 1e-4)
			{
				mBodyNode->yaw(Ogre::Degree(180));
			}
			else
			{
				Ogre::Quaternion quat = src.getRotationTo(mDirection);
				mBodyNode->rotate(quat);
			}
		}
		else if(this->mTopAnimID != AnimID::ANIM_IDLE_TOP ||
			this->mBaseAnimID != AnimID::ANIM_IDLE_BASE)
		{
			setTopAnimation(AnimID::ANIM_IDLE_TOP, true);
			setBaseAnimation(AnimID::ANIM_IDLE_BASE, true);
			mDestination = NULL;
		}
	}
	// There is a current destination
	else
	{
		// Are we at the current destination?
		if(mDistance <= NODESIZE/4.0) // Close enough
		{
			//mBodyNode->setPosition(mDestination);
			mDirection = Ogre::Vector3::ZERO;

			this->signalBoids(); // Signal the flock to change directions.
			
			// Is there another location?
			if(nextLocation())
			{
				// Turn towards the next location
				Ogre::Vector3 src =
					mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;
				if(1.0 + src.dotProduct(mDirection) < 1e-4)
				{
					mBodyNode->yaw(Ogre::Degree(180));
				}
				else
				{
					Ogre::Quaternion quat = src.getRotationTo(mDirection);
					mBodyNode->rotate(quat);
				}
			}
			else
			{
				setTopAnimation(AnimID::ANIM_IDLE_TOP, true);
				setBaseAnimation(AnimID::ANIM_IDLE_BASE, true);
				mDestination = NULL;
			}
		}
		else
		{
			/* Uniform speed relative to the frame rate. */
			Ogre::Real speed = mWalkSpeed * deltaTime;
		
			/* Recalculate direction to account for drift. */
			mDirection = mDestination - mBodyNode->getPosition();
			/* Recalculate distance to the goal. */
			mDistance = mDirection.normalise();

			//Ogre::Vector3 flockDirection = this->computeFlockDirection();
			/* Final velocity vector with magnitude = speed. */
			Ogre::Vector3 velocity = speed * (
				DESTINATION_COEFF * this->mDirection + 
				this->computeFlockDirection()).normalisedCopy();

			/* Face in the direction of the velocity. */
			Ogre::Vector3 src =
				mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;
			if(1.0 + src.dotProduct(velocity) < 1e-4)
			{
				mBodyNode->yaw(Ogre::Degree(180));
			}
			else
			{
				mBodyNode->rotate(src.getRotationTo(velocity));
			}

			/* Move the agent. */
			mBodyNode->translate(velocity);
		}
	}
}

Ogre::Vector3 Agent::computeFlockDirection()
{
	/* List of agents. */
	std::list<Agent*>* agents = this->game->getAgents();

	/* Components of the final flock velocity. */
	Ogre::Vector3 alignment      = Ogre::Vector3::ZERO;
	Ogre::Vector3 cohesion       = Ogre::Vector3::ZERO;
	Ogre::Vector3 neighborhoodCM = Ogre::Vector3::ZERO;
	Ogre::Vector3 separation     = Ogre::Vector3::ZERO;
	float sum_of_weights = 0;

	/* Loop over all agents */
	for(auto iter = agents->begin(); iter != agents->end(); iter++)
	{
		if((*iter) == this) /* Dont consider this agent. */
			continue;

		/* How far is the agent away. */
		Ogre::Vector3 distanceVec = this->mBodyNode->getPosition() - 
			(*iter)->mBodyNode->getPosition();
		float distSQ = distanceVec.squaredLength();
		/* If within the Radius^2 */
		if(distSQ > 1e-8 && distSQ <= NEIGHBORHOOD_RADIUS_SQ)
		{
			sum_of_weights += UNIFORM_WEIGHT;

			/* Speration += vector from agent to me. */
			separation += UNIFORM_WEIGHT * distanceVec / distSQ;

			/* Alignment += vector towards direction. */
			alignment += UNIFORM_WEIGHT * (*iter)->mDirection;

			/* Calculate center of mass for the neighborhood. */
			neighborhoodCM += UNIFORM_WEIGHT * 
				(*iter)->mBodyNode->getPosition();
		}
	}

	/* If no neighbors. */
	if(abs(sum_of_weights) < 1e-8)
		return Ogre::Vector3::ZERO;

	alignment /= sum_of_weights;
	
	/* Cohesion = vector towards center of mass. */
	cohesion = (neighborhoodCM / sum_of_weights) - 
		this->mBodyNode->getPosition();

	/* flockVelocity = Sum of normalized components weighted relativly. */
	Ogre::Vector3 flockVelocity = 
		ALIGNMENT_COEFF  * alignment.normalisedCopy() + 
		COHESION_COEFF   * cohesion.normalisedCopy() + 
		SEPARATION_COEFF * separation.normalisedCopy();
	/* Never move up in space. */
	flockVelocity.y = 0;

	/* This is the vector for only the flock not the destination. */
	return flockVelocity;
}

/* Signal all other agents in the flock to change directions. */
void Agent::signalBoids()
{
	std::list<Agent*>* boids = this->game->getAgents();

	for(auto iter = boids->begin(); iter != boids->end(); iter++)
	{
		if(*iter == this) continue;

		// Cause the agent to change directions to the next destination.
		(*iter)->mDirection   = Ogre::Vector3::ZERO;
		(*iter)->mDestination = NULL;
	}

	this->game->nextMarker();
}

/* Add a destination to the WalkList. */
void Agent::addDestinationLocation(GridNode* node)
{
	Ogre::Vector3 pos = this->game->getGrid()->getPosition(node);
	pos.y = this->height;
	mWalkList.push_back(pos);
}

/* Returns a unique name for loaded objects and agents */
void Agent::printPath(std::list<GridNode*>& pathToPrint)
{
	static int count = 0;	// keep counting the number of objects

	std::stringstream out;	// a stream for outputing to a string
	out << count++;			// make the current count into a string

	if(!(pathToPrint.empty()))
	{
		auto iter = pathToPrint.begin();
		(*(iter++))->contains = 'S';
		int i = 0;
		for(iter; iter != pathToPrint.end(); iter++)
		{
			(*(iter))->contains = '0' + (i++ % 10);
		}
		auto riter = pathToPrint.rbegin();
		(*(riter++))->contains = 'G';
	}
	else
	{
		this->positionNode->contains = 'S';
	}

	this->game->getGrid()->printToFile("Grid" + out.str() + ".txt");
	this->game->getGrid()->resetPathChars();
}

/* 
 * A* Path Finding from the current node of the agent to the given destination.
 */
void Agent::walkTo(GridNode* destination)
{
	if(destination == NULL || !(destination->isClear()))
		return;
	
	/* Start from the current position if not walking anymore, otherwise */
	/* start from the end of the current path.                           */
	GridNode* start = (this->path->empty()) ? 
		(this->positionNode):(this->path->back());

	/* Find the path from start to destination. */
	std::list<GridNode*> newPath = 
		this->game->getGrid()->findPath(start, destination);
	
	if(newPath.empty())
	{
		std::cout << "No possible path found from ("<< 
			this->positionNode->getRow()    << ", " << 
			this->positionNode->getColumn() << ") to (" << 
			destination->getRow() << ", " << destination->getColumn() 
			<< ")" << std::endl;
	}
	else if(newPath.size() == 1)
	{
		std::cout << "Path is from current node to current node." << std::endl;
	}
	else
	{
		this->path->insert(path->end(), newPath.begin(), newPath.end());
	}

	this->printPath(newPath);
}
