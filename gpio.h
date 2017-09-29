#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define SW1 12
#define SW2 16
#define SW3 20

#define RLED 26
#define GLED 5

unsigned int sleep();

void gpio_init(void);
void get_button(void);
int cmp_button_passwd(char* passwd);

