#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define PATH "/dev/scull"

char msg[] = "hello scull";

static void
file_test()
{
	int ret;
	FILE *fptr;
	char buff[1024] = {0};

	fptr = fopen(PATH, "r+");
	if(!fptr) {
		printf("open /dev/scull failed: %s\n", strerror(errno));
		return;
	}

	ret = fwrite(msg, strlen(msg), 1, fptr);
	if(ret < 0){
		printf("write /dev/scull failed: %s\n", strerror(errno));
		return;
	}

	ret = fread(buff, 1024, 1, fptr);
	if(ret < 0){
		printf("write /dev/scull failed: %s\n", strerror(errno));
		return;
	}

	printf("result:%s\n", buff);

}

static void 
file_test_2(void)
{
	int fd, ret;
	char buff[1024] = {0};

	fd = open("/dev/scull", O_RDWR);
	if(fd < 0){
		printf("open /dev/scull failed: %s\n", strerror(errno));
		return;
	}

	ret = write(fd, msg, strlen(msg));
	if(ret < 0){
		printf("write /dev/scull failed: %s\n", strerror(errno));
		return;
	}

	printf("%d bytes write\n", ret);

	ret = read(fd, buff, 1024);
	if(ret < 0){
		printf("read /dev/scull failed: %s\n", strerror(errno));
		return; 
	}
	printf("read size:%d :%s\n", ret, buff);
}

int main(int argc, char **argv)
{
	file_test_2();
	return 0;
}