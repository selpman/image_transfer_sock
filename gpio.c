#include "gpio.h"

static char numLine[3];

void gpio_init(void)
{
	wiringPiSetupGpio();

	pinMode(SW1,INPUT);
	pinMode(SW2,INPUT);
	pinMode(SW3,INPUT);	

	pinMode(RLED,OUTPUT);
	pinMode(GLED,OUTPUT);
	digitalWrite(RLED,LOW);
	digitalWrite(GLED,LOW);
}

void get_button(void)
{
	int cnt=0;

	while(cnt != 3)
	{
		if(digitalRead(SW1))
		{
			numLine[cnt++] = '1';
			sleep(1);
		}
		else if(digitalRead(SW2))
		{
			numLine[cnt++] = '2';
			sleep(1);
		}
		else if(digitalRead(SW3))
		{
			numLine[cnt++] = '3';
			sleep(1);
		}
	}
}

int cmp_button_passwd(char* passwd)
{
	for(int cnt=0;cnt<3;++cnt)
		printf("numLine[%d] : %d \n",cnt,numLine[cnt]);

	if(!strcmp(numLine,passwd))
	{
		digitalWrite(GLED,HIGH);
		digitalWrite(RLED,LOW);
		return 1;
	}
	else
	{
		digitalWrite(GLED,LOW);
		digitalWrite(RLED,HIGH);
		return 0;
	}
}
