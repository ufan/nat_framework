SHELL=/bin/bash
.PHONY : build clean libio libstrategy md td TunnelAgent TradeTool SignalAgent dump tools libccrypt test tdlib liblog logdb

SUBDIR = io strategy md td TunnelAgent TradeTool signal_agent dump tools execute_model test tdlib crypt logger logdb

build : md td TunnelAgent TradeTool SignalAgent libstrategy dump tools test tdlib logdb

md: libio
	make -C md

libio: liblog
	make -C io

liblog:
	make -C logger

td: libio
	make -C td

TunnelAgent: tools libccrypt
	make -C TunnelAgent

TradeTool:
	make -C TradeTool

SignalAgent:
	make -C signal_agent

libstrategy:
	@if [[ -f ./strategy/CSystemIO.o ]]; then \
		make -C strategy; \
	else make -B -C strategy; \
	fi
	make -C execute_model

dump:
	make -C dump

tools: libccrypt libio liblog
	make -C tools

libccrypt:
	make -C crypt

test:
	make -C test

tdlib: libstrategy
	make -C test

logdb:
	make -C logdb

installer: build
	mkdir -p ../release/lib/ThirdParty;
	cp -r ThirdParty/CTP ThirdParty/EES ThirdParty/EESV2 ThirdParty/EES3_1_3_47 ThirdParty/Xele ThirdParty/Xt ThirdParty/libev ../release/lib/ThirdParty/
	cp ./lib/*.so ../release/lib/
	scripts/make_installer.sh

SDK: strategylib tools
	cp account/*.h ../release/SDK/include
	cp risk/*.h ../release/SDK/include
	cp common/*.h* ../release/SDK/include
	cp config/*.h ../release/SDK/include
	cp include/*.h ../release/SDK/include
	cp io/*.h ../release/SDK/include
	cp logdb/*.h ../release/SDK/include
	cp logger/*.h ../release/SDK/include
	cp bar/*.h ../release/SDK/include
	cp md/*.h ../release/SDK/include
	cp td/*.h ../release/SDK/include
	cp pyext/*.h ../release/SDK/include
	cp strategy/*.h ../release/SDK/include
	cp execute_model/*.h ../release/SDK/include
  # cp -r ThirdParty/log4cplus/include/* ../release/SDK/include
	cp ./lib/*.so ../release/SDK/lib
	cd ../release; tar cfz share/SDK.tgz SDK/

clean:
	@for i in $(SUBDIR); \
	do \
		make -C $$i $@; \
	done
	-rm -r ../release/lib/* 2>/dev/null; true
	-rm ../release/install.sh 2>/dev/null; true
	-rm ../release/SDK/lib/*.so 2>/dev/null; true
	-rm ../release/SDK/include/* 2>/dev/null; true
	-rm -r ../release/share/SDK 2>/dev/null; true

