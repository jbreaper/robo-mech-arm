#include <cppQueue.h>
#include <ServoEasing.h>
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>

#define J1 10
#define J2 11
#define J3 12
#define J4 13
#define J5 14

const int joint_out[5] = {J1, J2, J3, J4, J5};

#define S1 15

#define ESO 32
#define ESI 31

Servo joint[5];

float angle[5];

int pos = 0;

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
    joint[i].attach(joint_out[i]);
}

// interupt function used to stop arm in case of emergancy
void e_stop()
{
  digitalWrite(ESO, LOW);
  emergancy_stop = true;
}

// bool move_to(float x, float y, float z, float theta){
//
// }

bool joint_to(float theta[5])
{
  for (int i = 0; i < (sizeof theta / sizeof *theta); i++)
  {
    joint[i].write(theta[i]);
  }
}

void input()
{
  if (Serial1.available() > 0)
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

void interperate(String msg)
{
  char type;
  // char
}

void selinoid(bool grab)
{
  if (grab)
  {
    digitalWrite(S1, HIGH);
  }
  else
  {
    digitalWrite(S1, LOW);
  }
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
}
