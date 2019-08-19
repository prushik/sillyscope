#include <stdio.h>			// For printf (default error handler)
#include <unistd.h>			// For syscall functions
#include <stdint.h>			// For uintxx_t types
#include <fcntl.h>			// For open() constants
#include <sys/mman.h>		// MMAP #defines

#include "bmp.h"

int main()
{
	int i = bmp_create(0,0,128,128);
	bmp_save("./test.bmp");
}
