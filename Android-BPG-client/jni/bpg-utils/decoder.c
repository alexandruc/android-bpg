#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <inttypes.h>
#include <string.h>

/* define it to include PNG output */
//#define USE_PNG

#ifdef USE_PNG
#include <png.h>
#endif

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

#ifdef USE_PNG
static void png_write_data (png_structp png_ptr, png_bytep data,
                            png_size_t length)
{
    FILE *f;
    int ret;

    f = png_get_io_ptr(png_ptr);
    ret = fwrite(data, 1, length, f);
    if (ret != length)
        png_error(png_ptr, "PNG Write Error");
}

static void png_save(BPGDecoderContext *img, const char *filename, int bit_depth)
{
    BPGImageInfo img_info_s, *img_info = &img_info_s;
    FILE *f;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer;
    int y, color_type, bpp;
    BPGDecoderOutputFormat out_fmt;

    if (bit_depth != 8 && bit_depth != 16) {
        fprintf(stderr, "Only bit_depth = 8 or 16 are supported for PNG output\n");
        exit(1);
    }

    bpg_decoder_get_info(img, img_info);

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "%s: I/O error\n", filename);
        exit(1);
    }

    png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING,
                                        NULL,
                                        NULL,  /* error */
                                        NULL, /* warning */
                                        NULL,
                                        NULL,
                                        NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_set_write_fn(png_ptr, (png_voidp)f, &png_write_data, NULL);

    if (setjmp(png_jmpbuf(png_ptr)) != 0) {
        fprintf(stderr, "PNG write error\n");
        exit(1);
    }

    if (img_info->has_alpha)
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    else
        color_type = PNG_COLOR_TYPE_RGB;

    png_set_IHDR(png_ptr, info_ptr, img_info->width, img_info->height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
    if (bit_depth == 16) {
        png_set_swap(png_ptr);
    }
#endif

    if (bit_depth == 16) {
        if (img_info->has_alpha)
            out_fmt = BPG_OUTPUT_FORMAT_RGBA64;
        else
            out_fmt = BPG_OUTPUT_FORMAT_RGB48;
    } else {
        if (img_info->has_alpha)
            out_fmt = BPG_OUTPUT_FORMAT_RGBA32;
        else
            out_fmt = BPG_OUTPUT_FORMAT_RGB24;
    }

    bpg_decoder_start(img, out_fmt);

    bpp = (3 + img_info->has_alpha) * (bit_depth / 8);
    row_pointer = (png_bytep)png_malloc(png_ptr, img_info->width * bpp);
    for (y = 0; y < img_info->height; y++) {
        bpg_decoder_get_line(img, row_pointer);
        png_write_row(png_ptr, row_pointer);
    }
    png_free(png_ptr, row_pointer);

    png_write_end(png_ptr, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(f);
}
#endif /* USE_PNG */

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

void decode_buffer(uint8_t* bufIn, unsigned int bufInLen, uint8_t** bufOut, unsigned int* bufOutLen){
    BPGDecoderContext *img;
    uint8_t *buf;
    int buf_len, bit_depth;

    if(NULL == bufIn || bufInLen==0 || NULL == bufOut){
        printf("Invalid input data \n");
        return;
    }

    img = bpg_decoder_open();

    if (bpg_decoder_decode(img, bufIn, bufInLen) < 0) {
        fprintf(stderr, "Could not decode image\n");
        return;
    }

#ifdef USE_PNG
    p = strrchr(outfilename, '.');
    if (p)
        p++;

    if (p && strcasecmp(p, "ppm") != 0) {
        png_save(img, outfilename, bit_depth);
    } else
#endif
    {
        ppm_save_to_buffer(img, bufOut, bufOutLen);
    }

    bpg_decoder_close(img);
}

void decode_file(char* bpgFilename, char* outFilename){
    FILE *f;
    BPGDecoderContext *img;
    uint8_t *buf;
    int buf_len, bit_depth;
    const char *outfilename, *filename, *p;

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
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

#ifdef USE_PNG
    p = strrchr(outfilename, '.');
    if (p)
        p++;

    if (p && strcasecmp(p, "ppm") != 0) {
        png_save(img, outfilename, bit_depth);
    } else
#endif
    {
        ppm_save_to_file(img, outfilename);
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

    decode_buffer(buf, buf_len, &decBuf, &decBuf_len);

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
