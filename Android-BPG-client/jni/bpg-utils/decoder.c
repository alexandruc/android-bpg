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

#pragma pack(1)  // ensure structure is packed
typedef struct
{
    uint16_t  bfType;
    uint32_t bfSize;
    uint16_t  bfReserved1;
    uint16_t  bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(0)  // restore normal structure packing rules

static void bmp_save_to_buffer(BPGDecoderContext *img, uint8_t** outBuf, unsigned int *outBufLen)
{
    BPGImageInfo img_info_s, *img_info = &img_info_s;
    int w, h, y, size_of_line, bufferIncrement, x;
    uint8_t *rgb_line/*, *bmp_line*/;
    uint8_t swap;
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER info;

    memset(&header, 0, sizeof(BITMAPFILEHEADER));
    memset(&info, 0, sizeof(BITMAPINFOHEADER));

    bpg_decoder_get_info(img, img_info);

    w = img_info->width;
    h = img_info->height;
    size_of_line = 3*w;
    rgb_line = malloc(size_of_line);
    if(NULL == rgb_line){
        printf("FAILED to allocate \n");
        return;
    }

    //prepare the bmp header
    header.bfType = 19778;
    header.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    header.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    //prepare the bmp dib header
    info.biSize = sizeof(BITMAPINFOHEADER);
    info.biWidth = w;
    info.biHeight = h;
    info.biPlanes = 1;
    info.biBitCount = 24;
    info.biSizeImage = w*h*(24/8);

    *outBufLen = size_of_line * h + sizeof(header) + sizeof(info);
    *outBuf = malloc( *outBufLen );
    if(NULL == *outBuf){
        printf("FAILED to allocate \n");
        free(rgb_line);
        return;
    }
    memset(*outBuf, 0, *outBufLen);
    //copy the header and info first
    memcpy(*outBuf, &header, sizeof(header));
    memcpy(*outBuf+sizeof(header), &info, sizeof(info));

    bpg_decoder_start(img, BPG_OUTPUT_FORMAT_RGB24);
    bufferIncrement = size_of_line;
    for (y = 0; y < h; y++) {
        bpg_decoder_get_line(img, rgb_line);

        // RGB needs to be BGR
        for (x=0; x < size_of_line; x+=3){
            swap = rgb_line[x+2];
            rgb_line[x+2] = rgb_line[x]; // swap r and b
            rgb_line[x] = swap; // swap b and r
        }
        memcpy( (*outBuf)+*outBufLen-bufferIncrement, rgb_line, size_of_line);

        bufferIncrement += size_of_line;
    }
    free(rgb_line);
}

static void bpg_show_info(const char *filename, int show_extensions)
{
    uint8_t *buf;
    int buf_len, ret, buf_len_max;
    FILE *f;
    BPGImageInfo p_s, *p = &p_s;
    BPGExtensionData *first_md, *md;
    static const char *format_str[6] = {
        "Gray",
        "4:2:0",
        "4:2:2",
        "4:4:4",
        "4:2:0_video",
        "4:2:2_video",
    };
    static const char *color_space_str[BPG_CS_COUNT] = {
        "YCbCr",
        "RGB",
        "YCgCo",
        "YCbCr_BT709",
        "YCbCr_BT2020",
    };
    static const char *extension_tag_str[] = {
        "Unknown",
        "EXIF",
        "ICC profile",
        "XMP",
        "Thumbnail",
        "Animation control",
    };

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    if (show_extensions) {
        fseek(f, 0, SEEK_END);
        buf_len_max = ftell(f);
        fseek(f, 0, SEEK_SET);
    } else {
        /* if no extension are shown, just need the header */
        buf_len_max = BPG_DECODER_INFO_BUF_SIZE;
    }
    buf = malloc(buf_len_max);
    buf_len = fread(buf, 1, buf_len_max, f);

    ret = bpg_decoder_get_info_from_buf(p, show_extensions ? &first_md : NULL,
                                        buf, buf_len);
    free(buf);
    fclose(f);
    if (ret < 0) {
        fprintf(stderr, "Not a BPG image\n");
        exit(1);
    }
    printf("size=%dx%d color_space=%s",
           p->width, p->height,
           p->format == BPG_FORMAT_GRAY ? "Gray" : color_space_str[p->color_space]);
    if (p->has_w_plane) {
        printf(" w_plane=%d", p->has_w_plane);
    }
    if (p->has_alpha) {
        printf(" alpha=%d premul=%d",
               p->has_alpha, p->premultiplied_alpha);
    }
    printf(" format=%s limited_range=%d bit_depth=%d animation=%d\n",
           format_str[p->format],
            p->limited_range,
            p->bit_depth,
            p->has_animation);

    if (first_md) {
        const char *tag_name;
        printf("Extension data:\n");
        for(md = first_md; md != NULL; md = md->next) {
            if (md->tag <= 5)
                tag_name = extension_tag_str[md->tag];
            else
                tag_name = extension_tag_str[0];
            printf("  tag=%d (%s) length=%d\n",
                   md->tag, tag_name, md->buf_len);
        }
        bpg_decoder_free_extension_data(first_md);
    }
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
