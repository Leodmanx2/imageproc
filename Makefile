################################################################################
# Progress indicator by Giovanni Funchal with modifications
# by phyatt and myself (https://stackoverflow.com/q/451413)
################################################################################

ifneq ($(words $(MAKECMDGOALS)),1)
.DEFAULT_GOAL = all
%:
	@$(MAKE) $@ --no-print-directory -rRf $(firstword $(MAKEFILE_LIST))
else
ifndef ECHO
T := $(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory \
	-nrRf $(firstword $(MAKEFILE_LIST)) \
	ECHO="COUNTTHIS" | grep -c "COUNTTHIS")

N := x
C = $(words $N)$(eval N := x $N)
ECHO = echo "[`expr $C '*' 100 / $T`%]"
endif


################################################################################
# Configuration
################################################################################

PROJECT_NAME = resize

SRCDIR = $(CURDIR)/src
OBJDIR = $(CURDIR)/obj
ASSDIR = $(CURDIR)/ass
BINDIR = $(CURDIR)/bin

OBJ = $(addprefix $(OBJDIR)/, Application.o Image.o bilinear.o IMDDT.o AIS_cubic.o)
TEST_OBJ = $(addprefix $(OBJDIR)/, AIS_cubic.o IMDDT.o TestRunner.o Image.o)

CXXFLAGS_WARNINGS = -pedantic -Wall -Wextra -Wcast-align -Wcast-qual \
                    -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 \
                    -Winit-self -Wmissing-declarations -Wmissing-include-dirs \
                    -Wold-style-cast -Woverloaded-virtual -Wredundant-decls \
                    -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef \
                    -Weffc++ -Winline -Wswitch-enum

ifeq ($(CXX), clang++)
	CXXFLAGS_WARNINGS += -Wdeprecated -Wdocumentation -Werror=documentation
endif

CXXFLAGS += -std=c++17 $(CXXFLAGS_CLANG) $(CXXFLAGS_WARNINGS) -c -D gsl_CONFIG_CONTRACT_VIOLATION_THROWS

# Certain library names and flags depend on the OS
ifeq ($(OS), Windows_NT)
	#LDFLAGS += -mwindows
	EXE_NAME = $(PROJECT_NAME).exe
	TEST_EXE_NAME = TestSuite.exe
else
	EXE_NAME = $(PROJECT_NAME)
	TEST_EXE_NAME = TestSuite
endif
LDLIBS += -lopenimageio
TEST_LDLIBS += -lopenimageio


################################################################################
# Targets
################################################################################

all: CXXFLAGS += -O2 -march=native
all: LDFLAGS += -s
all: executable

debug: CXXFLAGS += -D DEBUG -g
debug: executable

executable: $(OBJ)
	@$(ECHO) Linking $(EXE_NAME)
	@$(CXX) $(LDFLAGS) -o $(BINDIR)/$(EXE_NAME) $(OBJ) $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(ECHO) Compiling $<
	@$(CXX) $(CXXFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	@$(ECHO) Compiling $<
	@$(CXX) $(CXXFLAGS) -o $@ $<

$(OBJ): | $(OBJDIR)
$(TEST_OBJ): | $(OBJDIR)

$(OBJDIR):
	@$(ECHO) Making object code directory
	@mkdir $(OBJDIR)

.PHONY: clean
clean:
	@$(ECHO) Removing object and binary files
	@rm -rf $(OBJDIR) $(BINDIR)/$(EXE_NAME) $(BINDIR)/$(TEST_EXE_NAME)

.PHONY: test
test: $(TEST_OBJ)
	@$(ECHO) Linking $(TEST_EXE_NAME)
	@$(CXX) $(LDFLAGS) -o $(BINDIR)/$(TEST_EXE_NAME) $(TEST_OBJ) $(TEST_LDLIBS)
	$(BINDIR)/TestSuite.exe


# Progress indicator end
endif
