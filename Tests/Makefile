CC = gcc
CXX = g++
LD = g++

CPPUTEST_HOME ?= /usr
CPPUTEST_INCLUDE ?= /usr/include

CFLAGS   = -c -Wall -Wextra -std=c99   -g 
CXXFLAGS = -c -Wall -Wextra -std=c++11 -g 
LDFLAGS  =  -L $(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt

DIR_BUILD 	= build
DIR_ROOT 	= ..
DIR_COMMON 	= $(DIR_ROOT)/common

INCLUDES += -I .
INCLUDES += -I $(DIR_COMMON)/Inc
INCLUDES += -I $(CPPUTEST_INCLUDE)

PROD 	+= $(DIR_BUILD)/ring-buffer.o
PROD 	+= $(DIR_BUILD)/simple-timer.o
PROD 	+= $(DIR_BUILD)/pool.o
PROD 	+= $(DIR_BUILD)/led.o
PROD 	+= $(DIR_BUILD)/uprint.o

TESTS 	+= $(DIR_BUILD)/AllTestRunner.o
TESTS 	+= $(DIR_BUILD)/test_ring_buffer.o
TESTS 	+= $(DIR_BUILD)/test_simple_timer.o
TESTS 	+= $(DIR_BUILD)/test_led.o

MOCKS 	+= $(DIR_BUILD)/mock_timebase.o
MOCKS 	+= $(DIR_BUILD)/mock_interface_io.o

all: $(DIR_BUILD) test

$(DIR_BUILD):
	mkdir -p $(DIR_BUILD)

$(DIR_BUILD)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o $(*).cpp

$(DIR_BUILD)/%.o: $(DIR_COMMON)/Src/shared/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o $(DIR_COMMON)/Src/shared/$(*).c

$(DIR_BUILD)/%.o: $(DIR_COMMON)/Src/core/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o $(DIR_COMMON)/Src/core/$(*).c

$(DIR_BUILD)/%.o: $(DIR_COMMON)/Src/bsp/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o $(DIR_COMMON)/Src/bsp/$(*).c

$(DIR_BUILD)/%.o: $(DIR_COMMON)/Src/shared/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o $(DIR_COMMON)/Src/shared/$(*).c

$(DIR_BUILD)/%.o: mocks/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o mocks/$(*).c

$(DIR_BUILD)/%.o: unit/shared/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o unit/shared/$(*).cpp

$(DIR_BUILD)/%.o: unit/core/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o unit/core/$(*).cpp

$(DIR_BUILD)/%.o: unit/bsp/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(DIR_BUILD)/$(*).o unit/bsp/$(*).cpp

test: $(PROD) $(MOCKS) $(TESTS) 
	$(LD) -o $(DIR_BUILD)/run_tests $^ $(LDFLAGS) 
	$(DIR_BUILD)/run_tests -v


clean:
	rm -rf $(DIR_BUILD)/*