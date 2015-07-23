/*****************************************************************************/
/* imaio_win.c --- imaio for Windows                                         */
/* Copyright (C) 2015 katahiromz. All Rights Reserved.                       */
/*****************************************************************************/

#define IMAIO_BUILDING 1

#include "imaio.h"

#include <tchar.h>
#include <string.h>
#include <mbstring.h>
#include <assert.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

/*****************************************************************************/

#define II_WIDTHBYTES(i) (((i) + 31) / 32 * 4)

typedef struct tagII_BITMAPINFOEX
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[256];
} II_BITMAPINFOEX, FAR * LPII_BITMAPINFOEX;

#ifndef LR_LOADREALSIZE
    #define LR_LOADREALSIZE 128
#endif

/*****************************************************************************/
/* C/C++ switching */

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

IMAIO_API II_HIMAGE IIAPI
ii_create(int width, int height, int bpp, const II_PALETTE *table)
{
    II_BITMAPINFOEX bi;
    II_HIMAGE hbmNew;
    LPVOID pvBits;
    int i;
    HDC hdc;

    assert(width > 0);
    assert(height > 0);
    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = bpp;
    if (table)
    {
        for (i = 0; i < table->num_colors; ++i)
        {
            bi.bmiColors[i].rgbBlue = table->colors[i].value[0];
            bi.bmiColors[i].rgbGreen = table->colors[i].value[1];
            bi.bmiColors[i].rgbRed = table->colors[i].value[2];
            bi.bmiColors[i].rgbReserved = 0;
        }
    }
    hdc = CreateCompatibleDC(NULL);
    hbmNew = CreateDIBSection(hdc, (LPBITMAPINFO)&bi, DIB_RGB_COLORS,
                              &pvBits, NULL, 0);
    DeleteDC(hdc);
    assert(hbmNew);
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_8bpp_solid(int cx, int cy, const II_PALETTE *table, int iColorIndex)
{
    II_HIMAGE hbmNew;
    II_IMGINFO bm;

    assert(table);
    hbmNew = ii_create(cx, cy, 8, table);
    assert(hbmNew);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        FillMemory(bm.bmBits, bm.bmWidthBytes * cy, (uint8_t)iColorIndex);
    }
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_8bpp_grayscale(int cx, int cy)
{
    II_PALETTE *table;
    int i;
    II_HIMAGE hbmNew;

    table = ii_palette_create(256, NULL);
    if (table == NULL)
        return NULL;

    for (i = 0; i < 256; ++i)
    {
        table->colors[i].value[0] = (uint8_t)i;
        table->colors[i].value[1] = (uint8_t)i;
        table->colors[i].value[2] = (uint8_t)i;
    }

    hbmNew = ii_create(cx, cy, 8, table);
    ii_palette_destroy(table);

    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_24bpp(int cx, int cy)
{
    II_HIMAGE hbmNew;
    hbmNew = ii_create(cx, cy, 24, NULL);
    assert(hbmNew);
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_24bpp_solid(int cx, int cy, const II_COLOR8 *color)
{
    II_HIMAGE hbmNew;
    LPBYTE pbBits;
    II_IMGINFO bm;
    int x, y;

    hbmNew = ii_create(cx, cy, 24, NULL);
    assert(hbmNew);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pbBits = (LPBYTE)bm.bmBits;
        for (y = 0; y < cy; ++y)
        {
            for (x = 0; x < cx; ++x)
            {
                pbBits[x * 3 + y * bm.bmWidthBytes + 0] = color->value[0];
                pbBits[x * 3 + y * bm.bmWidthBytes + 1] = color->value[1];
                pbBits[x * 3 + y * bm.bmWidthBytes + 2] = color->value[2];
            }
        }
    }
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_32bpp(int cx, int cy)
{
    II_HIMAGE hbm;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create(cx, cy, 32, NULL);
    assert(hbm);
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_32bpp_trans(int cx, int cy)
{
    II_HIMAGE hbm;
    II_IMGINFO bm;
    uint32_t cb;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create(cx, cy, 32, NULL);
    assert(hbm);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    ZeroMemory(bm.bmBits, cb);
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_32bpp_solid(int cx, int cy, const II_COLOR8 *color)
{
    II_HIMAGE hbmNew;
    II_IMGINFO bm;
    DWORD dw, cdw;
    LPDWORD pdw;

    hbmNew = ii_create(cx, cy, 32, NULL);
    assert(hbmNew);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pdw = (LPDWORD)bm.bmBits;
        cdw = cx * cy;
        while (cdw--)
        {
            dw = color->value[3];
            dw <<= 8;
            dw |= color->value[2];
            dw <<= 8;
            dw |= color->value[1];
            dw <<= 8;
            dw |= color->value[0];
            *pdw++ = dw;
        }
    }
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_24bpp_black(int cx, int cy)
{
    II_HIMAGE hbm;
    II_IMGINFO bm;
    uint32_t cb;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_24bpp(cx, cy);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    ZeroMemory(bm.bmBits, cb);
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_32bpp_black_opaque(int cx, int cy)
{
    II_HIMAGE hbm;
    II_IMGINFO bm;
    LPBYTE pb;
    uint32_t cb, i;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_32bpp(cx, cy);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    pb = (LPBYTE)bm.bmBits;
    for (i = 0; i < cb; i += 4)
    {
        pb++; pb++; pb++;
        *pb++ = 0xFF;
    }
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_24bpp_white(int cx, int cy)
{
    II_HIMAGE hbm;
    II_IMGINFO bm;
    uint32_t cb;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_24bpp(cx, cy);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    FillMemory(bm.bmBits, cb, 0xFF);
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_32bpp_white(int cx, int cy)
{
    II_HIMAGE hbm;
    II_IMGINFO bm;
    uint32_t cb;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_32bpp(cx, cy);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    FillMemory(bm.bmBits, cb, 0xFF);
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_24bpp_checker(int cx, int cy)
{
    II_HIMAGE hbm;
    II_IMGINFO bm;
    LPBYTE pbBits, pb;
    int x, y;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_24bpp(cx, cy);
    if (hbm == NULL)
        return hbm;
    ii_get_info(hbm, &bm);
    pbBits = (LPBYTE)bm.bmBits;
    for (y = 0; y < cy; y++)
    {
        pb = &pbBits[y * bm.bmWidthBytes];
        for (x = 0; x < cx; x++)
        {
            if (((x >> 3) & 1) ^ (((bm.bmHeight - y - 1) >> 3) & 1))
                pb[x * 3 + 0] = pb[x * 3 + 1] = pb[x * 3 + 2] = 0x88;
            else
                pb[x * 3 + 0] = pb[x * 3 + 1] = pb[x * 3 + 2] = 0xFF;
        }
    }
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_create_32bpp_checker(int cx, int cy)
{
    II_HIMAGE hbm;
    II_IMGINFO bm;
    LPBYTE pbBits, pb;
    int x, y;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_32bpp(cx, cy);
    if (hbm == NULL)
        return hbm;
    ii_get_info(hbm, &bm);

    pbBits = (LPBYTE)bm.bmBits;
    for (y = 0; y < cy; y++)
    {
        pb = &pbBits[y * bm.bmWidthBytes];
        for (x = 0; x < cx; x++)
        {
            if (((x >> 3) & 1) ^ (((bm.bmHeight - y - 1) >> 3) & 1))
                pb[(x << 2) + 0] = pb[(x << 2) + 1] = pb[(x << 2) + 2] = 0x88;
            else
                pb[(x << 2) + 0] = pb[(x << 2) + 1] = pb[(x << 2) + 2] = 0xFF;
            pb[(x << 2) + 3] = 0xFF;
        }
    }
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_subimage_8bpp_minus(II_HIMAGE hbm, int x, int y, int cx, int cy, int bpp)
{
    II_HIMAGE hbmNew;
    II_PALETTE *table;
    int n;

    assert(hbm);
    table = ii_get_palette(hbm);

    assert(0 < bpp && bpp <= 8);
    if (table)
    {
        n = (1 << bpp);
        if (n < table->num_colors)
            table->num_colors = n;
    }

    hbmNew = ii_create(cx, cy, bpp, table);
    assert(hbmNew);
    ii_palette_destroy(table);

    ii_put(hbmNew, 0, 0, hbm, x, y, cx, cy, NULL, 255);

    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_subimage_24bpp(II_HIMAGE hbm, int x, int y, int cx, int cy)
{
    II_HIMAGE hbmNew;

    assert(hbm);
    hbmNew = ii_create_24bpp(cx, cy);
    ii_put(hbmNew, 0, 0, hbm, x, y, cx, cy, NULL, 255);

    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_subimage_32bpp(II_HIMAGE hbm, int x, int y, int cx, int cy)
{
    II_HIMAGE hbmNew, hbmSrc;

    assert(hbm);
    hbmNew = NULL;
    hbmSrc = ii_clone(hbm);
    if (hbmSrc)
    {
        ii_premultiply(hbmSrc);
        hbmNew = ii_create_32bpp(cx, cy);
        ii_put(hbmNew, 0, 0, hbmSrc, x, y, cx, cy, NULL, 255);
        ii_destroy(hbmSrc);
    }

    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_subimage(II_HIMAGE hbm, int x, int y, int cx, int cy)
{
    II_HIMAGE hbmNew;
    II_IMGINFO bm;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    switch (bm.bmBitsPixel)
    {
    case 32:
        hbmNew = ii_subimage_32bpp(hbm, x, y, cx, cy);
        break;
    case 24:
        hbmNew = ii_subimage_24bpp(hbm, x, y, cx, cy);
        break;
    default:
        hbmNew = ii_subimage_8bpp_minus(hbm, x, y, cx, cy, bm.bmBitsPixel);
    }
    assert(hbmNew);
    return hbmNew;
}

IMAIO_API void IIAPI
ii_destroy(II_HIMAGE hbm)
{
    if (hbm)
    {
        DeleteObject(hbm);
    }
}

IMAIO_API bool IIAPI
ii_get_info(II_HIMAGE hbm, II_IMGINFO *pbm)
{
    assert(hbm);
    assert(pbm);
    return GetObject(hbm, sizeof(II_IMGINFO), pbm) == sizeof(II_IMGINFO);
}

IMAIO_API uint32_t IIAPI
ii_get_bpp(II_HIMAGE hbm)
{
    II_IMGINFO bm;
    assert(hbm);
    if (ii_get_info(hbm, &bm))
        return bm.bmBitsPixel;
    return 0;
}

IMAIO_API uint8_t * IIAPI
ii_get_pixels(II_HIMAGE hbm)
{
    II_IMGINFO bm;
    assert(hbm);
    if (ii_get_info(hbm, &bm))
        return (LPBYTE)bm.bmBits;
    return NULL;
}

IMAIO_API int IIAPI
ii_get_width(II_HIMAGE hbm)
{
    II_IMGINFO bm;
    assert(hbm);
    if (ii_get_info(hbm, &bm))
        return bm.bmWidth;
    return 0;
}

IMAIO_API int IIAPI
ii_get_height(II_HIMAGE hbm)
{
    II_IMGINFO bm;
    assert(hbm);
    if (ii_get_info(hbm, &bm))
        return bm.bmHeight;
    return 0;
}

IMAIO_API II_PALETTE * IIAPI
ii_get_palette(II_HIMAGE hbm)
{
    HDC hdc;
    HGDIOBJ hbmOld;
    RGBQUAD colors[256];
    int i, num_colors;

    assert(hbm);
    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, hbm);
    num_colors = GetDIBColorTable(hdc, 0, 256, colors);
    SelectObject(hdc, hbmOld);
    DeleteDC(hdc);
    if (num_colors)
    {
        II_PALETTE *table = ii_palette_create(num_colors, NULL);
        if (table)
        {
            for (i = 0; i < num_colors; ++i)
            {
                table->colors[i].value[0] = colors[i].rgbBlue;
                table->colors[i].value[1] = colors[i].rgbGreen;
                table->colors[i].value[2] = colors[i].rgbRed;
            }
        }
        return table;
    }
    return NULL;
}

IMAIO_API bool IIAPI
ii_is_opaque(II_HIMAGE hbm)
{
    uint32_t cdw;
    uint8_t *pb;
    II_IMGINFO bm;

    assert(hbm);
    if (!ii_get_info(hbm, &bm))
        return false;

    if (bm.bmBitsPixel <= 24)
        return true;

    cdw = bm.bmWidth * bm.bmHeight;
    pb = (uint8_t *)bm.bmBits;
    while (cdw--)
    {
        pb++; pb++; pb++;
        if (*pb++ != 0xFF)
            return false;
    }
    return true;
}

IMAIO_API void IIAPI
ii_erase_semitrans(II_HIMAGE hbm)
{
    uint32_t cdw;
    uint8_t *pb;
    II_IMGINFO bm;

    assert(hbm);
    if (!ii_get_info(hbm, &bm) || bm.bmBitsPixel <= 24)
        return;

    cdw = bm.bmWidth * bm.bmHeight;
    pb = (uint8_t *)bm.bmBits;
    while (cdw--)
    {
        pb++; pb++; pb++;
        if (*pb != 0xFF)
            *pb = 0;
        pb++;
    }
}

IMAIO_API II_HIMAGE IIAPI
ii_clone(II_HIMAGE hbm)
{
    assert(hbm);
    hbm = (II_HIMAGE)CopyImage(hbm, IMAGE_BITMAP,
        0, 0, LR_CREATEDIBSECTION | LR_COPYRETURNORG);
    assert(hbm);
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_stretched(II_HIMAGE hbm, int cxNew, int cyNew)
{
    II_IMGINFO bm;

    assert(hbm);
    if (!ii_get_info(hbm, &bm))
        return NULL;

    if (bm.bmBitsPixel <= 24)
        return ii_stretched_24bpp(hbm, cxNew, cyNew);
    else
        return ii_stretched_32bpp(hbm, cxNew, cyNew);
}

IMAIO_API II_HIMAGE IIAPI
ii_stretched_24bpp(II_HIMAGE hbm, int cxNew, int cyNew)
{
    II_HIMAGE hbmNew;
    II_IMGINFO bm;
    II_DEVICE hdc1, hdc2;
    HGDIOBJ hbm1Old, hbm2Old;

    assert(cxNew > 0);
    assert(cyNew > 0);
    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbmNew = ii_create_24bpp(cxNew, cyNew);
    if (hbmNew)
    {
        hdc1 = CreateCompatibleDC(NULL);
        hdc2 = CreateCompatibleDC(NULL);
        if (hdc1 && hdc2)
        {
            hbm1Old = SelectObject(hdc1, hbm);
            hbm2Old = SelectObject(hdc2, hbmNew);

            SetStretchBltMode(hdc2, STRETCH_HALFTONE);
            StretchBlt(hdc2, 0, 0, cxNew, cyNew,
                hdc1, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

            SelectObject(hdc1, hbm1Old);
            SelectObject(hdc2, hbm2Old);
            DeleteDC(hdc1);
            DeleteDC(hdc1);
            return hbmNew;
        }
        ii_destroy(hbmNew);
    }
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_stretched_32bpp(II_HIMAGE hbm, int cxNew, int cyNew)
{
    BITMAP bm;
    HBITMAP hbmNew;
    HDC hdc;
    BITMAPINFO bi;
    BYTE *pbNewBits, *pbBits, *pbNewLine, *pbLine0, *pbLine1;
    LONG nWidthBytes, nWidthBytesNew;
    BOOL fAlpha;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbmNew = NULL;
    hdc = CreateCompatibleDC(NULL);
    nWidthBytes = bm.bmWidth * 4;
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = bm.bmWidth;
    bi.bmiHeader.biHeight = bm.bmHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    fAlpha = (bm.bmBitsPixel == 32);
    pbBits = (BYTE *)malloc(nWidthBytes * bm.bmHeight);
    if (pbBits == NULL)
    {
        DeleteDC(hdc);
        return NULL;
    }

    GetDIBits(hdc, hbm, 0, bm.bmHeight, pbBits, &bi, DIB_RGB_COLORS);
    bi.bmiHeader.biWidth = cxNew;
    bi.bmiHeader.biHeight = cyNew;
    hbmNew = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS,
                              (VOID **)&pbNewBits, NULL, 0);
    if (hbmNew)
    {
        int ix, iy, x0, y0, x1, y1;
        DWORD x, y;
        DWORD wfactor, hfactor;
        BYTE r0, g0, b0, a0, r1, g1, b1, a1;
        DWORD c00, c01, c10, c11;
        DWORD ex0, ey0, ex1, ey1;

        nWidthBytesNew = cxNew * 4;
        wfactor = (bm.bmWidth << 8) / cxNew;
        hfactor = (bm.bmHeight << 8) / cyNew;
        if (!fAlpha)
            a0 = 255;
        for (iy = 0; iy < cyNew; iy++)
        {
            y = hfactor * iy;
            y0 = y >> 8;
            y1 = min(y0 + 1, (int)bm.bmHeight - 1);
            ey1 = y & 0xFF;
            ey0 = 0x100 - ey1;
            pbNewLine = pbNewBits + iy * nWidthBytesNew;
            pbLine0 = pbBits + y0 * nWidthBytes;
            pbLine1 = pbBits + y1 * nWidthBytes;
            for (ix = 0; ix < cxNew; ix++)
            {
                x = wfactor * ix;
                x0 = x >> 8;
                x1 = min(x0 + 1, (int)bm.bmWidth - 1);
                ex1 = x & 0xFF;
                ex0 = 0x100 - ex1;
                c00 = ((LPDWORD)pbLine0)[x0];
                c01 = ((LPDWORD)pbLine1)[x0];
                c10 = ((LPDWORD)pbLine0)[x1];
                c11 = ((LPDWORD)pbLine1)[x1];

                b0 = (BYTE)(((ex0 * (c00 & 0xFF)) + 
                            (ex1 * (c10 & 0xFF))) >> 8);
                b1 = (BYTE)(((ex0 * (c01 & 0xFF)) + 
                            (ex1 * (c11 & 0xFF))) >> 8);
                g0 = (BYTE)(((ex0 * ((c00 >> 8) & 0xFF)) + 
                            (ex1 * ((c10 >> 8) & 0xFF))) >> 8);
                g1 = (BYTE)(((ex0 * ((c01 >> 8) & 0xFF)) + 
                            (ex1 * ((c11 >> 8) & 0xFF))) >> 8);
                r0 = (BYTE)(((ex0 * ((c00 >> 16) & 0xFF)) + 
                            (ex1 * ((c10 >> 16) & 0xFF))) >> 8);
                r1 = (BYTE)(((ex0 * ((c01 >> 16) & 0xFF)) + 
                            (ex1 * ((c11 >> 16) & 0xFF))) >> 8);
                b0 = (BYTE)((ey0 * b0 + ey1 * b1) >> 8);
                g0 = (BYTE)((ey0 * g0 + ey1 * g1) >> 8);
                r0 = (BYTE)((ey0 * r0 + ey1 * r1) >> 8);

                if (fAlpha)
                {
                    a0 = (BYTE)(((ex0 * ((c00 >> 24) & 0xFF)) + 
                                (ex1 * ((c10 >> 24) & 0xFF))) >> 8);
                    a1 = (BYTE)(((ex0 * ((c01 >> 24) & 0xFF)) + 
                                (ex1 * ((c11 >> 24) & 0xFF))) >> 8);
                    a0 = (BYTE)((ey0 * a0 + ey1 * a1) >> 8);
                }
                ((LPDWORD)pbNewLine)[ix] = 
                    MAKELONG(MAKEWORD(b0, g0), MAKEWORD(r0, a0));
            }
        }
    }
    free(pbBits);
    DeleteDC(hdc);
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_24bpp(II_HIMAGE hbm)
{
    II_IMGINFO bm;
    II_HIMAGE hbmNew;
    II_DEVICE hdc1, hdc2;
    HGDIOBJ hbm1Old, hbm2Old;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    if (bm.bmBitsPixel == 24)
        return ii_clone(hbm);

    hdc1 = CreateCompatibleDC(NULL);
    hdc2 = CreateCompatibleDC(NULL);
    hbmNew = ii_create_24bpp(bm.bmWidth, bm.bmHeight);
    if (hbmNew)
    {
        hbm1Old = SelectObject(hdc1, hbm);
        hbm2Old = SelectObject(hdc2, hbmNew);
        BitBlt(hdc2, 0, 0, bm.bmWidth, bm.bmHeight,
               hdc1, 0, 0, SRCCOPY);
        SelectObject(hdc1, hbm1Old);
        SelectObject(hdc2, hbm2Old);
    }

    DeleteDC(hdc2);
    DeleteDC(hdc1);

    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_32bpp(II_HIMAGE hbm)
{
    II_IMGINFO bm;
    II_HIMAGE hbmNew;
    II_DEVICE hdc1, hdc2;
    HGDIOBJ hbm1Old, hbm2Old;
    LPBYTE pb;
    uint32_t cdw;

    hbmNew = NULL;
    if (!ii_get_info(hbm, &bm))
        return hbmNew;

    if (bm.bmBitsPixel == 32)
    {
        return ii_clone(hbm);
    }

    hdc1 = CreateCompatibleDC(NULL);
    hdc2 = CreateCompatibleDC(NULL);
    hbmNew = ii_create_32bpp(bm.bmWidth, bm.bmHeight);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);

        hbm1Old = SelectObject(hdc1, hbm);
        hbm2Old = SelectObject(hdc2, hbmNew);
        BitBlt(hdc2, 0, 0, bm.bmWidth, bm.bmHeight,
               hdc1, 0, 0, SRCCOPY);
        SelectObject(hdc1, hbm1Old);
        SelectObject(hdc2, hbm2Old);

        pb = (LPBYTE)bm.bmBits;
        cdw = bm.bmWidth * bm.bmHeight;
        while (cdw--)
        {
            pb++; pb++; pb++;
            *pb++ = 0xFF;
        }
    }
    DeleteDC(hdc2);
    DeleteDC(hdc1);

    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_32bpp_from_trans_8bpp(II_HIMAGE hbm8bpp, const int *pi_trans)
{
    II_HIMAGE hbmNew;
    II_IMGINFO bm, bmNew;
    LPBYTE pb, pbNew;
    int x, y;

    assert(hbm8bpp);
    if (pi_trans == NULL || *pi_trans == -1)
    {
        return ii_32bpp(hbm8bpp);
    }

    hbmNew = ii_32bpp(hbm8bpp);
    if (hbmNew)
    {
        ii_get_info(hbm8bpp, &bm);
        pb = (LPBYTE)bm.bmBits;
        if (bm.bmBitsPixel != 8)
        {
            /* not 8bpp */
            DeleteObject(hbmNew);
            return NULL;
        }

        ii_get_info(hbmNew, &bmNew);
        pbNew = (LPBYTE)bmNew.bmBits;
        for (y = 0; y < bm.bmHeight; ++y)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                if (pb[x + y * bm.bmWidthBytes] == *pi_trans)
                {
                    pbNew[((x + y * bm.bmWidth) << 2) + 3] = 0;
                }
            }
        }
    }
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_24bpp_or_32bpp(II_HIMAGE hbm)
{
    II_IMGINFO bm;

    if (ii_get_info(hbm, &bm))
    {
        if (bm.bmBitsPixel == 32)
            return ii_clone(hbm);
        else
            return ii_24bpp(hbm);
    }
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_grayscale_8bpp(II_HIMAGE hbm)
{
    II_IMGINFO bm;
    int x, y;
    LPBYTE pbBits;
    II_HIMAGE hbmNew;
    II_DEVICE hdc;
    HGDIOBJ hbmOld;
    COLORREF rgb;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbmNew = ii_create_8bpp_grayscale(bm.bmWidth, bm.bmHeight);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pbBits = (LPBYTE)bm.bmBits;

        hdc = CreateCompatibleDC(NULL);
        hbmOld = SelectObject(hdc, hbm);
        for (y = 0; y < bm.bmHeight; ++y)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                rgb = GetPixel(hdc, x, bm.bmHeight - y - 1);
                pbBits[x + y * bm.bmWidthBytes] = (uint8_t)(
                    (
                        GetRValue(rgb) +
                        GetGValue(rgb) +
                        GetBValue(rgb)
                    ) / 3
                );
            }
        }
        SelectObject(hdc, hbmOld);
        DeleteDC(hdc);
    }
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_grayscale_32bpp(II_HIMAGE hbm)
{
    II_IMGINFO bm;
    II_HIMAGE hbmNew;
    II_DEVICE hdc;
    HGDIOBJ hbmOld;
    uint8_t b, alpha;
    uint32_t dw;
    LPDWORD pdw;
    int x, y;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbmNew = ii_32bpp(hbm);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pdw = (LPDWORD)bm.bmBits;

        hdc = CreateCompatibleDC(NULL);
        hbmOld = SelectObject(hdc, hbm);
        for (y = 0; y < bm.bmHeight; ++y)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                dw = pdw[x + y * bm.bmWidth];
                b = (uint8_t)(
                    (
                        GetRValue(dw) +
                        GetGValue(dw) +
                        GetBValue(dw)
                    ) / 3
                );
                alpha = (dw >> 24);
                pdw[x + y * bm.bmWidth] = (
                    b | (b << 8) | (b << 16) | (alpha << 24)
                );
            }
        }
        SelectObject(hdc, hbmOld);
        DeleteDC(hdc);
    }
    return hbmNew;
}

IMAIO_API uint8_t IIAPI
ii_bound(int value)
{
    if (value > 255)
        return 255;
    if (value < 0)
        return 0;
    return value;
}

IMAIO_API void IIAPI
ii_premultiply(II_HIMAGE hbm32bpp)
{
    II_IMGINFO bm;
    uint32_t cdw;
    LPBYTE pb;
    uint8_t alpha;
    ii_get_info(hbm32bpp, &bm);
    if (bm.bmBitsPixel == 32)
    {
        cdw = bm.bmWidth * bm.bmHeight;
        pb = (LPBYTE) bm.bmBits;
        while (cdw--)
        {
            alpha = pb[3];
            pb[0] = (uint8_t) ((uint32_t) pb[0] * alpha / 255);
            pb[1] = (uint8_t) ((uint32_t) pb[1] * alpha / 255);
            pb[2] = (uint8_t) ((uint32_t) pb[2] * alpha / 255);
            pb += 4;
        }
    }
}

IMAIO_API II_HIMAGE IIAPI
ii_rotated_32bpp(II_HIMAGE hbmSrc, double angle, bool fGrow)
{
    II_DEVICE hdc;
    II_HIMAGE hbm;
    II_IMGINFO bm;
    BITMAPINFO bi;
    LPBYTE pbBits, pbBitsSrc;
    int32_t widthbytes, widthbytesSrc;
    int cost, sint;
    int cx, cy, x0, x1, y0, y1, px, py, qx, qy;
    uint8_t r0, g0, b0, a0, r1, g1, b1, a1;
    int mx, my;
    int x, y, ex0, ey0, ex1, ey1;

    if (!ii_get_info(hbmSrc, &bm))
        return NULL;

    if (fGrow)
    {
        cx = (int)(fabs(bm.bmWidth * cos(angle)) + fabs(bm.bmHeight * sin(angle)) + 0.5);
        cy = (int)(fabs(bm.bmWidth * sin(angle)) + fabs(bm.bmHeight * cos(angle)) + 0.5);
    }
    else
    {
        cx = bm.bmWidth;
        cy = bm.bmHeight;
    }

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth    = cx;
    bi.bmiHeader.biHeight   = cy;
    bi.bmiHeader.biPlanes   = 1;
    bi.bmiHeader.biBitCount = 32;

    widthbytesSrc = (bm.bmWidth << 2);
    widthbytes = (cx << 2);

    do
    {
        hdc = CreateCompatibleDC(NULL);
        if (hdc != NULL)
        {
            hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (VOID **)&pbBits,
                                   NULL, 0);
            if (hbm != NULL)
            {
                pbBitsSrc = (LPBYTE)malloc(widthbytesSrc * bm.bmHeight);
                if (pbBitsSrc != NULL)
                    break;
                DeleteObject(hbm);
            }
            DeleteDC(hdc);
        }
        return NULL;
    } while (0);

    px = (bm.bmWidth - 1) << 15;
    py = (bm.bmHeight - 1) << 15;
    qx = (cx - 1) << 7;
    qy = (cy - 1) << 7;
    cost = (int)(cos(angle) * 256);
    sint = (int)(sin(angle) * 256);
    bi.bmiHeader.biWidth    = bm.bmWidth;
    bi.bmiHeader.biHeight   = bm.bmHeight;
    GetDIBits(hdc, hbmSrc, 0, bm.bmHeight, pbBitsSrc, &bi, DIB_RGB_COLORS);
    if (bm.bmBitsPixel < 32)
    {
        UINT cdw = bm.bmWidth * bm.bmHeight;
        LPBYTE pb = pbBitsSrc;
        while (cdw--)
        {
            pb++;
            pb++;
            pb++;
            *pb++ = 0xFF;
        }
    }
    ZeroMemory(pbBits, widthbytes * cy);

    x = (0 - qx) * cost + (0 - qy) * sint + px;
    y = -(0 - qx) * sint + (0 - qy) * cost + py;
    for (my = 0; my < cy; my++)
    {
        /* x = (0 - qx) * cost + ((my << 8) - qy) * sint + px; */
        /* y = -(0 - qx) * sint + ((my << 8) - qy) * cost + py; */
        for (mx = 0; mx < cx; mx++)
        {
            /* x = ((mx << 8) - qx) * cost + ((my << 8) - qy) * sint + px; */
            /* y = -((mx << 8) - qx) * sint + ((my << 8) - qy) * cost + py; */
            x0 = x >> 16;
            x1 = min(x0 + 1, (int)bm.bmWidth - 1);
            ex1 = x & 0xFFFF;
            ex0 = 0x10000 - ex1;
            y0 = y >> 16;
            y1 = min(y0 + 1, (int)bm.bmHeight - 1);
            ey1 = y & 0xFFFF;
            ey0 = 0x10000 - ey1;
            if (0 <= x0 && x0 < bm.bmWidth && 0 <= y0 && y0 < bm.bmHeight)
            {
                uint32_t c00 = *(uint32_t *)&pbBitsSrc[(x0 << 2) + y0 * widthbytesSrc];
                uint32_t c01 = *(uint32_t *)&pbBitsSrc[(x0 << 2) + y1 * widthbytesSrc];
                uint32_t c10 = *(uint32_t *)&pbBitsSrc[(x1 << 2) + y0 * widthbytesSrc];
                uint32_t c11 = *(uint32_t *)&pbBitsSrc[(x1 << 2) + y1 * widthbytesSrc];
                b0 = (uint8_t)(((ex0 * (c00 & 0xFF)) + (ex1 * (c10 & 0xFF))) >> 16);
                b1 = (uint8_t)(((ex0 * (c01 & 0xFF)) + (ex1 * (c11 & 0xFF))) >> 16);
                g0 = (uint8_t)(((ex0 * ((c00 >> 8) & 0xFF)) + (ex1 * ((c10 >> 8) & 0xFF))) >> 16);
                g1 = (uint8_t)(((ex0 * ((c01 >> 8) & 0xFF)) + (ex1 * ((c11 >> 8) & 0xFF))) >> 16);
                r0 = (uint8_t)(((ex0 * ((c00 >> 16) & 0xFF)) + (ex1 * ((c10 >> 16) & 0xFF))) >> 16);
                r1 = (uint8_t)(((ex0 * ((c01 >> 16) & 0xFF)) + (ex1 * ((c11 >> 16) & 0xFF))) >> 16);
                a0 = (uint8_t)(((ex0 * ((c00 >> 24) & 0xFF)) + (ex1 * ((c10 >> 24) & 0xFF))) >> 16);
                a1 = (uint8_t)(((ex0 * ((c01 >> 24) & 0xFF)) + (ex1 * ((c11 >> 24) & 0xFF))) >> 16);
                b0 = (ey0 * b0 + ey1 * b1) >> 16;
                g0 = (ey0 * g0 + ey1 * g1) >> 16;
                r0 = (ey0 * r0 + ey1 * r1) >> 16;
                a0 = (ey0 * a0 + ey1 * a1) >> 16;
                *(uint32_t *)&pbBits[(mx << 2) + my * widthbytes] =
                    MAKELONG(MAKEWORD(b0, g0), MAKEWORD(r0, a0));
            }
            x += cost << 8;
            y -= sint << 8;
        }
        x -= cx * cost << 8;
        x += sint << 8;
        y -= -cx * sint << 8;
        y += cost << 8;
    }
    free(pbBitsSrc);
    DeleteDC(hdc);
    return hbm;
}

/* three bytes */
typedef struct II_TRIBYTE
{
    uint8_t value[3];
} II_TRIBYTE;

IMAIO_API II_HIMAGE IIAPI
ii_flipped_horizontal(II_HIMAGE hbmSrc)
{
    II_IMGINFO bm;
    II_HIMAGE hbmNew;
    LPBYTE pbBits;
    int x, y;
    uint32_t dw;
    II_TRIBYTE tribytes;

    assert(hbmSrc);
    hbmNew = ii_24bpp_or_32bpp(hbmSrc);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pbBits = (LPBYTE)bm.bmBits;
        if (bm.bmBitsPixel == 32)
        {
            for (y = 0; y < bm.bmHeight; ++y)
            {
                for (x = 0; x < (bm.bmWidth >> 1); ++x)
                {
#define GETDWPIX(x, y) *(LPDWORD)&pbBits[((x) + (y) * bm.bmWidth) << 2]
                    dw = GETDWPIX(x, y);
                    GETDWPIX(x, y) = GETDWPIX(bm.bmWidth - x - 1, y);
                    GETDWPIX(bm.bmWidth - x - 1, y) = dw;
#undef GETDWPIX
                }
            }
        }
        else
        {
            for (y = 0; y < bm.bmHeight; ++y)
            {
                for (x = 0; x < (bm.bmWidth >> 1); ++x)
                {
#define GETDWPIX(x, y) *(II_TRIBYTE *)&pbBits[(x) * 3 + (y) * bm.bmWidthBytes]
                    tribytes = GETDWPIX(x, y);
                    GETDWPIX(x, y) = GETDWPIX(bm.bmWidth - x - 1, y);
                    GETDWPIX(bm.bmWidth - x - 1, y) = tribytes;
#undef GETDWPIX
                }
            }
        }
    }
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_flipped_vertical(II_HIMAGE hbmSrc)
{
    II_IMGINFO bm;
    II_HIMAGE hbmNew;
    LPBYTE pbBits;
    int x, y;
    uint32_t dw;
    II_TRIBYTE tribytes;

    assert(hbmSrc);
    hbmNew = ii_24bpp_or_32bpp(hbmSrc);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pbBits = (LPBYTE)bm.bmBits;
        if (bm.bmBitsPixel == 32)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                for (y = 0; y < (bm.bmHeight >> 1); ++y)
                {
#define GETDWPIX(x, y) *(LPDWORD)&pbBits[((x) + (y) * bm.bmWidth) << 2]
                    dw = GETDWPIX(x, y);
                    GETDWPIX(x, y) = GETDWPIX(x, bm.bmHeight - y - 1);
                    GETDWPIX(x, bm.bmHeight - y - 1) = dw;
#undef GETDWPIX
                }
            }
        }
        else
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                for (y = 0; y < (bm.bmHeight >> 1); ++y)
                {
#define GETDWPIX(x, y) *(II_TRIBYTE *)&pbBits[(x) * 3 + (y) * bm.bmWidthBytes]
                    tribytes = GETDWPIX(x, y);
                    GETDWPIX(x, y) = GETDWPIX(x, bm.bmHeight - y - 1);
                    GETDWPIX(x, bm.bmHeight - y - 1) = tribytes;
#undef GETDWPIX
                }
            }
        }
    }
    return hbmNew;
}

IMAIO_API void IIAPI
ii_make_opaque(II_HIMAGE hbm32bpp, int x, int y, int cx, int cy)
{
    II_IMGINFO bm;
    DWORD dw;
    LPDWORD pdw;
    int xx, yy;

    if (!ii_get_info(hbm32bpp, &bm) || bm.bmBitsPixel != 32)
    {
        return;
    }

    pdw = (LPDWORD)bm.bmBits;
    for (yy = y; yy < y + cy; ++yy)
    {
        for (xx = x; xx < x + cx; ++xx)
        {
            dw = pdw[xx + yy * cx];
            dw |= 0xFF000000;
            pdw[xx + yy * cx] = dw;
        }
    }
}

#ifdef __DMC__
    #define AC_SRC_OVER                 0x00
    #define AC_SRC_ALPHA                0x01
    #define CAPTUREBLT                  (DWORD)0x40000000

    typedef struct _BLENDFUNCTION
    {
        BYTE   BlendOp;
        BYTE   BlendFlags;
        BYTE   SourceConstantAlpha;
        BYTE   AlphaFormat;
    } BLENDFUNCTION,*PBLENDFUNCTION;

    typedef BOOL (WINAPI *ALPHABLEND)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

    static HINSTANCE hinstMSIMG32 = NULL;
    static ALPHABLEND AlphaBlend = NULL;
#endif  /* def __DMC__ */

IMAIO_API void IIAPI
ii_draw(
    II_DEVICE hdc, int x, int y,
    II_HIMAGE hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans, BYTE bSCA)
{
    II_IMGINFO bmSrc;
    II_DEVICE hdc2;
    HBITMAP hbmNewSrc;
    HGDIOBJ hbm2Old;
    BLENDFUNCTION bf;
#ifdef __DMC__
    #ifdef IMAIO_DLL
        if (AlphaBlend == NULL)
        {
            assert(0);
            return;
        }
    #else
        if (AlphaBlend == NULL)
        {
            hinstMSIMG32 = LoadLibraryA("msimg32.dll");
            AlphaBlend =
                (ALPHABLEND)GetProcAddress(hinstMSIMG32, "AlphaBlend");
            assert(AlphaBlend);
        }
        if (AlphaBlend == NULL)
        {
            return;
        }
    #endif
#endif  /* def __DMC__ */

    if (!ii_get_info(hbmSrc, &bmSrc))
        return;

    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = bSCA;
    bf.AlphaFormat = AC_SRC_ALPHA;

    if (bmSrc.bmBitsPixel >= 24)
    {
        assert(pi_trans == NULL || *pi_trans == -1);
        hbmNewSrc = ii_32bpp(hbmSrc);
        ii_premultiply(hbmNewSrc);

        hdc2 = CreateCompatibleDC(NULL);
        hbm2Old = SelectObject(hdc2, hbmNewSrc);
        AlphaBlend(hdc, x, y, cxSrc, cySrc,
                   hdc2, xSrc, ySrc, cxSrc, cySrc, bf);
        SelectObject(hdc2, hbm2Old);
        DeleteDC(hdc2);

        ii_destroy(hbmNewSrc);
    }
    else if (bmSrc.bmBitsPixel == 8)
    {
        hbmNewSrc = ii_32bpp_from_trans_8bpp(hbmSrc, pi_trans);
        ii_premultiply(hbmNewSrc);

        hdc2 = CreateCompatibleDC(NULL);
        hbm2Old = SelectObject(hdc2, hbmNewSrc);
        AlphaBlend(hdc, x, y, cxSrc, cySrc,
                   hdc2, xSrc, ySrc, cxSrc, cySrc, bf);
        SelectObject(hdc2, hbm2Old);
        DeleteDC(hdc2);

        ii_destroy(hbmNewSrc);
    }
    else
    {
        /* other bpps are not supported yet */
        assert(0);
    }
}

IMAIO_API void IIAPI
ii_draw_center(
    II_DEVICE hdc, int x, int y,
    II_HIMAGE hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans, BYTE bSCA)
{
    x -= cxSrc / 2;
    y -= cySrc / 2;

    ii_draw(hdc, x, y, hbmSrc, xSrc, ySrc, cxSrc, cySrc, pi_trans, bSCA);
}

IMAIO_API void IIAPI
ii_put(
    II_HIMAGE hbm, int x, int y,
    II_HIMAGE hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans, BYTE bSCA)
{
    II_DEVICE hdc;
    HGDIOBJ hbmOld;

    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, hbm);
    ii_draw(hdc, x, y, hbmSrc, xSrc, ySrc, cxSrc, cySrc, pi_trans, bSCA);
    SelectObject(hdc, hbmOld);
    DeleteDC(hdc);
}

IMAIO_API void IIAPI
ii_put_center(
    II_HIMAGE hbm, int x, int y,
    II_HIMAGE hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans, BYTE bSCA)
{
    x -= cxSrc / 2;
    y -= cySrc / 2;

    ii_put(hbm, x, y, hbmSrc, xSrc, ySrc, cxSrc, cySrc, pi_trans, bSCA);
}

IMAIO_API void IIAPI
ii_stamp(II_HIMAGE hbm, int x, int y, II_HIMAGE hbmSrc,
         const int *pi_trans, BYTE bSCA)
{
    II_IMGINFO bmSrc;
    ii_get_info(hbmSrc, &bmSrc);
    ii_put(hbm, x, y, hbmSrc, 0, 0, bmSrc.bmWidth, bmSrc.bmHeight,
           pi_trans, bSCA);
}

IMAIO_API void IIAPI
ii_stamp_center(
    II_HIMAGE hbm, int x, int y, II_HIMAGE hbmSrc,
    const int *pi_trans, BYTE bSCA)
{
    II_IMGINFO bmSrc;
    ii_get_info(hbmSrc, &bmSrc);
    ii_put_center(
        hbm, x, y, hbmSrc, 0, 0, bmSrc.bmWidth, bmSrc.bmHeight,
        pi_trans, bSCA);
}

/*****************************************************************************/
/* screenshot */

IMAIO_API II_HIMAGE IIAPI
ii_screenshot(II_WND window, const II_RECT *position)
{
    HBITMAP hbmNew;
    HDC hDC, hMemDC;
    HGDIOBJ hbmOld;
    RECT rc;
    int x, y, cx, cy;

    if (window == NULL)
        window = GetDesktopWindow();

    if (position)
    {
        x = position->left;
        y = position->top;
        cx = position->right - position->left;
        cy = position->bottom - position->top;
    }
    else
    {
        GetWindowRect(window, &rc);
        x = rc.left;
        y = rc.top;
        cx = rc.right - rc.left;
        cy = rc.bottom - rc.top;
    }

    hbmNew = NULL;
    hDC = GetWindowDC(window);
    if (hDC != NULL)
    {
        hMemDC = CreateCompatibleDC(hDC);
        if (hMemDC)
        {
            hbmNew = ii_create_24bpp(cx, cy);
            if (hbmNew)
            {
                hbmOld = SelectObject(hMemDC, hbmNew);
                BitBlt(hMemDC, 0, 0, cx, cy,
                       hDC, x, y, SRCCOPY | CAPTUREBLT);
                SelectObject(hMemDC, hbmOld);
            }
            DeleteDC(hMemDC);
        }
        ReleaseDC(window, hDC);
    }
    return hbmNew;
}

/*****************************************************************************/
/* colors */

IMAIO_API int32_t IIAPI
ii_color_distance(const II_COLOR8 *c1, const II_COLOR8 *c2)
{
    int32_t diff, sum = 0;
    diff = c1->value[0] - c2->value[0];
    sum += diff * diff;
    diff = c1->value[1] - c2->value[1];
    sum += diff * diff;
    diff = c1->value[2] - c2->value[2];
    sum += diff * diff;
    return sum;
}

IMAIO_API int32_t IIAPI
ii_color_distance_alpha(const II_COLOR8 *c1, const II_COLOR8 *c2)
{
    int32_t diff, sum = 0;
    diff = c1->value[0] - c2->value[0];
    sum += diff * diff;
    diff = c1->value[1] - c2->value[1];
    sum += diff * diff;
    diff = c1->value[2] - c2->value[2];
    sum += diff * diff;
    diff = c1->value[3] - c2->value[3];
    sum += diff * diff;
    return sum;
}

IMAIO_API II_PALETTE * IIAPI
ii_palette_create(int num_colors, const II_COLOR8 *colors)
{
    II_PALETTE *table;
    table = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
    if (table == NULL)
        return NULL;

    table->num_colors = num_colors;
    if (colors)
        CopyMemory(table->colors, colors, sizeof(II_COLOR8) * num_colors);
    return table;
}

IMAIO_API II_PALETTE * IIAPI
ii_palette_fixed(bool web_safe)
{
    II_PALETTE *    table;
    int i = 0, j, rr, gg, bb;

    table = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
    if (table == NULL)
        return NULL;
    for (rr = 0; rr <= 0xFF; rr += 0x33)
    {
        for (gg = 0; gg <= 0xFF; gg += 0x33)
        {
            for (bb = 0; bb <= 0xFF; bb += 0x33)
            {
                table->colors[i].value[0] = bb;
                table->colors[i].value[1] = gg;
                table->colors[i].value[2] = rr;
                ++i;
            }
        }
    }
    assert(i == 216);
    if (!web_safe)
    {
        for (j = 0; j < 40; ++j, ++i)
        {
            table->colors[i].value[0] = j * 0xFF / 40;
            table->colors[i].value[1] = j * 0xFF / 40;
            table->colors[i].value[2] = j * 0xFF / 40;
        }
    }
    table->num_colors = i;
    assert(i == 216 || i == 256);
    return table;
}

IMAIO_API void IIAPI
ii_palette_destroy(II_PALETTE *palette)
{
    free(palette);
}

/*****************************************************************************/
/* k-means */

/* entry of k-means */
typedef struct II_KMEANS_ENTRY
{
    II_COLOR8   true_color;
    II_COLOR8   trimmed_color;
    int32_t     count;
    int32_t     i_cluster;
} II_KMEANS_ENTRY;

/* cluster of k-means */
typedef struct II_KMEANS_CLUSTER
{
    II_COLOR32 centroid;
    int         count;
} II_KMEANS_CLUSTER;

/* maximum number of clusters */
#define II_KMEANS_MAX_CLUSTER 256

/* the k-means structure */
typedef struct II_KMEANS
{
    II_KMEANS_ENTRY *   entries;
    int                 num_entries;
    II_KMEANS_CLUSTER   clusters[II_KMEANS_MAX_CLUSTER];
} II_KMEANS;

/* trim a color */
IMAIO_API II_COLOR8 IIAPI
ii_color_trim(II_COLOR8 color)
{
    II_COLOR8 trimmed_color;
    trimmed_color.value[0] = (color.value[0] & 0xF8);
    trimmed_color.value[1] = (color.value[1] & 0xF8);
    trimmed_color.value[2] = (color.value[2] & 0xF8);
    trimmed_color.value[3] = 0;
    return trimmed_color;
}

/* compare two entries */
static ii_inline int IIAPI
ii_kmeans_equal_entry(const II_KMEANS_ENTRY *e1, const II_KMEANS_ENTRY *e2)
{
    return (
        e1->trimmed_color.value[0] == e2->trimmed_color.value[0] &&
        e1->trimmed_color.value[1] == e2->trimmed_color.value[1] &&
        e1->trimmed_color.value[2] == e2->trimmed_color.value[2]
    );
}

/* get distance */
static ii_inline int IIAPI
ii_kmeans_distance(const II_KMEANS_ENTRY *e1, const II_COLOR32 *c2)
{
    int diff, sum = 0;
    diff = e1->trimmed_color.value[0] - c2->value[0];
    sum += diff * diff;
    diff = e1->trimmed_color.value[1] - c2->value[1];
    sum += diff * diff;
    diff = e1->trimmed_color.value[2] - c2->value[2];
    sum += diff * diff;
    return sum;
}

/* add entry to the k-means structure */
static int IIAPI
ii_kmeans_add_entry(II_KMEANS *kms, II_KMEANS_ENTRY *entry)
{
    int i;
    II_KMEANS_ENTRY *entries;

    for (i = 0; i < kms->num_entries; ++i)
    {
        if (ii_kmeans_equal_entry(&kms->entries[i], entry))
        {
            kms->entries[i].true_color = entry->true_color;
            kms->entries[i].count++;
            return i;
        }
    }

    entries = (II_KMEANS_ENTRY *)realloc(
        kms->entries,
        (kms->num_entries + 1) * sizeof(II_KMEANS_ENTRY));
    if (entries)
    {
        entries[kms->num_entries] = *entry;
        kms->entries = entries;
        return kms->num_entries++;
    }
    return -1;
}

/* add color to the k-means structure */
static ii_inline int IIAPI
ii_kmeans_add_color(II_KMEANS *kms, II_COLOR8 color)
{
    II_KMEANS_ENTRY entry;
    entry.true_color = color;
    entry.trimmed_color = ii_color_trim(color);
    entry.count = 1;
    return ii_kmeans_add_entry(kms, &entry);
}

/* do the k-means */
static void IIAPI
ii_kmeans(II_KMEANS *kms, int num_colors)
{
    int i, j, m;
    const int times = 2;

    assert(num_colors > 0);
    if (num_colors > 256)
        num_colors = 256;

    for (m = 0; m < times; ++m)
    {
        for (i = 0; i < kms->num_entries; ++i)
        {
            int dist, min_dist = 0x7FFFFFFF, min_j = -1;
            for (j = 0; j < num_colors; ++j)
            {
                dist = ii_kmeans_distance(
                    &kms->entries[i], &kms->clusters[j].centroid);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    min_j = j;
                }
            }
            kms->entries[i].i_cluster = min_j;
            assert(min_j != -1);
        }

        for (j = 0; j < num_colors; ++j)
        {
            kms->clusters[j].centroid.value[0] = 0;
            kms->clusters[j].centroid.value[1] = 0;
            kms->clusters[j].centroid.value[2] = 0;
            kms->clusters[j].count = 0;
            for (i = 0; i < kms->num_entries; ++i)
            {
                if (j == kms->entries[i].i_cluster)
                {
                    kms->clusters[j].centroid.value[0] +=
                        kms->entries[i].true_color.value[0] *
                            kms->entries[i].count;
                    kms->clusters[j].centroid.value[1] +=
                        kms->entries[i].true_color.value[1] *
                            kms->entries[i].count;
                    kms->clusters[j].centroid.value[2] +=
                        kms->entries[i].true_color.value[2] *
                            kms->entries[i].count;
                    kms->clusters[j].count += kms->entries[i].count;
                }
            }
            if (kms->clusters[j].count)
            {
                kms->clusters[j].centroid.value[0] /= kms->clusters[j].count;
                kms->clusters[j].centroid.value[1] /= kms->clusters[j].count;
                kms->clusters[j].centroid.value[2] /= kms->clusters[j].count;
            }
            else
            {
                kms->clusters[j].centroid.value[0] = 0;
                kms->clusters[j].centroid.value[1] = 0;
                kms->clusters[j].centroid.value[2] = 0;
            }
        }
    }
}

/*****************************************************************************/
/* colors */

IMAIO_API II_PALETTE * IIAPI
ii_palette_for_pixels(int num_pixels, const uint32_t *pixels, int num_colors)
{
    int i;
    DWORD dw;
    II_COLOR8 color;
    II_PALETTE *table;
    II_KMEANS kms;

    if (num_colors < 0 || 256 < num_colors)
        num_colors = 256;
    
    if (num_pixels <= 1)
    {
        if (num_pixels == 1)
            dw = *pixels;
        else
            dw = 0;
        table = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
        table->num_colors = 2;
        table->colors[0].value[0] = (uint8_t)(dw >> 0);
        table->colors[0].value[1] = (uint8_t)(dw >> 8);
        table->colors[0].value[2] = (uint8_t)(dw >> 16);
        table->colors[1].value[0] = 0xFF;
        table->colors[1].value[1] = 0xFF;
        table->colors[1].value[2] = 0xFF;
        return table;
    }

    /* initialize the k-means structure */
    ZeroMemory(&kms, sizeof(kms));

    for (i = 0; i < num_colors; ++i)
    {
        dw = pixels[rand() % num_pixels];
        kms.clusters[i].centroid.value[0] = (uint8_t)(dw >> 0);
        kms.clusters[i].centroid.value[1] = (uint8_t)(dw >> 8);
        kms.clusters[i].centroid.value[2] = (uint8_t)(dw >> 16);
    }

    /* store colors to the k-means structure */
    for (i = 0; i < num_pixels; ++i)
    {
        dw = pixels[i];
        color.value[0] = (uint8_t)(dw >> 0);
        color.value[1] = (uint8_t)(dw >> 8);
        color.value[2] = (uint8_t)(dw >> 16);
        ii_kmeans_add_color(&kms, color);
    }

    /* store colors to the k-means structure */
    for (i = 0; i < num_pixels; ++i)
    {
        dw = pixels[i];
        color.value[0] = (uint8_t)(dw >> 0);
        color.value[1] = (uint8_t)(dw >> 8);
        color.value[2] = (uint8_t)(dw >> 16);
        ii_kmeans_add_color(&kms, color);
    }

    /* just do it */
    ii_kmeans(&kms, num_colors);

    /* store colors to table */
    table = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
    if (table)
    {
        table->num_colors = num_colors;
        for (i = 0; i < num_colors; ++i)
        {
            table->colors[i].value[0] =
                (uint8_t)kms.clusters[i].centroid.value[0];
            table->colors[i].value[1] =
                (uint8_t)kms.clusters[i].centroid.value[1];
            table->colors[i].value[2] =
                (uint8_t)kms.clusters[i].centroid.value[2];
        }
    }

    /* release allocated memory */
    free(kms.entries);

    return table;
}

IMAIO_API II_PALETTE * IIAPI
ii_palette_optimized(II_HIMAGE hbm, int num_colors)
{
    II_HIMAGE hbmNew;
    II_IMGINFO bm;
    LPDWORD pdw;
    DWORD dw, cdw;
    uint32_t *pixels;
    int num_pixels;
    II_PALETTE *table = NULL;

    if (num_colors < 0 || 256 < num_colors)
        num_colors = 256;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    /* get smaller one */
    if (bm.bmWidth > 256)
        bm.bmWidth = 256;
    if (bm.bmHeight > 256)
        bm.bmHeight = 256;
    hbmNew = ii_stretched_32bpp(hbm, bm.bmWidth, bm.bmHeight);
    if (hbmNew)
    {
        /* get pixels */
        ii_get_info(hbmNew, &bm);
        pdw = (LPDWORD)bm.bmBits;
        cdw = bm.bmWidth * bm.bmHeight;
        pixels = calloc(sizeof(DWORD), cdw);
        if (pixels)
        {
            num_pixels = 0;
            while (cdw--)
            {
                dw = *pdw++;
                if (dw >> 24)
                {
                    pixels[num_pixels++] = (dw & 0xFFFFFF);
                }
            }

            table = ii_palette_for_pixels(num_pixels, pixels, num_colors);

            free(pixels);
        }

        ii_destroy(hbmNew);
    }

    return table;
}

IMAIO_API void IIAPI 
ii_palette_shrink(II_PALETTE *table, const int *pi_trans)
{
    int i;
    for (i = table->num_colors - 1; i > 0; --i)
    {
        II_COLOR8 *pcolor1 = &table->colors[i];
        II_COLOR8 *pcolor2 = &table->colors[i - 1];
        if (pcolor1->value[0] == pcolor2->value[0] &&
            pcolor1->value[1] == pcolor2->value[1] &&
            pcolor1->value[2] == pcolor2->value[2])
        {
            if (pi_trans && i == *pi_trans)
                break;
            --table->num_colors;
        }
        else
        {
            break;
        }
    }
}

IMAIO_API II_PALETTE * IIAPI 
ii_palette_for_anigif(II_ANIGIF *anigif, int32_t num_colors)
{
    II_HIMAGE hbm, hbmNew;
    II_IMGINFO bm;
    LPDWORD pdw;
    DWORD dw, cdw;
    uint32_t *pixels, *all_pixels, *new_all_pixels;
    int num_pixels, num_all_pixels;
    II_PALETTE *table;
    int i;

    if (num_colors < 0 || 256 < num_colors)
        num_colors = 256;

    all_pixels = NULL;
    num_all_pixels = 0;

    assert(anigif->frames);
    if (anigif->frames == NULL)
        return NULL;

    for (i = 0; i < anigif->num_frames; ++i)
    {
        hbm = anigif->frames[i].hbmScreen;
        if (hbm == NULL)
            continue;

        if (!ii_get_info(hbm, &bm))
            return NULL;

        assert(bm.bmBitsPixel == 32);

        /* get smaller one */
        if (bm.bmWidth > 256)
            bm.bmWidth = 256;
        if (bm.bmHeight > 256)
            bm.bmHeight = 256;
        hbmNew = ii_stretched_32bpp(hbm, bm.bmWidth, bm.bmHeight);
        if (hbmNew)
        {
            /* get pixels */
            ii_get_info(hbmNew, &bm);
            assert(bm.bmBitsPixel == 32);

            pdw = (LPDWORD)bm.bmBits;
            cdw = bm.bmWidth * bm.bmHeight;
            num_pixels = 0;
            pixels = (uint32_t *)calloc(sizeof(uint32_t), cdw);
            if (pixels)
            {
                while (cdw--)
                {
                    dw = *pdw++;
                    if (dw >> 24)
                    {
                        pixels[num_pixels++] = (dw & 0xFFFFFF);
                    }
                }
            }
            ii_destroy(hbmNew);

            if (pixels == NULL)
            {
                free(all_pixels);
                return NULL;
            }

            new_all_pixels =
                (uint32_t *)realloc(all_pixels,
                    (num_all_pixels + num_pixels) * sizeof(uint32_t));
            if (new_all_pixels == NULL)
            {
                free(pixels);
                free(all_pixels);
                return NULL;
            }
            all_pixels = new_all_pixels;

            CopyMemory(all_pixels + num_all_pixels, pixels,
                       num_pixels * sizeof(uint32_t));
            num_all_pixels += num_pixels;

            free(pixels);
        }
    }

    table = ii_palette_for_pixels(num_all_pixels, all_pixels, num_colors);
    assert(table);
    free(all_pixels);
    return table;
}

IMAIO_API II_HIMAGE IIAPI 
ii_reduce_colors(
    II_HIMAGE hbm,
    const II_PALETTE *table,
    const int *pi_trans)
{
    II_IMGINFO bm, bm32bpp;
    LPBYTE pb8bpp, pb32bpp;
    int x, y, index;
    II_HIMAGE hbm8bpp, hbm32bpp;

    assert(table);
    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbm32bpp = ii_32bpp(hbm);
    if (hbm32bpp == NULL)
        return NULL;
    ii_get_info(hbm32bpp, &bm32bpp);
    pb32bpp = (LPBYTE)bm32bpp.bmBits;

    hbm8bpp = ii_create(bm.bmWidth, bm.bmHeight, 8, table);
    if (hbm8bpp)
    {
        uint8_t v0[4];
        uint8_t v1[4];
        int value[4];
        int k;

        ii_get_info(hbm8bpp, &bm);
        pb8bpp = (LPBYTE)bm.bmBits;

        /* Floyd-Steinberg dithering */
        for (y = 0; y < bm.bmHeight; ++y)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
#define GETPX(x,y,i) pb32bpp[((x) << 2) + (y) * bm32bpp.bmWidthBytes + (i)]
                v0[0] = GETPX(x, y, 0);
                v0[1] = GETPX(x, y, 1);
                v0[2] = GETPX(x, y, 2);
                v0[3] = GETPX(x, y, 3);
                if (v0[3] == 0 && pi_trans && *pi_trans != -1)
                {
                    pb8bpp[x + y * bm.bmWidthBytes] = *pi_trans;
                    continue;
                }

                index = ii_color_nearest_index(table,
                    (II_COLOR8 *)
                        &pb32bpp[(x << 2) + y * bm32bpp.bmWidthBytes]);
                pb8bpp[x + y * bm.bmWidthBytes] = (uint8_t)index;

                v1[0] = table->colors[index].value[0];
                v1[1] = table->colors[index].value[1];
                v1[2] = table->colors[index].value[2];

                value[0] = (int)v0[0] - (int)v1[0];
                value[1] = (int)v0[1] - (int)v1[1];
                value[2] = (int)v0[2] - (int)v1[2];
                GETPX(x, y, 0) = v1[0];
                GETPX(x, y, 1) = v1[1];
                GETPX(x, y, 2) = v1[2];

                for (k = 0; k < 3; ++k)
                {
                    if (x + 1 < bm.bmWidth)
                    {
                        GETPX(x + 1, y + 0, k) =
                            ii_bound(GETPX(x + 1, y + 0, k) + value[k] * 7 / 16);
                    }
                    if (x > 0 && y + 1 < bm.bmHeight)
                    {
                        GETPX(x - 1, y + 1, k) =
                            ii_bound(GETPX(x - 1, y + 1, k) + value[k] * 3 / 16);
                    }
                    if (y + 1 < bm.bmHeight)
                    {
                        GETPX(x + 0, y + 1, k) =
                            ii_bound(GETPX(x + 0, y + 1, k) + value[k] * 5 / 16);
                    }
                    if (x + 1 < bm.bmWidth && y + 1 < bm.bmHeight)
                    {
                        GETPX(x + 1, y + 1, k) =
                            ii_bound(GETPX(x + 1, y + 1, k) + value[k] * 1 / 16);
                    }
                }
#undef GETPX
            }
        }
    }

    ii_destroy(hbm32bpp);
    return hbm8bpp;
}

IMAIO_API II_HIMAGE IIAPI
ii_alpha_channel_from_32bpp(II_HIMAGE hbm32bpp)
{
    II_IMGINFO  bm;
    LPDWORD     pdw;
    II_HIMAGE   hbm8bpp;
    LPBYTE      pb;
    int         x, y;
    DWORD       dw;

    if (!ii_get_info(hbm32bpp, &bm))
        return NULL;

    pdw = (LPDWORD)bm.bmBits;

    hbm8bpp = ii_create_8bpp_grayscale(bm.bmWidth, bm.bmHeight);
    if (hbm8bpp)
    {
        ii_get_info(hbm8bpp, &bm);
        pb = (LPBYTE)bm.bmBits;
        if (bm.bmBitsPixel == 32)
        {
            for (y = 0; y < bm.bmHeight; ++y)
            {
                for (x = 0; x < bm.bmWidth; ++x)
                {
                    dw = pdw[x + y * bm.bmWidth];
                    dw >>= 24;
                    pb[x + y * bm.bmWidthBytes] = (uint8_t)dw;
                }
            }
        }
        else
        {
            FillMemory(pb, bm.bmWidthBytes * bm.bmHeight, 0xFF);
        }
    }
    return hbm8bpp;
}

IMAIO_API II_HIMAGE IIAPI
ii_add_alpha_channel(II_HIMAGE hbmAlpha, II_HIMAGE hbm)
{
    II_HIMAGE hbm32bpp;

    hbm32bpp = ii_32bpp(hbm);
    if (hbm32bpp == NULL)
        return NULL;

    ii_store_alpha_channel(hbmAlpha, hbm32bpp);
    return hbm32bpp;
}

IMAIO_API void IIAPI 
ii_store_alpha_channel(II_HIMAGE hbmAlpha, II_HIMAGE hbm32bpp)
{
    II_IMGINFO  bm;
    LPDWORD     pdw;
    int         x, y;
    DWORD       dw;
    HDC         hdc;
    HGDIOBJ     hbmOld;
    COLORREF    rgb;
    BYTE        r, g, b;

    if (!ii_get_info(hbm32bpp, &bm) || bm.bmBitsPixel != 32)
        return;

    pdw = (LPDWORD)bm.bmBits;

    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, hbmAlpha);
    for (y = 0; y < bm.bmHeight; ++y)
    {
        for (x = 0; x < bm.bmWidth; ++x)
        {
            rgb = GetPixel(hdc, x, y);
            r = GetRValue(rgb);
            g = GetGValue(rgb);
            b = GetBValue(rgb);
            dw = pdw[x + y * bm.bmWidth];
            dw |= (((uint8_t)((r + g + b) / 3)) << 24);
            pdw[x + y * bm.bmWidth] = dw;
        }
    }
    SelectObject(hdc, hbmOld);
}

