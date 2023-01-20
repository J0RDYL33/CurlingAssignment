/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include"vecmath.h"

/*-----------------------------------------------------------
  Macros
  -----------------------------------------------------------*/
#define TABLE_X			(0.6f) 
#define TABLE_Z			(5.2f)
#define TABLE_Y			(0.1f)
#define BALL_RADIUS		(0.05f)
#define BALL_MASS		(0.1f)
#define TWO_PI			(6.2832f)
#define	SIM_UPDATE_MS	(50)
#define NUM_BALLS		(16)		
#define NUM_CUSHIONS	(4)
#define NUM_TABLES		(5)

/*-----------------------------------------------------------
  plane normals
  -----------------------------------------------------------*/
/*
extern vec2	gPlaneNormal_Left;
extern vec2	gPlaneNormal_Top;
extern vec2	gPlaneNormal_Right;
extern vec2	gPlaneNormal_Bottom;
*/

/*------------------------------------------------------------
Player Class
--------------------------------------------------------------*/
class player
{
	//static int playerIndexCnt;
public:

};


/*-----------------------------------------------------------
  cushion class
  -----------------------------------------------------------*/
class cushion
{
public:
	vec2	vertices[2]; //2d
	vec2	centre;
	vec2	normal;

	void MakeNormal(void);
	void MakeCentre(void);
};

class fakeCushion : public cushion
{

};

/*-----------------------------------------------------------
  ball class
  -----------------------------------------------------------*/

class ball
{
	static int ballIndexCnt;
public:
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	float distanceFromTee;
	bool	team1;
	int		index;

	ball(): position(0.0), velocity(0.0), radius(BALL_RADIUS), 
		mass(BALL_MASS) {index = ballIndexCnt++; Reset();}
	
	void Reset(void);
	void ApplyImpulse(vec2 imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollision(const cushion &c);
	void DoBallCollision(ball &b);
	void Update(int ms);
	void NextUp(void);
	
	bool HasHitPlane(const cushion &c) const;
	bool HasHitBall(const ball &b) const;

	void HitPlane(const cushion &c);
	void HitBall(ball &b);
};

/*-------------------------------------------------------------
Score Board Class
---------------------------------------------------------------*/
class scoreboard
{
	static int scoreboardIndexCnt;
public:
	int tableNumber;
	int gameNumber;
	bool onTeam1;
	int maxTeamSize;
	int playersOn1;
	int playersOn2;
	int team1Score;
	int team2Score;
	int index;

	scoreboard() { index = scoreboardIndexCnt++; Reset(); }
	void AddScore1(int pointsToAdd);
	void AddScore2(int pointsToAdd);
	void IncrimentGame(void);
	void DecideWinner(void);
	void DisplayCurrentInfo(void);
	void Reset(void);
};

/*-----------------------------------------------------------
  table class
  -----------------------------------------------------------*/
class table
{
public:
	ball balls[NUM_BALLS];	
	cushion cushions[NUM_CUSHIONS];
	fakeCushion fakeCushions[NUM_CUSHIONS];
	scoreboard myBoard;
	int pointsToAdd;
	void SortBalls();
	void Reset();
	void SetupCushions(void);
	void SetCushionPosition(int cushionIndex, float vertices00, float vertices01, float vertices10, float vertices11);
	void SetupFakeCushions(void);
	void SetFakeCushionPosition(int cushionIndex, float vertices00, float vertices01, float vertices10, float vertices11);
	void Update(int ms);	
	bool AnyBallsMoving(void) const;
};



/*-----------------------------------------------------------
  global table
  -----------------------------------------------------------*/
//extern table gTable;
extern table gTables[NUM_TABLES];
