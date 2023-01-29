all:
	gcc -pthread csapp.c shellex.c -o sh257
clean:
	rm -f sh257
run:
	./sh257