#include "BaseApplication.h"
#include <deque>

class Agent
{
private:
	Ogre::SceneManager* mSceneMgr;		// pointer to scene graph
	Ogre::SceneNode* mBodyNode;			
	Ogre::Entity* mBodyEntity;
	float height;						// height the character should be moved up
	float scale;						// scale of character from original model

	// lecture 12
	bool projectile; // is this agent going to be launched?
	Ogre::Vector3 initPos; // initial position
	Ogre::Vector3 vel; // velocity of agent
	Ogre::Vector3 gravity; 
	void shoot(Ogre::Real deltaTime); // shoots the agent through the air
	Ogre::ParticleSystem* ps;

	// all of the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together
	// Delivermail, Drink, Drinksitting, Eat, Eatsitting, Nod, Run, Sit, Stand, Sweep, Talk, Talkphone
	//Talkphonesitting, Text, Textsitting, Think, Touch, Walk, Wave, Waveclose
	enum AnimID
	{
		ANIM_DELIVERMAIL,
		ANIM_DRINK,
		ANIM_DRINKSITTING,
		ANIM_EAT,
		ANIM_EATSITTING,
		ANIM_NOD,
		ANIM_RUN,
		ANIM_SIT,
		ANIM_STAND,
		ANIM_SWEEP,
		ANIM_TALK,
		ANIM_TALKPHONE,
		ANIM_TALKPHONESITTING,
		ANIM_TEXT,
		ANIM_TEXTSITTING,
		ANIM_THINK,
		ANIM_TOUCH,
		ANIM_WALK,
		ANIM_WAVE,
		ANIM_WAVECLOSE,
		ANIM_NONE
	};

	Ogre::AnimationState* mAnims[21];		// master animation list
	AnimID mBaseAnimID;						// current base (full- or lower-body) animation
	AnimID mTopAnimID;						// current top (upper-body) animation
	bool mFadingIn[20];						// which animations are fading in
	bool mFadingOut[20];					// which animations are fading out
	Ogre::Real mTimer;						// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity;			// for jumping

	void setupAnimations();					// load this character's animations
	void fadeAnimations(Ogre::Real deltaTime);				// blend from one animation to another
	void updateAnimations(Ogre::Real deltaTime);			// update the animation frame

	// for locomotion
	Ogre::Real mDistance;					// The distance the agent has left to travel
	Ogre::Vector3 mDirection;				// The direction the object is moving
	Ogre::Vector3 mDestination;				// The destination the object is moving towards
	std::deque<Ogre::Vector3> mWalkList;	// The list of points we are walking to
	Ogre::Real mWalkSpeed;					// The speed at which the object is moving
	bool nextLocation();					// Is there another destination?
	void updateLocomote(Ogre::Real deltaTime);			// update the character's walking

	//////////////////////////////////////////////
	// Lecture 4
	bool procedural;						// Is this character performing a procedural animation
    //////////////////////////////////////////////
public:
	Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale);
	~Agent();
	void setPosition(float x, float y, float z);

	void update(Ogre::Real deltaTime);		// update the agent
	
	void setBaseAnimation(AnimID id, bool reset = false);	// choose animation to display
	void setTopAnimation(AnimID id, bool reset = false);

	void fire();		// lecture 12: launches the character
};