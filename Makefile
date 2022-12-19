SRCDIR			= src
OUTDIR			= out
OBJDIR			= $(OUTDIR)/obj
SRCS			= $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/**/*.cpp)
OBJS			= $(subst $(SRCDIR), $(OBJDIR), $(SRCS:.cpp=.o))
TARGET			= $(OUTDIR)/main
CC				= g++
CFLAGS			= -std=c++17 -Wall -O2
CFLAGS_DEBUG	= -std=c++17 -Wall -O0 -g

main: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

debug: $(OBJS)
	$(CC) $(CFLAGS_DEBUG) -o $(TARGET) $^

clean:
	rm -f ./out/main ./out/obj/**/*.o ./out/obj/*.o
