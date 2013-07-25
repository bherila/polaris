/* 
 * File:   image.h
 * Author: psastras
 *
 * Created on April 28, 2010, 12:20 PM
 */

#ifndef _IMAGE_H
#define	_IMAGE_H

#include "vector.h"
#include <string.h>

#ifdef	__cplusplus
extern "C" {
#endif

    void write_image_bmp(const char *fname, const Color3 *image_data, const int w, const int h) {
        FILE *f;
        unsigned char *img;
        int filesize = 54 + 3 * w*h, i, j, x, y, r, g, b;
        img = (unsigned char *) malloc(3 * w * h);
        memset(img, 0, sizeof (img));
        for (i = 0; i < w; i++) {
            for (j = 0; j < h; j++) {
                x = i;
                y = j; ///(h - 1) - j;
                r = image_data[j * w + i].x * 255;
                g = image_data[j * w + i].y * 255;
                b = image_data[j * w + i].z * 255;
                img[(x + y * w)*3 + 2] = (unsigned char) (r > 255 ? 255 : r);
                img[(x + y * w)*3 + 1] = (unsigned char) (g > 255 ? 255 : g);
                img[(x + y * w)*3 + 0] = (unsigned char) (b > 255 ? 255 : b);
            }
        }

        unsigned char bmpfileheader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0},
        bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0},
        bmppad[3] = {0, 0, 0};

        bmpfileheader[2] = (unsigned char) (filesize);
        bmpfileheader[3] = (unsigned char) (filesize >> 8);
        bmpfileheader[4] = (unsigned char) (filesize >> 16);
        bmpfileheader[5] = (unsigned char) (filesize >> 24);
        bmpinfoheader[4] = (unsigned char) (w);
        bmpinfoheader[5] = (unsigned char) (w >> 8);
        bmpinfoheader[6] = (unsigned char) (w >> 16);
        bmpinfoheader[7] = (unsigned char) (w >> 24);
        bmpinfoheader[8] = (unsigned char) (h);
        bmpinfoheader[9] = (unsigned char) (h >> 8);
        bmpinfoheader[10] = (unsigned char) (h >> 16);
        bmpinfoheader[11] = (unsigned char) (h >> 24);

        f = fopen(fname, "wb");
        fwrite(bmpfileheader, 1, 14, f);
        fwrite(bmpinfoheader, 1, 40, f);
        for (i = 0; i < h; i++) {
            fwrite(img + (w * (h - i - 1)*3), 3, w, f);
            fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
        }
        fclose(f);
        free(img);

        //printf("\033[33mImage written to\033[0m \033[04m%s.\n\033[0m", fname);
    }


#ifdef	__cplusplus
}
#endif

#endif	/* _IMAGE_H */

