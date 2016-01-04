#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <inttypes.h>
#include <string.h>

#include "decode.h"

#include "libbpg.h"

static void ppm_save_to_file(BPGDecoderContext *img, const char *filename)
{
    BPGImageInfo img_info_s, *img_info = &img_info_s;
    FILE *f;
    int w, h, y;
    uint8_t *rgb_line;

    bpg_decoder_get_info(img, img_info);

    w = img_info->width;
    h = img_info->height;

    rgb_line = malloc(3 * w);

    f = fopen(filename,"wb");
    if (!f) {
        fprintf(stderr, "%s: I/O error\n", filename);
        exit(1);
    }

    fprintf(f, "P6\n%d %d\n%d\n", w, h, 255);

    bpg_decoder_start(img, BPG_OUTPUT_FORMAT_RGB24);
    for (y = 0; y < h; y++) {
        bpg_decoder_get_line(img, rgb_line);
        fwrite(rgb_line, 1, w * 3, f);
    }
    fclose(f);

    free(rgb_line);
}

static void ppm_save_to_buffer(BPGDecoderContext *img, uint8_t** outBuf, unsigned int *outBufLen)
{
    BPGImageInfo img_info_s, *img_info = &img_info_s;
    int w, h, y, size_of_line, size_of_header, bufferIncrement;
    uint8_t *rgb_line;
    char header[255];

    memset(header, 0, 255*sizeof(char));
    bpg_decoder_get_info(img, img_info);

    w = img_info->width;
    h = img_info->height;
    size_of_line = 3*w;
    rgb_line = malloc(size_of_line);
    if(NULL == rgb_line){
        printf("FAILED to allocate \n");
        return;
    }

    snprintf(header, 255, "P6\n%d %d\n%d\n", w, h, 255);
    *outBufLen = size_of_line*h + strlen(header) /*+1*/;
    *outBuf = (uint8_t*)malloc( *outBufLen );
    if(NULL == *outBuf){
        printf("FAILED to allocate \n");
        free(rgb_line);
        return;
    }
    //copy the header first
    size_of_header = strlen(header)*sizeof(char);
    memcpy(*outBuf, header, size_of_header);

    bpg_decoder_start(img, BPG_OUTPUT_FORMAT_RGB24);
    bufferIncrement = 0;
    for (y = 0; y < h; y++) {
        bpg_decoder_get_line(img, rgb_line);
        memcpy( (*outBuf)+size_of_header+bufferIncrement, rgb_line, size_of_line);
        bufferIncrement += size_of_line;
    }
    free(rgb_line);
}

#define BITS_PER_PIXEL 24
#define X_PIXESLPERMETER 0x130B
#define Y_PIXESLPERMETER 0x130B

#pragma pack(push,1)

