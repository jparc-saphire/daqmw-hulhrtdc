CXX = g++
CXXFLAGS += -O3 -Wall -g -MMD -fPIC -std=c++14 $(shell root-config --cflags)
CXXFLAGS += -I/usr/include/daqmw
LDFLAGS += -shared
LIBS = -L/usr/lib64/daqmw -lSock

LIBDIR = ../lib

BINDIR = ../bin

all: libBitDump.so libUDPRBCP.so libFPGAModule.so libdaq_funcs.so $(BINDIR)/debug

# RPATH must point at ../lib (libs live next to bin/, not inside bin/)
$(BINDIR)/debug: debug_main.o | $(LIBDIR) $(BINDIR)
	$(CXX) -o $@ debug_main.o -L$(LIBDIR) -ldaq_funcs -lFPGAModule -lUDPRBCP -lBitDump -Wl,-rpath,'$$ORIGIN/../lib'

$(BINDIR):
	mkdir -p $(BINDIR)

libBitDump.so: BitDump.o | $(LIBDIR)
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $(LIBDIR)/$@

libUDPRBCP.so: UDPRBCP.o | $(LIBDIR)
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $(LIBDIR)/$@

libFPGAModule.so: FPGAModule.o | $(LIBDIR)
	$(CXX) $(LDFLAGS) $^ -L$(LIBDIR) -lUDPRBCP -Wl,-rpath,'$$ORIGIN' -o $(LIBDIR)/$@

libdaq_funcs.so: daq_funcs.o BctBusBridgeFunc.o | $(LIBDIR)
	$(CXX) $(LDFLAGS) $^ -L$(LIBDIR) -lFPGAModule -Wl,-rpath,'$$ORIGIN' -o $(LIBDIR)/$@

$(LIBDIR):
	mkdir -p $(LIBDIR)

clean:
	rm -rf $(LIBDIR)/* *.o *~ *.so *.d

-include *.d
