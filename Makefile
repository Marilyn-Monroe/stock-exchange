CMAKE_COMMON_FLAGS ?= -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
CMAKE_DEBUG_FLAGS ?=
CMAKE_RELEASE_FLAGS ?=
NPROCS ?= $(shell nproc)

# NOTE: use Makefile.local to override the options defined above.
-include Makefile.local

CMAKE_DEBUG_FLAGS += -DCMAKE_BUILD_TYPE=Debug $(CMAKE_COMMON_FLAGS)
CMAKE_RELEASE_FLAGS += -DCMAKE_BUILD_TYPE=Release $(CMAKE_COMMON_FLAGS)

.PHONY: all
all: test-debug test-release

# Run cmake
.PHONY: cmake-debug
cmake-debug:
	cmake -B build_debug $(CMAKE_DEBUG_FLAGS)

.PHONY: cmake-release
cmake-release:
	cmake -B build_release $(CMAKE_RELEASE_FLAGS)

build_debug/CMakeCache.txt: cmake-debug
build_release/CMakeCache.txt: cmake-release

# Build using cmake
.PHONY: build-debug build-release
build-debug build-release: build-%: build_%/CMakeCache.txt
	cmake --build build_$* -j $(NPROCS) --target stock_exchange

# Test
.PHONY: test-debug test-release
test-debug test-release: test-%: build-%
	cmake --build build_$* -j $(NPROCS) --target stock_exchange_unittest
	cd build_$* && ((test -t 1 && GTEST_COLOR=1 PYTEST_ADDOPTS="--color=yes" ctest -V) || ctest -V)

# Start
.PHONY: start-debug start-release
start-debug start-release: start-%: build-%
	./build_$*/stock_exchange

# Cleanup data
.PHONY: clean-debug clean-release
clean-debug clean-release: clean-%:
	cmake --build build_$* --target clean

.PHONY: dist-clean
dist-clean:
	rm -rf build_*
