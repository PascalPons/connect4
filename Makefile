CXX=g++
CXXFLAGS=--std=c++11 -W -Wall -O3 -DNDEBUG

SRCS=Solver.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

c4solver:$(OBJS) main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o c4solver main.o $(OBJS) $(LDLIBS)

generator: generator.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o generator generator.o $(LDLIBS)

.depend: $(SRCS)
	$(CXX) $(CXXFLAGS) -MM $^ > ./.depend
	
-include .depend

clean:
	rm -f *.o .depend c4solver generator


