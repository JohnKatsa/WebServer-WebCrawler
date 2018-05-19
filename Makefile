all: myhttpd mycrawler

myhttpd: myhttpd.o QueueImplementation.o
	gcc -pthread -o myhttpd myhttpd.o QueueImplementation.o

mycrawler: mycrawler.o QueueImplementation.o
	gcc -pthread -o mycrawler mycrawler.o QueueImplementation.o

myhttp.o: myhttp.c header.h
	gcc -c -g myhttpd.c

QueueImplementation.o: QueueImplementation.c header.h
	gcc -c -g QueueImplementation.c

clean:
	rm myhttpd myhttpd.o mycrawler mycrawler.o QueueImplementation.o
