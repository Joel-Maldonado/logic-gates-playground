# Makefile for logic_gates project

CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Wall
LDFLAGS_APP = -Llib -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -O3

# Core module
SRC_CORE = src/core/GatePin.cpp src/core/LogicGate.cpp src/core/Wire.cpp

# Simulation module
SRC_SIM = src/simulation/CircuitSimulator.cpp

# UI module
SRC_UI = src/ui/UIManager.cpp src/ui/PaletteManager.cpp src/ui/GateRenderer.cpp src/ui/WireRenderer.cpp src/ui/WireRouter.cpp src/ui/InputHandler.cpp
		
# Application module
SRC_APP = src/app/Application.cpp src/app/Config.cpp src/app/main.cpp

# Derived gates
SRC_GATES = src/core/DerivedGates.cpp src/core/InputSource.cpp src/core/OutputSink.cpp

# All source files
SRC_ALL = $(SRC_CORE) $(SRC_SIM) $(SRC_UI) $(SRC_APP) $(SRC_GATES)

OUT_APP = main

all: $(OUT_APP)

$(OUT_APP): $(SRC_ALL)
	$(CXX) $(CXXFLAGS) -o $(OUT_APP) $(SRC_ALL) $(LDFLAGS_APP)

clean:
	rm -f $(OUT_APP)