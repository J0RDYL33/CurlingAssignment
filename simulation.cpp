/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
//#include <cstdlib>
#include <iostream>
#include<algorithm>
using namespace std;

/*-----------------------------------------------------------
  macros
  -----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

/*-----------------------------------------------------------
  globals
  -----------------------------------------------------------*/
/*
vec2	gPlaneNormal_Left(1.0,0.0);
vec2	gPlaneNormal_Top(0.0,1.0);
vec2	gPlaneNormal_Right(-1.0,0.0);
vec2	gPlaneNormal_Bottom(0.0,-1.0);
*/

//table gTable;
table gTables[NUM_TABLES];

static const float gRackPositionX[] = {0.0f,0.0f,(BALL_RADIUS*2.0f),(-BALL_RADIUS*2.0f),(BALL_RADIUS*4.0f)}; 
static const float gRackPositionZ[] = {0.5f,0.0f,(-BALL_RADIUS*3.0f),(-BALL_RADIUS*3.0f)}; 

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;


/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//can do this in 2d
	vec2 temp = vertices[1]-vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre/=2.0;
}

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int ball::ballIndexCnt = 0;

void ball::Reset(void)
{
	//set velocity to zero
	velocity = 0.0;

	if ((index % NUM_BALLS) % 2 == 0)
		team1 = true;
	else
		team1 = false;

	//work out rack position
	if(index % NUM_BALLS==0)
	{
		position(1) = 4;
		position(0) = 0.0;
		return;
	}
	static const float sep = (BALL_RADIUS * 3.0f);
	static const float rowSep = (BALL_RADIUS*2.5f);
	int rowIndex = index % NUM_BALLS;
	int row = rowIndex / 4;
	/*while (rowIndex > row)
	{
		rowIndex -= row;
		row++;
	}
	if (rowIndex % 4 == 0)
	{
		row++;
	}*/


	position(1) = 5.7 + (row * rowSep);
	position(0) = 0 + (sep * (rowIndex%4));
}

void ball::ApplyImpulse(vec2 imp)
{
	velocity = imp;
}

void ball::ApplyFrictionForce(int ms)
{
	if(velocity.Magnitude()<=0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms)/1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if(velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;

	int tableNo = index / NUM_BALLS;


	distanceFromTee = sqrt((position(0) - (0)) + (position(0) - (0)) + (position(1) - (-4)) * (position(1) - (-4)));
	//cout << "Table " << tableNo << "ball " << index % NUM_BALLS << " Distance from tee: " << distanceFromTee << endl;
}

void ball::DoBallCollision(ball &b)
{
	if(HasHitBall(b)) HitBall(b);
}

void ball::DoPlaneCollision(const cushion &b)
{
	if(HasHitPlane(b)) HitPlane(b);
}

void ball::Update(int ms)
{
	//apply friction
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms)/1000.0f);
	//set small velocities to zero
	if(velocity.Magnitude()<SMALL_VELOCITY) velocity = 0.0;
}

bool ball::HasHitPlane(const cushion &c) const
{
	//if moving away from plane, cannot hit
	if(velocity.Dot(c.normal) >= 0.0) return false;
	
	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);
	if(sep > radius) return false;
	return true;
}

bool ball::HasHitBall(const ball &b) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - b.position;
	float dist = (float) relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if(relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if(dist > (radius+b.radius)) return false;
	return true;
}

void ball::HitPlane(const cushion &c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0+gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta; 

/*
	//assume elastic collision
	//find plane normal
	vec2 planeNorm = gPlaneNormal_Left;
	//split velocity into 2 components:
	//find velocity component perpendicular to plane
	vec2 perp = planeNorm*(velocity.Dot(planeNorm));
	//find velocity component parallel to plane
	vec2 parallel = velocity - perp;
	//reverse perpendicular component
	//parallel component is unchanged
	velocity = parallel + (-perp)*gCoeffRestitution;
*/
}

void ball::HitBall(ball &b)
{
	//find direction from other ball to this ball
	vec2 relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2 parallelV = velocity-(relDir*perpV);
	vec2 parallelV2 = b.velocity-(relDir*perpV2);
	
	//Calculate new perpendicluar components:
	//v1 = (2*m2 / m1+m2)*u2 + ((m1 - m2)/(m1+m2))*u1;
	//v2 = (2*m1 / m1+m2)*u1 + ((m2 - m1)/(m1+m2))*u2;
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV*(mass-b.mass))/sumMass) + (float)((perpV2*(2.0*b.mass))/sumMass);
	float perpVNew2 = (float)((perpV2*(b.mass-mass))/sumMass) + (float)((perpV*(2.0*mass))/sumMass);
	
	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir*perpVNew);
	b.velocity = parallelV2 + (relDir*perpVNew2);
}

void ball::NextUp(void)
{
	position(1) = 4;
	position(0) = 0.0;
}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/

void table::SetupCushions(void)
{
	SetCushionPosition(0, -TABLE_X, -TABLE_Z, -TABLE_X, TABLE_Z);
	SetCushionPosition(1, -TABLE_X, TABLE_Z, TABLE_X, TABLE_Z);
	SetCushionPosition(2, TABLE_X, TABLE_Z, TABLE_X, -TABLE_Z);
	//SetCushionPosition(3, TABLE_X, -TABLE_Z + 0.3, TABLE_X - 0.3, -TABLE_Z);
	SetCushionPosition(3, TABLE_X, -TABLE_Z, -TABLE_X, -TABLE_Z);
}