IMAIO_API II_HIMAGE IIAPI
ii_8bpp(II_HIMAGE hbm, int num_colors)
{
    II_HIMAGE hbmNew = NULL;
    II_PALETTE *table = ii_palette_optimized(hbm, num_colors);
    if (table)
    {
        hbmNew = ii_reduce_colors(hbm, table, NULL);
        free(table);
    }
    return hbmNew;
}

IMAIO_API II_HIMAGE IIAPI
ii_trans_8bpp(II_HIMAGE hbm, int *pi_trans)
{
    II_HIMAGE hbm8bpp, hbm32bpp;
    hbm32bpp = ii_32bpp(hbm);
    hbm8bpp = ii_trans_8bpp_from_32bpp(hbm32bpp, pi_trans);
    ii_destroy(hbm32bpp);
    return hbm8bpp;
}

IMAIO_API II_HIMAGE IIAPI
ii_trans_8bpp_from_32bpp(II_HIMAGE hbm32bpp, int *pi_trans)
{
    II_PALETTE *table;
    II_IMGINFO bm32bpp, bm8bpp;
    LPBYTE pb;
    LPDWORD pdw;
    uint32_t dw;
    int x, y;
    II_HIMAGE hbm8bpp = NULL;

    if (pi_trans)
        table = ii_palette_optimized(hbm32bpp, 255);
    else
        table = ii_palette_optimized(hbm32bpp, 256);

    if (table)
    {
        if (pi_trans)
        {
            *pi_trans = table->num_colors;
            table->num_colors++;
        }
        hbm8bpp = ii_reduce_colors(hbm32bpp, table, pi_trans);
        free(table);
        if (hbm8bpp && pi_trans)
        {
            ii_get_info(hbm8bpp, &bm8bpp);
            pb = (LPBYTE)bm8bpp.bmBits;
            ii_get_info(hbm32bpp, &bm32bpp);
            pdw = (LPDWORD)bm32bpp.bmBits;
            for (y = 0; y < bm32bpp.bmHeight; ++y)
            {
                for (x = 0; x < bm32bpp.bmWidth; ++x)
                {
                    dw = *pdw++;
                    if ((dw >> 24) == 0)
                    {
                        pb[x + y * bm8bpp.bmWidthBytes] = (uint8_t)*pi_trans;
                    }
                }
            }
        }
    }
    return hbm8bpp;
}

