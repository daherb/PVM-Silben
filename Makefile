ifndef PVM_ROOT
	ERROR=1
endif
ifndef PVM_ARCH
	ERROR=1
endif
all: master slave

master: master.cpp
ifeq ($(ERROR),1)
	@echo "Variable PVM_ROOT or PVM_ARCH not set -> Not building master"
else
	g++ -std=c++0x -o silben_master master.cpp -I ${PVM_ROOT}/include -I /usr/pkg/include -L ${PVM_ROOT}/lib/${PVM_ARCH}/ -lpvm3 -Wl,-R ${PVM_ROOT}/lib/${PVM_ARCH} -ggdb
endif

slave: slave.cpp
ifeq ($(ERROR),1)
	@echo "Variable PVM_ROOT or PVM_ARCH not set -> Not building slave"
else
	g++ -std=c++0x -o silben_slave slave.cpp -I ${PVM_ROOT}/include  -L ${PVM_ROOT}/lib/${PVM_ARCH}/ -lpvm3 -Wl,-R ${PVM_ROOT}/lib/${PVM_ARCH} -ggdb
endif
	
install:
	cp silben_slave ${PVM_ROOT}/bin/${PVM_ARCH}
	cp silben_master ${PVM_ROOT}/bin/${PVM_ARCH}