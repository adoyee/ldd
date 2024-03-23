
obj-m	:=scull.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	cc main.c -o scull_main
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean 
	-rm scull_main