IMAIO_API int IIAPI
ii_color_nearest_index(const II_PALETTE *table, const II_COLOR8 *pcolor)
{
    int i, i_near;
    int32_t norm, norm_near, i_value[3], k_value[3];

    assert(table);
    if (table == NULL)
        return 0;
    i_value[0] = pcolor->value[0];
    i_value[1] = pcolor->value[1];
    i_value[2] = pcolor->value[2];

    i_near = 0;
    norm_near = 255 * 255 * 3 + 1;
    for (i = 0; i < table->num_colors; ++i)
    {
        k_value[0] = (int)table->colors[i].value[0] - i_value[0];
        k_value[1] = (int)table->colors[i].value[1] - i_value[1];
        k_value[2] = (int)table->colors[i].value[2] - i_value[2];
        norm = k_value[0] * k_value[0] +
               k_value[1] * k_value[1] +
               k_value[2] * k_value[2];
        if (norm < norm_near)
        {
            i_near = i;
            norm_near = norm;
            if (norm == 0)
                break;
        }
    }

    assert(0 <= i_near && i_near < table->num_colors);
    return i_near;
}

/*****************************************************************************/

IMAIO_API II_HIMAGE IIAPI
ii_bmp_load_common(II_HFILE hFile, HBITMAP hbm, float *dpi)
{
    BITMAPFILEHEADER bf;
    II_BITMAPINFOEX bi;
    DWORD cb, cbImage;
    LPVOID pBits, pBits2;
    II_DEVICE hDC, hMemDC;

    if (!ReadFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL))
    {
        CloseHandle(hFile);
        return hbm;
    }

    pBits = NULL;
    if (bf.bfType == 0x4D42 && bf.bfReserved1 == 0 && bf.bfReserved2 == 0 &&
        bf.bfSize > bf.bfOffBits && bf.bfOffBits > sizeof(BITMAPFILEHEADER) &&
        bf.bfOffBits <= sizeof(BITMAPFILEHEADER) + sizeof(II_BITMAPINFOEX))
    {
        if (ReadFile(hFile, &bi, bf.bfOffBits -
                     sizeof(BITMAPFILEHEADER), &cb, NULL))
        {
            if (dpi)
                *dpi = (float)(bi.bmiHeader.biXPelsPerMeter * 2.54 / 100.0);

            cbImage = bf.bfSize - bf.bfOffBits;
            pBits = malloc(cbImage);
            if (pBits)
            {
                if (ReadFile(hFile, pBits, cbImage, &cb, NULL))
                {
                    ;
                }
                else
                {
                    free(pBits);
                    pBits = NULL;
                }
            }
        }
    }
    CloseHandle(hFile);

    if (hbm)
    {
        free(pBits);
        return hbm;
    }

    hDC = CreateCompatibleDC(NULL);
    hMemDC = CreateCompatibleDC(hDC);
    hbm = CreateDIBSection(hMemDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
                           &pBits2, NULL, 0);
    if (hbm)
    {
        if (SetDIBits(hMemDC, hbm, 0, abs(bi.bmiHeader.biHeight),
                      pBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
        {
            ;
        }
        else
        {
            DeleteObject(hbm);
            hbm = NULL;
        }
    }
    DeleteDC(hMemDC);
    DeleteDC(hDC);

    free(pBits);

    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_bmp_load_a(II_CSTR pszFileName, float *dpi)
{
    HANDLE hFile;
    II_HIMAGE hbm;

    hbm = (II_HIMAGE)LoadImageA(NULL, pszFileName, IMAGE_BITMAP,
        0, 0, LR_LOADFROMFILE | LR_LOADREALSIZE | LR_CREATEDIBSECTION);

    hFile = CreateFileA(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
        return ii_bmp_load_common(hFile, hbm, dpi);
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_bmp_load_w(II_CWSTR pszFileName, float *dpi)
{
    HANDLE hFile;
    II_HIMAGE hbm;

    hbm = (II_HIMAGE)LoadImageW(NULL, pszFileName, IMAGE_BITMAP,
        0, 0, LR_LOADFROMFILE | LR_LOADREALSIZE | LR_CREATEDIBSECTION);

    hFile = CreateFileW(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
        return ii_bmp_load_common(hFile, hbm, dpi);
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_bmp_load_res_a(II_INST hInstance, II_CSTR pszResName)
{
    return LoadImageA(hInstance, pszResName, IMAGE_BITMAP,
                      0, 0, LR_LOADREALSIZE);
}

IMAIO_API II_HIMAGE IIAPI
ii_bmp_load_res_w(II_INST hInstance, II_CWSTR pszResName)
{
    return LoadImageW(hInstance, pszResName, IMAGE_BITMAP,
                      0, 0, LR_LOADREALSIZE);
}

IMAIO_API bool IIAPI
ii_bmp_save_common(II_HFILE hFile, II_HIMAGE hbm, float dpi)
{
    BITMAPFILEHEADER bf;
    II_BITMAPINFOEX bi;
    BITMAPINFOHEADER *pbmih;
    DWORD cb;
    uint32_t cColors, cbColors;
    II_DEVICE hDC;
    LPVOID pvBits;
    II_IMGINFO bm;
    bool f;

    if (!ii_get_info(hbm, &bm))
    {
        CloseHandle(hFile);
        return false;
    }

    pbmih = &bi.bmiHeader;
    ZeroMemory(pbmih, sizeof(BITMAPINFOHEADER));
    pbmih->biSize             = sizeof(BITMAPINFOHEADER);
    pbmih->biWidth            = bm.bmWidth;
    pbmih->biHeight           = bm.bmHeight;
    pbmih->biPlanes           = 1;
    pbmih->biBitCount         = bm.bmBitsPixel;
    pbmih->biCompression      = BI_RGB;
    pbmih->biSizeImage        = bm.bmWidthBytes * bm.bmHeight;
    if (dpi != 0.0)
    {
        pbmih->biXPelsPerMeter = (int32_t)(dpi * 100 / 2.54 + 0.5);
        pbmih->biYPelsPerMeter = (int32_t)(dpi * 100 / 2.54 + 0.5);
    }

    if (bm.bmBitsPixel < 16)
        cColors = 1 << bm.bmBitsPixel;
    else
        cColors = 0;
    cbColors = cColors * sizeof(RGBQUAD);

    bf.bfType = 0x4d42;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    cb = sizeof(BITMAPFILEHEADER) + pbmih->biSize + cbColors;
    bf.bfOffBits = cb;
    bf.bfSize = cb + pbmih->biSizeImage;

    pvBits = malloc(pbmih->biSizeImage);
    if (pvBits == NULL)
    {
        CloseHandle(hFile);
        return false;
    }

    f = false;
    hDC = CreateCompatibleDC(NULL);
    if (GetDIBits(hDC, hbm, 0, bm.bmHeight, pvBits, (BITMAPINFO*)&bi,
                  DIB_RGB_COLORS))
    {
        f = WriteFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL) &&
            WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &cb, NULL) &&
            WriteFile(hFile, bi.bmiColors, cbColors, &cb, NULL) &&
            WriteFile(hFile, pvBits, pbmih->biSizeImage, &cb, NULL);
    }
    DeleteDC(hDC);
    free(pvBits);
    if (!CloseHandle(hFile))
        f = false;
    return f;
}

IMAIO_API bool IIAPI
ii_bmp_save_a(II_CSTR pszFileName, II_HIMAGE hbm, float dpi)
{
    HANDLE hFile;
    hFile = CreateFileA(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_WRITE_THROUGH, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (ii_bmp_save_common(hFile, hbm, dpi))
        {
            return true;
        }
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_bmp_save_w(II_CWSTR pszFileName, II_HIMAGE hbm, float dpi)
{
    HANDLE hFile;
    hFile = CreateFileW(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_WRITE_THROUGH, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (ii_bmp_save_common(hFile, hbm, dpi))
        {
            return true;
        }
        DeleteFileW(pszFileName);
    }
    return false;
}

/*****************************************************************************/

IMAIO_API II_HIMAGE IIAPI
ii_jpg_load_common(FILE *fp, float *dpi)
{
    struct jpeg_decompress_struct decomp;
    struct jpeg_error_mgr jerror;
    BITMAPINFO bi;
    uint8_t *lpBuf, *pb;
    II_HIMAGE hbm;
    JSAMPARRAY buffer;
    int row;
    HDC hdc;

    assert(fp);
    if (fp == NULL)
        return NULL;

    decomp.err = jpeg_std_error(&jerror);

    jpeg_create_decompress(&decomp);
    jpeg_stdio_src(&decomp, fp);

    jpeg_read_header(&decomp, true);
    jpeg_start_decompress(&decomp);

    if (dpi)
    {
        switch(decomp.density_unit)
        {
        case 1: /* dots/inch */
            *dpi = decomp.X_density;
            break;

        case 2: /* dots/cm */
            *dpi = (float)(decomp.X_density * 2.54);
            break;

        default:
            *dpi = 0.0;
        }
    }

    row = ((decomp.output_width * 3 + 3) & ~3);
    buffer = (*decomp.mem->alloc_sarray)((j_common_ptr)&decomp, JPOOL_IMAGE,
                                         row, 1);

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth        = decomp.output_width;
    bi.bmiHeader.biHeight       = decomp.output_height;
    bi.bmiHeader.biPlanes       = 1;
    bi.bmiHeader.biBitCount     = 24;
    bi.bmiHeader.biCompression  = BI_RGB;
    bi.bmiHeader.biSizeImage    = row * decomp.output_height;

    hdc = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void**)&lpBuf, NULL, 0);
    DeleteDC(hdc);
    if (hbm == NULL)
    {
        jpeg_destroy_decompress(&decomp);
        fclose(fp);
        return NULL;
    }

    pb = lpBuf + row * decomp.output_height;
    while (decomp.output_scanline < decomp.output_height)
    {
        pb -= row;
        jpeg_read_scanlines(&decomp, buffer, 1);

        if (decomp.out_color_components == 1)
        {
            UINT i;
            uint8_t *p = (uint8_t *)buffer[0];
            for (i = 0; i < decomp.output_width; i++)
            {
                pb[3 * i + 0] = p[i];
                pb[3 * i + 1] = p[i];
                pb[3 * i + 2] = p[i];
            }
        }
        else if (decomp.out_color_components == 3)
        {
            int i;
            for (i = 0; i < row; i += 3)
            {
                pb[i + 0] = buffer[0][i + 2];
                pb[i + 1] = buffer[0][i + 1];
                pb[i + 2] = buffer[0][i + 0];
            }
        }
        else
        {
            jpeg_destroy_decompress(&decomp);
            fclose(fp);
            DeleteObject(hbm);
            return NULL;
        }
    }

    SetDIBits(NULL, hbm, 0, decomp.output_height, lpBuf, &bi, DIB_RGB_COLORS);

    jpeg_finish_decompress(&decomp);
    jpeg_destroy_decompress(&decomp);

    fclose(fp);

    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_jpg_load_a(II_CSTR pszFileName, float *dpi)
{
    FILE *fp;
    fp = fopen(pszFileName, "rb");
    if (fp)
        return ii_jpg_load_common(fp, dpi);
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_jpg_load_w(II_CWSTR pszFileName, float *dpi)
{
    FILE *fp;
    fp = _wfopen(pszFileName, L"rb");
    if (fp)
        return ii_jpg_load_common(fp, dpi);
    return NULL;
}

IMAIO_API bool IIAPI
ii_jpg_save_common(FILE *fp, II_HIMAGE hbm,
                   int quality, bool progression, float dpi)
{
    II_IMGINFO bm;
    struct jpeg_compress_struct comp;
    struct jpeg_error_mgr jerr;
    JSAMPLE * image_buffer;
    BITMAPINFO bi;
    II_DEVICE hDC, hMemDC;
    uint8_t *pbBits;
    int nWidthBytes;
    uint32_t cbBits;
    bool f;

    if (fp == NULL)
        return false;

    if (!ii_get_info(hbm, &bm))
    {
        fclose(fp);
        return false;
    }

    comp.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&comp);
    jpeg_stdio_dest(&comp, fp);

    comp.image_width  = bm.bmWidth;
    comp.image_height = bm.bmHeight;
    comp.input_components = 3;
    comp.in_color_space = JCS_RGB;
    jpeg_set_defaults(&comp);
    if (dpi != 0.0)
    {
        comp.density_unit = 1; /* dots/inch */
        comp.X_density = (UINT16)(dpi + 0.5);
        comp.Y_density = (UINT16)(dpi + 0.5);
    }
    jpeg_set_quality(&comp, quality, true);
    if (progression)
        jpeg_simple_progression(&comp);

    jpeg_start_compress(&comp, true);
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth    = bm.bmWidth;
    bi.bmiHeader.biHeight   = bm.bmHeight;
    bi.bmiHeader.biPlanes   = 1;
    bi.bmiHeader.biBitCount = 24;

    f = false;
    nWidthBytes = II_WIDTHBYTES(bm.bmWidth * 24);
    cbBits = nWidthBytes * bm.bmHeight;
    pbBits = (uint8_t *)malloc(cbBits);
    if (pbBits != NULL)
    {
        image_buffer = (JSAMPLE *)malloc(nWidthBytes);
        if (image_buffer != NULL)
        {
            hDC = GetDC(NULL);
            if (hDC != NULL)
            {
                hMemDC = CreateCompatibleDC(hDC);
                if (hMemDC != NULL)
                {
                    f = GetDIBits(hMemDC, hbm, 0, bm.bmHeight, pbBits,
                                  (BITMAPINFO*)&bi, DIB_RGB_COLORS);
                    DeleteDC(hMemDC);
                }
                ReleaseDC(NULL, hDC);
            }
            if (f)
            {
                int x, y;
                uint8_t *src, *dest;
                for (y = 0; y < bm.bmHeight; y++)
                {
                    dest = image_buffer;
                    src = &pbBits[(bm.bmHeight - y - 1) * nWidthBytes];
                    for (x = 0; x < bm.bmWidth; x++)
                    {
                        dest[0] = src[2];
                        dest[1] = src[1];
                        dest[2] = src[0];
                        dest += 3;
                        src += 3;
                    }
                    jpeg_write_scanlines(&comp, &image_buffer, 1);
                }
            }
            free(image_buffer);
        }
        free(pbBits);
    }

    jpeg_finish_compress(&comp);
    jpeg_destroy_compress(&comp);

    fclose(fp);
    return f;
}

IMAIO_API bool IIAPI
ii_jpg_save_a(II_CSTR pszFileName, II_HIMAGE hbm,
              int quality, bool progression, float dpi)
{
    FILE *fp;
    fp = fopen(pszFileName, "wb");
    if (fp)
    {
        if (ii_jpg_save_common(fp, hbm, quality, progression, dpi))
            return true;
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_jpg_save_w(II_CWSTR pszFileName, II_HIMAGE hbm,
              int quality, bool progression, float dpi)
{
    FILE *fp;
    fp = _wfopen(pszFileName, L"wb");
    if (fp)
    {
        if (ii_jpg_save_common(fp, hbm, quality, progression, dpi))
            return true;
        DeleteFileW(pszFileName);
    }
    return false;
}

/*****************************************************************************/

IMAIO_API void IIAPI
ii_gif_uninterlace(GifByteType *bits, int width, int height)
{
    int i, j, k;
    GifByteType *new_bits = malloc(width * height);
    if (new_bits == NULL)
        return;
    k = 0;
    for (i = 0; i < 4; i++)
    {
        static int InterlacedOffset[] = { 0, 4, 2, 1 };
        static int InterlacedJumps[] = { 8, 8, 4, 2 };
        for (j = InterlacedOffset[i]; j < height; j += InterlacedJumps[i])
        {
            CopyMemory(&new_bits[j * width], &bits[k * width], width);
        }
    }
    CopyMemory(bits, new_bits, width * height);
    free(new_bits);
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_common(GifFileType *gif, int *pi_trans/* = NULL*/)
{
    GifRowType *ScreenBuffer;
    int i, j, Size;
    int top, left, Width, Height;
    GifRecordType RecordType;
    ColorMapObject *ColorMap;
    II_BITMAPINFOEX bi;
    LPBYTE pbBits;
    II_HIMAGE hbm;
    HDC hdc;

    if (pi_trans)
        *pi_trans = -1;

    ScreenBuffer = (GifRowType *)calloc(gif->SHeight, sizeof(GifRowType));
    if (ScreenBuffer == NULL)
    {
        DGifCloseFile(gif, NULL);
        return NULL;
    }

    Size = gif->SWidth * sizeof(GifPixelType);
    for (i = 0; i < gif->SHeight; i++)
    {
        ScreenBuffer[i] = (GifRowType) malloc(Size);
        for (j = 0; j < gif->SWidth; j++)
            ScreenBuffer[i][j] = gif->SBackGroundColor;
    }

    do
    {
        DGifGetRecordType(gif, &RecordType);
        switch (RecordType)
        {
        case IMAGE_DESC_RECORD_TYPE:
            DGifGetImageDesc(gif);

            top = gif->Image.Top;
            left = gif->Image.Left;
            Width = gif->Image.Width;
            Height = gif->Image.Height;

            if (gif->Image.Interlace)
            {
                for (i = 0; i < 4; i++)
                {
                    static int InterlacedOffset[] = { 0, 4, 2, 1 };
                    static int InterlacedJumps[] = { 8, 8, 4, 2 };
                    for (j = top + InterlacedOffset[i]; j < top + Height;
                         j += InterlacedJumps[i])
                    {
                        DGifGetLine(gif, &ScreenBuffer[j][left], Width);
                    }
                }
            }
            else
            {
                j = top;
                for (i = 0; i < Height; i++)
                {
                    DGifGetLine(gif, &ScreenBuffer[j++][left], Width);
                }
            }
            goto hell;

        case EXTENSION_RECORD_TYPE:
            {
                GifByteType *Extension;
                int ExtCode;
                DGifGetExtension(gif, &ExtCode, &Extension);
                while (Extension != NULL)
                {
                    if (ExtCode == GRAPHICS_EXT_FUNC_CODE)
                    {
                        /* WORD Delay = Extension[2] | (Extension[3] << 8); */
                        if (Extension[1] & 1)
                        {
                            if (pi_trans)
                                *pi_trans = Extension[4];
                        }
                    }
                    DGifGetExtensionNext(gif, &Extension);
                }
            }
            break;

        default:
            break;
        }
    } while (RecordType != TERMINATE_RECORD_TYPE);

hell:
    if (gif->Image.ColorMap)
        ColorMap = gif->Image.ColorMap;
    else
        ColorMap = gif->SColorMap;

    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = gif->SWidth;
    bi.bmiHeader.biHeight = gif->SHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 8;
    bi.bmiHeader.biClrUsed = ColorMap->ColorCount;
    assert(ColorMap);

    for (i = 0; i < ColorMap->ColorCount; ++i)
    {
        bi.bmiColors[i].rgbBlue = ColorMap->Colors[i].Blue;
        bi.bmiColors[i].rgbGreen = ColorMap->Colors[i].Green;
        bi.bmiColors[i].rgbRed = ColorMap->Colors[i].Red;
        bi.bmiColors[i].rgbReserved = 0;
    }

    DGifCloseFile(gif, NULL);

    hdc = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hdc, (LPBITMAPINFO)&bi, DIB_RGB_COLORS,
                           (void **)&pbBits, NULL, 0);
    DeleteDC(hdc);
    if (hbm)
    {
        int widthbytes = II_WIDTHBYTES(bi.bmiHeader.biWidth * 8);
        for (i = 0; i < bi.bmiHeader.biHeight; i++)
        {
            GifRowType row = ScreenBuffer[i];
            for (j = 0; j < bi.bmiHeader.biWidth; j++)
            {
                int y = bi.bmiHeader.biHeight - i - 1;
                pbBits[y * widthbytes + j] = row[j];
            }
        }
    }

    for (i = 0; i < bi.bmiHeader.biHeight; i++)
    {
        free(ScreenBuffer[i]);
    }
    free(ScreenBuffer);

    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_a(II_CSTR pszFileName, int *pi_trans/* = NULL*/)
{
    GifFileType *gif = DGifOpenFileName(pszFileName, NULL);
    if (gif)
        return ii_gif_load_8bpp_common(gif, pi_trans);
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_w(II_CWSTR pszFileName, int *pi_trans/* = NULL*/)
{
    GifFileType *gif;
    int fd;

    fd = _wopen(pszFileName, O_RDONLY);
    if (fd != -1) {
        gif = DGifOpenFileHandle(fd, NULL);
        if (gif)
            return ii_gif_load_8bpp_common(gif, pi_trans);
        _close(fd);
    }
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_32bpp_a(II_CSTR pszFileName)
{
    int i_trans;
    II_HIMAGE hbm8bpp, hbm32bpp;
    assert(pszFileName);
    hbm8bpp = ii_gif_load_8bpp_a(pszFileName, &i_trans);
    assert(hbm8bpp);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        assert(hbm32bpp);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_32bpp_w(II_CWSTR pszFileName)
{
    int i_trans;
    II_HIMAGE hbm8bpp, hbm32bpp;
    assert(pszFileName);
    hbm8bpp = ii_gif_load_8bpp_w(pszFileName, &i_trans);
    assert(hbm8bpp);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        assert(hbm32bpp);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

static int IICAPI
ii_gif_mem_read(GifFileType *gif, GifByteType *bytes, int length)
{
    II_MEMORY *memory;
    assert(gif);
    memory = (II_MEMORY *)gif->UserData;
    assert(memory);
    assert(bytes);
    if (memory->m_i + length <= memory->m_size)
    {
        CopyMemory(bytes, memory->m_pb + memory->m_i, length);
        memory->m_i += length;
        return length;
    }
    return 0;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_mem(II_LPCVOID pv, uint32_t cb, int *pi_trans)
{
    GifFileType *gif;
    II_MEMORY memory;

    memory.m_pb = (const uint8_t *)pv;
    memory.m_i = 0;
    memory.m_size = cb;

    gif = DGifOpen(&memory, ii_gif_mem_read, NULL);
    assert(gif);
    if (gif)
        return ii_gif_load_8bpp_common(gif, pi_trans);
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_32bpp_mem(II_LPCVOID pv, uint32_t cb)
{
    int i_trans;
    II_HIMAGE hbm8bpp, hbm32bpp;
    hbm8bpp = ii_gif_load_8bpp_mem(pv, cb, &i_trans);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_res_a(II_INST hInstance, II_CSTR pszResName, int *pi_trans)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    II_HIMAGE hbm;
    LPVOID lpData;
    HRSRC hRsrc;

    assert(pszResName);
    hRsrc = FindResourceA(hInstance, pszResName, "GIF");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    hbm = ii_gif_load_8bpp_mem(lpData, dwSize, pi_trans);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_32bpp_res_a(II_INST hInstance, II_CSTR pszResName)
{
    int i_trans;
    II_HIMAGE hbm8bpp, hbm32bpp;
    assert(pszResName);
    hbm8bpp = ii_gif_load_8bpp_res_a(hInstance, pszResName, &i_trans);
    assert(hbm8bpp);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        assert(hbm32bpp);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_8bpp_res_w(II_INST hInstance, II_CWSTR pszResName, int *pi_trans)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    II_HIMAGE hbm;
    LPVOID lpData;
    HRSRC hRsrc;

    assert(pszResName);
    hRsrc = FindResourceW(hInstance, pszResName, L"GIF");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    hbm = ii_gif_load_8bpp_mem(lpData, dwSize, pi_trans);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_gif_load_32bpp_res_w(II_INST hInstance, II_CWSTR pszResName)
{
    int i_trans;
    II_HIMAGE hbm8bpp, hbm32bpp;
    assert(pszResName);
    hbm8bpp = ii_gif_load_8bpp_res_w(hInstance, pszResName, &i_trans);
    assert(hbm8bpp);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        assert(hbm32bpp);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

IMAIO_API bool IIAPI 
ii_gif_save_common(GifFileType *gif, II_HIMAGE hbm8bpp, const int *pi_trans)
{
    II_IMGINFO bm;
    RGBQUAD table[256];
    LPBYTE pbBits;
    II_DEVICE hdc;
    HGDIOBJ hbmOld;
    int nColorCount;
    ColorMapObject cm;
    GifColorType colors[256];
    int i;
    GifPixelType* ScanLines;

    if (!ii_get_info(hbm8bpp, &bm) || bm.bmBitsPixel != 8)
    {
        assert(0);
        EGifCloseFile(gif, NULL);
        return false;
    }
    pbBits = (LPBYTE)bm.bmBits;
    assert(pbBits);

    ScanLines = (GifPixelType*)calloc(sizeof(GifPixelType), bm.bmWidth);
    if (ScanLines == NULL)
    {
        EGifCloseFile(gif, NULL);
        return false;
    }

    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, hbm8bpp);
    nColorCount = GetDIBColorTable(hdc, 0, 256, table);
    SelectObject(hdc, hbmOld);
    DeleteDC(hdc);

    cm.ColorCount = nColorCount;
    cm.BitsPerPixel = 8;
    cm.Colors = colors;
    for (i = 0; i < nColorCount; i++)
    {
        cm.Colors[i].Blue = table[i].rgbBlue;
        cm.Colors[i].Green = table[i].rgbGreen;
        cm.Colors[i].Red = table[i].rgbRed;
    }

    EGifPutScreenDesc(gif, bm.bmWidth, bm.bmHeight, 256, 0, &cm);
    if (pi_trans && *pi_trans != -1)
    {
        uint8_t extension[4];
        extension[0] = 1;   /* enable transparency */
        extension[1] = 0;   /* no delay */
        extension[2] = 0;   /* no delay */
        extension[3] = (uint8_t)*pi_trans;     /* tranparent index */
        EGifPutExtension(gif, GRAPHICS_EXT_FUNC_CODE, 4, extension);
    }
    EGifPutImageDesc(gif, 0, 0, bm.bmWidth, bm.bmHeight, false, NULL);
    for (i = 0; i < bm.bmHeight; i++)
    {
        int y = bm.bmHeight - i - 1;
        EGifPutLine(gif, &pbBits[y * bm.bmWidthBytes], bm.bmWidth);
    }

    EGifCloseFile(gif, NULL);
    free(ScanLines);

    return true;
}

IMAIO_API bool IIAPI
ii_gif_save_a(II_CSTR pszFileName, II_HIMAGE hbm8bpp, const int *pi_trans)
{
    GifFileType *gif;

    gif = EGifOpenFileName(pszFileName, false, NULL);
    assert(gif);
    if (gif)
    {
        if (ii_gif_save_common(gif, hbm8bpp, pi_trans))
        {
            return true;
        }
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_gif_save_w(II_CWSTR pszFileName, II_HIMAGE hbm8bpp, const int *pi_trans)
{
    GifFileType *gif = NULL;
    int fd = _wopen(pszFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if (fd != -1)
    {
        gif = EGifOpenFileHandle(fd, NULL);
        if (gif == NULL)
            _close(fd);
    }
    assert(gif);
    if (gif)
    {
        if (ii_gif_save_common(gif, hbm8bpp, pi_trans))
        {
            return true;
        }
        DeleteFileW(pszFileName);
    }
    return false;
}

/*****************************************************************************/

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_common(GifFileType *gif, II_FLAGS flags)
{
    ColorMapObject *cm;
    II_PALETTE *palette;
    II_ANIGIF *anigif;
    int i, k, n, x, y;
    int ret;
    SavedImage *image;
    II_ANIGIF_FRAME *frame;

    assert(gif);
    if (gif == NULL)
    {
        return NULL;
    }
    ret = DGifSlurp(gif);
    if (ret == GIF_ERROR)
        return NULL;

    anigif = (II_ANIGIF *)calloc(sizeof(II_ANIGIF), 1);
    if (anigif == NULL)
    {
        DGifCloseFile(gif, NULL);
        return NULL;
    }

    if (flags & II_FLAG_USE_SCREEN)
        anigif->flags = II_FLAG_USE_SCREEN;
    else
        anigif->flags = 0;
    anigif->width = gif->SWidth;
    anigif->height = gif->SHeight;
    anigif->num_frames = gif->ImageCount;
    anigif->iBackground = gif->SBackGroundColor;

    anigif->frames =
        (II_ANIGIF_FRAME *)
            calloc(sizeof(II_ANIGIF_FRAME), anigif->num_frames);
    if (anigif->frames == NULL)
    {
        ii_anigif_destroy(anigif);
        DGifCloseFile(gif, NULL);
        return NULL;
    }

    cm = gif->SColorMap;
    if (cm)
    {
        palette = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
        if (palette == NULL)
        {
            ii_anigif_destroy(anigif);
            DGifCloseFile(gif, NULL);
            return NULL;
        }
        palette->num_colors = cm->ColorCount;
        for (i = 0; i < cm->ColorCount; ++i)
        {
            palette->colors[i].value[0] = cm->Colors[i].Blue;
            palette->colors[i].value[1] = cm->Colors[i].Green;
            palette->colors[i].value[2] = cm->Colors[i].Red;
        }
        anigif->global_palette = palette;
    }
    else
    {
        anigif->global_palette = NULL;
    }

    for (i = 0; i < anigif->num_frames; ++i)
    {
        image = &(gif->SavedImages[i]);
        frame = &anigif->frames[i];

        frame->x = image->ImageDesc.Left;
        frame->y = image->ImageDesc.Top;
        frame->width = image->ImageDesc.Width;
        frame->height = image->ImageDesc.Height;
        frame->iTransparent = -1;
        frame->disposal = 0;
        frame->delay = 0;
        if (image->ImageDesc.ColorMap)
        {
            cm = image->ImageDesc.ColorMap;
            frame->local_palette =
                (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
            if (frame->local_palette == NULL)
            {
                ii_anigif_destroy(anigif);
                DGifCloseFile(gif, NULL);
                return NULL;
            }
            palette = frame->local_palette;
            palette->num_colors = cm->ColorCount;
            for (n = 0; n < cm->ColorCount; ++n)
            {
                palette->colors[n].value[0] = cm->Colors[n].Blue;
                palette->colors[n].value[1] = cm->Colors[n].Green;
                palette->colors[n].value[2] = cm->Colors[n].Red;
            }
        }
        for (k = 0; k < image->ExtensionBlockCount; ++k)
        {
            ExtensionBlock *block = &image->ExtensionBlocks[k];
            switch (block->Function)
            {
            case GRAPHICS_EXT_FUNC_CODE:
                if (block->ByteCount >= 4)
                {
                    n = block->Bytes[0];
                    if (n & 1)
                        frame->iTransparent = block->Bytes[3];
                    frame->disposal = ((n >> 2) & 0x07);
                    frame->delay = 10 *
                        (block->Bytes[1] | (block->Bytes[2] << 8));
                }
                break;
            case APPLICATION_EXT_FUNC_CODE:
                if (block->ByteCount == 11)
                {
                    if (memcmp(block->Bytes, "NETSCAPE2.0", 11) == 0)
                    {
                        if (i + 1 < image->ExtensionBlockCount)
                        {
                            ++block;
                            if (block->ByteCount == 3 && (block->Bytes[0] & 7) == 1)
                            {
                                anigif->loop_count =
                                    ((block->Bytes[1] & 0xFF) |
                                        ((block->Bytes[2] & 0xFF) << 8));
                            }
                        }
                    }
                }
                break;
            }
        }

        if (frame->local_palette)
            palette = frame->local_palette;
        else
            palette = anigif->global_palette;
        frame->hbmPart = ii_create(frame->width, frame->height, 8, palette);
        assert(frame->hbmPart);
        if (frame->hbmPart == NULL)
        {
            ii_anigif_destroy(anigif);
            DGifCloseFile(gif, NULL);
            return NULL;
        }
        {
            II_IMGINFO bm;
            LPBYTE pb;

            ii_get_info(frame->hbmPart, &bm);
            pb = (LPBYTE)bm.bmBits;
            if (image->ImageDesc.Interlace)
            {
                ii_gif_uninterlace(
                    image->RasterBits, frame->width, frame->height);
            }
            for (y = 0; y < frame->height; ++y)
            {
                for (x = 0; x < frame->width; ++x)
                {
                    pb[x + (bm.bmHeight - y - 1) * bm.bmWidthBytes] =
                       image->RasterBits[x + y * frame->width];
                }
            }
        }
    }

    /* realize 32bpp */
    if (flags & II_FLAG_USE_SCREEN)
    {
        HBITMAP hbmScreen;
        II_IMGINFO bmScreen;
        II_ANIGIF_FRAME *old_frame;

        /* create screen */
        hbmScreen = ii_create_32bpp_trans(anigif->width, anigif->height);
        if (hbmScreen == NULL)
        {
            ii_anigif_destroy(anigif);
            DGifCloseFile(gif, NULL);
            return NULL;
        }
        ii_get_info(hbmScreen, &bmScreen);

        /* realize */
        old_frame = NULL;
        for (i = 0; i < anigif->num_frames; ++i)
        {
            frame = &anigif->frames[i];

            ii_stamp(hbmScreen, frame->x, frame->y, frame->hbmPart,
                     &frame->iTransparent, 255);
            if (frame->hbmScreen)
                ii_destroy(frame->hbmScreen);
            frame->hbmScreen = ii_clone(hbmScreen);
            assert(hbmScreen);
            assert(frame->hbmScreen);

            switch (frame->disposal)
            {
            case 2:
                if (frame->local_palette)
                    palette = frame->local_palette;
                else
                    palette = anigif->global_palette;
                if (anigif->iBackground != -1)
                {
                    int xx, yy;
                    LPDWORD pdw;
                    DWORD dw;

                    dw = *(LPDWORD)(&palette->colors[anigif->iBackground]);
                    dw &= 0xFFFFFF;
                    pdw = (LPDWORD)bmScreen.bmBits;

                    for (yy = frame->y; yy < frame->y + frame->height; ++yy)
                    {
                        for (xx = frame->x; xx < frame->x + frame->width; ++xx)
                        {
                            pdw[xx + yy * frame->width] = dw;
                        }
                    }
                }
                break;
            case 3:
                if (old_frame)
                {
                    ii_destroy(hbmScreen);
                    hbmScreen = ii_clone(old_frame->hbmScreen);
                }
                break;
            }

            old_frame = frame;
        }

        ii_destroy(hbmScreen);
    }

    DGifCloseFile(gif, NULL);
    return anigif;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_a(II_CSTR pszFileName, II_FLAGS flags)
{
    GifFileType *gif = DGifOpenFileName(pszFileName, NULL);
    if (gif)
        return ii_anigif_load_common(gif, flags);
    return NULL;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_w(II_CWSTR pszFileName, II_FLAGS flags)
{
    GifFileType *gif;
    int fd;

    fd = _wopen(pszFileName, O_RDONLY);
    if (fd != -1) {
        gif = DGifOpenFileHandle(fd, NULL);
        if (gif)
            return ii_anigif_load_common(gif, flags);
        _close(fd);
    }
    return NULL;
}

IMAIO_API bool IIAPI
ii_anigif_save_common(GifFileType *gif, II_ANIGIF *anigif)
{
    int i, k;
    II_PALETTE *palette;
    GifColorType colors[256];
    int ret, iTransparent;
    II_COLOR8 bg_color;

    assert(gif);
    assert(anigif);

    if (gif == NULL)
    {
        return false;
    }
    if (anigif == NULL)
    {
        EGifCloseFile(gif, NULL);
        return false;
    }

    assert(anigif->width > 0);
    assert(anigif->height > 0);
    assert(anigif->num_frames > 0);
    if (anigif->iBackground < 0 || 256 <= anigif->iBackground)
        anigif->iBackground = 0;

    /* initialize basic info */
    gif->SWidth = anigif->width;
    gif->SHeight = anigif->height;
    gif->SColorResolution = 8;
    gif->SBackGroundColor = anigif->iBackground;
    gif->ImageCount = anigif->num_frames;
    gif->SavedImages =
        (SavedImage *)calloc(anigif->num_frames, sizeof(SavedImage));
    if (gif->SavedImages == NULL)
    {
        EGifCloseFile(gif, NULL);
        return false;
    }

    palette = anigif->global_palette;
    if (palette && anigif->iBackground < palette->num_colors)
    {
        bg_color = palette->colors[anigif->iBackground];
    }
    else
    {
        ZeroMemory(&bg_color, sizeof(bg_color));
    }

    /* realize 8bpp */
    if (anigif->flags & II_FLAG_USE_SCREEN)
    {
        II_HIMAGE hbm8bpp;
        iTransparent = -1;
        if (anigif->global_palette == NULL)
        {
            bool fulfilled = true;
            for (i = 0; i < anigif->num_frames; ++i)
            {
                if (anigif->frames[i].local_palette == NULL)
                {
                    fulfilled = false;
                }
            }
            if (!fulfilled)
            {
                palette = ii_palette_for_anigif(anigif, 255);
                assert(palette);
                if (palette == NULL)
                {
                    EGifCloseFile(gif, NULL);
                    return false;
                }
                iTransparent = palette->num_colors;
                palette->num_colors++;
                anigif->global_palette = palette;
            }
        }
        for (i = 0; i < anigif->num_frames; ++i)
        {
            II_IMGINFO info;
            II_ANIGIF_FRAME *frame = &(anigif->frames[i]);
            if (frame->hbmPart)
            {
                ii_destroy(frame->hbmPart);
                frame->hbmPart = NULL;
            }
            if (frame->hbmScreen == NULL)
                continue;

            ii_get_info(frame->hbmScreen, &info);
            if (info.bmWidth != frame->width ||
                info.bmHeight != frame->height)
            {
                frame->x = 0;
                frame->y = 0;
                frame->width = info.bmWidth;
                frame->height = info.bmHeight;
            }

            if (iTransparent != -1)
                frame->iTransparent = iTransparent;

            if (frame->local_palette)
            {
                hbm8bpp = ii_reduce_colors(
                    frame->hbmScreen, frame->local_palette,
                    &frame->iTransparent);
            }
            else
            {
                hbm8bpp = ii_reduce_colors(
                    frame->hbmScreen, anigif->global_palette,
                    &frame->iTransparent);
            }
            assert(hbm8bpp);
            if (hbm8bpp == NULL)
            {
                EGifCloseFile(gif, NULL);
                return false;
            }
            frame->hbmPart = hbm8bpp;
        }
    }

    /* global palette */
    if (anigif->global_palette)
    {
        palette = anigif->global_palette;
        for (i = 0; i < palette->num_colors; ++i)
        {
            II_COLOR8 *pcolor = &palette->colors[i];
            colors[i].Red = pcolor->value[2];
            colors[i].Green = pcolor->value[1];
            colors[i].Blue = pcolor->value[0];
        }
        gif->SColorMap = GifMakeMapObject(palette->num_colors, colors);
        if (gif->SColorMap == NULL)
        {
            EGifCloseFile(gif, NULL);
            return false;
        }
    }
    else
    {
        gif->SColorMap = NULL;
    }

    for (i = 0; i < anigif->num_frames; ++i)
    {
        II_ANIGIF_FRAME *frame = &(anigif->frames[i]);
        SavedImage *image = &(gif->SavedImages[i]);

        /* frame info */
        image->ImageDesc.Left = frame->x;
        image->ImageDesc.Top = frame->y;
        image->ImageDesc.Width = frame->width;
        image->ImageDesc.Height = frame->height;
        image->ImageDesc.Interlace = false;

        /* local palette */
        if (frame->local_palette)
        {
            palette = frame->local_palette;
            for (k = 0; k < palette->num_colors; ++k)
            {
                II_COLOR8 *pcolor = &palette->colors[k];
                colors[k].Red = pcolor->value[2];
                colors[k].Green = pcolor->value[1];
                colors[k].Blue = pcolor->value[0];
            }
            image->ImageDesc.ColorMap =
                GifMakeMapObject(palette->num_colors, colors);
            if (image->ImageDesc.ColorMap == NULL)
            {
                EGifCloseFile(gif, NULL);
                return false;
            }
        }

        /* extension */
        if (image->ExtensionBlocks)
        {
            GifFreeExtensions(
                &image->ExtensionBlockCount, &image->ExtensionBlocks);
        }
        if (i == 0 && anigif->loop_count)
        {
            /* APPLICATION_EXT_FUNC_CODE */
            uint8_t params[3];

            GifAddExtensionBlock(
                &image->ExtensionBlockCount, &image->ExtensionBlocks,
                APPLICATION_EXT_FUNC_CODE, 11, (uint8_t *)"NETSCAPE2.0");

            params[0] = 1;
            params[1] = (uint8_t)(anigif->loop_count);
            params[2] = (uint8_t)(anigif->loop_count >> 8);
            GifAddExtensionBlock(
                &image->ExtensionBlockCount, &image->ExtensionBlocks,
                0, 3, params);
        }
        {
            /* GRAPHICS_EXT_FUNC_CODE */
            uint8_t extension[4];
            uint16_t delay = frame->delay / 10;
            extension[0] = (uint8_t)(frame->iTransparent != -1);
            extension[0] |= (uint8_t)((frame->disposal & 0x07) << 2);
            extension[1] = (uint8_t)delay;
            extension[2] = (uint8_t)(delay >> 8);
            extension[3] = (uint8_t)frame->iTransparent;
            GifAddExtensionBlock(
                &image->ExtensionBlockCount, &image->ExtensionBlocks,
                GRAPHICS_EXT_FUNC_CODE, 4, extension);
        }

        /* image bits */
        if (frame->hbmPart == NULL)
        {
            free(image->RasterBits);
            image->RasterBits = NULL;
            assert(0);
        }
        else
        {
            II_IMGINFO bm;
            LPBYTE pb;
            int x, y;
            BYTE b;

            ii_get_info(frame->hbmPart, &bm);
            pb = (LPBYTE)bm.bmBits;

            /* allocate bits */
            free(image->RasterBits);
            image->RasterBits = (uint8_t *)malloc(bm.bmWidth * bm.bmHeight);
            if (image->RasterBits == NULL)
            {
                EGifCloseFile(gif, NULL);
                return false;
            }

            /* store bits */
            k = 0;
            for (y = 0; y < bm.bmHeight; ++y)
            {
                for (x = 0; x < bm.bmWidth; ++x)
                {
                    b = pb[x + (bm.bmHeight - y - 1) * bm.bmWidthBytes];
                    image->RasterBits[k++] = b;
                }
            }
        }
    }

    ret = EGifSpew(gif);
    assert(ret != GIF_ERROR);

    if (ret == GIF_ERROR)
        EGifCloseFile(gif, NULL);

    return ret != GIF_ERROR;
}

IMAIO_API bool IIAPI
ii_anigif_save_a(II_CSTR pszFileName, II_ANIGIF *anigif)
{
    GifFileType *gif;

    assert(anigif);
    gif = EGifOpenFileName(pszFileName, false, NULL);
    assert(gif);
    if (gif)
    {
        if (ii_anigif_save_common(gif, anigif))
        {
            return true;
        }
    }
    DeleteFileA(pszFileName);
    return false;
}

IMAIO_API bool IIAPI
ii_anigif_save_w(II_CWSTR pszFileName, II_ANIGIF *anigif)
{
    GifFileType *gif = NULL;
    int fd = _wopen(pszFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if (fd != -1)
    {
        gif = EGifOpenFileHandle(fd, NULL);
        if (gif == NULL)
            _close(fd);
    }
    assert(gif);
    assert(anigif);
    if (gif)
    {
        if (ii_anigif_save_common(gif, anigif))
        {
            return true;
        }
    }
    DeleteFileW(pszFileName);
    return false;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_mem(II_LPCVOID pv, uint32_t cb, II_FLAGS flags)
{
    GifFileType *gif;
    II_MEMORY memory;

    memory.m_pb = (const uint8_t *)pv;
    memory.m_i = 0;
    memory.m_size = cb;
    gif = DGifOpen(&memory, ii_gif_mem_read, NULL);
    assert(gif);
    if (gif)
        return ii_anigif_load_common(gif, flags);
    return NULL;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_res_a(
    II_INST hInstance, II_CSTR pszResName, II_FLAGS flags)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    LPVOID lpData;
    HRSRC hRsrc;
    II_ANIGIF *anigif;

    assert(pszResName);
    hRsrc = FindResourceA(hInstance, pszResName, "GIF");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    anigif = ii_anigif_load_mem(lpData, dwSize, flags);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return anigif;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_res_w(
    II_INST hInstance, II_CWSTR pszResName, II_FLAGS flags)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    LPVOID lpData;
    HRSRC hRsrc;
    II_ANIGIF *anigif;

    assert(pszResName);
    hRsrc = FindResourceW(hInstance, pszResName, L"GIF");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    anigif = ii_anigif_load_mem(lpData, dwSize, flags);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return anigif;
}

IMAIO_API void IIAPI
ii_anigif_destroy(II_ANIGIF *anigif)
{
    int i;
    II_ANIGIF_FRAME *frame;

    if (anigif)
    {
        if (anigif->frames)
        {
            for (i = 0; i < anigif->num_frames; ++i)
            {
                frame = &anigif->frames[i];
                if (frame->hbmPart)
                {
                    ii_destroy(frame->hbmPart);
                    frame->hbmPart = NULL;
                }
                if (frame->hbmScreen)
                {
                    ii_destroy(frame->hbmScreen);
                    frame->hbmScreen = NULL;
                }
                if (frame->local_palette)
                {
                    ii_palette_destroy(frame->local_palette);
                    frame->local_palette = NULL;
                }
            }
            free(anigif->frames);
            anigif->frames = NULL;
        }
        if (anigif->global_palette)
        {
            ii_palette_destroy(anigif->global_palette);
            anigif->global_palette = NULL;
        }
        free(anigif);
    }
}

/*****************************************************************************/

IMAIO_API II_HIMAGE IIAPI
ii_png_load_common(FILE *inf, float *dpi)
{
    II_HIMAGE       hbm;
    png_structp     png;
    png_infop       info;
    png_uint_32     y, width, height, rowbytes;
    int             color_type, depth, widthbytes;
    double          gamma;
    BITMAPINFO      bi;
    uint8_t            *pbBits;
    png_uint_32     res_x, res_y;
    int             unit_type;
    png_bytepp      rows;
    HDC             hdc;

    assert(inf);
    if (inf == NULL)
        return NULL;

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info = png_create_info_struct(png);
    if (png == NULL || info == NULL || setjmp(png_jmpbuf(png)))
    {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(inf);
        return NULL;
    }

    png_init_io(png, inf);
    png_read_info(png, info);

    png_get_IHDR(png, info, &width, &height, &depth, &color_type,
                 NULL, NULL, NULL);
    png_set_strip_16(png);
    png_set_gray_to_rgb(png);
    png_set_palette_to_rgb(png);
    png_set_bgr(png);
    png_set_packing(png);
    png_set_interlace_handling(png);
    if (png_get_gAMA(png, info, &gamma))
        png_set_gamma(png, 2.2, gamma);
    else
        png_set_gamma(png, 2.2, 0.45455);

    png_read_update_info(png, info);
    png_get_IHDR(png, info, &width, &height, &depth, &color_type,
                 NULL, NULL, NULL);

    if (dpi)
    {
        *dpi = 0.0;
        if (png_get_pHYs(png, info, &res_x, &res_y, &unit_type))
        {
            if (unit_type == PNG_RESOLUTION_METER)
                *dpi = (float)(res_x * 2.54 / 100.0);
        }
    }

    rowbytes = (png_uint_32)png_get_rowbytes(png, info);
    rows = (png_bytepp)malloc(height * sizeof(png_bytep));
    for (y = 0; y < height; y++)
    {
        rows[y] = (png_bytep)malloc(rowbytes);
    }

    png_read_image(png, rows);
    png_read_end(png, NULL);
    fclose(inf);

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth       = width;
    bi.bmiHeader.biHeight      = height;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = depth * png_get_channels(png, info);

    hdc = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void **)&pbBits,
                           NULL, 0);
    DeleteDC(hdc);
    if (hbm == NULL)
    {
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    widthbytes = II_WIDTHBYTES(width * bi.bmiHeader.biBitCount);
    for (y = 0; y < height; y++)
    {
        CopyMemory(pbBits + y * widthbytes,
                   rows[height - 1 - y], rowbytes);
    }

    png_destroy_read_struct(&png, &info, NULL);
    free(rows);
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_png_load_a(II_CSTR pszFileName, float *dpi)
{
    FILE            *inf;
    inf = fopen(pszFileName, "rb");
    if (inf)
        return ii_png_load_common(inf, dpi);
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_png_load_w(II_CWSTR pszFileName, float *dpi)
{
    FILE            *inf;
    inf = _wfopen(pszFileName, L"rb");
    if (inf)
        return ii_png_load_common(inf, dpi);
    return NULL;
}

static void IICAPI
ii_png_mem_read(png_structp png, png_bytep data, png_size_t length)
{
    II_MEMORY *memory;
    assert(png);
    memory = (II_MEMORY *)png_get_io_ptr(png);
    assert(memory);
    if (memory->m_i + length <= memory->m_size)
    {
        CopyMemory(data, memory->m_pb + memory->m_i, length);
        memory->m_i += (uint32_t)length;
    }
}

IMAIO_API II_HIMAGE IIAPI
ii_png_load_mem(II_LPCVOID pv, uint32_t cb)
{
    II_HIMAGE       hbm;
    png_structp     png;
    png_infop       info;
    png_uint_32     y, width, height, rowbytes;
    int             color_type, depth, widthbytes;
    double          gamma;
    BITMAPINFO      bi;
    LPBYTE          pbBits;
    II_MEMORY       memory;
    png_bytepp      rows;
    HDC             hdc;

    memory.m_pb = (const uint8_t *)pv;
    memory.m_i = 0;
    memory.m_size = cb;

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info = png_create_info_struct(png);
    if (png == NULL || info == NULL || setjmp(png_jmpbuf(png)))
    {
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    png_set_read_fn(png, &memory, ii_png_mem_read);
    png_read_info(png, info);

    png_get_IHDR(png, info, &width, &height, &depth, &color_type,
                 NULL, NULL, NULL);
    png_set_expand(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    png_set_strip_16(png);
    png_set_gray_to_rgb(png);
    png_set_palette_to_rgb(png);
    png_set_bgr(png);
    png_set_packing(png);
    png_set_interlace_handling(png);
    if (png_get_gAMA(png, info, &gamma))
        png_set_gamma(png, 2.2, gamma);
    else
        png_set_gamma(png, 2.2, 0.45455);

    png_read_update_info(png, info);
    png_get_IHDR(png, info, &width, &height, &depth, &color_type,
                 NULL, NULL, NULL);

    rowbytes = (png_uint_32)png_get_rowbytes(png, info);
    rows = (png_bytepp)malloc(height * sizeof(png_bytep));
    for (y = 0; y < height; y++)
    {
        rows[y] = (png_bytep)malloc(rowbytes);
    }

    png_read_image(png, rows);
    png_read_end(png, NULL);

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth       = width;
    bi.bmiHeader.biHeight      = height;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = (WORD)(depth * png_get_channels(png, info));

    hdc = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (VOID **)&pbBits,
                           NULL, 0);
    DeleteDC(hdc);
    if (hbm == NULL)
    {
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    widthbytes = II_WIDTHBYTES(width * bi.bmiHeader.biBitCount);
    for (y = 0; y < height; y++)
    {
        CopyMemory(pbBits + y * widthbytes,
                   rows[height - 1 - y], rowbytes);
    }

    png_destroy_read_struct(&png, &info, NULL);
    free(rows);
    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_png_load_res_a(II_INST hInstance, II_CSTR pszResName)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    II_HIMAGE hbm;
    LPVOID lpData;
    HRSRC hRsrc;

    assert(pszResName);
    hRsrc = FindResourceA(hInstance, pszResName, "PNG");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    hbm = ii_png_load_mem(lpData, dwSize);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_png_load_res_w(II_INST hInstance, II_CWSTR pszResName)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    II_HIMAGE hbm;
    LPVOID lpData;
    HRSRC hRsrc;

    assert(pszResName);
    hRsrc = FindResourceW(hInstance, pszResName, L"PNG");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    hbm = ii_png_load_mem(lpData, dwSize);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hbm;
}

IMAIO_API bool IIAPI
ii_png_save_common(FILE *outf, II_HIMAGE hbm, float dpi)
{
    png_structp png;
    png_infop info;
    png_color_8 sBIT;
    II_DEVICE hMemDC;
    BITMAPINFO bi;
    II_IMGINFO bm;
    uint32_t rowbytes, cbBits;
    LPBYTE pbBits;
    int y, nDepth;
    png_bytep *lines = NULL;
    bool ok = false;

    assert(outf);
    if (outf == NULL)
        return false;

    if (!ii_get_info(hbm, &bm))
    {
        fclose(outf);
        return false;
    }

    nDepth = (bm.bmBitsPixel == 32 ? 32 : 24);
    rowbytes = II_WIDTHBYTES(bm.bmWidth * nDepth);
    cbBits = rowbytes * bm.bmHeight;

    do
    {
        pbBits = (LPBYTE)malloc(cbBits);
        if (pbBits == NULL)
            break;

        ok = false;
        hMemDC = CreateCompatibleDC(NULL);
        if (hMemDC != NULL)
        {
            ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
            bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth    = bm.bmWidth;
            bi.bmiHeader.biHeight   = bm.bmHeight;
            bi.bmiHeader.biPlanes   = 1;
            bi.bmiHeader.biBitCount = (WORD)nDepth;
            ok = GetDIBits(hMemDC, hbm, 0, bm.bmHeight, pbBits, &bi,
                           DIB_RGB_COLORS);
            DeleteDC(hMemDC);
        }
        if (!ok)
            break;
        ok = false;

        png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info = png_create_info_struct(png);
        if (png == NULL || info == NULL)
            break;

        if (setjmp(png_jmpbuf(png)))
            break;

        png_init_io(png, outf);
        png_set_IHDR(png, info, bm.bmWidth, bm.bmHeight, 8,
            (nDepth == 32 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB),
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

        sBIT.red = 8;
        sBIT.green = 8;
        sBIT.blue = 8;
        sBIT.alpha = (png_byte)(nDepth == 32 ? 8 : 0);
        png_set_sBIT(png, info, &sBIT);

        if (dpi != 0.0)
        {
            png_uint_32 res = (png_uint_32)(dpi * 100 / 2.54 + 0.5);
            png_set_pHYs(png, info, res, res, PNG_RESOLUTION_METER);
        }

        png_write_info(png, info);
        png_set_bgr(png);

        lines = (png_bytep *)malloc(sizeof(png_bytep *) * bm.bmHeight);
        if (lines == NULL)
            break;
        for (y = 0; y < bm.bmHeight; y++)
        {
            lines[y] = (png_bytep)&pbBits[rowbytes * (bm.bmHeight - y - 1)];
        }

        png_write_image(png, lines);
        png_write_end(png, info);
        ok = true;
    } while (0);

    png_destroy_write_struct(&png, &info);

    free(lines);
    free(pbBits);
    fclose(outf);

    return ok;
}

IMAIO_API bool IIAPI
ii_png_save_a(II_CSTR pszFileName, II_HIMAGE hbm, float dpi)
{
    FILE *outf;
    outf = fopen(pszFileName, "wb");
    if (outf)
    {
        if (ii_png_save_common(outf, hbm, dpi))
            return true;
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_png_save_w(II_CWSTR pszFileName, II_HIMAGE hbm, float dpi)
{
    FILE *outf;
    outf = _wfopen(pszFileName, L"wb");
    if (outf)
    {
        if (ii_png_save_common(outf, hbm, dpi))
            return true;
        DeleteFileW(pszFileName);
    }
    return false;
}

/*****************************************************************************/

#ifdef PNG_APNG_SUPPORTED
    IMAIO_API II_HIMAGE IIAPI
    ii_image_from_32bpp_rows(int width, int height, png_bytepp rows)
    {
        II_HIMAGE hbm;
        LPDWORD pdw, row;
        int x, y;

        assert(width > 0);
        assert(height > 0);
        assert(rows);

        hbm = ii_create_32bpp(width, height);
        if (hbm)
        {
            pdw = (LPDWORD)ii_get_pixels(hbm);
            for (y = 0; y < height; ++y)
            {
                row = (LPDWORD)rows[height - y - 1 + 0];
                for (x = 0; x < width; ++x)
                {
                    pdw[y * width + x] = row[x];
                }
            }
        }
        return hbm;
    }

    IMAIO_API void IIAPI
    ii_32bpp_rows_from_image(png_bytepp rows, II_HIMAGE hbmImage)
    {
        II_IMGINFO bm;
        int x, y;
        LPDWORD pdw, row;

        if (ii_get_info(hbmImage, &bm))
        {
            pdw = (LPDWORD)bm.bmBits;
            for (y = 0; y < bm.bmHeight; ++y)
            {
                row = (LPDWORD)rows[bm.bmHeight - y - 1];
                for (x = 0; x < bm.bmWidth; ++x)
                {
                    row[x] = pdw[y * bm.bmWidth + x];
                }
            }
        }
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_from_anigif(II_ANIGIF *anigif)
    {
        II_APNG *apng;
        uint32_t i;

        apng = (II_APNG *)calloc(sizeof(II_APNG), 1);
        if (apng == NULL)
            return NULL;

        if (anigif->flags & II_FLAG_USE_SCREEN)
            apng->flags = II_FLAG_USE_SCREEN;
        else
            apng->flags = 0;

        apng->width = anigif->width;
        apng->height = anigif->height;
        apng->num_frames = anigif->num_frames;
        apng->num_plays = anigif->loop_count;
        apng->frames = calloc(sizeof(II_APNG_FRAME), apng->num_frames);
        if (apng->frames)
        {
            for (i = 0; i < apng->num_frames; ++i)
            {
                II_ANIGIF_FRAME *anigif_frame = &anigif->frames[i];
                II_APNG_FRAME *apng_frame = &apng->frames[i];
                apng_frame->x_offset = anigif_frame->x;
                apng_frame->y_offset = anigif_frame->y;
                apng_frame->width = anigif_frame->width;
                apng_frame->height = anigif_frame->height;
                apng_frame->delay = anigif_frame->delay;
                switch (anigif_frame->disposal)
                {
                case 0:
                case 1:
                    apng_frame->dispose_op = PNG_DISPOSE_OP_NONE;
                    break;
                case 2:
                    apng_frame->dispose_op = PNG_DISPOSE_OP_BACKGROUND;
                    break;
                case 3:
                    apng_frame->dispose_op = PNG_DISPOSE_OP_PREVIOUS;
                    break;
                }
                apng_frame->blend_op = PNG_BLEND_OP_OVER;
                if ((apng->flags & II_FLAG_USE_SCREEN) && anigif_frame->hbmScreen)
                {
                    apng_frame->x_offset = 0;
                    apng_frame->y_offset = 0;
                    apng_frame->width = anigif->width;
                    apng_frame->height = anigif->height;
                    apng_frame->hbmScreen = ii_clone(anigif_frame->hbmScreen);
                }
                if (anigif_frame->hbmPart)
                {
                    apng_frame->hbmPart =
                        ii_32bpp_from_trans_8bpp(
                            anigif_frame->hbmPart, &anigif_frame->iTransparent);
                }
            }
        }
        return apng;
    }

    IMAIO_API II_ANIGIF * IIAPI
    ii_anigif_from_apng(II_APNG *apng, bool kill_semitrans)
    {
        II_ANIGIF *anigif;
        uint32_t i;
        int iTransparent;

        iTransparent = -1;

        anigif = (II_ANIGIF *)calloc(sizeof(II_ANIGIF), 1);
        if (anigif == NULL)
            return NULL;

        if (apng->flags & II_FLAG_USE_SCREEN)
            anigif->flags = II_FLAG_USE_SCREEN;
        else
            anigif->flags = 0;
        anigif->width = apng->width;
        anigif->height = apng->height;
        anigif->num_frames = apng->num_frames;
        anigif->loop_count = apng->num_plays;
        anigif->frames = (II_ANIGIF_FRAME *)
                calloc(sizeof(II_ANIGIF_FRAME), anigif->num_frames);
        if (anigif->frames)
        {
            for (i = 0; i < apng->num_frames; ++i)
            {
                II_APNG_FRAME *apng_frame = &apng->frames[i];
                II_ANIGIF_FRAME *anigif_frame = &anigif->frames[i];

                anigif_frame->iTransparent = -1;
                anigif_frame->delay = apng_frame->delay;

                anigif_frame->x = apng_frame->x_offset;
                anigif_frame->y = apng_frame->y_offset;
                anigif_frame->width = apng_frame->width;
                anigif_frame->height = apng_frame->height;
                anigif_frame->hbmScreen = NULL;

                if ((apng->flags & II_FLAG_USE_SCREEN) && apng_frame->hbmScreen)
                {
                    anigif_frame->hbmScreen = ii_clone(apng_frame->hbmScreen);
                    anigif_frame->disposal = 0;
                }
                else
                {
                    anigif_frame->hbmScreen = ii_clone(apng_frame->hbmPart);
                }
                if (kill_semitrans)
                {
                    ii_erase_semitrans(anigif_frame->hbmScreen);
                }
            }

            anigif->global_palette = ii_palette_for_anigif(anigif, 255);
            iTransparent = anigif->global_palette->num_colors;
            anigif->global_palette->num_colors++;

            for (i = 0; i < apng->num_frames; ++i)
            {
                II_APNG_FRAME *apng_frame = &apng->frames[i];
                II_ANIGIF_FRAME *anigif_frame = &anigif->frames[i];
                if ((apng->flags & II_FLAG_USE_SCREEN) == 0 ||
                    apng_frame->hbmScreen == NULL)
                {
                    ii_destroy(anigif_frame->hbmScreen);
                    anigif_frame->hbmScreen = NULL;
                }
                if (apng_frame->hbmPart)
                {
                    anigif_frame->hbmPart =
                        ii_reduce_colors(
                            apng_frame->hbmPart,
                            anigif->global_palette, &iTransparent);
                }
                anigif_frame->iTransparent = iTransparent;
            }
        }
        else
        {
            ii_anigif_destroy(anigif);
            anigif = NULL;
        }

        return anigif;
    }

    IMAIO_API void IIAPI
    ii_apng_destroy(II_APNG *apng)
    {
        uint32_t i;

        if (apng)
        {
            for (i = 0; i < apng->num_frames; ++i)
            {
                II_APNG_FRAME *frame = &apng->frames[i];
                if (frame->hbmScreen)
                {
                    ii_destroy(frame->hbmScreen);
                    frame->hbmScreen = NULL;
                }
                if (frame->hbmPart)
                {
                    ii_destroy(frame->hbmPart);
                    frame->hbmPart = NULL;
                }
            }
            if (apng->hbmDefault)
            {
                ii_destroy(apng->hbmDefault);
                apng->hbmDefault = NULL;
            }
            free(apng->frames);
            free(apng);
        }
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_fp(FILE *fp, II_FLAGS flags)
    {
        png_byte sig[8];
        II_APNG *apng = NULL;
        png_structp png;
        png_infop info;
        png_bytepp rows = NULL;
        png_uint_32 i, k, rowbytes, y;
        bool ok = false;
        II_HIMAGE hbm, hbmScreen = NULL;
        II_APNG_FRAME *frame, *old_frame;
        double gamma;

        if (fp == NULL)
            return NULL;

        /* check signature */
        if (!fread(sig, 8, 1, fp) || !png_check_sig(sig, 8))
        {
            fclose(fp);
            return NULL;
        }

        png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info = png_create_info_struct(png);
        if (png == NULL || info == NULL)
        {
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fp);
            return NULL;
        }

        png_init_io(png, fp);
        png_set_sig_bytes(png, 8);

        png_set_strip_16(png);
        png_set_gray_to_rgb(png);
        png_set_palette_to_rgb(png);
        png_set_bgr(png);
        png_set_packing(png);
        png_set_interlace_handling(png);
        png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
        if (png_get_gAMA(png, info, &gamma))
            png_set_gamma(png, 2.2, gamma);
        else
            png_set_gamma(png, 2.2, 0.45455);

        do
        {
            if (setjmp(png_jmpbuf(png)))
                break;

            png_read_info(png, info);

            /* is it an APNG? */
            if (!png_get_valid(png, info, PNG_INFO_acTL))
            {
                break;
            }

            apng = (II_APNG *)calloc(1, sizeof(II_APNG));
            if (apng == NULL)
                break;

            /* store info */
            apng->width = png_get_image_width(png, info);
            apng->height = png_get_image_height(png, info);
            apng->num_plays = png_get_num_plays(png, info);
            if (flags & II_FLAG_USE_SCREEN)
                apng->flags = II_FLAG_USE_SCREEN;
            else
                apng->flags = 0;

            /* get resolution */
            {
                png_uint_32 res_x, res_y;
                int unit_type;

                apng->dpi = 0.0;
                if (png_get_pHYs(png, info, &res_x, &res_y, &unit_type))
                {
                    if (unit_type == PNG_RESOLUTION_METER)
                        apng->dpi = (float)(res_x * 2.54 / 100.0);
                }
            }

            /* create screen */
            hbmScreen = ii_create_32bpp_trans(apng->width, apng->height);
            if (hbmScreen == NULL)
                break;

            /* allocate rows */
            rows = (png_bytepp)malloc(sizeof(png_bytep) * apng->height);
            if (rows == NULL)
                break;
            rowbytes = (png_uint_32)png_get_rowbytes(png, info);
            for (y = 0; y < apng->height; ++y)
            {
                rows[y] = (png_bytep)malloc(rowbytes);
                if (rows[y] == NULL)
                {
                    for (--y; y < apng->height; --y)
                        free(rows[y]);
                    free(rows);
                    rows = NULL;
                    break;
                }
            }
            if (rows == NULL)
                break;

            /* allocate for frames */
            apng->num_frames = png_get_num_frames(png, info);
            apng->frames = (II_APNG_FRAME *)
                calloc(apng->num_frames, sizeof(II_APNG_FRAME));
            if (apng->frames == NULL)
                break;

            /* for each frame */
            k = 0;
            old_frame = NULL;
            for (i = 0; i < apng->num_frames; ++i)
            {
                frame = &apng->frames[k];

                png_read_frame_head(png, info);

                if (png_get_valid(png, info, PNG_INFO_fcTL))
                {
                    /* frame */
                    uint16_t delay_num, delay_den;

                    png_get_next_frame_fcTL(png, info,
                                            (png_uint_32 *)&frame->width,
                                            (png_uint_32 *)&frame->height,
                                            (png_uint_32 *)&frame->x_offset,
                                            (png_uint_32 *)&frame->y_offset,
                                            &delay_num, &delay_den,
                                            &frame->dispose_op, &frame->blend_op);

                    /* calculate delay time */
                    if (delay_den == 0)
                        delay_den = 100;
                    frame->delay = (delay_num * 1000) / delay_den;

                    /* create a part */
                    png_read_image(png, rows);
                    hbm = ii_image_from_32bpp_rows(frame->width, frame->height, rows);
                    frame->hbmPart = hbm;

                    /* blending */
                    switch (frame->blend_op)
                    {
                    case PNG_BLEND_OP_SOURCE:
                        {
                            HDC hdc1, hdc2;
                            HGDIOBJ hbm1Old, hbm2Old;
                            hdc1 = CreateCompatibleDC(NULL);
                            hdc2 = CreateCompatibleDC(NULL);
                            hbm1Old = SelectObject(hdc1, hbmScreen);
                            hbm2Old = SelectObject(hdc2, hbm);
                            BitBlt(hdc1,
                                frame->x_offset, frame->y_offset,
                                frame->width, frame->height,
                                hdc2, 0, 0, SRCCOPY);
                            SelectObject(hdc1, hbm1Old);
                            SelectObject(hdc1, hbm2Old);
                            DeleteDC(hdc1);
                            DeleteDC(hdc2);
                        }
                        break;
                    case PNG_BLEND_OP_OVER:
                        ii_stamp(hbmScreen, frame->x_offset, frame->y_offset,
                                 hbm, NULL, 255);
                        break;
                    }

                    /* create a screen image */
                    if (flags & II_FLAG_USE_SCREEN)
                    {
                        frame->hbmScreen = ii_clone(hbmScreen);

                        /* dispose */
                        switch (frame->dispose_op)
                        {
                        case PNG_DISPOSE_OP_BACKGROUND:
                            ii_destroy(hbmScreen);
                            hbmScreen = ii_create_32bpp_trans(apng->width, apng->height);
                            break;
                        case PNG_DISPOSE_OP_PREVIOUS:
                            if (old_frame)
                            {
                                ii_destroy(hbmScreen);
                                hbmScreen = ii_clone(old_frame->hbmScreen);
                            }
                            break;
                        }
                    }

                    old_frame = frame;
                    ++k;
                }
                else
                {
                    /* default image */
                    png_read_image(png, rows);
                    apng->flags |= II_FLAG_DEFAULT_PRESENT;
                    hbm = ii_image_from_32bpp_rows(apng->width, apng->height, rows);
                    apng->hbmDefault = hbm;
                }
            }
            ok = true;
            png_read_end(png, info);
        } while (0);

        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);

        if (apng && rows)
        {
            for (y = 0; y < apng->height; ++y)
            {
                free(rows[y]);
            }
            free(rows);
        }

        if (apng)
        {
            if (apng->flags & II_FLAG_DEFAULT_PRESENT)
                apng->num_frames -= 1;

            if (ok && apng->num_frames > 0)
            {
                if (apng->hbmDefault == NULL)
                {
                    apng->hbmDefault = ii_clone(apng->frames[0].hbmScreen);
                }
            }
            else
            {
                ii_apng_destroy(apng);
                apng = NULL;
            }
        }

        if (hbmScreen)
        {
            ii_destroy(hbmScreen);
        }

        return apng;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_mem(II_LPCVOID pv, uint32_t cb, II_FLAGS flags)
    {
        II_APNG *apng = NULL;
        png_structp png;
        png_infop info;
        png_bytepp rows = NULL;
        png_uint_32 i, k, rowbytes, y;
        bool ok = false;
        II_HIMAGE hbm, hbmScreen = NULL;
        II_APNG_FRAME *frame, *old_frame;
        double gamma;
        II_MEMORY       memory;

        memory.m_pb = (const uint8_t *)pv;
        memory.m_i = 0;
        memory.m_size = cb;

        /* check signature */
        if (cb < 8 || !png_check_sig(memory.m_pb, 8))
        {
            return NULL;
        }
        memory.m_i = 8;

        png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info = png_create_info_struct(png);
        if (png == NULL || info == NULL)
        {
            png_destroy_read_struct(&png, &info, NULL);
            return NULL;
        }

        png_set_read_fn(png, &memory, ii_png_mem_read);
        png_set_sig_bytes(png, 8);

        png_set_strip_16(png);
        png_set_gray_to_rgb(png);
        png_set_palette_to_rgb(png);
        png_set_bgr(png);
        png_set_packing(png);
        png_set_interlace_handling(png);
        png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
        if (png_get_gAMA(png, info, &gamma))
            png_set_gamma(png, 2.2, gamma);
        else
            png_set_gamma(png, 2.2, 0.45455);

        do
        {
            if (setjmp(png_jmpbuf(png)))
                break;

            png_read_info(png, info);

            /* is it an APNG? */
            if (!png_get_valid(png, info, PNG_INFO_acTL))
                break;

            apng = (II_APNG *)calloc(1, sizeof(II_APNG));
            if (apng == NULL)
                break;

            /* store info */
            apng->width = png_get_image_width(png, info);
            apng->height = png_get_image_height(png, info);
            apng->num_plays = png_get_num_plays(png, info);
            if (flags & II_FLAG_USE_SCREEN)
                apng->flags = II_FLAG_USE_SCREEN;
            else
                apng->flags = 0;

            /* get resolution */
            {
                png_uint_32 res_x, res_y;
                int unit_type;

                apng->dpi = 0.0;
                if (png_get_pHYs(png, info, &res_x, &res_y, &unit_type))
                {
                    if (unit_type == PNG_RESOLUTION_METER)
                        apng->dpi = (float)(res_x * 2.54 / 100.0);
                }
            }

            /* create screen */
            hbmScreen = ii_create_32bpp_trans(apng->width, apng->height);
            if (hbmScreen == NULL)
                break;

            /* allocate rows */
            rows = (png_bytepp)malloc(sizeof(png_bytep) * apng->height);
            if (rows == NULL)
                break;
			rowbytes = (png_uint_32)png_get_rowbytes(png, info);
            for (y = 0; y < apng->height; ++y)
            {
                rows[y] = (png_bytep)malloc(rowbytes);
                if (rows[y] == NULL)
                {
                    for (--y; y < apng->height; --y)
                        free(rows[y]);
                    free(rows);
                    rows = NULL;
                    break;
                }
            }
            if (rows == NULL)
                break;

            /* allocate for frames */
            apng->num_frames = png_get_num_frames(png, info);
            apng->frames = (II_APNG_FRAME *)
                calloc(apng->num_frames, sizeof(II_APNG_FRAME));
            if (apng->frames == NULL)
                break;

            /* for each frame */
            k = 0;
            old_frame = NULL;
            for (i = 0; i < apng->num_frames; ++i)
            {
                frame = &apng->frames[k];

                png_read_frame_head(png, info);

                if (png_get_valid(png, info, PNG_INFO_fcTL))
                {
                    /* frame */
                    uint16_t delay_num, delay_den;

                    png_get_next_frame_fcTL(png, info,
                                            (png_uint_32 *)&frame->width,
                                            (png_uint_32 *)&frame->height,
                                            (png_uint_32 *)&frame->x_offset,
                                            (png_uint_32 *)&frame->y_offset,
                                            &delay_num, &delay_den,
                                            &frame->dispose_op, &frame->blend_op);

                    /* calculate delay time */
                    if (delay_den == 0)
                        delay_den = 100;
                    frame->delay = (delay_num * 1000) / delay_den;

                    /* create a part */
                    png_read_image(png, rows);
                    hbm = ii_image_from_32bpp_rows(frame->width, frame->height, rows);
                    frame->hbmPart = hbm;

                    /* blending */
                    switch (frame->blend_op)
                    {
                    case PNG_BLEND_OP_SOURCE:
                        {
                            HDC hdc1, hdc2;
                            HGDIOBJ hbm1Old, hbm2Old;
                            hdc1 = CreateCompatibleDC(NULL);
                            hdc2 = CreateCompatibleDC(NULL);
                            hbm1Old = SelectObject(hdc1, hbmScreen);
                            hbm2Old = SelectObject(hdc2, hbm);
                            BitBlt(hdc1,
                                frame->x_offset, frame->y_offset,
                                frame->width, frame->height,
                                hdc2, 0, 0, SRCCOPY);
                            SelectObject(hdc1, hbm1Old);
                            SelectObject(hdc1, hbm2Old);
                            DeleteDC(hdc1);
                            DeleteDC(hdc2);
                        }
                        break;
                    case PNG_BLEND_OP_OVER:
                        ii_stamp(hbmScreen, frame->x_offset, frame->y_offset,
                                 hbm, NULL, 255);
                        break;
                    }

                    /* create a screen image */
                    if (flags & II_FLAG_USE_SCREEN)
                    {
                        frame->hbmScreen = ii_clone(hbmScreen);

                        /* dispose */
                        switch (frame->dispose_op)
                        {
                        case PNG_DISPOSE_OP_BACKGROUND:
                            ii_destroy(hbmScreen);
                            hbmScreen = ii_create_32bpp_trans(apng->width, apng->height);
                            break;
                        case PNG_DISPOSE_OP_PREVIOUS:
                            if (old_frame)
                            {
                                ii_destroy(hbmScreen);
                                hbmScreen = ii_clone(old_frame->hbmScreen);
                            }
                            break;
                        }
                    }

                    old_frame = frame;
                    ++k;
                }
                else
                {
                    /* default image */
                    png_read_image(png, rows);
                    apng->flags |= II_FLAG_DEFAULT_PRESENT;
                    hbm = ii_image_from_32bpp_rows(apng->width, apng->height, rows);
                    apng->hbmDefault = hbm;
                }
            }
            ok = true;
            png_read_end(png, info);
        } while (0);

        png_destroy_read_struct(&png, &info, NULL);

        if (apng && rows)
        {
            for (y = 0; y < apng->height; ++y)
            {
                free(rows[y]);
            }
            free(rows);
        }

        if (apng)
        {
            if (apng->flags & II_FLAG_DEFAULT_PRESENT)
                apng->num_frames -= 1;

            if (ok && apng->num_frames > 0)
            {
                if (apng->hbmDefault == NULL)
                {
                    apng->hbmDefault = ii_clone(apng->frames[0].hbmScreen);
                }
            }
            else
            {
                ii_apng_destroy(apng);
                apng = NULL;
            }
        }

        if (hbmScreen)
        {
            ii_destroy(hbmScreen);
        }

        return apng;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_a(II_CSTR pszFileName, II_FLAGS flags)
    {
        FILE *fp;
        fp = fopen(pszFileName, "rb");
        if (fp)
            return ii_apng_load_fp(fp, flags);
        return NULL;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_w(II_CWSTR pszFileName, II_FLAGS flags)
    {
        FILE *fp;
        fp = _wfopen(pszFileName, L"rb");
        if (fp)
            return ii_apng_load_fp(fp, flags);
        return NULL;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_res_a(II_INST hInstance, II_CSTR pszResName, II_FLAGS flags)
    {
        II_APNG *apng;
        HGLOBAL hGlobal;
        uint32_t dwSize;
        LPVOID lpData;
        HRSRC hRsrc;

        assert(pszResName);
        hRsrc = FindResourceA(hInstance, pszResName, "PNG");
        if (hRsrc == NULL)
        {
            hRsrc = FindResourceA(hInstance, pszResName, "APNG");
            if (hRsrc == NULL)
            {
                return NULL;
            }
        }

        dwSize = SizeofResource(hInstance, hRsrc);
        hGlobal = LoadResource(hInstance, hRsrc);
        if (hGlobal == NULL)
            return NULL;

        lpData = LockResource(hGlobal);
        apng = ii_apng_load_mem(lpData, dwSize, flags);

    #ifdef WIN16
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
    #endif

        return apng;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_res_w(II_INST hInstance, II_CWSTR pszResName, II_FLAGS flags)
    {
        II_APNG *apng;
        HGLOBAL hGlobal;
        uint32_t dwSize;
        LPVOID lpData;
        HRSRC hRsrc;

        assert(pszResName);
        hRsrc = FindResourceW(hInstance, pszResName, L"PNG");
        if (hRsrc == NULL)
        {
            hRsrc = FindResourceW(hInstance, pszResName, L"APNG");
            if (hRsrc == NULL)
            {
                return NULL;
            }
        }

        dwSize = SizeofResource(hInstance, hRsrc);
        hGlobal = LoadResource(hInstance, hRsrc);
        if (hGlobal == NULL)
            return NULL;

        lpData = LockResource(hGlobal);
        apng = ii_apng_load_mem(lpData, dwSize, flags);

    #ifdef WIN16
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
    #endif

        return apng;
    }

    IMAIO_API bool IIAPI
    ii_apng_save_fp(FILE *fp, II_APNG *apng)
    {
        png_structp png;
        png_infop info;
        png_bytepp rows = NULL;
        uint32_t y, i, k;
        png_color_8 sBIT;
        bool ok = false;
        png_uint_32 rowbytes;

        if (apng == NULL)
        {
            fclose(fp);
            return false;
        }

        png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info = png_create_info_struct(png);
        if (png == NULL || info == NULL)
        {
            png_destroy_write_struct(&png, &info);
            fclose(fp);
            return false;
        }

        png_init_io(png, fp);

        do
        {
            if (setjmp(png_jmpbuf(png)))
            {
                break;
            }

            png_set_IHDR(png, info, apng->width, apng->height,
                         8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

            sBIT.red = 8;
            sBIT.green = 8;
            sBIT.blue = 8;
            sBIT.alpha = 8;
            png_set_sBIT(png, info, &sBIT);

            if (apng->dpi != 0.0)
            {
                png_uint_32 res = (png_uint_32)(apng->dpi * 100 / 2.54 + 0.5);
                png_set_pHYs(png, info, res, res, PNG_RESOLUTION_METER);
            }

            png_set_bgr(png);
            if ((apng->flags & II_FLAG_DEFAULT_PRESENT) && apng->hbmDefault)
                png_set_acTL(png, info, apng->num_frames + 1, apng->num_plays);
            else
                png_set_acTL(png, info, apng->num_frames, apng->num_plays);

            /* allocate rows */
            rows = (png_bytepp)calloc(sizeof(png_bytep), apng->height);
            if (rows == NULL)
                break;
			rowbytes = (png_uint_32)png_get_rowbytes(png, info);
            for (y = 0; y < apng->height; ++y)
            {
                rows[y] = (png_bytep)malloc(rowbytes);
                if (rows[y] == NULL)
                {
                    for (k = y - 1; k < apng->height; --k)
                        free(rows[y]);
                    free(rows);
                    rows = NULL;
                    break;
                }
            }
            if (rows == NULL)
                break;

            if ((apng->flags & II_FLAG_DEFAULT_PRESENT) && apng->hbmDefault)
            {
                /* write the default image */
                png_set_first_frame_is_hidden(png, info, 1);

                png_write_info(png, info);

                ii_32bpp_rows_from_image(rows, apng->hbmDefault);
                png_write_frame_head(
                    png, info, rows, 
                    apng->width, apng->height, 0, 0,
                    0, 0,
                    PNG_DISPOSE_OP_NONE,
                    PNG_BLEND_OP_OVER
                );

                png_write_image(png, rows);
                png_write_frame_tail(png, info);
            }

            for (i = 0; i < apng->num_frames; ++i)
            {
                II_APNG_FRAME *frame = &apng->frames[i];

                png_write_info(png, info);
                if (apng->flags & II_FLAG_USE_SCREEN)
                {
                    ii_32bpp_rows_from_image(rows, frame->hbmScreen);
                    png_write_frame_head(
                        png, info, rows, 
                        apng->width, apng->height, 0, 0,
                        frame->delay, 1000, 
                        frame->dispose_op,
                        frame->blend_op
                    );
                }
                else
                {
                    ii_32bpp_rows_from_image(rows, frame->hbmPart);
                    png_write_frame_head(
                        png, info, rows, 
                        frame->width, frame->height,
                        frame->x_offset, frame->y_offset,
                        frame->delay, 1000, 
                        frame->dispose_op,
                        frame->blend_op
                    );
                }

                png_write_image(png, rows);
                png_write_frame_tail(png, info);
            }
            ok = true;
            png_write_end(png, NULL);
        } while (0);

        if (rows)
        {
            for (y = 0; y < apng->height; ++y)
            {
                free(rows[y]);
            }
            free(rows);
        }

        png_destroy_write_struct(&png, &info);
        fclose(fp);

        return ok;
    }

    IMAIO_API bool IIAPI
    ii_apng_save_a(II_CSTR pszFileName, II_APNG *apng)
    {
        FILE *fp;
        fp = fopen(pszFileName, "wb");
        if (fp)
        {
            if (ii_apng_save_fp(fp, apng))
                return true;
        }
        DeleteFileA(pszFileName);
        return false;
    }

    IMAIO_API bool IIAPI
    ii_apng_save_w(II_CWSTR pszFileName, II_APNG *apng)
    {
        FILE *fp;
        fp = _wfopen(pszFileName, L"wb");
        if (fp)
        {
            if (ii_apng_save_fp(fp, apng))
                return true;
        }
        DeleteFileW(pszFileName);
        return false;
    }
#endif  /* def PNG_APNG_SUPPORTED */

/*****************************************************************************/

IMAIO_API II_HIMAGE IIAPI
ii_tif_load_common(TIFF *tif, float *dpi)
{
    BITMAPINFO bi;
    II_HIMAGE hbm, hbm32Bpp, hbm24Bpp;
    uint32_t *pdwBits, *pdw;
    void *pvBits;
    II_DEVICE hdc1, hdc2;
    uint16 resunit;
    uint32 w, h;
    size_t cPixels;
    uint8 r, g, b, a;
    bool fOpaque;

    assert(tif);
    if (tif == NULL)
        return NULL;

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biPlanes   = 1;
    bi.bmiHeader.biBitCount = 32;

    hdc1 = CreateCompatibleDC(NULL);
    hdc2 = CreateCompatibleDC(NULL);
    if (hdc1 == NULL || hdc2 == NULL)
    {
        DeleteDC(hdc1);
        DeleteDC(hdc2);
        TIFFClose(tif);
        return NULL;
    }

    hbm = NULL;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &resunit);

    if (dpi)
    {
        if (!TIFFGetField(tif, TIFFTAG_XRESOLUTION, dpi))
        {
            *dpi = 0.0f;
        }
        else if (resunit == RESUNIT_CENTIMETER)
        {
            *dpi *= 2.54f;
        }
        else
        {
            *dpi = 0.0f;
        }
    }
    bi.bmiHeader.biWidth    = w;
    bi.bmiHeader.biHeight   = h;
    cPixels = w * h;
    hbm32Bpp = CreateDIBSection(hdc1, &bi, DIB_RGB_COLORS,
        (void **)&pdwBits, NULL, 0);
    if (hbm32Bpp)
    {
        if (TIFFReadRGBAImageOriented(tif, w, h, (uint32 *)pdwBits,
            ORIENTATION_BOTLEFT, 0))
        {
            pdw = pdwBits;
            fOpaque = true;
            while (cPixels--)
            {
                r = (uint8)TIFFGetR(*pdw);
                g = (uint8)TIFFGetG(*pdw);
                b = (uint8)TIFFGetB(*pdw);
                a = (uint8)TIFFGetA(*pdw);
                if (a != 255)
                {
                    fOpaque = false;
                }
                *pdw++ = (uint32)b | (((uint32)g) << 8) |
                    (((uint32)r) << 16) | (((uint32)a) << 24);
            }
            if (fOpaque)
            {
                bi.bmiHeader.biBitCount = 24;
                hbm24Bpp = CreateDIBSection(hdc2, &bi, DIB_RGB_COLORS,
                    &pvBits, NULL, 0);
                if (hbm24Bpp)
                {
                    HGDIOBJ hbmOld1 = SelectObject(hdc1, hbm32Bpp);
                    HGDIOBJ hbmOld2 = SelectObject(hdc2, hbm24Bpp);
                    BitBlt(hdc2, 0, 0, w, h, hdc1, 0, 0, SRCCOPY);
                    SelectObject(hdc1, hbmOld1);
                    SelectObject(hdc2, hbmOld2);
                    hbm = hbm24Bpp;
                    DeleteObject(hbm32Bpp);
                }
            }
            else
            {
                hbm = hbm32Bpp;
            }
        }
    }

    TIFFClose(tif);
    DeleteDC(hdc1);
    DeleteDC(hdc2);

    return hbm;
}

IMAIO_API II_HIMAGE IIAPI
ii_tif_load_a(II_CSTR pszFileName, float *dpi)
{
    TIFF* tif;
    TIFFSetWarningHandler(NULL);
    TIFFSetWarningHandlerExt(NULL);
    tif = TIFFOpen(pszFileName, "r");
    if (tif)
        return ii_tif_load_common(tif, dpi);
    return NULL;
}

IMAIO_API II_HIMAGE IIAPI
ii_tif_load_w(II_CWSTR pszFileName, float *dpi)
{
    TIFF* tif;
    TIFFSetWarningHandler(NULL);
    TIFFSetWarningHandlerExt(NULL);
    tif = TIFFOpenW(pszFileName, "r");
    if (tif)
        return ii_tif_load_common(tif, dpi);
    return NULL;
}

IMAIO_API bool IIAPI
ii_tif_save_common(TIFF *tif, II_HIMAGE hbm, float dpi)
{
    II_IMGINFO bm;
    bool no_alpha;
    BITMAPINFO bi;
    int32_t widthbytes;
    uint8_t *pbBits, *pb;
    int c, y;
    bool f;
    II_DEVICE hdc;

    assert(tif);
    if (tif == NULL)
        return false;

    if (!ii_get_info(hbm, &bm))
    {
        TIFFClose(tif);
        return false;
    }

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth    = bm.bmWidth;
    bi.bmiHeader.biHeight   = bm.bmHeight;
    bi.bmiHeader.biPlanes   = 1;

    no_alpha = (bm.bmBitsPixel <= 24 || ii_is_opaque(hbm));
    bi.bmiHeader.biBitCount = (WORD)(no_alpha ? 24 : 32);
    widthbytes = II_WIDTHBYTES(bm.bmWidth * bi.bmiHeader.biBitCount);
    pbBits = (uint8_t *)malloc(widthbytes * bm.bmHeight);
    if (pbBits == NULL)
    {
        TIFFClose(tif);
        return false;
    }

    hdc = CreateCompatibleDC(NULL);
    if (!GetDIBits(hdc, hbm, 0, bm.bmHeight, pbBits, &bi, DIB_RGB_COLORS))
    {
        DeleteDC(hdc);
        free(pbBits);
        TIFFClose(tif);
        return false;
    }
    DeleteDC(hdc);

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, bm.bmWidth);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, bm.bmHeight);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, no_alpha ? 3 : 4);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    if (dpi != 0.0)
    {
        TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
        TIFFSetField(tif, TIFFTAG_XRESOLUTION, dpi);
        TIFFSetField(tif, TIFFTAG_YRESOLUTION, dpi);
    }
    TIFFSetField(tif, TIFFTAG_SOFTWARE, "katayama_hirofumi_mz's software");
    f = true;
    for (y = 0; y < bm.bmHeight; y++)
    {
        pb = &pbBits[(bm.bmHeight - 1 - y) * widthbytes];
        c = bm.bmWidth;
        if (no_alpha)
        {
            uint8_t b;
            while (c--)
            {
                b = pb[2];
                pb[2] = *pb;
                *pb = b;
                pb++; pb++; pb++;
            }
        }
        else
        {
            uint8_t b;
            while (c--)
            {
                b = pb[2];
                pb[2] = *pb;
                *pb = b;
                pb++; pb++; pb++; pb++;
            }
        }
        if (TIFFWriteScanline(tif,
            &pbBits[(bm.bmHeight - 1 - y) * widthbytes], y, 0) < 0)
        {
            f = false;
            break;
        }
    }
    TIFFClose(tif);

    free(pbBits);
    return f;
}

IMAIO_API bool IIAPI
ii_tif_save_a(II_CSTR pszFileName, II_HIMAGE hbm, float dpi)
{
    TIFF *tif;
    TIFFSetWarningHandler(NULL);
    TIFFSetWarningHandlerExt(NULL);
    tif = TIFFOpen(pszFileName, "w");
    if (tif)
    {
        if (ii_tif_save_common(tif, hbm, dpi))
            return true;
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_tif_save_w(II_CWSTR pszFileName, II_HIMAGE hbm, float dpi)
{
    TIFF *tif;
    TIFFSetWarningHandler(NULL);
    TIFFSetWarningHandlerExt(NULL);
    tif = TIFFOpenW(pszFileName, "w");
    if (tif)
    {
        if (ii_tif_save_common(tif, hbm, dpi))
            return true;
        DeleteFileW(pszFileName);
    }
    return false;
}

/*****************************************************************************/
/* image types */

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_path_name_a(II_CSTR  pszFileName)
{
    II_CSTR pchDotExt = ii_find_dotext_a(pszFileName);
    return ii_image_type_from_dotext_a(pchDotExt);
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_path_name_w(II_CWSTR pszFileName)
{
    II_CWSTR pchDotExt = ii_find_dotext_w(pszFileName);
    return ii_image_type_from_dotext_w(pchDotExt);
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_dotext_a(II_CSTR  pchDotExt)
{
    II_CSTR pch;

    if (pchDotExt == NULL || *pchDotExt == 0)
        return II_IMAGE_TYPE_INVALID;
    if (*pchDotExt == '.')
        pch = pchDotExt + 1;
    else
        pch = pchDotExt;

    if (lstrcmpiA(pch, "jpg") == 0 || lstrcmpiA(pch, "jpe") == 0 ||
        lstrcmpiA(pch, "jpeg") == 0 || lstrcmpiA(pch, "jfif") == 0)
    {
        return II_IMAGE_TYPE_JPG;
    }
    else if (lstrcmpiA(pch, "gif") == 0)
    {
        return II_IMAGE_TYPE_GIF;
    }
    else if (lstrcmpiA(pch, "png") == 0)
    {
        return II_IMAGE_TYPE_PNG;
    }
    else if (lstrcmpiA(pch, "apng") == 0)
    {
        return II_IMAGE_TYPE_APNG;
    }
    else if (lstrcmpiA(pch, "tif") == 0 || lstrcmpiA(pch, "tiff") == 0)
    {
        return II_IMAGE_TYPE_TIF;
    }
    else if (lstrcmpiA(pch, "bmp") == 0 || lstrcmpiA(pch, "dib") == 0)
    {
        return II_IMAGE_TYPE_BMP;
    }
    return II_IMAGE_TYPE_INVALID;
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_dotext_w(II_CWSTR pchDotExt)
{
    II_CWSTR pch;

    if (pchDotExt == NULL || *pchDotExt == 0)
        return II_IMAGE_TYPE_INVALID;
    if (*pchDotExt == L'.')
        pch = pchDotExt + 1;
    else
        pch = pchDotExt;

    if (lstrcmpiW(pch, L"jpg") == 0 || lstrcmpiW(pch, L"jpe") == 0 ||
        lstrcmpiW(pch, L"jpeg") == 0 || lstrcmpiW(pch, L"jfif") == 0)
    {
        return II_IMAGE_TYPE_JPG;
    }
    else if (lstrcmpiW(pch, L"gif") == 0)
    {
        return II_IMAGE_TYPE_GIF;
    }
    else if (lstrcmpiW(pch, L"png") == 0)
    {
        return II_IMAGE_TYPE_PNG;
    }
    else if (lstrcmpiW(pch, L"apng") == 0)
    {
        return II_IMAGE_TYPE_APNG;
    }
    else if (lstrcmpiW(pch, L"tif") == 0 || lstrcmpiW(pch, L"tiff") == 0)
    {
        return II_IMAGE_TYPE_TIF;
    }
    else if (lstrcmpiW(pch, L"bmp") == 0 || lstrcmpiW(pch, L"dib") == 0)
    {
        return II_IMAGE_TYPE_BMP;
    }
    return II_IMAGE_TYPE_INVALID;
}

IMAIO_API II_CSTR IIAPI
ii_wildcards_from_image_type_a(II_IMAGE_TYPE type)
{
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return "*.jpg;*.jpe;*.jpeg;*.jfif";
    case II_IMAGE_TYPE_GIF:
    case II_IMAGE_TYPE_ANIGIF:
        return "*.gif";
    case II_IMAGE_TYPE_PNG:
        return "*.png";
    case II_IMAGE_TYPE_APNG:
        return "*.png;*.apng";
    case II_IMAGE_TYPE_TIF:
        return "*.tif;*.tiff";
    case II_IMAGE_TYPE_BMP:
        return "*.bmp;*.dib";
    default:
        return "";
    }
}

IMAIO_API II_CWSTR IIAPI
ii_wildcards_from_image_type_w(II_IMAGE_TYPE type)
{
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return L"*.jpg;*.jpe;*.jpeg;*.jfif";
    case II_IMAGE_TYPE_GIF:
    case II_IMAGE_TYPE_ANIGIF:
        return L"*.gif";
    case II_IMAGE_TYPE_PNG:
        return L"*.png";
    case II_IMAGE_TYPE_APNG:
        return L"*.png;*.apng";
    case II_IMAGE_TYPE_TIF:
        return L"*.tif;*.tiff";
    case II_IMAGE_TYPE_BMP:
        return L"*.bmp;*.dib";
    default:
        return L"";
    }
}

IMAIO_API II_CSTR IIAPI
ii_find_file_title_a(II_CSTR pszPath)
{
    LPCSTR pch1, pch2;

    assert(pszPath);
    #ifdef ANSI
        pch1 = strrchr(pszPath, '\\');
        pch2 = strrchr(pszPath, '/');
    #else
        pch1 = (LPCSTR)_mbsrchr((const BYTE *)(pszPath), '\\');
        pch2 = (LPCSTR)_mbsrchr((const BYTE *)(pszPath), '/');
    #endif
    if (pch1 == NULL && pch2 == NULL)
        return pszPath;
    if (pch1 == NULL)
        return pch2 + 1;
    if (pch2 == NULL)
        return pch1 + 1;
    if (pch1 < pch2)
        return pch2 + 1;
    else
        return pch1 + 1;
}

IMAIO_API II_CWSTR IIAPI
ii_find_file_title_w(II_CWSTR pszPath)
{
    LPCWSTR pch1, pch2;

    assert(pszPath);
    pch1 = wcsrchr(pszPath, L'\\');
    pch2 = wcsrchr(pszPath, L'/');
    if (pch1 == NULL && pch2 == NULL)
        return pszPath;
    if (pch1 == NULL)
        return pch2 + 1;
    if (pch2 == NULL)
        return pch1 + 1;
    if (pch1 < pch2)
        return pch2 + 1;
    else
        return pch1 + 1;
}

IMAIO_API II_CSTR IIAPI
ii_find_dotext_a(II_CSTR pszPath)
{
    LPCSTR pch;

    assert(pszPath);
    pszPath = ii_find_file_title_a(pszPath);
    #ifdef ANSI
        pch = strrchr(pszPath, '.');
    #else
        pch = (LPCSTR)_mbsrchr((const BYTE *)pszPath, '.');
    #endif
    if (pch)
        return pch;
    else
        return pszPath + lstrlenA(pszPath);
}

IMAIO_API II_CWSTR IIAPI
ii_find_dotext_w(II_CWSTR pszPath)
{
    LPCWSTR pch;

    assert(pszPath);
    pszPath = ii_find_file_title_w(pszPath);
    pch = wcsrchr(pszPath, L'.');
    if (pch)
        return pch;
    else
        return pszPath + lstrlenW(pszPath);
}

IMAIO_API II_CSTR IIAPI
ii_mime_from_path_name_a(II_CSTR pszFileName)
{
    II_CSTR pchDotExt = ii_find_dotext_a(pszFileName);
    return ii_mime_from_dotext_a(pchDotExt);
}

IMAIO_API II_CWSTR IIAPI
ii_mime_from_path_name_w(II_CWSTR pszFileName)
{
    II_CWSTR pchDotExt = ii_find_dotext_w(pszFileName);
    return ii_mime_from_dotext_w(pchDotExt);
}

IMAIO_API II_CSTR IIAPI
ii_mime_from_dotext_a(II_CSTR pchDotExt)
{
    II_IMAGE_TYPE type = ii_image_type_from_dotext_a(pchDotExt);
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return "image/jpeg";
    case II_IMAGE_TYPE_GIF:
    case II_IMAGE_TYPE_ANIGIF:
        return "image/gif";
    case II_IMAGE_TYPE_PNG:
    case II_IMAGE_TYPE_APNG:
        return "image/png";
    case II_IMAGE_TYPE_TIF:
        return "image/tiff";
    case II_IMAGE_TYPE_BMP:
        return "image/bmp";
    default:
        return "";
    }
}

IMAIO_API II_CWSTR IIAPI
ii_mime_from_dotext_w(II_CWSTR pchDotExt)
{
    II_IMAGE_TYPE type = ii_image_type_from_dotext_w(pchDotExt);
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return L"image/jpeg";
    case II_IMAGE_TYPE_GIF:
    case II_IMAGE_TYPE_ANIGIF:
        return L"image/gif";
    case II_IMAGE_TYPE_PNG:
    case II_IMAGE_TYPE_APNG:
        return L"image/png";
    case II_IMAGE_TYPE_TIF:
        return L"image/tiff";
    case II_IMAGE_TYPE_BMP:
        return L"image/bmp";
    default:
        return L"";
    }
}

IMAIO_API II_CSTR IIAPI
ii_dotext_from_mime_a(II_CSTR  pszMIME)
{
    if (lstrcmpiA(pszMIME, "image/jpeg") == 0)
        return ".jpg";
    if (lstrcmpiA(pszMIME, "image/gif") == 0)
        return ".gif";
    if (lstrcmpiA(pszMIME, "image/png") == 0)
        return ".png";
    if (lstrcmpiA(pszMIME, "image/tiff") == 0)
        return ".tif";
    if (lstrcmpiA(pszMIME, "image/bmp") == 0)
        return ".bmp";
    return "";
}

IMAIO_API II_CWSTR IIAPI
ii_dotext_from_mime_w(II_CWSTR pszMIME)
{
    if (lstrcmpiW(pszMIME, L"image/jpeg") == 0)
        return L".jpg";
    if (lstrcmpiW(pszMIME, L"image/gif") == 0)
        return L".gif";
    if (lstrcmpiW(pszMIME, L"image/png") == 0)
        return L".png";
    if (lstrcmpiW(pszMIME, L"image/tiff") == 0)
        return L".tif";
    if (lstrcmpiW(pszMIME, L"image/bmp") == 0)
        return L".bmp";
    return L"";
}

IMAIO_API II_CSTR IIAPI
ii_mime_from_image_type_a(II_IMAGE_TYPE type)
{
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return "image/jpeg";
    case II_IMAGE_TYPE_GIF:
        return "image/gif";
    case II_IMAGE_TYPE_PNG:
    case II_IMAGE_TYPE_APNG:
        return "image/png";
    case II_IMAGE_TYPE_TIF:
        return "image/tiff";
    case II_IMAGE_TYPE_BMP:
        return "image/bmp";
    default:
        return "";
    }
}

IMAIO_API II_CWSTR IIAPI
ii_mime_from_image_type_w(II_IMAGE_TYPE type)
{
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return L"image/jpeg";
    case II_IMAGE_TYPE_GIF:
        return L"image/gif";
    case II_IMAGE_TYPE_PNG:
    case II_IMAGE_TYPE_APNG:
        return L"image/png";
    case II_IMAGE_TYPE_TIF:
        return L"image/tiff";
    case II_IMAGE_TYPE_BMP:
        return L"image/bmp";
    default:
        return L"";
    }
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_mime_a(II_CSTR pszMIME)
{
    if (lstrcmpiA(pszMIME, "image/jpeg") == 0)
        return II_IMAGE_TYPE_JPG;
    if (lstrcmpiA(pszMIME, "image/gif") == 0)
        return II_IMAGE_TYPE_GIF;
    if (lstrcmpiA(pszMIME, "image/png") == 0)
        return II_IMAGE_TYPE_PNG;
    if (lstrcmpiA(pszMIME, "image/apng") == 0)
        return II_IMAGE_TYPE_APNG;
    if (lstrcmpiA(pszMIME, "image/tiff") == 0)
        return II_IMAGE_TYPE_TIF;
    if (lstrcmpiA(pszMIME, "image/bmp") == 0)
        return II_IMAGE_TYPE_BMP;
    return II_IMAGE_TYPE_INVALID;
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_mime_w(II_CWSTR pszMIME)
{
    if (lstrcmpiW(pszMIME, L"image/jpeg") == 0)
        return II_IMAGE_TYPE_JPG;
    if (lstrcmpiW(pszMIME, L"image/gif") == 0)
        return II_IMAGE_TYPE_GIF;
    if (lstrcmpiW(pszMIME, L"image/png") == 0)
        return II_IMAGE_TYPE_PNG;
    if (lstrcmpiW(pszMIME, L"image/apng") == 0)
        return II_IMAGE_TYPE_APNG;
    if (lstrcmpiW(pszMIME, L"image/tiff") == 0)
        return II_IMAGE_TYPE_TIF;
    if (lstrcmpiW(pszMIME, L"image/bmp") == 0)
        return II_IMAGE_TYPE_BMP;
    return II_IMAGE_TYPE_INVALID;
}

IMAIO_API char * IIAPI
ii_make_filter_a(char *pszFilter)
{
    char *pszSave;

    assert(pszFilter);
    pszSave = pszFilter;
    while (*pszFilter)
    {
        if (*pszFilter == '|')
        {
            *pszFilter = 0;
            pszFilter++;
        }
        else
        {
            pszFilter = CharNextA(pszFilter);
        }
    }
    return pszSave;
}

IMAIO_API wchar_t * IIAPI
ii_make_filter_w(wchar_t *pszFilter)
{
    wchar_t *pszSave;

    assert(pszFilter);
    pszSave = pszFilter;
    while (*pszFilter)
    {
        if (*pszFilter == L'|')
        {
            *pszFilter = 0;
        }
        pszFilter++;
    }
    return pszSave;
}

/*****************************************************************************/
/* DLL main function */

#ifdef IMAIO_DLL
    BOOL WINAPI DllMain(
        HINSTANCE   hinstDLL,
        DWORD       fdwReason,
        LPVOID      lpvReserved)
    {
        switch (fdwReason)
        {
        case DLL_PROCESS_ATTACH:
#ifdef __DMC__
            hinstMSIMG32 = LoadLibraryA("msimg32.dll");
            AlphaBlend =
                (ALPHABLEND)GetProcAddress(hinstMSIMG32, "AlphaBlend");
            assert(AlphaBlend);
#endif  /* def __DMC__ */
            DisableThreadLibraryCalls(hinstDLL);
            break;
#ifdef __DMC__
        case DLL_PROCESS_DETACH:
            FreeLibrary(hinstMSIMG32);
            break;
#endif
        }
        return TRUE;
    }
#endif  /* def IMAIO_DLL */

/*****************************************************************************/
/* C/C++ switching */

#ifdef __cplusplus
} /* extern "C" */
#endif

/*****************************************************************************/
