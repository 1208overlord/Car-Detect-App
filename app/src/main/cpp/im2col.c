#include "im2col.h"
#include <stdio.h>
#include <string.h>


float im2col_get_pixel(float *im, int height, int width, int channels,
                        int row, int col, int channel, int pad)
{
    row -= pad;
    col -= pad;

    if (row < 0 || col < 0 ||
        row >= height || col >= width) return 0;
    return im[col + width*(row + height*channel)];
}

//From Berkeley Vision's Caffe!
//https://github.com/BVLC/caffe/blob/master/LICENSE
void im2col_cpu(float* data_im,
     int channels,  int height,  int width,
     int ksize,  int stride, int pad, float* data_col) 
{
    int c;
    const int height_col = (height + 2 * pad - ksize) / stride + 1;
    const int width_col = (width + 2 * pad - ksize) / stride + 1;
    const int channels_col = channels * ksize * ksize;
    /*int c,h,w;
    int height_col = (height + 2*pad - ksize) / stride + 1;
    int width_col = (width + 2*pad - ksize) / stride + 1;

    int channels_col = channels * ksize * ksize;
    for (c = 0; c < channels_col; ++c) {
        int w_offset = c % ksize;
        int h_offset = (c / ksize) % ksize;
        int c_im = c / ksize / ksize;
        for (h = 0; h < height_col; ++h) {
            for (w = 0; w < width_col; ++w) {
                int im_row = h_offset + h * stride;
                int im_col = w_offset + w * stride;
                int col_index = (c * height_col + h) * width_col + w;
                data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                        im_row, im_col, c_im, pad);
            }
        }
    }*/
    #pragma omp parallel for
    for (c = 0; c < channels_col; ++c) {
        int h, w;
        int w_offset = c % ksize;
        int h_offset = (c / ksize) % ksize;
        int c_im = c / ksize / ksize;
        for (h = pad; h < height_col-pad; ++h) {
            for (w = pad; w < width_col-pad-8; w += 8) {
                int im_row = h_offset + h - pad;
                int im_col = w_offset + w - pad;
                int col_index = (c * height_col + h) * width_col + w;

                memcpy(&data_col[col_index], (float *)(&data_im[im_col + width*(im_row + height*c_im)]), 32);
            }

            for (; w < width_col - pad; ++w) {
                int im_row = h_offset + h - pad;
                int im_col = w_offset + w - pad;
                int col_index = (c * height_col + h) * width_col + w;

                data_col[col_index] = data_im[im_col + width*(im_row + height*c_im)];
            }
        }

        {
            w = 0;
            for (h = 0; h < height_col; ++h) {
                int im_row = h_offset + h;
                int im_col = w_offset + w;
                int col_index = (c * height_col + h) * width_col + w;
                data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                                                       im_row, im_col, c_im, pad);
            }
        }

        {
            w = width_col-1;
            for (h = 0; h < height_col; ++h) {
                int im_row = h_offset + h;
                int im_col = w_offset + w;
                int col_index = (c * height_col + h) * width_col + w;
                data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                                                       im_row, im_col, c_im, pad);
            }
        }

        {
            h = 0;
            for (w = 0; w < width_col; ++w) {
                int im_row = h_offset + h;
                int im_col = w_offset + w;
                int col_index = (c * height_col + h) * width_col + w;
                data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                                                       im_row, im_col, c_im, pad);
            }
        }

        {
            h = height_col-1;
            for (w = 0; w < width_col; ++w) {
                int im_row = h_offset + h;
                int im_col = w_offset + w;
                int col_index = (c * height_col + h) * width_col + w;
                data_col[col_index] = im2col_get_pixel(data_im, height, width, channels,
                                                       im_row, im_col, c_im, pad);
            }
        }
    }
}

