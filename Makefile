all: project3

project3: project3.o node0.o node1.o node2.o node3.o
	gcc project3.o node0.o node1.o node2.o node3.o -o project3

project3.o: project3.c project3.h
	gcc -c project3.c -w

node0.o: node0.c project3.h
	gcc -c node0.c

node1.o: node1.c project3.h
	gcc -c node1.c

node2.o: node2.c project3.h
	gcc -c node2.c

node3.o: node3.c project3.h
	gcc -c node3.c

clean:
	rm *.o project3