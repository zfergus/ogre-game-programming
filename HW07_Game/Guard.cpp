/*
 * Child class of the Agent class. Class for a guard that roams and searches 
 * for the player.
 * Author: Zachary Ferguson
 */

#include "Guard.h"
#include "Player.h"

///////////////////////////////////////////////////////////////////////////////
// Static siren control variables.
std::string Guard::sirenFName = "";
int Guard::chasingPlayer = 0;
///////////////////////////////////////////////////////////////////////////////

Guard::Guard(GameApplication* game, std::string name, 
		std::string filename, float height, float scale, GridNode* posNode)
		: Agent(game, name, filename, height, scale, posNode)
{
	setupAnimations(); // load the animation for this character

	state = GuardState::ROAMING;
	this->mWalkSpeed = GUARD_WALK_SPEED;

	this->facingVector = Ogre::Vector3::UNIT_X;

	if(Guard::sirenFName == "")
	{
		//gets the current cpp file's path
		Guard::sirenFName = __FILE__;
		//removes filename
		Guard::sirenFName = Guard::sirenFName.substr(0, 1 + 
			Guard::sirenFName.find_last_of('\\')); 
		Guard::sirenFName = Guard::sirenFName + "siren.wav";
	}
}

Guard::~Guard(){}

/* Load this character's animations */
void Guard::setupAnimations()
{
	this->mTimer = 0;	// Start from the beginning
	this->mVerticalVelocity = 0;	// Not jumping

	// this is very important due to the nature of the exported animations
	mBodyEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

	// Name of the animations for this character
	Ogre::String animNames[] =
		{"Idle", "Idle", "Walk", "Walk", "Idle", 
		"Idle", "Idle", "Idle", "Idle", 
		"Idle", "Idle", "Idle", "Idle"};

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

/* 
 * Update is called at every frame from GameApplication::addTime.
 */
void Guard::update(Ogre::Real deltaTime)
{
	this->updateAnimations(deltaTime);	// Update animation playback
	this->collisionDetection();
	this->checkForPlayer();
	this->updateLocomote(deltaTime);	// Update Locomotion
}

/*
 * Returns true if there is a location left, and sets the destionation vars.
 */
bool Guard::nextLocation()
{
	if(this->path->empty())
	{
		if(this->state == GuardState::ROAMING)
		{
			GridNode *gn;

			// Loop until a clear node is found. This loop is guaranteed to
			// terminate iff an agent exists because an agent is always 
			// spawned on a clear node.
			do
			{
				/* Random (row, col) coordinates in grid. */
				int r = rand() % 10 - 5 + this->positionNode->getRow();
				int c = rand() % 10 - 5 + this->positionNode->getColumn();
				gn = this->game->getGrid()->getNode(r, c);
			}while(gn == NULL || !(gn->isClear()));
			this->walkTo(gn);
			return this->nextLocation();
		}
		else
		{
			Guard::chasingPlayer--;
			if(this->chasingPlayer == 0)
			{
				PlaySound(NULL, NULL, 0);
			}

			this->state = GuardState::ROAMING;
			this->mWalkSpeed = GUARD_WALK_SPEED;
			return false;
		}
	}

	this->positionNode = this->path->front();
	this->path->pop_front();

	mDestination = this->game->getGrid()->getPosition(this->positionNode);
	mDestination.y = this->height;
	mDirection = mDestination - mBodyNode->getPosition();
	mDistance = mDirection.normalise();
	return true;
}


/* Check if the player is in the linesight of the guard. */
void Guard::checkForPlayer()
{
	Player* player = this->game->getPlayer();
	if(player == NULL)
		return;

	GridNode* playerPos = this->game->getPlayer()->getPosition();
	
	// Searching for the player and the player has not moved.
	if(this->state == GuardState::SEARCHING && !this->path->empty() &&
		this->path->back() == playerPos)
	{
		return;
	}

	// Check that the guard is facing the player (Player is within 90deg FOV)
	Ogre::Vector3 toPlayer = this->game->getPlayer()->getAbsolutePosition() - 
		this->mBodyNode->getPosition();
	toPlayer.normalise();
	Ogre::Vector3 dir = this->mDirection.normalisedCopy();
	Ogre::Real angle = acos(degToRad(toPlayer.dotProduct(this->mDirection)));
	if(angle > PI_OVER_2) // Angle is greater than 90deg
	{
		return; // Player is behind the guard
	}
	

	// Check for line of sight in the row.
	if(this->positionNode->getRow() == playerPos->getRow())
	{
		// Check for walls
		bool canSeePlayer = true;
		int i = playerPos->getRow();
		int minCol = std::min(this->positionNode->getColumn(), 
			playerPos->getColumn());
		int maxCol = std::max(this->positionNode->getColumn(), 
			playerPos->getColumn());
		for(int j = minCol; j <= maxCol; j++)
		{
			if(!(this->game->getGrid()->getNode(i, j)->isClear()))
			{
				canSeePlayer = false;
				break;
			}
		}

		if(canSeePlayer)
		{
			this->path->clear();
			this->walkTo(playerPos);
			this->state = GuardState::SEARCHING;
			this->mWalkSpeed = GUARD_RUN_SPEED;
		
			if(Guard::chasingPlayer == 0)
			{
				PlaySound(TEXT(Guard::sirenFName.c_str()), NULL, 
					SND_FILENAME | SND_ASYNC | SND_LOOP);
				Guard::chasingPlayer++;
			}
		}
	}
	else if(this->positionNode->getColumn() == playerPos->getColumn())
	{
				// Check for walls
		bool canSeePlayer = true;
		int j = playerPos->getColumn();
		int minRow = std::min(this->positionNode->getRow(), 
			playerPos->getRow());
		int maxRow = std::max(this->positionNode->getRow(), 
			playerPos->getRow());
		for(int i = minRow; i <= maxRow; i++)
		{
			if(!(this->game->getGrid()->getNode(i, j)->isClear()))
			{
				canSeePlayer = false;
				break;
			}
		}

		if(canSeePlayer)
		{
			// Check for walls
			this->path->clear();
			this->walkTo(playerPos);
			this->state = GuardState::SEARCHING;
			this->mWalkSpeed = GUARD_RUN_SPEED;

			if(Guard::chasingPlayer == 0)
			{
				PlaySound(TEXT(Guard::sirenFName.c_str()), NULL, 
					SND_FILENAME | SND_ASYNC | SND_LOOP);
				Guard::chasingPlayer++;
			}
		}
	}
	//else if(this->state == GuardState::SEARCHING)
	//{
	//	this->state = GuardState::ROAMING;
	//	this->mWalkSpeed = GUARD_WALK_SPEED;

	//	//Guard::chasingPlayer--;
	//	//if(this->chasingPlayer == 0)
	//	//{
	//	//	PlaySound(NULL, NULL, 0);
	//	//}
	//}
}


/* Check for a collision with the Player. */
void Guard::collisionDetection()
{
	Player* player = this->game->getPlayer();
	if(player == NULL)
		return;

	//this->mBodyNode->showBoundingBox(true);
	this->mBodyNode->_update(true, true);
	Ogre::AxisAlignedBox mAABB = this->mBodyNode->_getWorldAABB();
	Ogre::AxisAlignedBox pAABB = player->getAABB();
	if(mAABB.intersects(pAABB))
	{
		this->game->gameOver();
	}
}