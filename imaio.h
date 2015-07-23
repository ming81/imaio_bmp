/*****************************************************************************/
/* imaio.h --- image file I/O libraries collection                           */
/* Copyright (C) 2015 katahiromz. All Rights Reserved.                       */
/*****************************************************************************/

#ifndef KATAHIROMZ_IMAIO_H_
#define KATAHIROMZ_IMAIO_H_ 0x002   /* Version 0.2 */

/*****************************************************************************/

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "stdint.h"     /* int??_t, uint??_t */
#ifndef __cplusplus
    #include "stdbool.h"    /* bool, true, false */
#endif

#ifdef _WIN32
    #ifndef _INC_WINDOWS
        #include <windows.h>
    #endif
#else
    #ifndef _X11_XUTIL_H_H
        #include <X11/Xutil.h>
    #endif
#endif

/*****************************************************************************/
/* C/C++ switching */

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

#ifdef _WIN32
    #ifdef IMAIO_BUILDING
        #ifdef IMAIO_DLL
            #define IMAIO_API   __declspec(dllexport)
        #else
            #define IMAIO_API   /* empty */
        #endif
    #else
        #ifdef IMAIO_DLL
            #define IMAIO_API   __declspec(dllimport)
        #else
            #define IMAIO_API   /* empty */
        #endif
    #endif

    #ifndef __GNUC__
        #ifndef IMAIO_BUILDING
            #ifdef IMAIO_DLL
                #ifdef _WIN64
                    #pragma comment(lib, "imaio64_dll.lib")
                #else
                    #pragma comment(lib, "imaio_dll.lib")
                #endif
            #else
                #ifdef _WIN64
                    #pragma comment(lib, "imaio64.lib")
                #else
                    #pragma comment(lib, "imaio.lib")
                #endif
            #endif
        #endif
        #ifndef __DMC__
            #pragma comment(lib, "msimg32.lib")
        #endif
    #endif  /* ndef __GNUC__ */
#else
    #define IMAIO_API   /* empty */
#endif  /* def _WIN32 */

/*****************************************************************************/
/* base types */

#ifdef _WIN32
    /* Windows */
    typedef HBITMAP         II_HIMAGE;      /* image handle */
    typedef BITMAP          II_IMGINFO;     /* image info */
    typedef HDC             II_DEVICE;      /* device handle */
    typedef LPVOID          II_LPVOID;      /* pointer to void */
    typedef LPCVOID         II_LPCVOID;     /* pointer to const void */
    typedef LPCSTR          II_CSTR;        /* pointer to const string */
    typedef LPCWSTR         II_CWSTR;       /* pointer to const wide string */
    typedef HINSTANCE       II_INST;        /* instance handle */
    typedef HWND            II_WND;         /* window */
    typedef RECT            II_RECT;        /* rectangle */
    typedef HANDLE          II_HFILE;       /* file handle */
#else
    /* X Window System */
    typedef XImage *        II_HIMAGE;
    typedef XImage *        II_IMGINFO;
    typedef Display *       II_DEVICE;
    typedef void *          II_LPVOID;
    typedef const void *    II_LPCVOID;
    typedef const char *    II_CSTR;
    typedef const wchar_t * II_CWSTR;
    typedef void *          II_INST;
    typedef Window          II_WND;
    typedef XRectangle      II_RECT;
    typedef void *          II_HFILE;
#endif

/*****************************************************************************/
/* inline keyword wrapper */

#ifndef ii_inline
    #ifdef __BORLANDC__
        #define ii_inline  inline
    #elif defined(__GNUC__)
        #define ii_inline  inline
    #else
        #define ii_inline  /* empty */
    #endif
#endif

/*****************************************************************************/
/* for optional parameters */

#ifdef __cplusplus
    #define ii_optional        = 0
    #define ii_optional_(def)  = def
#else
    #define ii_optional
    #define ii_optional_(def)
#endif

/*****************************************************************************/
/* IIAPI --- imaio API calling convention */

#ifdef _WIN32
    #define IIAPI   __stdcall
    #define IICAPI  __cdecl
#else
    #define IIAPI   /* empty */
    #define IICAPI  /* empty */
#endif

/*****************************************************************************/
/* flags */

typedef unsigned int II_FLAGS;

