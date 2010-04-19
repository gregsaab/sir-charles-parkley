#pragma config(Sensor, S1,     sonarSensor,         sensorSONAR)
#pragma config(Sensor, S4,     lightSensor,         sensorLightActive)
#pragma config(Motor,  motorA,          driveMotor,    tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  motorB,          steerMotor,    tmotorNormal, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/**********************************************************************************
* Tweakable macros
**********************************************************************************/
// light sensor
#define LIGHT_VALUE_CARPET 68
#define LIGHT_VALUE_TAPE 48

// sonar sensor
#define SONAR_THRESHOLD 200

// steer motor
#define STEER_VALUE_LEFTMOST 0
#define STEER_VALUE_RIGHTMOST 90
#define STEER_SPEED 50

// drive motor
#define DRIVE_SPEED 50


/**********************************************************************************
* Global variables
**********************************************************************************/
// PID
float Kp;     //the Konstant for the proportional controller
float Tp;     //the Target steering angle (straight)
float offset; //average of the tape and carpet readings

// Light sensor
float error = 0.0;

// Steer motor
int steeringAngle = (STEER_VALUE_LEFTMOST + STEER_VALUE_RIGHTMOST) / 2; //this requires initial steering angle to be straigh ahead

// Park
bool isParking = false;


/**********************************************************************************
* Function prototypes
**********************************************************************************/
int GetPID();
void Park();
void SetSteeringAngle(int targetAngle);


/**********************************************************************************
* Light sensor thread
**********************************************************************************/
task tLightSensor()
{
	nSchedulePriority = kDefaultTaskPriority;
	SensorMode[lightSensor] = modePercentage;
	int LightValue;

	while(!isParking)
	{
	  // delay 3 milliseconds since that's about the fastest the light sensor can read
		wait1Msec(3);

		// take a sensor reading and calculate the error by subtracting the offset
		LightValue = SensorValue(lightSensor);
		nxtDisplayCenteredBigTextLine(1, "S4=%d", LightValue);
		error = LightValue - offset;
	}
	return;
}

/**********************************************************************************
* Sonar sensor thread
**********************************************************************************/
task tSonarSensor()
{
	nSchedulePriority = kDefaultTaskPriority;
	int SonarValue;

	while(!isParking)
	{
	  // delay 15 milliseconds since that's about the fastest the sonar sensor can read
		wait1Msec(15);

		// take a sensor reading
		SonarValue = SensorValue(lightSensor);

		// TODO: if(adequate parking space is found), then set isParking flag to true
	}

	return;
}

/**********************************************************************************
* Main thread
**********************************************************************************/
task main()
{
  int controllerOutput;

	nSchedulePriority = kDefaultTaskPriority;
	bFloatDuringInactiveMotorPWM = false;
	eraseDisplay();

	// initialize values of PID globals
	offset = (LIGHT_VALUE_CARPET + LIGHT_VALUE_TAPE) / 2;
	Tp = (STEER_VALUE_LEFTMOST + STEER_VALUE_RIGHTMOST) / 2;
	Kp = (STEER_VALUE_LEFTMOST - STEER_VALUE_RIGHTMOST) / (LIGHT_VALUE_CARPET - LIGHT_VALUE_TAPE);

	// start sonar sensor
	StartTask(tSonarSensor);
	wait1Msec(1000);

	// start light sensor
	StartTask(tLightSensor);
	wait1Msec(1000);

	// perform line-following
	while(!isParking)
	{
	  controllerOutput = Tp + GetPID();
	  SetSteeringAngle(controllerOutput);
	}

	// perform parallel parking
	Park();

}

/**********************************************************************************
* Function: float GetPID()
* Parameters: None
* Return: None
* Description: This method returns the PID output (a steering angle adjustment)
**********************************************************************************/
float GetPID()
{
  float pTerm = 0.0, dTerm = 0.0, iTerm = 0.0;

  // [p]roportional term calculations
  pTerm = Kp * error;

  // [i]ntegral term calculations

  // [d]erivative term calculations

	return (pTerm + iTerm - dTerm);
}

/**********************************************************************************
* Function: void Park()
* Parameters: None
* Return: None
* Description: This function performs parallel parking.
**********************************************************************************/
void Park()
{
	return;
}

/**********************************************************************************
* Function: void SetSteeringAngle()
* Parameters: the target angle, between 0 (left-most) and 90 (right-most)
* Return: None
* Description: This function sets the angle of steering.
**********************************************************************************/
void SetSteeringAngle(int targetAngle)
{
  int targetMotorEncoderValue;
  int direction;
  nxtDisplayCenteredBigTextLine(5, "target=%d", targetAngle);

  // error-handling
  if(targetAngle < STEER_VALUE_LEFTMOST) targetAngle = STEER_VALUE_LEFTMOST;
  if(targetAngle > STEER_VALUE_RIGHTMOST) targetAngle = STEER_VALUE_RIGHTMOST;

  // base case
  if(targetAngle == steeringAngle) return;

  if(targetAngle < steeringAngle)
  {
    // left turn case
    direction = -1;
    targetMotorEncoderValue = steeringAngle - targetAngle;
  }
  else
  {
    // right turn case
    direction = 1;
    targetMotorEncoderValue = targetAngle - steeringAngle;
  }

	// reset motor encoder value and set motor speeds
	nMotorEncoder[steerMotor] = 0;
	motor[steerMotor] = STEER_SPEED * direction;

	// wait for desired motor encoder value
	while( abs(nMotorEncoder[steerMotor]) <= targetMotorEncoderValue );

	// stop motor and return
	steeringAngle = targetAngle;
	motor[steerMotor] = 0;
	return;
}
