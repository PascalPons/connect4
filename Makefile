CXX=g++
CXXFLAGS=--std=c++11 -W -Wall -O3

SRCS=solver.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

c4solver:$(OBJS)
	$(CXX) $(LDFLAGS) -o c4solver $(OBJS) $(LOADLIBES) $(LDLIBS)

.depend: $(SRCS)
	$(CXX) $(CXXFLAGS) -MM $^ > ./.depend
	
include .depend

clean:
	rm -f *.o .depend c4solver


