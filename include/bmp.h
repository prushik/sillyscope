struct bmp_file_header
{
	uint16_t magic;
	uint32_t fsize;
	uint32_t garbage;
	uint32_t data_offset;
};

struct bmp_img_header
{
	uint32_t img_header_size;
	uint32_t w,h;
	uint16_t planes;
	uint16_t bpp;
	uint32_t enc;
	uint32_t data_size;
	uint32_t hres,vres;
	uint32_t colors,i_colors;
};

struct bmp_image
{
	struct bmp_file_header file_header;
	struct bmp_img_header img_header;
	void *pixel;
	int index;
};

void bmp_set_draw_callback(void (*callback)(int,int,uint32_t));

void bmp_set_error_handler(void (*callback)(char*,...));

void bmp_draw(int x, int y, int index);

void bmp_draw_filter(int x, int y, int index, uint32_t filter);

void bmp_draw_part(int dest_x, int dest_y, int source_x, int source_y, int source_w, int source_h, int index);

void bmp_draw_rev_part(int dest_x, int dest_y, int source_x, int source_y, int source_w, int source_h, int index);

int bmp_load(char *bmp_fname);

int bmp_create(int source_x, int source_y, int h, int w);

void bmp_save(char *bmp_fname);
