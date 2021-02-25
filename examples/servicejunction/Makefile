all: bin/cts bin/email bin/ircBot bin/mysqlQuery bin/nwp bin/oracleQuery bin/page bin/qBot bin/request bin/wms

bin/%: %.cpp
	-if [ ! -d bin ]; then mkdir bin; fi;
	g++ -ggdb -o $@ $< -I../../ -L../../ -lcommon -lcrypto -lexpat -lpthread -lssl -lz

clean:
	-rm -fr bin