void table::SetCushionPosition(int cushionIndex, float vertices00, float vertices01, float vertices10, float vertices11)
{
	cushions[cushionIndex].vertices[0](0) = vertices00;
	cushions[cushionIndex].vertices[0](1) = vertices01;
	cushions[cushionIndex].vertices[1](0) = vertices10;
	cushions[cushionIndex].vertices[1](1) = vertices11;

	cushions[cushionIndex].MakeCentre();
	cushions[cushionIndex].MakeNormal();
}

void table::SetupFakeCushions(void)
{
	SetFakeCushionPosition(0, -TABLE_X, -TABLE_Z, -TABLE_X, TABLE_Z);
	SetFakeCushionPosition(1, -TABLE_X, TABLE_Z, TABLE_X, TABLE_Z);
	SetFakeCushionPosition(2, TABLE_X, TABLE_Z, TABLE_X, -TABLE_Z);
	SetFakeCushionPosition(3, TABLE_X, -TABLE_Z, -TABLE_X, -TABLE_Z);
}

void table::SetFakeCushionPosition(int cushionIndex, float vertices00, float vertices01, float vertices10, float vertices11)
{
	fakeCushions[cushionIndex].vertices[0](0) = vertices00;
	fakeCushions[cushionIndex].vertices[0](1) = vertices01;
	fakeCushions[cushionIndex].vertices[1](0) = vertices10;
	fakeCushions[cushionIndex].vertices[1](1) = vertices11;

	fakeCushions[cushionIndex].MakeCentre();
	fakeCushions[cushionIndex].MakeNormal();
}

void table::Update(int ms)
{
	//check for collisions for each ball
	for(int i=0;i<NUM_BALLS;i++) 
	{
		for(int j=0;j<NUM_CUSHIONS;j++)
		{
			balls[i].DoPlaneCollision(cushions[j]);
		}

		for(int j=(i+1);j<NUM_BALLS;j++) 
		{
			balls[i].DoBallCollision(balls[j]);
		}
	}
	
	//update all balls
	for(int i=0;i<NUM_BALLS;i++) balls[i].Update(ms);
}

bool table::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for(int i=0;i<NUM_BALLS;i++) 
	{
		if(balls[i].velocity(0)!=0.0) return true;
		if(balls[i].velocity(1)!=0.0) return true;
	}
	return false;
}

void table::SortBalls()
{
	int ballsOrder[NUM_BALLS];

	for (int i = 0; i < NUM_BALLS; i++)
	{
		ballsOrder[i] = i;
	}

	//Sort the balls in the order of which they are from the tee
	std::sort(std::begin(ballsOrder), std::end(ballsOrder), [&](int a, int b) { return balls[a].distanceFromTee < balls[b].distanceFromTee; });

	bool othersNotFound = false;
	int posInOrder = 1;
	//If the ball in first position of the order is of team 1, then they'll be getting the points
	if (balls[ballsOrder[0]].team1)
	{
		pointsToAdd++;
		while (othersNotFound == false)
		{
			if (balls[ballsOrder[posInOrder]].team1)
			{
				pointsToAdd++;
				posInOrder++;
			}
			else
			{
				othersNotFound = true;
			}
		}

		myBoard.AddScore1(pointsToAdd);
	}
	else if (!balls[ballsOrder[0]].team1)
	{
		pointsToAdd++;
		while (othersNotFound == false)
		{
			if (!balls[ballsOrder[posInOrder]].team1)
			{
				pointsToAdd++;
				posInOrder++;
			}
			else
			{
				othersNotFound = true;
			}
		}

		myBoard.AddScore2(pointsToAdd);
	}

	Reset();
}

void table::Reset()
{
	//Reset the position of the stones
	for (int i = 0; i < NUM_BALLS; i++)
		balls[i].Reset();

	myBoard.IncrimentGame();
}

/*-------------------------------------------------------
Scoreboard Class Members
---------------------------------------------------------*/
int scoreboard::scoreboardIndexCnt = 0;

void scoreboard::AddScore1(int pointsToAdd)
{
	team1Score += pointsToAdd;
}

void scoreboard::AddScore2(int pointsToAdd)
{
	team2Score += pointsToAdd;
}

void scoreboard::IncrimentGame(void)
{
	gameNumber++;

	if (gameNumber > 7)
	{
		DecideWinner();
		return;
	}

	DisplayCurrentInfo();
}

void scoreboard::DecideWinner(void)
{
	if (team1Score > team2Score)
	{
		//Team 1 wins
	}
	else if (team1Score < team2Score)
	{
		//Team 2 wins
	}
	else
	{
		//It's a draw
	}
}

void scoreboard::Reset(void)
{
	gameNumber = 0;
	team1Score = 0;
	team2Score = 0;

	DisplayCurrentInfo();
}

void scoreboard::DisplayCurrentInfo(void)
{
	cout << "-------------------------" << endl;
	cout << "Table " << index << "		Game " << gameNumber << endl;
	cout << "Team 1 Players:		" << playersOn1 << endl;
	cout << "Team 2 Players:		" << playersOn2 << endl;
	cout << "Team 1 Total Score:	" << team1Score << endl;
	cout << "Team 2 Total Score:	" << team2Score << endl;
	cout << "-------------------------" << endl;
}