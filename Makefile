all: myhttpd mycrawler jobExecutor

myhttpd: myhttpd.o QueueImplementation.o
	gcc -pthread -o myhttpd myhttpd.o QueueImplementation.o

mycrawler: mycrawler.o QueueImplementation.o
	gcc -pthread -o mycrawler mycrawler.o QueueImplementation.o

jobExecutor: jobExecutor.o worker.o trie.o list.o
	gcc -o jobExecutor jobExecutor.o worker.o trie.o list.o

mycrawler.o: mycrawler.c headerje.h header.h
	gcc -c mycrawler.c

myhttp.o: myhttp.c header.h
	gcc -c -g myhttpd.c

QueueImplementation.o: QueueImplementation.c header.h
	gcc -c -g QueueImplementation.c

worker.o: worker.c headerje.h
	gcc -c -g worker.c

jobExecutor.o: jobExecutor.c headerje.h
	gcc -c -g jobExecutor.c

trie.o: trie.c headerje.h
	gcc -c -g trie.c

list.o: list.c headerje.h
	gcc -c -g list.c

clean:
	rm myhttpd myhttpd.o mycrawler mycrawler.o QueueImplementation.o

	rm jobExecutor jobExecutor.o worker.o trie.o list.o
	rm -rf log
	rm search
