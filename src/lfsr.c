#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int in_fd = 0, out_fd = 1;

static short int lfsr;

int lfsr_configure(short int state)
{
	lfsr = state;
}

int lfsr_step(int bit)
{
	int output = lfsr & 0x01;

	lfsr = (lfsr << 1) | (((lfsr>>9)^(lfsr>>11))&0x01);

	return output ^ (bit & 0x01);
}

int main(int argc, char ** argv)
{
	char data_in;
	signed char data_out;
	int i;
	if (argc >= 2) in_fd  = open(argv[1], O_RDONLY);
	if (argc == 3) out_fd = open(argv[2], O_WRONLY | O_CREAT);

	while (read(in_fd, &data_in, 1))
	{
		data_out = 0;
		for (i=0;i<8;i++)
		{
			data_out = (data_out << 1) | lfsr_step(data_in&0x01);
			data_in = data_in>>1;
		}
		write(out_fd, &data_out, 1);
	}

	return 0;
}
