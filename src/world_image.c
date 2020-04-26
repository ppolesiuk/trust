#include "world_image.h"

#include <error.h>
#include <errno.h>
#include <png.h>

static void setRGB(png_bytep pixel, const world_t *world, int score) {
  int area = world->settings.play_area;
  area *= 2 + 1;
  score *= 256;
  score /= (2*area*area - 2)*world->settings.turn_n;
  if (score < -255) {
    pixel[0] = 255;
    pixel[1] = 0;
    pixel[2] = 0;
  } else if (score < 0) {
    pixel[0] = -score;
    pixel[1] = 0;
    pixel[2] = 0;
  } else if (score < 256) {
    pixel[0] = 0;
    pixel[1] = score;
    pixel[2] = 0;
  } else if (score < 512) {
    pixel[0] = 0;
    pixel[1] = 255;
    pixel[2] = score-256;
  } else if (score < 768) {
    pixel[0] = score-512;
    pixel[1] = 767 - score;
    pixel[2] = 255;
  } else {
    pixel[0] = 255;
    pixel[1] = 0;
    pixel[2] = 255;
  }
}

/* Based on Andrew Greensted's quick introduction to libPNG:
 * http://www.labbookpages.co.uk/software/imgProc/libPNG.html
 */
void write_world_image(
  const char    *fname,
  const world_t *world,
  char          *title)
{
  FILE       *fp       = NULL;
  png_structp png_ptr  = NULL;
  png_infop   info_ptr = NULL;
  png_bytep   row      = NULL;

  do {
    /* open file */
    fp = fopen(fname, "wb");
    if (fp == NULL) {
      error(0, errno, "cannot open file `%s'", fname);
      break;
    }

    /* initialize write structure */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);
    if (png_ptr == NULL) {
      error(0, 0, "png_create_write_struct failed");
      break;
    }

    /* initialize info structure */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
      error(0, 0, "png_create_info_struct failed");
      break;
    }

    /* setup exception handler */
    if (setjmp(png_jmpbuf(png_ptr))) {
      error(0, 0, "error during PNG creation");
      break;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr,
      world->settings.board_size_x,
      world->settings.board_size_y,
      8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    /* write title */
    png_text title_text;
    title_text.compression = PNG_TEXT_COMPRESSION_NONE;
    title_text.key  = "Title";
    title_text.text = title;
    png_set_text(png_ptr, info_ptr, &title_text, 1);

    png_write_info(png_ptr, info_ptr);

    /* allocate memory for one row */
    row = malloc(3 * world->settings.board_size_x * sizeof(png_byte));

    /* write data */
    for (int y = 0; y < world->settings.board_size_y; y++) {
      for (int x = 0; x < world->settings.board_size_x; x++) {
        int i = y * world->settings.board_size_x + x;
        setRGB(&row[x*3], world, world->pop[i].score);
      }
      png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, NULL);
  } while (0);

  if (row != NULL)      free(row);
  if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  if (png_ptr != NULL)  png_destroy_write_struct(&png_ptr, NULL);
  if (fp != NULL)       fclose(fp);
}
