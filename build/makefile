VPATH = ../src

CF = -fmax-errors=2

OBJS = Socket.o Consold.o ConnectionListener.o SdrIQ.o Server.o Dsp.o

radio:	$(OBJS)
	c++ $(CF) -g -o radio $(OBJS) -lfftw3

radiodata: radiodata.o
	c++ $(CF) -o radiodata radiodata.o 

%.o:%.cpp
	c++ $(CF) -g -I../src -c $<

clean:
	@rm -f *.o radio radiodata