typedef struct {
    uint8_t signature[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t fileoffset_to_pixelarray;
} BmpFileHeader;

typedef struct {
    uint32_t dib_header_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    uint32_t y_pixelpermeter;
    uint32_t x_pixelpermeter;
    uint32_t num_colors_pallette;
    uint32_t most_imp_color;
} BmpInfoHeader;

typedef struct {
    BmpFileHeader fileheader;
    BmpInfoHeader bitmapinfoheader;
} Bitmap;

#pragma pack(pop)

static int bmp_save_to_buf(BPGDecoderContext *img, uint8_t** bmp_buf, unsigned int *buf_len) {
    BPGImageInfo img_info_s, *img_info = &img_info_s;
    int w, h, y, size_of_line, x;
    uint8_t *rgb_line/*, *bmp_line*/;
    uint8_t swap;

    int bmp_header_len = sizeof(Bitmap);
    Bitmap *bmp = (Bitmap*) calloc(1, bmp_header_len);
    bmp->fileheader.signature[0] = 'B';
    bmp->fileheader.signature[1] = 'M';
    bmp->fileheader.fileoffset_to_pixelarray = bmp_header_len;

    if (bpg_decoder_get_info(img, img_info) < 0) {
        return -1;
    }

    w = img_info->width;
    h = img_info->height;

    bmp->bitmapinfoheader.dib_header_size = sizeof(BmpInfoHeader);
    bmp->bitmapinfoheader.width = w;
    bmp->bitmapinfoheader.height = h;
    bmp->bitmapinfoheader.planes = 1;
    bmp->bitmapinfoheader.bits_per_pixel = BITS_PER_PIXEL;
    bmp->bitmapinfoheader.compression = 0;
    bmp->bitmapinfoheader.image_size = w * h * (24 / 8);
    bmp->bitmapinfoheader.x_pixelpermeter = X_PIXESLPERMETER;
    bmp->bitmapinfoheader.y_pixelpermeter = Y_PIXESLPERMETER;
    bmp->bitmapinfoheader.num_colors_pallette = 0;

    int padding = 0;
    int scanlinebytes = w * 3;
    while ((scanlinebytes + padding) % sizeof(uint32_t) != 0) {
        padding++;
    }

    // get the padded scanline width
    size_of_line = scanlinebytes + padding;
    rgb_line = malloc(size_of_line);
    if (rgb_line == NULL) {
        printf("Could not allocate the RGB line buffer! \n");
        return -1;
    }

    bmp->fileheader.file_size = size_of_line * h + bmp_header_len;

    *buf_len = bmp->fileheader.file_size;
    *bmp_buf = malloc(*buf_len);

    if (*bmp_buf == NULL) {
        printf("Could not allocate the RGB line buffer! \n");
        free(rgb_line);
        return -1;
    }

    memset(*bmp_buf, 0, *buf_len);
    memcpy(*bmp_buf, bmp, bmp_header_len);

    bpg_decoder_start(img, BPG_OUTPUT_FORMAT_RGB24);
    int idx = 0, buf_idx;
    for (y = 0; y < h; y++) {
        bpg_decoder_get_line(img, rgb_line);

        // RGB needs to be BGR
        for (x = 0; x < size_of_line; x += 3) {
            swap = rgb_line[x + 2];
            rgb_line[x + 2] = rgb_line[x]; // swap r and b
            rgb_line[x] = swap; // swap b and r
        }

        idx += size_of_line;
        buf_idx = (*buf_len - idx);
        memcpy((*bmp_buf) + buf_idx, rgb_line, size_of_line);
    }
    //	FILE *f = fopen("out.bmp", "ab+");
    //	fwrite (*bmp_buf, 1, *buf_len, f);
    //	fclose(f);
    free(rgb_line);
}

int bpg_get_buffer_size_from_bpg(uint8_t *bpgBuffer, int bpgBufferSize)
{
    BPGImageInfo p;
    int bmp_buffer_size, ret;

    ret = bpg_decoder_get_info_from_buf(&p, NULL, bpgBuffer, bpgBufferSize);
    if (ret < 0) {
        return 0;//not a bpg image
    }

    bmp_buffer_size = 3 * p.width * p.height + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    return bmp_buffer_size;
}

void decode_buffer(uint8_t* bufIn, unsigned int bufInLen, uint8_t** bufOut, unsigned int* bufOutLen, enum DecodeTo format){
    BPGDecoderContext *img;

    if(NULL == bufIn || bufInLen==0 || NULL == bufOut){
        printf("Invalid input data \n");
        return;
    }

    img = bpg_decoder_open();

    if (bpg_decoder_decode(img, bufIn, bufInLen) < 0) {
        fprintf(stderr, "Could not decode image\n");
        return;
    }

    switch(format){
    case BMP:{
        bmp_save_to_buffer(img, bufOut, bufOutLen);
        break;
    }
    case PPM:{
        ppm_save_to_buffer(img, bufOut, bufOutLen);
        break;
    }
    case PNG:{
        //TBD
        break;
    }
    default:{
        break;
    }
    }

    bpg_decoder_close(img);
}

void decode_file(char* bpgFilename, char* outFilename){
    FILE *f;
    BPGDecoderContext *img;
    uint8_t *buf;
    int buf_len, bit_depth;
    const char *p;

    f = fopen(bpgFilename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", bpgFilename);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    buf_len = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = malloc(buf_len);
    if (fread(buf, 1, buf_len, f) != buf_len) {
        fprintf(stderr, "Error while reading file\n");
        exit(1);
    }

    fclose(f);

    img = bpg_decoder_open();

    if (bpg_decoder_decode(img, buf, buf_len) < 0) {
        fprintf(stderr, "Could not decode image\n");
        exit(1);
    }
    free(buf);
    {
        ppm_save_to_file(img, outFilename);
    }

    bpg_decoder_close(img);
}

#if 0
int main(int argc, char **argv)
{
    FILE *f;
    uint8_t *buf;
    uint8_t *decBuf;
    unsigned int buf_len, decBuf_len;

    if(argc < 2){
        printf("Usage: \n libbpg-decode <input file> <ouput file> \n");
        exit(1);
    }

    f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    buf_len = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = malloc(buf_len);
    if (fread(buf, 1, buf_len, f) != buf_len) {
        fprintf(stderr, "Error while reading file\n");
        exit(1);
    }

    fclose(f);

    decode_buffer(buf, buf_len, &decBuf, &decBuf_len, BMP);

    f = fopen(argv[2], "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", argv[2]);
        exit(1);
    }

    fwrite(decBuf, 1, decBuf_len, f);
    fclose(f);

    return 0;
}
#endif
