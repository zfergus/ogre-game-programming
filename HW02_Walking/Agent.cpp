#include "Agent.h"

Agent::Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename)
{
	using namespace Ogre;

	mSceneMgr = SceneManager; // keep a pointer to where this agent will be

	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Agent constructor" << std::endl;
		return;
	}

	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity);	// attach the model to the scene node

	mBodyNode->translate(0,4.5,0); // make the Ogre stand on the plane (almost)

	setupAnimations();  // load the animation for this character

	// configure walking parameters
	mWalkSpeed = 35.0f;
	mDirection = Ogre::Vector3::ZERO;
	mDestination = Ogre::Vector3::ZERO;

	// create random points to walk to
	float x, y;
	for (int i = 0; i < 10; i++)
	{
		x = rand() % 60 - 30;
		y = rand() % 60 - 30;
		mWalkList.push_back(Ogre::Vector3(x, 4.5, y));
	}
}

Agent::~Agent(){
	// mSceneMgr->destroySceneNode(mBodyNode); // Note that OGRE does not recommend doing this. It prefers to use clear scene
	// mSceneMgr->destroyEntity(mBodyEntity);
}

// update is called at every frame from GameApplication::addTime
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
		{"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
		"SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"};

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
	if (mBaseAnimID != ANIM_NONE) mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
	if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

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

/* Returns true if there is a location left. */
bool Agent::nextLocation()
{
	if(mWalkList.empty())
		return false;
	mDestination = mWalkList.front();
	mWalkList.pop_front();
	mDirection = mDestination - mBodyNode->getPosition();
	mDistance = mDirection.normalise();
	return true;
}

/* Moves the agent to the next location. */
void Agent::updateLocomote(Ogre::Real deltaTime)
{
	// If no current destination
	if(mDirection == Ogre::Vector3::ZERO)
	{
		// Is there a next destination
		if(nextLocation())
		{
			// Start Animations
			setTopAnimation(AnimID::ANIM_RUN_TOP, true);
			setBaseAnimation(AnimID::ANIM_RUN_BASE, true);

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
	}
	// There is a current destination
	else
	{
		Ogre::Real move = mWalkSpeed * deltaTime;
		mDistance -= move;
		// Are we at the current destination?
		if(mDistance <= 0)
		{
			mBodyNode->setPosition(mDestination);
			mDirection = Ogre::Vector3::ZERO;
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
			}
		}
		else
		{
			mBodyNode->translate(move * mDirection);
		}
	}
}
