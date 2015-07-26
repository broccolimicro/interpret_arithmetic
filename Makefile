SRCDIR       =  interpret_arithmetic
CXXFLAGS	 =  -O2 -g -Wall -fmessage-length=0 -I../ucs -I../parse_ucs -I../interpret_ucs -I../parse_expression -I../arithmetic -I../parse -I../common
SOURCES	    :=  $(shell find $(SRCDIR) -name '*.cpp')
OBJECTS	    :=  $(SOURCES:%.cpp=%.o)
TARGET		 =  lib$(SRCDIR).a

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rvs $(TARGET) $(OBJECTS)

%.o: $(SRCDIR)/%.cpp 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c -o $@ $<
	
clean:
	rm -f $(OBJECTS) $(TARGET)
