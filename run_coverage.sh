#!/usr/bin/env bash

set -e

echo "Cleaning old coverage data..."
ls
find . -name "*.gcno" -delete
find . -name "*.gcda" -delete
rm -f coverage.info cov.info gcd.info main.exe
rm -rf report

echo "Compiling with coverage flags..."
g++ --coverage \
  -I ./include \
  -I ./include/rapidcheck \
  -I ./include/rapidcheck/detail \
  -I ./include/rapidcheck/fn \
  -I ./include/rapidcheck/gen \
  -I ./include/rapidcheck/gen/detail \
  -I ./include/rapidcheck/shrink \
  -I ./include/rapidcheck/shrinkable \
  -I ./include/rapidcheck/state \
  -I ./include/rapidcheck/state/gen \
  main.cpp \
  ./src/*.cpp \
  ./src/detail/*.cpp \
  ./src/gen/*.cpp \
  ./src/gen/detail/*.cpp \
  -o main.exe

echo "Running test binary..."
./main.exe

echo "Capturing coverage..."
lcov --gcov-tool /mingw64/bin/gcov -c -d . -o gcd.info

echo "Filtering only main.cpp..."
lcov --remove gcd.info '*/rapidcheck/*' '*/mingw64/include/*' -o cov.info

echo "Generating HTML report..."
genhtml -o report cov.info

echo "Report ready: ./report/index.html"
