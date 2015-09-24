#makefile

ADDITIONAL_INCLUDE:=-I/usr/local/include


OBJS:=./linux.cpp
OBJS+=../src/chacha7.c
OBJS+=../src/http.cpp
OBJS+=../src/Lobby.cpp
OBJS+=../src/main.cpp
OBJS+=../src/Obfs.cpp
OBJS+=../src/ObfsSession.cpp
OBJS+=../src/ObfsStream.cpp
OBJS+=../src/ObfsSvc.cpp
OBJS+=../src/PermanentConnection.cpp
OBJS+=../src/PermanentConnectionEntry.cpp
OBJS+=../src/PermanentConnectionRelaySession.cpp
OBJS+=../src/PermanentConnectionRelaySvc.cpp
OBJS+=../src/PermanentConnectionStream.cpp
OBJS+=../src/ProxySvc.cpp
OBJS+=../src/RelaySession.cpp
OBJS+=../src/RelaySvc.cpp
OBJS+=../src/Socks5Session.cpp
OBJS+=../src/Socks5Svc.cpp
OBJS+=../src/SSLHelper.cpp
OBJS+=../src/SslStream.cpp
OBJS+=../src/static.cpp
OBJS+=../src/TcpEntry.cpp
OBJS+=../src/TcpStream.cpp
OBJS+=../src/testPermanentConnectionRemote.cpp
OBJS+=../src/UpStreamProxy.cpp


STATIC_LIBS:=-lssl
STATIC_LIBS+=-lcrypto
STATIC_LIBS+=-lboost_system
STATIC_LIBS+=-lboost_coroutine
STATIC_LIBS+=-lboost_thread
STATIC_LIBS+=-lboost_context
STATIC_LIBS+=-lstdc++
#STATIC_LIBS+=-lgcc

TARGET:=PolyProxyPermanentConnectionRemote
OUTPUTDIR:=.


OPT=-O3 -fno-stack-protector -fno-stack-check -s -mtune=native -fpermissive -pthread -std=c++11 -DNDEBUG -DBOOST_ASIO_SEPARATE_COMPILATION

${TARGET}:${OBJS} 
	i686-linux-musl-gcc -o ${OUTPUTDIR}/${TARGET} ${OBJS} ${ADDITIONAL_INCLUDE} -L/usr/local/lib ${OPT} -Wl,-Bstatic ${STATIC_LIBS} -static
