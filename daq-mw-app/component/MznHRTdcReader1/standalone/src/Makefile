CXX = g++
CXXFLAGS	+= -O3 -Wall -g -MMD -fPIC $(shell root-config --cflags)
CXXFLAGS  += -I/usr/include/daqmw
LDFLAGS += -shared
LIBS = -L/usr/lib64/daqmw -lSock

LIBDIR = ../lib
SRCALL		= $(wildcard *.cc)
OBJECTS = $(addprefix $(LIBDIR)/, $(SRCALL:.cpp=.o))

all: libBitDump.so\
   libdaq_funcs.so\
   libUDPRBCP.so\
   libFPGAModule.so\
	 libmif_func.so
	 

libBitDump.so: BitDump.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $(LIBDIR)/$@

libdaq_funcs.so: daq_funcs.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $(LIBDIR)/$@

libUDPRBCP.so: UDPRBCP.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $(LIBDIR)/$@

libFPGAModule.so: FPGAModule.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $(LIBDIR)/$@

libmif_func.so: mif_func.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $(LIBDIR)/$@


#%.o: %.cc
	#$(CXX) $(CXXFLAGS)  -c -MMD $< -o $(LIBDIR)/$@
	
clean:
	rm -rf $(LIBDIR)/* *.o *~ *.so *.d