/* NOTE: II_FLAG_USE_SCREEN indicates the function uses screen image. */
#define II_FLAG_USE_SCREEN          1
/* NOTE: II_FLAG_DEFAULT_PRESENT indicates the default image of APNG exists. */
#define II_FLAG_DEFAULT_PRESENT     2

/*****************************************************************************/
/* structures */

/* color */
typedef struct II_COLOR8
{
    /* value[0: blue, 1: green, 2: red, 3:alpha] */
    uint8_t value[4];
} II_COLOR8;
/* NOTE: II_COLOR8 is compatible to RGBQUAD and COLORREF. */

/* wide color */
typedef struct II_COLOR32
{
    /* value[0: blue, 1: green, 2: red, 3:alpha] */
    int32_t value[4];
} II_COLOR32;

/* palette (color table) */
typedef struct II_PALETTE
{
    II_COLOR8  colors[256];
    int         num_colors;
} II_PALETTE;

/* animated gif frame */
typedef struct II_ANIGIF_FRAME
{
    int                 x, y;           /* position */
    int                 width, height;  /* size */
    int                 iTransparent;   /* -1 if not transparent */
    int                 disposal;       /* gif disposal method */
    int                 delay;          /* in milliseconds */
    II_PALETTE *        local_palette;  /* local color table */
    II_HIMAGE           hbmPart;        /* 8bpp part image */
    II_HIMAGE           hbmScreen;      /* must be a 32bpp or NULL */
    void *              p_user;         /* user data pointer */
    size_t              i_user;         /* user data integer */
    size_t              i_user_2;       /* user data integer 2nd */
} II_ANIGIF_FRAME;

/* animated gif */
typedef struct II_ANIGIF
{
    II_FLAGS            flags;
    int                 width;
    int                 height;
    II_PALETTE *        global_palette; /* global color table */
    int                 iBackground;    /* background color index */
    int                 num_frames;     /* the number of frames */
    II_ANIGIF_FRAME *   frames;         /* malloc'ed */
    int                 loop_count;     /* loop count (0 for infinity) */
    void *              p_user;         /* user data pointer */
    size_t              i_user;         /* user data integer */
    size_t              i_user_2;       /* user data integer 2nd */
} II_ANIGIF;

typedef struct II_MEMORY
{
    const uint8_t *     m_pb;           /* pointer to memory */
    uint32_t            m_i;            /* reading position */
    uint32_t            m_size;         /* memory size */
    void *              p_user;         /* user data pointer */
    size_t              i_user;         /* user data integer */
    size_t              i_user_2;       /* user data integer 2nd */
} II_MEMORY;

typedef struct II_APNG_FRAME
{
    II_HIMAGE       hbmScreen;      /* must be 32bpp or NULL */
    II_HIMAGE       hbmPart;        /* must be 32bpp */
    uint32_t        x_offset;
    uint32_t        y_offset;
    uint32_t        width;
    uint32_t        height;
    uint32_t        delay;          /* in milliseconds */
    uint8_t         dispose_op;
    uint8_t         blend_op;
    void *          p_user;         /* user data pointer */
    size_t          i_user;         /* user data integer */
    size_t          i_user_2;       /* user data integer 2nd */
} II_APNG_FRAME;

typedef struct II_APNG
{
    II_APNG_FRAME * frames;         /* malloc'ed */
    II_HIMAGE       hbmDefault;     /* must be 32bpp or NULL */
    uint32_t        width;
    uint32_t        height;
    uint32_t        num_frames;     /* number of frames */
    uint32_t        num_plays;      /* number of plays */
    void *          p_user;         /* user data pointer */
    size_t          i_user;         /* user data integer */
    size_t          i_user_2;       /* user data integer 2nd */
    II_FLAGS        flags;
    float           dpi;
} II_APNG;

/*****************************************************************************/
/* bitmap image manipulation */

/*
 * creation and destroying
 */
IMAIO_API II_HIMAGE IIAPI
ii_create(int width, int height,
          int bpp ii_optional_(24), const II_PALETTE *table ii_optional);

IMAIO_API II_HIMAGE IIAPI
ii_create_8bpp_solid(int cx, int cy, const II_PALETTE *table, int iColorIndex);

IMAIO_API II_HIMAGE IIAPI ii_create_8bpp_grayscale(int cx, int cy);

