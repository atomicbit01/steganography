#ifndef DECODE_H
#define DECODE_H

#include "types.h"
#include <stdio.h>

/*
 * Structure to store information required for
 * decoding secret file from Stego Image
 */
typedef struct _DecodeInfo {
  /* Stego Image Info */
  char *stego_image_fname;
  FILE *fptr_stego_image;

  /* Secret File Info */
  char secret_fname[100];
  FILE *fptr_secret;
  char extn_secret_file[10];
  uint extn_size;
  uint size_secret_file;

  /* Temporary Buffer */
  char image_data[8];

} DecodeInfo;

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding process */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Skip BMP Header (54 bytes) */
Status skip_bmp_header(FILE *fptr_stego_image);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode 1 byte from 8 LSBs */
char decode_lsb_to_byte(char *image_buffer);

/* Decode size (integer) from 32 LSBs */
uint decode_lsb_to_size(FILE *fptr_stego_image);

/* Decode generic string/data stream */
Status decode_data_from_image(char *data, int size, DecodeInfo *decInfo);

#endif
