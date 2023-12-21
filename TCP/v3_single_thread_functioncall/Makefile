CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Add any additional libraries or flags here if needed
LIBS = -lws2_32

SRCS_B = machineb_window.cpp  # Replace with your actual file name
SRCS_C = machinec.cpp mars.cpp  # Add mars.cpp here

OBJS_B = $(SRCS_B:.cpp=.o)
OBJS_C = $(SRCS_C:.cpp=.o)

EXEC_B = machineb.exe  # Replace with your desired executable name
EXEC_C = machinec.exe  # Replace with your desired executable name

all: $(EXEC_B) $(EXEC_C)

$(EXEC_B): $(OBJS_B)
	$(CC) $(CFLAGS) $(OBJS_B) -o $(EXEC_B) $(LIBS)

$(EXEC_C): $(OBJS_C)
	$(CC) $(CFLAGS) $(OBJS_C) -o $(EXEC_C) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del $(OBJS_B) $(OBJS_C) $(EXEC_B) $(EXEC_C)
