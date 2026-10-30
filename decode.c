#include "decode.h"
#include "common.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

/* Open files needed for decoding */
Status open_decode_files(DecodeInfo *decInfo) {
  // Open stego image file in binary read mode
  decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
  if (decInfo->fptr_stego_image == NULL) {
    perror("fopen");
    fprintf(stderr, "ERROR: Unable to open file %s\n",
            decInfo->stego_image_fname);
    return e_failure;
  }
  return e_success;
}

/* Validate command line arguments for decoding */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo) {
  // Validate input stego BMP image
  if (argv[2] != NULL && strstr(argv[2], ".bmp") != NULL) {
    decInfo->stego_image_fname = argv[2];
  } else {
    return e_failure;
  }

  // Set the exact output file name (defaults to "output.txt" if omitted)
  if (argv[3] != NULL) {
    strcpy(decInfo->secret_fname, argv[3]);
  } else {
    strcpy(decInfo->secret_fname, "output.txt");
  }

  return e_success;
}

/* Skip the 54-byte BMP header to reach the LSB data payload */
Status skip_bmp_header(FILE *fptr_stego_image) {
  fseek(fptr_stego_image, 54, SEEK_SET);
  return e_success;
}

/* Decode a single byte from 8 pixel bytes' LSBs */
char decode_lsb_to_byte(char *image_buffer) {
  char data = 0;
  for (int i = 0; i < 8; i++) {
    data |= ((image_buffer[i] & 1) << i);
  }
  return data;
}

/* Decode a 32-bit integer size from 32 pixel bytes' LSBs */
uint decode_lsb_to_size(FILE *fptr_stego_image) {
  char buffer[32];
  uint size = 0;

  fread(buffer, 32, 1, fptr_stego_image);
  for (int i = 0; i < 32; i++) {
    size |= ((buffer[i] & 1) << i);
  }

  return size;
}

Status decode_data_from_image(char *data, int size, DecodeInfo *decInfo) {
  for (int i = 0; i < size; i++) {
    fread(decInfo->image_data, 8, 1, decInfo->fptr_stego_image);
    data[i] = decode_lsb_to_byte(decInfo->image_data);
  }
  data[size] = '\0';
  return e_success;
}

/* Decode and verify Magic String */
Status decode_magic_string(DecodeInfo *decInfo) {
  int magic_len = strlen(MAGIC_STRING);
  char magic_str[magic_len + 1];

  decode_data_from_image(magic_str, magic_len, decInfo);

  if (strcmp(magic_str, MAGIC_STRING) == 0) {
    return e_success;
  }
  return e_failure;
}

/* Decode size of secret file extension */
Status decode_secret_file_extn_size(DecodeInfo *decInfo) {
  decInfo->extn_size = decode_lsb_to_size(decInfo->fptr_stego_image);
  return e_success;
}

/* Decode secret file extension string */
Status decode_secret_file_extn(DecodeInfo *decInfo) {
  decode_data_from_image(decInfo->extn_secret_file, decInfo->extn_size,
                         decInfo);
  return e_success;
}

/* Decode size of secret data */
Status decode_secret_file_size(DecodeInfo *decInfo) {
  decInfo->size_secret_file = decode_lsb_to_size(decInfo->fptr_stego_image);
  return e_success;
}

/* save secret data to output file */
Status decode_secret_file_data(DecodeInfo *decInfo) {
  char secret_data[decInfo->size_secret_file];

  decode_data_from_image(secret_data, decInfo->size_secret_file, decInfo);

  decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");
  if (decInfo->fptr_secret == NULL) {
    perror("fopen");
    return e_failure;
  }

  // Write decoded secret data
  fwrite(secret_data, decInfo->size_secret_file, 1, decInfo->fptr_secret);
  fclose(decInfo->fptr_secret);

  return e_success;
}

Status do_decoding(DecodeInfo *decInfo) {
  if (open_decode_files(decInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to open decode files\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Opened stego image file\n");

  if (skip_bmp_header(decInfo->fptr_stego_image) != e_success) {
    fprintf(stderr, "ERROR: Failed to skip BMP header\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Skipped BMP Header\n");

  if (decode_magic_string(decInfo) != e_success) {
    fprintf(stderr, "ERROR: Magic string mismatch / Image not encoded\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Decoded and matched magic string\n");

  if (decode_secret_file_extn_size(decInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to decode secret file extension size\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Decoded secret file extension size\n");

  if (decode_secret_file_extn(decInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to decode secret file extension\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Decoded secret file extension\n");

  if (decode_secret_file_size(decInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to decode secret file size\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Decoded secret file size\n");

  if (decode_secret_file_data(decInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to decode secret file data\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Decoded secret file data successfully\n");

  return e_success;
}
