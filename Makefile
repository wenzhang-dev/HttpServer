DIR_CODE := code
DIR_TEST := test
DIR_OBJ := build

CXX := g++
CFLAGS := -g -O0 -Wall -std=c++11
LIBS := -lpthread
INCLUDES := $(shell pwd)/code

CODE_SOURCE := $(wildcard ${DIR_CODE}/*.cc)
TEST_SOURCE := $(wildcard ${DIR_TEST}/*.cc)

OBJECT := $(patsubst %.cc,${DIR_OBJ}/%.o,$(notdir ${CODE_SOURCE}))
TARGET := $(basename ${TEST_SOURCE})

.PYONY : all
all : prepare ${OBJECT}
	@for obj in ${TARGET}; do \
		echo "compiling $${obj}"; \
		${CXX} ${CFLAGS} -I${INCLUDES} ${OBJECT} $${obj}".cc" -o ${DIR_OBJ}/$$obj ${LIBS}; \
	done
	@echo "[BUILD] done!"
	
prepare : 
	@if [ ! -d ./build ]; then mkdir -p build/test; fi

${DIR_OBJ}/%.o : ${DIR_CODE}/%.cc
	${CXX} -c ${CFLAGS} $< -o $@

.PYONY : clean
clean:
	rm -rf build/ 
	