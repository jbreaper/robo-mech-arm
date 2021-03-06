#include <cppQueue.h>
#include <Ticker.h>
#include <ESP32Servo.h>
#include <ServoEasing.hpp>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>

#define J1 32
#define J2 33
#define J3 25
#define J4 26
#define J5 27
#define S1 12

#define J1_Rate 90 // deg/s
#define J1_CW 77
#define J1_CCW 109.5

#define J5_Rate 180 // deg/s
#define J5_CW 76.9
#define J5_CCW 109

#define G_OPEN 180
#define G_CLOSE 90

int joint_out[5] = {J1, J2, J3, J4, J5};
int ini_angle[5] = {90, 90, 45, 45, 90};

int j1_cur = 90;
int j5_cur = 90;

#define ESO 15
#define ESI 16

Servo joint1;
ServoEasing joint2;
ServoEasing joint3;
ServoEasing joint4;
Servo joint5;

Servo gripper;

int angle[5] = {90, 90, 45, 45, 90};
int coord[3];

int pos = 0;

bool smooth = true;
bool grab = false;
bool grabbed = false;
bool first = true;

volatile bool emergancy_stop = false;

cppQueue cmds(sizeof(String), 7, FIFO, false);

void setup()
{
	delay(1000);
	Serial.begin(115200);

	Serial.println("booting: begin");
	pinMode(ESO, OUTPUT);
	digitalWrite(ESO, HIGH);
	pinMode(ESI, INPUT);
	attachInterrupt(ESI, e_stop, HIGH);

	gripper.attach(S1);
	//  gripper.setEasingType(EASE_QUADRATIC_IN_OUT);

	Serial.println("booting: pins assigned");
	for (int i = 0; i < 5; i++)
	{
		switch (i)
		{
		case 0:
			joint1.attach(joint_out[i]);
			break;
		case 1:
			joint2.attach(joint_out[i]);
			joint2.setEasingType(EASE_QUADRATIC_IN_OUT);
			break;
		case 2:
			joint3.attach(joint_out[i]);
			joint3.setEasingType(EASE_QUADRATIC_IN_OUT);
			break;
		case 3:
			joint4.attach(joint_out[i]);
			joint4.setEasingType(EASE_QUADRATIC_IN_OUT);
			break;
		case 4:
			joint5.attach(joint_out[i]);
			break;
		default:
			break;
		}
		// ServoEasing::ServoEasingArray[i]->attach(joint_out[i], ini_angle[i]);
		// ServoEasing::ServoEasingArray[i]->setEasingType(EASE_QUADRATIC_IN_OUT);
	}

	Serial.println("booting: servos assigned");

	while (!Serial)
		;

	joint2.write(ini_angle[1] - 15);
	joint3.write(ini_angle[2] - 15);
	joint4.write(ini_angle[3] + 15);

	Serial.println("booting: complete");
	delay(1000);
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

void j1_move(float angle)
{
	float x = 0;

	if ((j1_cur - angle) > 0)
	{
		x = (j1_cur - angle);
		joint1.write(J1_CW);
	}
	else if ((j1_cur - angle) < 0)
	{
		x = (angle - j1_cur);
		joint1.write(J1_CCW);
	}
	else if (j1_cur == angle)
	{
		return;
	}

	delay(1000 * (x / J1_Rate));
	joint1.write(90);
	j1_cur = angle;
}

void j5_move(float angle)
{
	float x = 0;

	if ((j5_cur - angle) > 0)
	{
		x = (j5_cur - angle);
		joint5.write(J5_CW);
	}
	else if ((j5_cur - angle) < 0)
	{
		x = (angle - j5_cur);
		joint5.write(J5_CCW);
	}
	else if (j5_cur == angle)
	{
		return;
	}

	delay(1000 * (x / J5_Rate));
	joint5.write(90);
	j5_cur = angle;
}

bool joint_to(int theta[5], int rate = 90)
{
	for (int i = 0; i < 5; i++)
	{
		switch (i)
		{
		case 0:
			j1_move(theta[i]);
			break;
		case 1:
			joint2.easeTo((theta[i] - 15), rate);
			// ServoEasing::ServoEasingArray[i]->easeTo((theta[i] - 15), rate);
			break;
		case 2:
			joint3.easeTo((theta[i] - 15), rate);
			// ServoEasing::ServoEasingArray[i]->easeTo((theta[i] - 15), rate);
			break;
		case 3:
			joint4.easeTo((theta[i] + 15), rate);
			// ServoEasing::ServoEasingArray[i]->easeTo((theta[i] + 15), rate);
			break;
		case 4:
			j5_move(theta[i]);
			break;
		}
	}
}

void joint_reset()
{
	for (int i = 0; i < 5; i++)
	{
		angle[i] = ini_angle[i];
	}
	joint_to(angle, 20);
}

void input()
{
	if (Serial.available() > 0)
	{
		char *x;
		int availableBytes = Serial.available();
		char in[(availableBytes + 1)];

		for (int i = 0; i < availableBytes; i++)
		{
			in[i] = Serial.read();
		}

		x = strtok(in, " ");

		while (x != NULL)
		{
			cmds.push(x);
			Serial.print("cmd: ");
			Serial.println(x);
			x = strtok(NULL, " ");
		}
	}
}

void interpreter()
{
	char type[5];

	while (!(cmds.isEmpty()))
	{
		memset(&type, 0, sizeof(type));
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
					grabbed != grabbed;
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
					sscanf(type, "a%u", &angle[0]);
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

bool grip(bool g)
{
	if (g)
	{
		gripper.write(G_CLOSE);
	}
	else
	{
		gripper.write(G_OPEN);
	}

	return (!g);
}

void reply(bool out_msg)
{
	if (true)
	{
		Serial.println("Move: successful");
	}
	else
	{
		Serial.println("Move: failed");
	}
}

void loop()
{
	while (cmds.isEmpty())
	{
		if (Serial.available() > 0)
		{
			input();
		}
	}

	interpreter();

	if (emergancy_stop == false)
	{
		joint_to(angle);

		if (grabbed != grab)
		{
			grab = grip(grabbed);
		}

		if (cmds.isEmpty())
		{
			reply(true);
		}
	}
}
