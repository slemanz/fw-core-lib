CC  = gcc
CXX = g++
LD  = g++

CFLAGS   = -c -Wall -Wextra -Wno-unused-parameter -std=c99   -g
CXXFLAGS = -c -Wall -Wextra -Wno-unused-parameter -std=c++11 -g
LDFLAGS  = -L /usr/lib/x86_64-linux-gnu -lCppUTest -lCppUTestExt

DIR_BUILD  = build
DIR_COMMON = ..

INCLUDES += -I .
INCLUDES += -I $(DIR_COMMON)/Inc
INCLUDES += -I /usr/include

# Production code under test
PROD += $(DIR_BUILD)/ring-buffer.o
PROD += $(DIR_BUILD)/slist.o
PROD += $(DIR_BUILD)/pool.o
PROD += $(DIR_BUILD)/simple-timer.o
PROD += $(DIR_BUILD)/pool.o
PROD += $(DIR_BUILD)/led.o
PROD += $(DIR_BUILD)/uprint.o

# Test runner
TESTS += $(DIR_BUILD)/AllTestRunner.o
TESTS += $(DIR_BUILD)/test_ring_buffer.o
TESTS += $(DIR_BUILD)/test_slist.o
TESTS += $(DIR_BUILD)/test_simple_timer.o
TESTS += $(DIR_BUILD)/test_led.o
TESTS += $(DIR_BUILD)/test_pool.o

# Mocks
MOCKS += $(DIR_BUILD)/mock_timebase.o
MOCKS += $(DIR_BUILD)/mock_interface_io.o
MOCKS += $(DIR_BUILD)/mock_comm.o

all: $(DIR_BUILD) test

$(DIR_BUILD):
	mkdir -p $(DIR_BUILD)

# Test runner & test files (C++)
$(DIR_BUILD)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

$(DIR_BUILD)/%.o: unit/shared/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

$(DIR_BUILD)/%.o: unit/core/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

$(DIR_BUILD)/%.o: unit/bsp/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

# Production code (C)
$(DIR_BUILD)/%.o: $(DIR_COMMON)/Src/shared/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(DIR_BUILD)/%.o: $(DIR_COMMON)/Src/core/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(DIR_BUILD)/%.o: $(DIR_COMMON)/Src/bsp/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

# Mocks (C)
$(DIR_BUILD)/%.o: mocks/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

# can use -v for verbose
test: $(PROD) $(MOCKS) $(TESTS)
	$(LD) -o $(DIR_BUILD)/run_tests $^ $(LDFLAGS)
	$(DIR_BUILD)/run_tests 

clean:
	rm -rf $(DIR_BUILD)
