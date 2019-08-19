#include <stdio.h>			// For printf (default error handler)
#include <unistd.h>			// For syscall functions
#include <stdint.h>			// For uintxx_t types
#include <fcntl.h>			// For open() constants
#include <sys/mman.h>		// MMAP #defines

#include "bmp.h"

static struct bmp_image *bmp[512];

static int bmpc=0;

static void (*bmp_draw_callback)(int,int,uint32_t);
static void (*error_callback)(char*,...) = printf;

void bmp_set_draw_callback(void (*callback)(int,int,uint32_t))
{
	bmp_draw_callback = callback;
}

void bmp_set_error_handler(void (*callback)(char*,...))
{
	error_callback = callback;
}

void bmp_draw(int x, int y, int index)
{
	int i,j;
	for (i=0;i<bmp[index]->img_header.h;i++)
		for (j=0;j<bmp[index]->img_header.w;j++)
			if ((*((uint32_t*)(bmp[index]->pixel)+(j+i*bmp[index]->img_header.w)))&0xff000000)
				bmp_draw_callback(x+j,y+i,(*((uint32_t*)(bmp[index]->pixel)+(j+i*bmp[index]->img_header.w))));
}

void bmp_draw_filter(int x, int y, int index, uint32_t filter)
{
	int i,j;
	for (i=0;i<bmp[index]->img_header.h;i++)
		for (j=0;j<bmp[index]->img_header.w;j++)
			if ((*((uint32_t*)(bmp[index]->pixel)+(j+i*bmp[index]->img_header.w)))&0xff000000)
				bmp_draw_callback(x+j,y+i,(*((uint32_t*)(bmp[index]->pixel)+(j+i*bmp[index]->img_header.w)))&filter);
}

void bmp_draw_part(int dest_x, int dest_y, int source_x, int source_y, int source_w, int source_h, int index)
{
	int x,y;
	for (y=source_y;y<source_y+source_h;y++)
		for (x=source_x;x<source_x+source_w;x++)
			if ((*((uint32_t*)(bmp[index]->pixel)+(x+y*bmp[index]->img_header.w)))&0xff000000)
				bmp_draw_callback(dest_x+x-source_x,dest_y+y-source_y,(*((uint32_t*)(bmp[index]->pixel)+(x+y*bmp[index]->img_header.w))));
}

void bmp_draw_rev_part(int dest_x, int dest_y, int source_x, int source_y, int source_w, int source_h, int index)
{
	int x,y;
	for (y=source_y;y<source_y+source_h;y++)
		for (x=source_x+source_w;x>source_x;x--)
			if ((*((uint32_t*)(bmp[index]->pixel)+(x+y*bmp[index]->img_header.w)))&0xff000000)
				bmp_draw_callback(dest_x+(source_w-x)+source_x,dest_y+y-source_y,(*((uint32_t*)(bmp[index]->pixel)+(x+y*bmp[index]->img_header.w))));
}

