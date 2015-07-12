/*****************************************************************************/
/* imaio_xws.c --- imaio for X Window System                                 */
/* Copyright (C) 2015 katahiromz. All Rights Reserved.                       */
/*****************************************************************************/

#define IMAIO_BUILDING 1

#include "imaio.h"

#include <assert.h>
#include <math.h>
#include <fcntl.h>
#include <io.h>

/*****************************************************************************/

/* TODO: Please port imaio to X Window System */

/*****************************************************************************/
/* C/C++ switching */

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

IMAIO_API bool ii_get_info(II_HIMAGE hbm, II_IMGINFO *pbm)
{
    assert(hbm);
    if (hbm)
    {
        *pbm = hbm;
        return true;
    }
    return false;
}

IMAIO_API uint32_t ii_get_bpp(II_HIMAGE hbm)
{
    assert(hbm);
    if (hbm)
    {
        return hbm->bits_per_pixel;
    }
    return 0;
}

IMAIO_API uint8_t *ii_get_pixels(II_HIMAGE hbm)
{
    assert(hbm);
    if (hbm)
    {
        return (uint8_t *)hbm->data;
    }
    return NULL;
}

IMAIO_API int ii_get_width(II_HIMAGE hbm)
{
    assert(hbm);
    if (hbm)
    {
        return hbm->width;
    }
    return 0;
}

IMAIO_API int ii_get_height(II_HIMAGE hbm)
{
    assert(hbm);
    if (hbm)
    {
        return hbm->height;
    }
    return 0;
}

IMAIO_API bool ii_is_opaque(II_HIMAGE hbm)
{
    /* TODO: */
    return false;
}

IMAIO_API void ii_destroy(II_HIMAGE hbm)
{
    assert(hbm);
    if (hbm)
    {
        XDestroyImage(hbm);
    }
}

IMAIO_API II_HIMAGE ii_jpg_load_a(II_CSTR pszFileName, float *dpi)
{
    /* FIXME */
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE * infile;
    JSAMPARRAY buffer, *data_dst;
    int row_stride;
    infile = fopen(filename, "rb");
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
  data_src = buffer[0];
  data_dst = image->data;
  memset(data_dst, 0xff, 4 * image->width * image->height);
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
}

IMAIO_API
bool ii_jpg_save_a(II_CSTR pszFileName, II_HIMAGE hbm,
                    int quality, bool progression, float dpi)
{
    /* FIXME */
    struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
FILE * outfile;
    JSAMPROW row_pointer;
    int row_stride;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);;
    outfile = fopen(filename, "wb");
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = image_width * 3;
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
}

IMAIO_API uint8_t ii_bound(int value)
{
    if (value > 255)
        return 255;
    if (value < 0)
        return 0;
    return value;
}

IMAIO_API II_HIMAGE ii_clone(II_HIMAGE hbm)
{
    assert(hbm);
    hbm = XSubImage(hbm, 0, 0, hbm->width, hbm->height);
    assert(hbm);
    return hbm;
}

IMAIO_API II_HIMAGE ii_subimage(II_HIMAGE hbm, int x, int y, int cx, int cy)
{
    assert(hbm);
    hbm = XSubImage(hbm, x, y, cx, cy);
    assert(hbm);
    return hbm;
}

/*****************************************************************************/
/* C/C++ switching */

#ifdef __cplusplus
} /* extern "C" */
#endif

/*****************************************************************************/
