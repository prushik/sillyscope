#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int in_fd = 0, out_fd = 1;

static char _4b5b_table[] = {
	0b11110,
	0b01001,
	0b10100,
	0b10101,
	0b01010,
	0b01011,
	0b01110,
	0b01111,
	0b10010,
	0b10011,
	0b10110,
	0b10111,
	0b11010,
	0b11011,
	0b11100,
	0b11101
};

char _4b5b_encode(char nibble)
{
	return _4b5b_table[nibble&0x0f];
}

int main(int argc, char ** argv)
{
	char data_in[32];
	char data_out[41];
	int i,j;
	if (argc >= 2) in_fd  = open(argv[1], O_RDONLY);
	if (argc == 3) out_fd = open(argv[2], O_WRONLY | O_CREAT);

	while (read(in_fd, data_in, 32))
	{
		for (i=0;i<40;i++)
			data_out[i]=0;
		for (i=0,j=0;i<64;i++)
		{
			char tmp_in,tmp_out;
			tmp_in = data_in[i>>1]>>((!(i&1))<<2);
			tmp_out = _4b5b_encode(tmp_in);
			data_out[((i*5)>>2)>>1] |= (tmp_out<<3)>>((i*5)&0x07);
			data_out[(((i*5)>>2)>>1)+1] |= tmp_out<<(11-((i*5)&0x07));
		}
		write(out_fd, data_out, 40);
	}

	return 0;
}
