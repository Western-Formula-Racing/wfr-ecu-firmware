# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Include paths
INCLUDES = -Icore/include -Igenerated/include

# ===== Generated sources =====
GEN_CODEC = generated/src/CanCodec.cpp
GEN_CACHE = generated/src/CanStateCacheUpdate.cpp

# ===== Core sources =====
CORE_SRC = core/src/CanPermissions.cpp core/src/CanInterface.cpp

# ===== Test sources =====
TEST_CODEC_SRC = tests/generated/test_codec_generated.cpp
TEST_INTERFACE_SRC = tests/generated/test_interface_generated.cpp

# ===== Targets =====
TARGET_CODEC = test_codec
TARGET_INTERFACE = test_interface

# ===== Default =====
all: codec

# ===== Build codec test =====
codec: $(TARGET_CODEC)

$(TARGET_CODEC): $(TEST_CODEC_SRC) $(GEN_CODEC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

# ===== Build interface test =====
interface: $(TARGET_INTERFACE)

$(TARGET_INTERFACE): $(TEST_INTERFACE_SRC) $(GEN_CODEC) $(GEN_CACHE) $(CORE_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

# ===== Build both =====
both: codec interface

# ===== Run targets =====
run_codec: codec
	./$(TARGET_CODEC)

run_interface: interface
	./$(TARGET_INTERFACE)

run_both: both
	./$(TARGET_CODEC)
	./$(TARGET_INTERFACE)

# ===== Clean =====
clean:
	rm -f $(TARGET_CODEC) $(TARGET_INTERFACE) *.o

.PHONY: all codec interface both run_codec run_interface run_both clean