compile :
TARGET = serv, clnt
OBJS = ../capture.c ../gpio.c ../sock.c
LIB = -lwringPi -lpthread

gcc -o $(TARGET) $(OBJS) $(LIB)
