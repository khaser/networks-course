CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++20 -Iinclude

EXE = server
SRCDIR = src
OBJDIR = obj

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))

build $(EXE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXE) $(LDFLAGS)

run:
	./$(EXE)

# test:
# 	@$(foreach test, $(TESTCASES), g++ $(CXXTESTFLAGS) $(test) -Iinclude -o $(TESTEXE) && ./$(TESTEXE) && echo OK;)
# 	@$(foreach test, $(TESTCASES), g++ $(CXXTESTFLAGS) $(test) -Iinclude -o $(TESTEXE) && valgrind -q ./$(TESTEXE) && echo VALGRIND OK;)
# 	@rm $(TESTEXE)


$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

include $(wildcard $(OBJDIR)/*.d)
include $(wildcard $(TEST_OBJ_DIR)/*.d)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(EXE)

.PHONY: clean all
