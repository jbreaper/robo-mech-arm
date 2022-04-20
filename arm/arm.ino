#include <cppQueue.h>
#include <ESP32Servo.h>
#include <Ticker.h>
#include <ServoEasing.hpp>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>

#define J1 10
#define J2 11
#define J3 12
#define J4 13
#define J5 14

const int joint_out[5] = {J1, J2, J3, J4, J5};
int ini_angle[5] = {45, 45, 45, 45, 45};

#define S1 15

#define ESO 32
#define ESI 31

ServoEasing joint1;
ServoEasing joint2;
ServoEasing joint3;
ServoEasing joint4;
ServoEasing joint5;

int angle[5];
int coord[3];

int pos = 0;

bool smooth = true;
bool grab = false;
bool grabbed = false;

volatile bool emergancy_stop = false;

cppQueue cmds(sizeof(String), 7, FIFO, false);

void setup()
{
	pinMode(ESO, OUTPUT);
	digitalWrite(ESO, HIGH);
	pinMode(ESI, INPUT);
	attachInterrupt(ESI, e_stop, HIGH);

	pinMode(S1, OUTPUT);

	Serial.begin(9600);
	for (int i = 0; i < 6; i++)
	{
		ServoEasing::ServoEasingArray[i]->attach(joint_out[i], ini_angle[i]);
		ServoEasing::ServoEasingArray[i]->setEasingType(EASE_QUADRATIC_IN_OUT);
	}

	while (!Serial)
	{
	}
}

// interupt function used to stop arm in case of emergancy
void e_stop()
{
	digitalWrite(ESO, LOW);
	emergancy_stop = true;

	cmds.flush();
}

// bool move_to(float x, float y, float z, float theta){
//
// }

bool joint_to(int theta[5], int rate = 90)
{
	for (int i = 0; i < 6; i++)
	{
		ServoEasing::ServoEasingArray[i]->easeTo(theta[i], rate);
	}
}

void joint_reset()
{
	int zeroes[5] = {
		0,
		0,
		0,
		0,
		0,
	};

	joint_to(ini_angle, 180);
}

void input()
{
	if (Serial.available() > 0)
	{
		char *x;
		int availableBytes = Serial.available();
		char in[availableBytes];

		for (int i = 0; i < availableBytes; i++)
		{
			in[i] = Serial.read();
		}

		x = strtok(in, " ");

		while (x != NULL)
		{
			cmds.push(x);
			x = strtok(NULL, " ");
		}
	}
}

void interperate()
{
	char type[5];

	while (!(cmds.isEmpty()))
	{
		cmds.pop(&type);

		if (((type[0] == 'e' || type[0] == 'E') && (type[1] == 's' || type[1] == 'S')) && emergancy_stop == false)
		{
			e_stop();
			break;
		}
		else if (((type[0] == 'e' || type[0] == 'E') && (type[1] == 's' || type[1] == 'S')) && emergancy_stop == true)
		{
			digitalWrite(ESO, HIGH);
			emergancy_stop = false;
			break;
		}

		if (emergancy_stop == false)
		{
			switch (type[0])
			{
			case 'g':
			case 'G':
				switch (type[1])
				{
				case '0':
					joint_reset();
					break;
				case '1':
					smooth = true;
					for (int i = 0; i < 6; i++)
						ServoEasing::ServoEasingArray[i]->setEasingType(EASE_QUADRATIC_IN_OUT);
					break;
				case '2':
					smooth = false;
					for (int i = 0; i < 6; i++)
						ServoEasing::ServoEasingArray[i]->setEasingType(EASE_LINEAR);
					break;
				case '3':
					gripper(grab);
					grabbed != grab;
					break;
				}
				break;
			case 'x':
			case 'X':
				if (type[0] == 'x')
				{
					sscanf(type, "x%u", &coord[0]);
				}
				else
				{
					sscanf(type, "X%u", &coord[0]);
				}
				break;
			case 'y':
			case 'Y':
				if (type[0] == 'y')
				{
					sscanf(type, "y%u", &coord[1]);
				}
				else
				{
					sscanf(type, "Y%u", &coord[1]);
				}
				break;
			case 'z':
			case 'Z':
				if (type[0] == 'z')
				{
					sscanf(type, "z%u", &coord[2]);
				}
				else
				{
					sscanf(type, "Z%u", &coord[2]);
				}
				break;
			case 'a':
			case 'A':
				if (type[0] == 'a')
				{
					sscanf(type, "A%u", &angle[0]);
				}
				else
				{
					sscanf(type, "A%u", &angle[0]);
				}
				break;
			case 'b':
			case 'B':
				if (type[0] == 'b')
				{
					sscanf(type, "b%u", &angle[1]);
				}
				else
				{
					sscanf(type, "B%u", &angle[1]);
				}
				break;
			case 'c':
			case 'C':
				if (type[0] == 'c')
				{
					sscanf(type, "c%u", &angle[2]);
				}
				else
				{
					sscanf(type, "C%u", &angle[2]);
				}
				break;
			case 'd':
			case 'D':
				if (type[0] == 'd')
				{
					sscanf(type, "d%u", &angle[3]);
				}
				else
				{
					sscanf(type, "D%u", &angle[3]);
				}
				break;
			case 'e':
			case 'E':
				if (type[0] == 'e')
				{
					sscanf(type, "e%u", &angle[4]);
				}
				else
				{
					sscanf(type, "E%u", &angle[4]);
				}
				break;
			}
		}
		else
		{
			joint_reset();
			Serial.println("!!EMERCANCY STOPPED!!");
			delay(1000);
		}
	}
}

bool gripper(bool g)
{
	if (g)
	{
		digitalWrite(S1, HIGH);
	}
	else
	{
		digitalWrite(S1, LOW);
	}

	return (g);
}

void reply(bool out_msg)
{
	if (true)
	{
		Serial.print("Move: successful");
	}
	else
	{
		Serial.print("Move: failed");
	}
}

void loop()
{
	if (Serial.available() > 0)
	{
		input();
	}

	interperate();

	if (emergancy_stop == false)
	{
		joint_to(angle);

		if (grabbed != grab)
		{
			grab = gripper(grabbed);
		}

		if (cmds.isEmpty())
		{
			reply(true);
		}
	}
}