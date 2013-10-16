objects = main.o Init_blcr.o daemon.o parser.o pwcache.o readproc.o select.o checkpoint.o
edit:$(objects)
	gcc -o edit $(objects)
main.o:main.c readproc.h select.h parse.h
	gcc -c main.c
Init_blcr.o:Init_blcr.c 
	gcc -c Init_blcr.c
daemon.o:daemon.c 
	gcc -c daemon.c
parser.o:parser.c parse.h readproc.h
	gcc -c parser.c
pwcache.o:pwcache.c 
	gcc -c pwcache.c
readproc.o:readproc.c pwcache.h readproc.h  
	gcc -c readproc.c
select.o:select.c parse.h readproc.h
	gcc -c select.c
checkponit.o:checkpoint.c parse.h checkpoint.h libcr.h
	gcc -c checkpoint.c
.PHONY:clean
clean:
	rm edit $(objects) 