int bmp_load(char *bmp_fname)
{
	int bmp_fd;

	bmp_fd = open(bmp_fname,O_RDONLY);

	//New bmp
	bmpc=(bmpc+1)&0xff; //Increase the bmp count, but don't overflow
	bmp[bmpc]=sbrk(sizeof(struct bmp_image));
	struct bmp_image *img=bmp[bmpc];
	img->index=bmpc;

	read(bmp_fd,(void *)&img->file_header,2);
	if (img->file_header.magic!=0x4D42)
	{
		//Not a BMP
		error_callback("E: Not bmp (%X)\n",img->file_header.magic);
		return 0;
	}

	//Read the rest of the file header
	read(bmp_fd,(void *)&img->file_header.fsize,12);
	//Notice here that we cannot just start reading into the memory directly 
	//after file_header.magic because the compiler will add padding between 
	//the fields magic and fsize

	//Read the image header
	read(bmp_fd,(void *)&img->img_header,sizeof(struct bmp_img_header));

	if (img->img_header.enc)
	{
		//Unsupported compression
		error_callback("E: Encoded\n");
		return 0;
	}

	//Jump to the pixel data
	lseek(bmp_fd,img->file_header.data_offset,SEEK_SET);

	//Allocate the memory for the pixel data
	img->pixel = (void*)mmap(0, (img->img_header.w*img->img_header.h)<<2, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	int bytes_per_pixel = img->img_header.bpp>>3;

	//temporary storage for 1 pixel of data
	uint32_t *tmp_pixel = (void*)sbrk(bytes_per_pixel);

	int y,x;
	for (y=0;y<img->img_header.h;y++)
	{
		for (x=0;x<img->img_header.w;x++)
		{
			//Read the pixel into the temporary buffer
			read(bmp_fd,tmp_pixel,bytes_per_pixel);

			uint32_t *location;
			location = img->pixel + ((((img->img_header.h-y) * img->img_header.w) + (x)) << 2);//bytes_per_pixel;

			if (bytes_per_pixel==2)
					*location = (*tmp_pixel&0x7c00)<<6|(*tmp_pixel&0x03e0)<<3|(*tmp_pixel&0x001f); //not tested
			else if (bytes_per_pixel==3)
					*location = (*tmp_pixel)|0xff000000; //I just hard code 0xff for the destination alpha
			else if (bytes_per_pixel==4)
					*location = *tmp_pixel; //on little endian systems, the BMP data is already in the correct format
		}
	}

	//Free the temporary memory
	sbrk(-(bytes_per_pixel));

	close(bmp_fd);

	//Return the index of the BMP
	return img->index;
}

int bmp_create(int source_x, int source_y, int w, int h)
{
	//New bmp
	bmpc=(bmpc+1)&0xff;
	bmp[bmpc]=sbrk(sizeof(struct bmp_image));
	struct bmp_image *img=bmp[bmpc];
	img->index=bmpc;

	img->file_header.magic=0x4D42;
	img->file_header.data_offset=14+sizeof(struct bmp_img_header);
	img->file_header.fsize=14+sizeof(struct bmp_img_header)+((w*h)<<2);
	img->img_header.img_header_size=sizeof(struct bmp_img_header);
	img->img_header.enc=0;
	img->img_header.bpp=32;
	img->img_header.planes=1;
	img->img_header.w=w;
	img->img_header.h=h;
	img->img_header.hres=320;
	img->img_header.vres=320;
	img->img_header.colors=0;
	img->img_header.i_colors=0;

	//Allocate the memory for the pixel data
	img->pixel = (void*)mmap(0, (img->img_header.w*img->img_header.h)<<2, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	int bytes_per_pixel = img->img_header.bpp>>3;

	//temporary storage for 1 pixel of data
	uint32_t tmp_pixel = 0xffffffff;

	int y,x;
	for (y=1;y<img->img_header.h;y++)
	{
		for (x=0;x<img->img_header.w;x++)
		{
			uint32_t *location;
			location = img->pixel + (((((img->img_header.h-y)<<2) * img->img_header.w) + (x << 2)));//bytes_per_pixel;

			if (bytes_per_pixel==2)
					*location = (tmp_pixel&0x7c00)<<6|(tmp_pixel&0x03e0)<<3|(tmp_pixel&0x001f); //not tested
			else if (bytes_per_pixel==3)
					*location = (tmp_pixel)|0xff000000; //I just hard code 0xff for the destination alpha
			else if (bytes_per_pixel==4)
					*location = (x|(x<<8)|(y<<16)); //on little endian systems, the BMP data is already in the correct format
		}
	}

	return img->index;
}

void bmp_save(char *bmp_fname)
{
	int bmp_fd;

	bmp_fd = open(bmp_fname,O_WRONLY|O_CREAT);

	struct bmp_image *img=bmp[bmpc];

	if (img->file_header.magic!=0x4D42)
	{
		//Not a BMP
		error_callback("E: Not bmp (%X)\n",img->file_header.magic);
		return;
	}

	if (img->img_header.enc!=0)
	{
		//Unsupported compression
		error_callback("E: Cannot encode\n");
		return;
	}

	write(bmp_fd,(void *)&img->file_header,2);
	write(bmp_fd,(void *)&img->file_header.fsize,12);
	write(bmp_fd,(void *)&img->img_header,sizeof(struct bmp_img_header));

	//Jump to the pixel data
//	lseek(bmp_fd,img->file_header.data_offset,SEEK_SET);

	write(bmp_fd,img->pixel,(img->img_header.h*img->img_header.w)<<2);

	close(bmp_fd);

	//Return the index of the BMP
	return;
}
