#include <stdio.h>			// For printf (default error handler)
#include <unistd.h>			// For syscall functions
#include <stdint.h>			// For uintxx_t types
#include <fcntl.h>			// For open() constants
#include <sys/mman.h>		// MMAP #defines

#include "bmp.h"

int main(int argc, char ** argv)
{
	int x, y;
	int fd;
	signed char data;
	int i = bmp_create(0,0,1280,128);

	fd = open(argv[1], O_RDONLY);

	x = 0;
	while (read(fd, &data, 1))
	{
		y = 64+(data);
		bmp_set_pixel(x, y-1, 0xffffffff, i);
		bmp_set_pixel(x, y+1, 0xffffffff, i);
		bmp_set_pixel(x++, y, 0xff00ff00, i);
	}

	close(fd);

	bmp_save("./test.bmp", i);
}
