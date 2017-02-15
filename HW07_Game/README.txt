CS 425: Game Programming
HW 07: Game
Zachary Ferguson

Controls:

W - Move forwards
A - Turn left
S - Move backwards
D - Turn right
Mouse - Look around
	
Objective:

	The objective of this game is to reach the house with out being caught by 
the robot guards. If the guards spot you they will chase after you. Break their 
line of sight to hide again. If the guards catch you, you loose and are sent 
back to the first level.
	Hidden in the world is a quad-copter that can be used to get a birds eye 
view of the map. This quad-copter has a limited battery/flight time after which
the camera will return to the player.
	To advance to the next level reach the goal in the current level. To beat 
the game reach the goal at the end of the third level.

Requirements:

01. This game is implemented using Ogre. The player moves around the world and 
	avoids enemies.
 
02. The game states are as follows:
	1) The guards are roaming and the player is hidden from them.
	2) The guards spot the player and chase after him/her. The player must run
		away to avoid being caught.
	3) The player unlocks the quad-copter which gives him/her the limited 
		ability to see from above the map.
		
03. Both the character and robot guards are animated and switch animations 
	based on the users actions.
	
04. The guards roam around to different location nearby them. This is a random
	node and is just luck, where the guard moves to.

05. The game includes three levels. The levels switch when the player reaches 
	the goal.
	
06. The guards use A* to both roam around and move to the player. This helps 
	them avoid obstacles.
	
07. This game uses collision detection when checking if the guards have 
	collided with the player. This is done by examining bounding boxes. The 
	player also collides with the walls and obstacles by doing a grid node 
	check.
	
08. The GUI includes a meter for the remaining battery life and an alarm sound
	for when the player is seen.
	
09. This is an original game that is different from any previous assignment.

10. I attempt to balance the game using the number of guards, the number of
	obstacles to block their line of sight, and the speed of the player and 
	guards.
	
11. The code is efficient and properly designed for easy of readability.

12. Included are screen shots and a video clip of the game.

13. This README file provides an explanation of the game play and requirements.
