#include "Agent.h"
#include "MovableText.h"

Agent::Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale)
{
	using namespace Ogre;

	mSceneMgr = SceneManager; // keep a pointer to where this agent will be

	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Agent constructor" << std::endl;
		return;
	}

	this->height = height;
	this->scale = scale;

	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity);	// attach the model to the scene node

	mBodyNode->translate(0,height,0); // make the Ogre stand on the plane (almost)
	mBodyNode->scale(scale,scale,scale); // Scale the figure

	// Lecture 12GUI: Movable text
	Ogre::MovableText* msg = new Ogre::MovableText("TXT_001", "this is the caption");
	msg->setTextAlignment(Ogre::MovableText::H_CENTER, Ogre::MovableText::V_ABOVE); // Center horizontally and display above the node
	/* msg->setAdditionalHeight( 2.0f ); //msg->setAdditionalHeight( ei.getRadius() ) // apparently not needed from 1.7*/
	mBodyNode->attachObject(msg);

	//AnimationStateSet* aSet = mBodyEntity->getAllAnimationStates();
	//AnimationStateIterator iter = mBodyEntity->getAllAnimationStates()->getAnimationStateIterator();
	//while (iter.hasMoreElements())
	//{
	//	AnimationState *a = iter.getNext();
	//	std::string s = a->getAnimationName();
	//}

	setupAnimations();  // load the animation for this character

	projectile = false; // lecture 12
	ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);  // set nonvisible timeout
	ps = mSceneMgr->createParticleSystem("Fountain1", "Examples/PurpleFountain");
	Ogre::SceneNode* mnode = mBodyNode->createChildSceneNode();
	mnode->roll(Degree(180));
	mnode->attachObject(ps);
	ps->setVisible(false);

	// configure walking parameters
	mWalkSpeed = 35.0f;	
	mDirection = Ogre::Vector3::ZERO;
}

Agent::~Agent(){
	// mSceneMgr->destroySceneNode(mBodyNode); // Note that OGRE does not recommend doing this. It prefers to use clear scene
	// mSceneMgr->destroyEntity(mBodyEntity);
}

void 
Agent::setPosition(float x, float y, float z)
{
	this->mBodyNode->setPosition(x, y + height, z);
}

// update is called at every frame from GameApplication::addTime
void
Agent::update(Ogre::Real deltaTime) 
{
	if (projectile) // Lecture 12
		shoot(deltaTime);
	else
		this->updateLocomote(deltaTime);	// Update Locomotion
	
	this->updateAnimations(deltaTime);	// Update animation playback
}


void 
Agent::setupAnimations()
{
	this->mTimer = 0;	// Start from the beginning
	this->mVerticalVelocity = 0;	// Not jumping

	// this is very important due to the nature of the exported animations
	mBodyEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

	// Name of the animations for this character
	Ogre::String animNames[] =
	{"Delivermail", "Drink", "Drinksitting", "Eat", "Eatsitting", "Nod", "Run", "Sit", "Stand", "Sweep", "Talk", "Talkphone",
	"Talkphonesitting", "Text", "Textsitting", "Think", "Touch", "Walk", "Wave", "Waveclose"};
		//{"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
		//"SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"};

	// populate our animation list
	for (int i = 0; i < 20; i++)
	{
		mAnims[i] = mBodyEntity->getAnimationState(animNames[i]);
		mAnims[i]->setLoop(true);
		mFadingIn[i] = false;
		mFadingOut[i] = false;
	}

	// start off in the idle state (top and bottom together)
	setBaseAnimation(ANIM_THINK);
	setTopAnimation(ANIM_THINK);
}

void 
Agent::setBaseAnimation(AnimID id, bool reset)
{
	if (mBaseAnimID >= 0 && mBaseAnimID < 20)
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
	if (mTopAnimID >= 0 && mTopAnimID < 20)
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

void 
Agent::updateAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	Real baseAnimSpeed = 1;
	Real topAnimSpeed = 1;

	mTimer += deltaTime; // how much time has passed since the last update

	if (mTopAnimID != ANIM_THINK)
	if (mTopAnimID != ANIM_NONE)
	if (mTimer >= mAnims[mTopAnimID]->getLength())
		{
			setTopAnimation(ANIM_THINK, true);
			setBaseAnimation(ANIM_THINK, true);
			mTimer = 0;
		}
	
	// increment the current base and top animation times
	if (mBaseAnimID != ANIM_NONE) mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
	if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

	// apply smooth transitioning between our animations
	fadeAnimations(deltaTime);
}

void 
Agent::fadeAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	for (int i = 0; i < 20; i++)
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

bool 
Agent::nextLocation()
{
	return true;
}

void 
Agent::updateLocomote(Ogre::Real deltaTime)
{
}

void
Agent::fire() // lecture 12
{
	projectile = true; // turns on the movement
	this->setBaseAnimation(ANIM_NOD);

	// set up the initial state
	initPos = this->mBodyNode->getPosition();
	vel.x = 0;
	vel.y = 30;
	vel.z = -30;
	gravity.x = 0;
	gravity.y = -9.81;
	gravity.z = 0;
	ps->setVisible(true);
	this->mBodyNode->yaw(Ogre::Degree(180));
	this->mBodyNode->pitch(Ogre::Degree(45));
	this->mBodyNode->showBoundingBox(true); 
}

void
Agent::shoot(Ogre::Real deltaTime) // lecture 12 call for every frame of the animation
{
	using namespace Ogre;

	Vector3 pos = this->mBodyNode->getPosition();

	// velocity and acceleration? 

	this->mBodyNode->setPosition(pos);


	Ogre::AxisAlignedBox objBox = this->mBodyEntity->getWorldBoundingBox();
	objBox.intersects(objBox); 


	if (this->mBodyNode->getPosition().y <= -0.5) // if it get close to the ground, stop
	{
		// when finished reset
		projectile = false;
		setBaseAnimation(ANIM_WAVE);
		ps->setVisible(false);
		this->mBodyNode->pitch(Ogre::Degree(-45));
		this->mBodyNode->yaw(Ogre::Degree(180));
	}
}

