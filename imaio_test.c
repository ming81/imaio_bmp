/*****************************************************************************/
/* imaio_test.c --- test program for imaio                                   */
/* Copyright (C) 2015 katahiromz. All Rights Reserved.                       */
/*****************************************************************************/

#include "imaio.h"
#include <tchar.h>
#include <assert.h>

int main(void)
{
    int i, i_trans;
    II_ANIGIF *anigif;
    II_PALETTE *table;
    II_APNG *apng;

    II_HIMAGE ahbm[10] = {NULL};

    /* anigif to gif */
    printf("anigif to gif\n");
    fflush(stdout);
    anigif = ii_anigif_load(_T("anime.gif"), II_FLAG_USE_SCREEN);
    printf("anigif: %p\n", (void *)anigif);
    fflush(stdout);
    printf("num_frames: %d\n", anigif->num_frames);
    printf("flags: %d\n", anigif->flags);
    printf("width: %d\n", anigif->width);
    printf("height: %d\n", anigif->height);
    printf("global_palette: %s\n", (anigif->global_palette? "present" : "none"));
    printf("iBackground: %d\n", anigif->iBackground);
    printf("loop_count: %d\n", anigif->loop_count);
    fflush(stdout);
    for (i = 0; i < anigif->num_frames; ++i)
    {
        char fname[64];
        II_ANIGIF_FRAME *frame = &anigif->frames[i];
        printf("\n[ anigif frame #%d ]\n", i);
        printf("x, y: %d, %d\n", frame->x, frame->y);
        printf("width, height: %d, %d\n", frame->width, frame->height);
        printf("iTransparent: %d\n", frame->iTransparent);
        printf("disposal: %d\n", frame->disposal);
        printf("delay: %d\n", frame->delay);
        printf("local_palette: %s\n", (frame->local_palette ? "present" : "none"));
        fflush(stdout);
        sprintf(fname, "frame-%02d.gif", i);
        ii_gif_save_a(fname, frame->hbmPart, &frame->iTransparent);
        sprintf(fname, "frame-%02d.png", i);
        ii_png_save_a(fname, frame->hbmScreen, 0);
    }
    printf("\nsave anigif\n");
    fflush(stdout);
    anigif->loop_count = 3;
    anigif->flags = II_FLAG_USE_SCREEN;
    ii_anigif_save(_T("new_anime.gif"), anigif);
    printf("apng from anigif\n");
    fflush(stdout);
    apng = ii_apng_from_anigif(anigif);
    ii_apng_save(_T("new_anime.png"), apng);
    ii_apng_destroy(apng);
    ii_anigif_destroy(anigif);
    printf("\n");

    /* apng to png */
    printf("apng to png\n");
    fflush(stdout);
    apng = ii_apng_load(_T("clock-opt.png"), II_FLAG_USE_SCREEN);
    printf("width: %d\n", apng->width);
    printf("height: %d\n", apng->height);
    printf("num_frames: %d\n", apng->num_frames);
    for (i = 0; i < apng->num_frames; ++i)
    {
        char fname[64];
        II_APNG_FRAME *frame = &apng->frames[i];
        printf("\n[ apng frame #%d ]\n", i);
        printf("x_offset: %d\n", frame->x_offset);
        printf("y_offset: %d\n", frame->y_offset);
        printf("width: %d\n", frame->width);
        printf("height: %d\n", frame->height);
        if (frame->hbmScreen)
        {
            sprintf(fname, "screen-%03d.png", i);
            ii_png_save_a(fname, frame->hbmScreen, 0);
        }
        sprintf(fname, "part-%03d.png", i);
        ii_png_save_a(fname, frame->hbmPart, 0);
    }
    printf("\nsave apng\n");
    fflush(stdout);
    ii_apng_save(_T("new-clock-opt.png"), apng);
    printf("anigif from apng\n");
    fflush(stdout);
    anigif = ii_anigif_from_apng(apng, true);
    ii_anigif_save(_T("new-clock-opt.gif"), anigif);
    ii_anigif_destroy(anigif);
    ii_apng_destroy(apng);

    /* jpeg to bmp */
    printf("jpeg to bmp\n");
    fflush(stdout);
    ahbm[0] = ii_jpg_load(_T("grad.jpg"), NULL);
    ii_bmp_save(_T("grad.bmp"), ahbm[0], 0);

    /* gif to bmp */
    printf("gif to bmp\n");
    fflush(stdout);
    ahbm[1] = ii_gif_load_8bpp(_T("circle.gif"), &i_trans);
    ii_bmp_save(_T("circle.bmp"), ahbm[1], 0);

    /* getting palette */
    printf("getting palette\n");
    fflush(stdout);
    table = ii_get_palette(ahbm[1]);
    if (table)
    {
        ii_palette_shrink(table, &i_trans);
        printf("i_trans: %d\n", i_trans);
        printf("num_colors: %d\n", table->num_colors);
        for (i = 0; i < table->num_colors; ++i)
        {
            printf("%d: #%02X%02X%02X\n", i,
                table->colors[i].value[2],
                table->colors[i].value[1],
                table->colors[i].value[0]);
        }
        ii_palette_destroy(table);
    }

    /* gif to gif */
    printf("gif to gif\n");
    ii_gif_save(_T("new_circle.gif"), ahbm[1], &i_trans);

    /* png to bmp */
    printf("png to bmp\n");
    fflush(stdout);
    ahbm[2] = ii_png_load(_T("star.png"), NULL);
    ahbm[9] = ii_trans_8bpp(ahbm[2], &i_trans);
    ii_gif_save(_T("star.gif"), ahbm[9], &i_trans);
    ahbm[9] = NULL;

    /* stamp */
    printf("stamp\n");
    {
        II_HIMAGE hbm = ii_create_32bpp_trans(400, 400);
        ii_stamp(hbm, 0, 0, ahbm[2], NULL, 255);
        ii_png_save(_T("another_star.png"), hbm, 0);
        ii_destroy(hbm);
    }

    /* tiff to bmp */
    printf("tiff to bmp\n");
    fflush(stdout);
    ahbm[3] = ii_tif_load(_T("xwordgiver-title.tif"), NULL);
    ii_bmp_save(_T("xwordgiver-title.bmp"), ahbm[3], 0);

    ahbm[4] = ii_bmp_load(_T("money.bmp"), NULL);

    /* bmp to jpeg */
    printf("bmp to jpeg\n");
    fflush(stdout);
    ii_jpg_save(_T("money.jpg"), ahbm[4], 100, FALSE, 0);

    /* bmp to gif */
    printf("bmp to gif\n");
    fflush(stdout);
    ii_gif_save(_T("money.gif"), ahbm[4], NULL);

    /* bmp to png */
    printf("bmp to png\n");
    fflush(stdout);
    ii_png_save(_T("money.png"), ahbm[4], 0);

    /* bmp to tiff */
    printf("bmp to tiff\n");
    fflush(stdout);
    ii_tif_save(_T("money.tif"), ahbm[4], 0);

    /* loading from resource */
    printf("res gif to file bmp\n");
    fflush(stdout);
    ahbm[9] = ii_gif_load_8bpp_res(GetModuleHandle(NULL), MAKEINTRESOURCE(1), NULL);
    assert(ahbm[9]);
    ii_bmp_save(_T("circle_res.bmp"), ahbm[9], 0);
    ii_destroy(ahbm[9]);
    ahbm[9] = NULL;

    printf("res png to file bmp\n");
    fflush(stdout);
    ahbm[5] = ii_png_load_res(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
    ii_bmp_save(_T("star_res.bmp"), ahbm[5], 0);

    printf("res bmp to file bmp\n");
    fflush(stdout);
    ahbm[6] = ii_bmp_load_res(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
    ii_bmp_save(_T("money_res.bmp"), ahbm[6], 0);

    ahbm[7] = ii_create_32bpp(400, 400);
    #if 1
        ii_destroy(ahbm[7]);
        ahbm[7] = ii_8bpp(ahbm[0], 2);
    #elif 1
        ii_put_center(ahbm[7], 50, 50, ahbm[3], NULL);
        ii_put(ahbm[7], 100, 20, ahbm[2], NULL);
    #elif 1
        ii_destroy(ahbm[7]);
        ahbm[7] = ii_flipped_vertical(ahbm[2]);
    #elif 1
        ahbm[9] = ii_rotated_32bpp(ahbm[3], 3.14159 / 2, TRUE);
        ii_put_center(ahbm[7], 50, 50, ahbm[9], NULL);
        ahbm[8] = ii_32bpp_from_trans_8bpp(ahbm[1], &i_trans);
        ii_put(ahbm[7], 100, 20, ahbm[8], NULL);
    #else
        ii_put_center(ahbm[7], 50, 50, ahbm[3], NULL);
        ii_put(ahbm[7], 100, 20, ahbm[1], &i_trans);
    #endif
    ii_bmp_save(_T("created.bmp"), ahbm[7], 0);

    for (i = 0; i < 10; ++i)
    {
        ii_destroy(ahbm[i]);
        ahbm[i] = NULL;
    }

    return 0;
}
