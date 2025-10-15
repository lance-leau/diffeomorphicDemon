MATLAB_ROOT =   "C:/Program Files/MATLAB/R2024b/"
MEX         =   $(MATLAB_ROOT)bin/win64/mex.exe

TARGET	    = 	diffeoDemon

SRC 	    = 	diffeoDemon.c \
			    demon.c \
			    tools.c
                
HEADERS     = 	demon.h \
			    tools.h

MEX_FLAGS   =   -g -O
STRICT_WARNINGS = COMPFLAGS="$(COMPFLAGS) /W4 /WX"

OUTPUT_FILE =   $(TARGET).mexw64

all: remake

$(OUTPUT_FILE): $(SRC) $(HEADERS)
	$(MEX) $(MEX_FLAGS) $(STRICT_WARNINGS) -output $(TARGET) $(SRC)

remake:
	make clean
	make $(OUTPUT_FILE)

clean:
	$(RM) $(OUTPUT_FILE) $(OUTPUT_FILE).pdb

.PHONY: all clean remake