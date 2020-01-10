#ifndef IMAGE_H
#define IMAGE_H

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include "box.h"

typedef struct {
	int w;
	int h;
    int c;
    float *data;
} image;

float get_color(int c, int x, int max);
void flip_image(image a);
void draw_box(image a, int x1, int y1, int x2, int y2, float r, float g, float b);
void scale_image(image m, float s);
image crop_image(image im, int dx, int dy, int w, int h);
image random_crop_image(image im, int w, int h);
image random_augment_image(image im, float angle, float aspect, int low, int high, int size);
void random_distort_image(image im, float hue, float saturation, float exposure);
YOLODLL_API image resize_image(image im, int w, int h);
void fill_image(image m, float s);
YOLODLL_API image letterbox_image(image im, int w, int h);
void normalize_image(image p);
image rotate_image(image m, float rad);
void embed_image(image source, image dest, int dx, int dy);
void distort_image(image im, float hue, float sat, float val);
void hsv_to_rgb(image im);
YOLODLL_API void rgbgr_image(image im);
void constrain_image(image im);
int best_3d_shift_r(image a, image b, int min, int max);

image grayscale_image(image im);
image collapse_image_layers(image source, int border);
image collapse_images_vert(image *ims, int n);

void show_image(image p, const char *name);
void save_image_png(image im, const char *name);
void save_image(image p, const char *name);
void show_images(image *ims, int n, char *window);
YOLODLL_API image make_image(int w, int h, int c);
image make_random_image(int w, int h, int c);
image make_empty_image(int w, int h, int c);
image float_to_image(int w, int h, int c, float *data);
image copy_image(image p);
image load_image(char *filename, int w, int h, int c);
YOLODLL_API image load_image_color(char *filename, int w, int h);

YOLODLL_API float get_pixel(image m, int x, int y, int c);
float get_pixel_extend(image m, int x, int y, int c);
YOLODLL_API void set_pixel(image m, int x, int y, int c, float val);
void add_pixel(image m, int x, int y, int c, float val);
float bilinear_interpolate(image im, float x, float y, int c);

image get_image_layer(image m, int l);

YOLODLL_API void free_image(image m);

// compare to sort detection** by bbox.x
int compare_by_lefts(const void *a_ptr, const void *b_ptr);
// compare to sort detection** by best_class probability
int compare_by_probs(const void *a_ptr, const void *b_ptr);
#endif

