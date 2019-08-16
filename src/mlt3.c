#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int in_fd = 0, out_fd = 1;

signed char mlt3_encode(char bit)
{
	static signed char level=0, direction=1;

	if (bit)
	{
		level += direction;
		if (level == 1) direction = -1;
		if (level == -1) direction = 1;
	}

	return level;
}

int main(int argc, char ** argv)
{
	char data_in;
	signed char data_out;
	int i;
	if (argc >= 2) in_fd  = open(argv[1], O_RDONLY);
	if (argc == 3) out_fd = open(argv[1], O_WRONLY);

	while (read(in_fd, &data_in, 1))
	{
		for (i=0;i<8;i++)
		{
			data_out = mlt3_encode(data_in & 0x80);
			write(out_fd, &data_out, 1);
			data_in = data_in<<1;
		}
	}

	return 0;
}