IMAIO_API II_HIMAGE IIAPI ii_create_24bpp(int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_create_24bpp_solid(int cx, int cy, const II_COLOR8 *color);
IMAIO_API II_HIMAGE IIAPI ii_create_24bpp_black(int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_create_24bpp_white(int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_create_24bpp_checker(int cx, int cy);

IMAIO_API II_HIMAGE IIAPI ii_create_32bpp(int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_create_32bpp_trans(int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_create_32bpp_solid(int cx, int cy, const II_COLOR8 *color);
IMAIO_API II_HIMAGE IIAPI ii_create_32bpp_black_opaque(int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_create_32bpp_white(int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_create_32bpp_checker(int cx, int cy);

IMAIO_API II_HIMAGE IIAPI ii_clone(II_HIMAGE hbm);
IMAIO_API void IIAPI      ii_destroy(II_HIMAGE hbm);

/*
 * stretching size
 */
IMAIO_API II_HIMAGE IIAPI ii_stretched(II_HIMAGE hbm, int cxNew, int cyNew);
IMAIO_API II_HIMAGE IIAPI ii_stretched_24bpp(II_HIMAGE hbm, int cxNew, int cyNew);
IMAIO_API II_HIMAGE IIAPI ii_stretched_32bpp(II_HIMAGE hbm, int cxNew, int cyNew);

/*
 * conversion
 */
IMAIO_API II_HIMAGE IIAPI ii_8bpp(II_HIMAGE hbm, int num_colors ii_optional_(256));
IMAIO_API II_HIMAGE IIAPI ii_trans_8bpp(II_HIMAGE hbm, int *pi_trans);
IMAIO_API II_HIMAGE IIAPI ii_trans_8bpp_from_32bpp(II_HIMAGE hbm32bpp, int *pi_trans);
IMAIO_API II_HIMAGE IIAPI ii_24bpp(II_HIMAGE hbm);
IMAIO_API II_HIMAGE IIAPI ii_32bpp(II_HIMAGE hbm);
IMAIO_API II_HIMAGE IIAPI ii_32bpp_from_trans_8bpp(II_HIMAGE hbm8bpp, const int *pi_trans);
IMAIO_API II_HIMAGE IIAPI ii_24bpp_or_32bpp(II_HIMAGE hbm);
IMAIO_API II_HIMAGE IIAPI ii_grayscale_8bpp(II_HIMAGE hbm);
IMAIO_API II_HIMAGE IIAPI ii_grayscale_32bpp(II_HIMAGE hbm);

/*
 * rotation or flipping
 */
IMAIO_API II_HIMAGE IIAPI ii_rotated_32bpp(II_HIMAGE hbmSrc, double angle, bool fGrow);
IMAIO_API II_HIMAGE IIAPI ii_flipped_horizontal(II_HIMAGE hbmSrc);
IMAIO_API II_HIMAGE IIAPI ii_flipped_vertical(II_HIMAGE hbmSrc);

/*
 * getting info
 */
IMAIO_API bool          IIAPI ii_get_info(II_HIMAGE hbm, II_IMGINFO *pbm);
IMAIO_API uint32_t      IIAPI ii_get_bpp(II_HIMAGE hbm);
IMAIO_API uint8_t *     IIAPI ii_get_pixels(II_HIMAGE hbm);
IMAIO_API int           IIAPI ii_get_width(II_HIMAGE hbm);
IMAIO_API int           IIAPI ii_get_height(II_HIMAGE hbm);
IMAIO_API II_PALETTE *  IIAPI ii_get_palette(II_HIMAGE hbm);
IMAIO_API bool          IIAPI ii_is_opaque(II_HIMAGE hbm);

/*
 * drawing
 */

/* for AlphaBlend API and/or layered windows */
IMAIO_API VOID IIAPI ii_premultiply(II_HIMAGE hbm32bpp);

IMAIO_API void IIAPI
ii_make_opaque(II_HIMAGE hbm32bpp, int x, int y, int cx, int cy);

IMAIO_API
void IIAPI ii_draw(
    II_DEVICE hdc, int x, int y,
    II_HIMAGE hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));
IMAIO_API
void IIAPI ii_draw_center(
    II_DEVICE hdc, int x, int y,
    II_HIMAGE hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));

IMAIO_API
void IIAPI ii_put(
    II_HIMAGE hbm, int x, int y,
    II_HIMAGE hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));
IMAIO_API
void IIAPI ii_put_center(
    II_HIMAGE hbm, int x, int y,
    II_HIMAGE hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));

IMAIO_API void IIAPI
    ii_stamp(II_HIMAGE hbm, int x, int y, II_HIMAGE hbmSrc,
        const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));
IMAIO_API void IIAPI
    ii_stamp_center(
        II_HIMAGE hbm, int x, int y, II_HIMAGE hbmSrc,
        const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));
/*
 * trimming
 */
IMAIO_API II_HIMAGE IIAPI ii_subimage(II_HIMAGE hbm, int x, int y, int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_subimage_24bpp(II_HIMAGE hbm, int x, int y, int cx, int cy);
IMAIO_API II_HIMAGE IIAPI ii_subimage_32bpp(II_HIMAGE hbm, int x, int y, int cx, int cy);

IMAIO_API II_HIMAGE IIAPI
ii_subimage_8bpp_minus(
    II_HIMAGE hbm, int x, int y, int cx, int cy, int bpp ii_optional_(8));

/*****************************************************************************/
/* screenshot */

IMAIO_API II_HIMAGE IIAPI ii_screenshot(
    II_WND window ii_optional, const II_RECT *position ii_optional);

/*****************************************************************************/
/* colors */

IMAIO_API II_COLOR8 IIAPI ii_color_trim(II_COLOR8 color);
IMAIO_API int32_t IIAPI
    ii_color_distance(const II_COLOR8 *c1, const II_COLOR8 *c2);

IMAIO_API int32_t IIAPI
ii_color_distance_alpha(const II_COLOR8 *c1, const II_COLOR8 *c2);

IMAIO_API uint8_t IIAPI ii_bound(int value);

IMAIO_API II_PALETTE * IIAPI
ii_palette_create(int num_colors, const II_COLOR8 *colors ii_optional);

IMAIO_API II_PALETTE * IIAPI ii_palette_fixed(bool web_safe ii_optional);
IMAIO_API II_PALETTE * IIAPI ii_palette_optimized(II_HIMAGE hbm, int num_colors);

IMAIO_API void IIAPI
ii_palette_shrink(II_PALETTE *table, const int *pi_trans ii_optional);

IMAIO_API II_PALETTE * IIAPI
ii_palette_for_anigif(
    II_ANIGIF *anigif, int32_t num_colors);

IMAIO_API II_PALETTE * IIAPI
ii_palette_for_pixels(int num_pixels, const uint32_t *pixels, int num_colors);

IMAIO_API void IIAPI ii_palette_destroy(II_PALETTE *palette);

IMAIO_API int IIAPI
ii_color_nearest_index(const II_PALETTE *table, const II_COLOR8 *pcolor);

IMAIO_API II_HIMAGE IIAPI ii_reduce_colors(
    II_HIMAGE hbm, const II_PALETTE *table, const int *pi_trans ii_optional);

IMAIO_API void IIAPI ii_erase_semitrans(II_HIMAGE hbm);

/*****************************************************************************/
/* alpha channel */

IMAIO_API II_HIMAGE IIAPI
    ii_alpha_channel_from_32bpp(II_HIMAGE hbm32bpp);
IMAIO_API II_HIMAGE IIAPI
    ii_add_alpha_channel(II_HIMAGE hbmAlpha, II_HIMAGE hbm);
IMAIO_API void IIAPI
    ii_store_alpha_channel(II_HIMAGE hbmAlpha, II_HIMAGE hbm32bpp);

/*****************************************************************************/
/* Windows bitmap */

IMAIO_API II_HIMAGE IIAPI ii_bmp_load_a(II_CSTR pszFileName, float *dpi ii_optional);
IMAIO_API II_HIMAGE IIAPI ii_bmp_load_w(II_CWSTR pszFileName, float *dpi ii_optional);
IMAIO_API II_HIMAGE IIAPI ii_bmp_load_common(II_HFILE hFile, II_HIMAGE hbm, float *dpi);

IMAIO_API bool IIAPI
ii_bmp_save_a(II_CSTR pszFileName, II_HIMAGE hbm, float dpi ii_optional);
IMAIO_API bool IIAPI
ii_bmp_save_w(II_CWSTR pszFileName, II_HIMAGE hbm, float dpi ii_optional);
IMAIO_API bool IIAPI
ii_bmp_save_common(II_HFILE hFile, II_HIMAGE hbm, float dpi);

/* load from resource
 *  ex) II_HIMAGE hbm = ii_bmp_load_res(hInst, MAKEINTRESOURCE(1));
 *      for resource (1 BITMAP "myfile.bmp")
 */
IMAIO_API II_HIMAGE IIAPI ii_bmp_load_res_a(II_INST hInstance, II_CSTR pszResName);
IMAIO_API II_HIMAGE IIAPI ii_bmp_load_res_w(II_INST hInstance, II_CWSTR pszResName);

#ifdef UNICODE
    #define ii_bmp_load ii_bmp_load_w
    #define ii_bmp_save ii_bmp_save_w
    #define ii_bmp_load_res ii_bmp_load_res_w
#else
    #define ii_bmp_load ii_bmp_load_a
    #define ii_bmp_save ii_bmp_save_a
    #define ii_bmp_load_res ii_bmp_load_res_a
#endif

/*****************************************************************************/
/* jpeg */

#ifndef XMD_H
    #define XMD_H
#endif
#define HAVE_BOOLEAN
typedef char ii_jpeg_boolean;
#undef boolean
#define boolean ii_jpeg_boolean
#include "jpeglib.h"
#include "jerror.h"
#undef boolean
IMAIO_API II_HIMAGE IIAPI ii_jpg_load_a(II_CSTR pszFileName, float *dpi ii_optional);
IMAIO_API II_HIMAGE IIAPI ii_jpg_load_w(II_CWSTR pszFileName, float *dpi ii_optional);

IMAIO_API bool IIAPI
ii_jpg_save_a(II_CSTR pszFileName, II_HIMAGE hbm,
                   int quality ii_optional_(100),
                   bool progression ii_optional,
                   float dpi ii_optional);
IMAIO_API bool IIAPI
ii_jpg_save_w(II_CWSTR pszFileName, II_HIMAGE hbm,
              int quality ii_optional_(100),
              bool progression ii_optional,
              float dpi ii_optional);
#ifndef __GNUC__
    #ifdef _WIN32
        #pragma comment(lib, "jpeg.lib")
    #endif
#endif

#ifdef UNICODE
    #define ii_jpg_load ii_jpg_load_w
    #define ii_jpg_save ii_jpg_save_w
#else
    #define ii_jpg_load ii_jpg_load_a
    #define ii_jpg_save ii_jpg_save_a
#endif

IMAIO_API II_HIMAGE IIAPI ii_jpg_load_common(FILE *fp, float *dpi);

IMAIO_API bool IIAPI
ii_jpg_save_common(FILE *fp, II_HIMAGE hbm,
                   int quality, bool progression, float dpi);

/*****************************************************************************/
/* gif */

#include "gif_lib.h"
IMAIO_API II_HIMAGE IIAPI ii_gif_load_8bpp_a(II_CSTR pszFileName, int *pi_trans ii_optional);
IMAIO_API II_HIMAGE IIAPI ii_gif_load_8bpp_w(II_CWSTR pszFileName, int *pi_trans ii_optional);
IMAIO_API II_HIMAGE IIAPI ii_gif_load_32bpp_a(II_CSTR pszFileName);
IMAIO_API II_HIMAGE IIAPI ii_gif_load_32bpp_w(II_CWSTR pszFileName);

IMAIO_API bool IIAPI
ii_gif_save_a(II_CSTR pszFileName, II_HIMAGE hbm8bpp,
              const int *pi_trans ii_optional);
IMAIO_API bool IIAPI
ii_gif_save_w(II_CWSTR pszFileName, II_HIMAGE hbm8bpp,
              const int *pi_trans ii_optional);
#ifndef __GNUC__
    #ifdef _WIN32
        #pragma comment(lib, "giflib.lib")
    #endif
#endif

/* load from memory */
IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_mem(II_LPCVOID pv, uint32_t cb, int *pi_trans ii_optional);

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_32bpp_mem(II_LPCVOID pv, uint32_t cb);

/* load from resource
 *  ex) II_HIMAGE hbm = ii_gif_load_8bpp_res(hInst, MAKEINTRESOURCE(1), NULL);
 *      for resource (1 GIF "myfile.gif")
 */
IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_res_a(II_INST hInstance, II_CSTR pszResName,
                       int *pi_trans ii_optional);
IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_res_w(II_INST hInstance, II_CWSTR pszResName,
                       int *pi_trans ii_optional);

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_32bpp_res_a(II_INST hInstance, II_CSTR pszResName);

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_32bpp_res_w(II_INST hInstance, II_CWSTR pszResName);

#ifdef UNICODE
    #define ii_gif_load_8bpp ii_gif_load_8bpp_w
    #define ii_gif_load_32bpp ii_gif_load_32bpp_w
    #define ii_gif_save ii_gif_save_w
    #define ii_gif_load_8bpp_res ii_gif_load_8bpp_res_w
    #define ii_gif_load_32bpp_res ii_gif_load_32bpp_res_w
#else
    #define ii_gif_load_8bpp ii_gif_load_8bpp_a
    #define ii_gif_load_32bpp ii_gif_load_32bpp_a
    #define ii_gif_save ii_gif_save_a
    #define ii_gif_load_8bpp_res ii_gif_load_8bpp_res_a
    #define ii_gif_load_32bpp_res ii_gif_load_32bpp_res_a
#endif

IMAIO_API void IIAPI
ii_gif_uninterlace(GifByteType *bits, int width, int height);

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_common(GifFileType *gif, int *pi_trans ii_optional);

IMAIO_API bool IIAPI ii_gif_save_common(
    GifFileType *gif, II_HIMAGE hbm8bpp, const int *pi_trans ii_optional);

/*****************************************************************************/
/* animated gif */

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_a(II_CSTR pszFileName, II_FLAGS flags);
IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_w(II_CWSTR pszFileName, II_FLAGS flags);

IMAIO_API bool IIAPI
ii_anigif_save_a(II_CSTR pszFileName, II_ANIGIF *anigif);

IMAIO_API bool IIAPI
ii_anigif_save_w(II_CWSTR pszFileName, II_ANIGIF *anigif);

IMAIO_API II_ANIGIF * IIAPI ii_anigif_load_mem(
    II_LPCVOID pv, uint32_t cb, II_FLAGS flags);

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_res_a(
    II_INST hInstance, II_CSTR pszResName, II_FLAGS flags);

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_res_w(
    II_INST hInstance, II_CWSTR pszResName, II_FLAGS flags);

IMAIO_API void IIAPI
ii_anigif_destroy(II_ANIGIF *anigif);

#ifdef UNICODE
    #define ii_anigif_load ii_anigif_load_w
    #define ii_anigif_save ii_anigif_save_w
    #define ii_anigif_load_res ii_anigif_load_res_w
#else
    #define ii_anigif_load ii_anigif_load_a
    #define ii_anigif_save ii_anigif_save_a
    #define ii_anigif_load_res ii_anigif_load_res_a
#endif

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_common(GifFileType *gif, II_FLAGS flags);
IMAIO_API bool IIAPI
ii_anigif_save_common(GifFileType *gif, II_ANIGIF *anigif);

/*****************************************************************************/
/* png */

#include "png.h"
#include "zlib.h"

IMAIO_API II_HIMAGE IIAPI
ii_png_load_a(II_CSTR pszFileName, float *dpi ii_optional);

IMAIO_API II_HIMAGE IIAPI
ii_png_load_w(II_CWSTR pszFileName, float *dpi ii_optional);

IMAIO_API bool IIAPI
ii_png_save_a(II_CSTR pszFileName, II_HIMAGE hbm, float dpi ii_optional);

IMAIO_API bool IIAPI
ii_png_save_w(II_CWSTR pszFileName, II_HIMAGE hbm, float dpi ii_optional);

#ifndef __GNUC__
    #ifdef _WIN32
        #ifdef ZLIB_DLL
            #pragma comment(lib, "zdll.lib")
        #else
            #pragma comment(lib, "zlib.lib")
        #endif
        #pragma comment(lib, "zlib.lib")
        #pragma comment(lib, "libpng.lib")
    #endif
#endif

/* load from memory */
IMAIO_API II_HIMAGE IIAPI ii_png_load_mem(II_LPCVOID pv, uint32_t cb);

/* load from resource
 *  ex) II_HIMAGE hbm = ii_png_load_res(hInst, MAKEINTRESOURCE(1));
 *      for resource (1 PNG "myfile.png")
 */
IMAIO_API II_HIMAGE IIAPI
ii_png_load_res_a(II_INST hInstance, II_CSTR pszResName);

IMAIO_API II_HIMAGE IIAPI
 ii_png_load_res_w(II_INST hInstance, II_CWSTR pszResName);

#ifdef UNICODE
    #define ii_png_load ii_png_load_w
    #define ii_png_save ii_png_save_w
    #define ii_png_load_res ii_png_load_res_w
#else
    #define ii_png_load ii_png_load_a
    #define ii_png_save ii_png_save_a
    #define ii_png_load_res ii_png_load_res_a
#endif

IMAIO_API II_HIMAGE IIAPI ii_png_load_common(FILE *inf, float *dpi);
IMAIO_API bool IIAPI ii_png_save_common(FILE *outf, II_HIMAGE hbm, float dpi);

/*****************************************************************************/
/* animated PNG (APNG) */

#ifdef PNG_APNG_SUPPORTED
    IMAIO_API II_APNG * IIAPI
    ii_apng_load_a(II_CSTR pszFileName,
                   II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_w(II_CWSTR pszFileName,
                   II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_res_a(II_INST hInstance, II_CSTR pszResName,
                       II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_res_w(II_INST hInstance, II_CWSTR pszResName,
                       II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_mem(II_LPCVOID pv, uint32_t cb,
                     II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

    IMAIO_API bool IIAPI ii_apng_save_a(II_CSTR pszFileName, II_APNG *apng);
    IMAIO_API bool IIAPI ii_apng_save_w(II_CWSTR pszFileName, II_APNG *apng);

    IMAIO_API void IIAPI ii_apng_destroy(II_APNG *apng);

    IMAIO_API II_APNG * IIAPI ii_apng_load_fp(FILE *fp, II_FLAGS flags);
    IMAIO_API bool IIAPI ii_apng_save_fp(FILE *fp, II_APNG *apng);

    #ifdef UNICODE
        #define ii_apng_load ii_apng_load_w
        #define ii_apng_load_res ii_apng_load_res_w
        #define ii_apng_save ii_apng_save_w
    #else
        #define ii_apng_load ii_apng_load_a
        #define ii_apng_load_res ii_apng_load_res_a
        #define ii_apng_save ii_apng_save_a
    #endif

    IMAIO_API II_HIMAGE IIAPI
    ii_image_from_32bpp_rows(int width, int height, png_bytepp rows);

    IMAIO_API void IIAPI
    ii_32bpp_rows_from_image(png_bytepp rows, II_HIMAGE hbmImage);

    IMAIO_API II_APNG * IIAPI
    ii_apng_from_anigif(II_ANIGIF *anigif);

    IMAIO_API II_ANIGIF * IIAPI
    ii_anigif_from_apng(II_APNG *apng, bool kill_semitrans ii_optional_(true));
#endif  /* def PNG_APNG_SUPPORTED */

/*****************************************************************************/
/* tiff */

#include "tiffio.h"

IMAIO_API II_HIMAGE IIAPI
ii_tif_load_a(II_CSTR pszFileName, float *dpi ii_optional);

IMAIO_API II_HIMAGE IIAPI
ii_tif_load_w(II_CWSTR pszFileName, float *dpi ii_optional);

IMAIO_API bool IIAPI
ii_tif_save_a(II_CSTR pszFileName, II_HIMAGE hbm, float dpi ii_optional);

IMAIO_API bool IIAPI
ii_tif_save_w(II_CWSTR pszFileName, II_HIMAGE hbm, float dpi ii_optional);

#ifndef __GNUC__
    #ifdef _WIN32
        #pragma comment(lib, "libtiff.lib")
    #endif
#endif

#ifdef UNICODE
    #define ii_tif_load ii_tif_load_w
    #define ii_tif_save ii_tif_save_w
#else
    #define ii_tif_load ii_tif_load_a
    #define ii_tif_save ii_tif_save_a
#endif

IMAIO_API II_HIMAGE IIAPI ii_tif_load_common(TIFF *tif, float *dpi);
IMAIO_API bool IIAPI ii_tif_save_common(TIFF *tif, II_HIMAGE hbm, float dpi);

/*****************************************************************************/
/* image types */

typedef enum II_IMAGE_TYPE
{
    II_IMAGE_TYPE_INVALID,  /* not supported */
    II_IMAGE_TYPE_JPG,      /* JPEG */
    II_IMAGE_TYPE_GIF,      /* GIF */
    II_IMAGE_TYPE_ANIGIF,   /* animated GIF */
    II_IMAGE_TYPE_PNG,      /* PNG */
    II_IMAGE_TYPE_APNG,     /* animated PNG */
    II_IMAGE_TYPE_TIF,      /* TIFF */
    II_IMAGE_TYPE_BMP       /* BMP */
} II_IMAGE_TYPE;

/* image type from path name or dotext */
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_path_name_a(II_CSTR  pszFileName);
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_path_name_w(II_CWSTR pszFileName);
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_dotext_a(II_CSTR  pchDotExt);
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_dotext_w(II_CWSTR pchDotExt);

/* wildcards from image type */
IMAIO_API II_CSTR       IIAPI ii_wildcards_from_image_type_a(II_IMAGE_TYPE type);
IMAIO_API II_CWSTR      IIAPI ii_wildcards_from_image_type_w(II_IMAGE_TYPE type);

/* file title and dotext */
IMAIO_API II_CSTR       IIAPI ii_find_file_title_a(II_CSTR  pszPath);
IMAIO_API II_CWSTR      IIAPI ii_find_file_title_w(II_CWSTR pszPath);
IMAIO_API II_CSTR       IIAPI ii_find_dotext_a(II_CSTR  pszPath);
IMAIO_API II_CWSTR      IIAPI ii_find_dotext_w(II_CWSTR pszPath);

/* MIME */
IMAIO_API II_CSTR       IIAPI ii_mime_from_path_name_a(II_CSTR  pszFileName);
IMAIO_API II_CWSTR      IIAPI ii_mime_from_path_name_w(II_CWSTR pszFileName);
IMAIO_API II_CSTR       IIAPI ii_mime_from_dotext_a(II_CSTR  pchDotExt);
IMAIO_API II_CWSTR      IIAPI ii_mime_from_dotext_w(II_CWSTR pchDotExt);
IMAIO_API II_CSTR       IIAPI ii_dotext_from_mime_a(II_CSTR  pszMIME);
IMAIO_API II_CWSTR      IIAPI ii_dotext_from_mime_w(II_CWSTR pszMIME);
IMAIO_API II_CSTR       IIAPI ii_mime_from_image_type_a(II_IMAGE_TYPE type);
IMAIO_API II_CWSTR      IIAPI ii_mime_from_image_type_w(II_IMAGE_TYPE type);
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_mime_a(II_CSTR  pszMIME);
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_mime_w(II_CWSTR pszMIME);

/* make a filter string for "Open" / "Save As" dialog */
IMAIO_API char *        IIAPI ii_make_filter_a(char *     pszFilter);
IMAIO_API wchar_t *     IIAPI ii_make_filter_w(wchar_t *  pszFilter);

#ifdef UNICODE
    #define ii_image_type_from_path_name ii_image_type_from_path_name_w
    #define ii_image_type_from_dotext ii_image_type_from_dotext_w
    #define ii_wildcards_from_image_type ii_wildcards_from_image_type_w
    #define ii_find_file_title ii_find_file_title_w
    #define ii_find_dotext ii_find_dotext_w
    #define ii_mime_from_path_name ii_mime_from_path_name_w
    #define ii_mime_from_dotext ii_mime_from_dotext_w
    #define ii_dotext_from_mime ii_dotext_from_mime_w
    #define ii_mime_from_image_type ii_mime_from_image_type_w
    #define ii_image_type_from_mime ii_image_type_from_mime_w
    #define ii_make_filter ii_make_filter_w
#else
    #define ii_image_type_from_path_name ii_image_type_from_path_name_a
    #define ii_image_type_from_dotext ii_image_type_from_dotext_a
    #define ii_wildcards_from_image_type ii_wildcards_from_image_type_a
    #define ii_find_file_title ii_find_file_title_a
    #define ii_find_dotext ii_find_dotext_a
    #define ii_mime_from_path_name ii_mime_from_path_name_a
    #define ii_mime_from_dotext ii_mime_from_dotext_a
    #define ii_dotext_from_mime ii_dotext_from_mime_a
    #define ii_mime_from_image_type ii_mime_from_image_type_a
    #define ii_image_type_from_mime ii_image_type_from_mime_a
    #define ii_make_filter ii_make_filter_a
#endif

/*****************************************************************************/
/* C/C++ switching */

#ifdef __cplusplus
} /* extern "C" */
#endif

/*****************************************************************************/

#endif  /* ndef KATAHIROMZ_IMAIO_H_ */
