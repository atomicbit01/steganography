#include "encode.h"
#include "common.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image) {
  uint width, height;
  // Seek to 18th byte
  fseek(fptr_image, 18, SEEK_SET);

  // Read the width (an int)
  fread(&width, sizeof(int), 1, fptr_image);
#if defined DEBUG
  printf("width = %u\n", width);
#endif

  // Read the height (an int)
  fread(&height, sizeof(int), 1, fptr_image);
#if defined DEBUG
  printf("height = %u\n", height);
#endif

  // Return image capacity
  return width * height * 3;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo) {
  // Src Image file
  encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
  // Do Error handling
  if (encInfo->fptr_src_image == NULL) {
    perror("fopen");
    fprintf(stderr, "ERROR: Unable to open file %s\n",
            encInfo->src_image_fname);

    return e_failure;
  }

  // Secret file
  encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
  // Do Error handling
  if (encInfo->fptr_secret == NULL) {
    perror("fopen");
    fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    return e_failure;
  }

  // Stego Image file
  encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
  // Do Error handling
  if (encInfo->fptr_stego_image == NULL) {
    perror("fopen");
    fprintf(stderr, "ERROR: Unable to open file %s\n",
            encInfo->stego_image_fname);

    return e_failure;
  }

  // No failure return e_success
  return e_success;
}

OperationType check_operation_type(int argc, char *argv[]) {
  /* check if cli args are valid */
  if (argc < 2) {
    return e_unsupported;
  }
  if (strcmp(argv[1], "-e") == 0) {
    if (argc < 4) {
      return e_unsupported;
    }
    return e_encode;

  } else if (strcmp(argv[1], "-d") == 0) {
    if (argc < 3) {
      return e_unsupported;
    }
    return e_decode;

  } else
    return e_unsupported;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo) {

  /* check if cli args for encoding are valid */

  if (argv[2] != NULL && strstr(argv[2], ".bmp") != NULL) {
    encInfo->src_image_fname = argv[2];
  } else {
    return e_failure;
  }

  if (argv[3] != NULL && strstr(argv[3], ".txt") != NULL) {
    encInfo->secret_fname = argv[3];
  } else {
    return e_failure;
  }

  if (argv[4] != NULL) {
    if (strstr(argv[4], ".bmp") != NULL) {
      encInfo->stego_image_fname = argv[4];
    }
  } else {
    encInfo->stego_image_fname = "default.bmp";
  }

  return e_success;
}

Status check_capacity(EncodeInfo *encInfo) {

  /* check if image has the required size */

  encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

  encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

  if (encInfo->image_capacity >
      16 + 32 + 32 + 32 + encInfo->size_secret_file * 8) {
    return e_success;

  } else {
    return e_failure;
  }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image) {

  /* copy bmp header */
  char buffer[54];

  rewind(fptr_src_image);

  fread(buffer, 54, 1, fptr_src_image);

  fwrite(buffer, 54, 1, fptr_dest_image);

  return e_success;
}

Status encode_magic_string(char *magic_string, EncodeInfo *encInfo) {
  /* encode magic string */
  return encode_data_to_image(magic_string, strlen(magic_string), encInfo);
}

Status encode_byte_to_lsb(char data, char *image_buffer) {
  /* encode a byte to lsb */
  for (uint i = 0; i < 8; i++) {
    image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> i) & 1);
  }
  return e_success;
}

Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo) {
  /* encode data to image */
  for (int i = 0; i < size; i++) {
    fread(encInfo->image_data, 8, 1, encInfo->fptr_src_image);

    encode_byte_to_lsb(data[i], encInfo->image_data);

    fwrite(encInfo->image_data, 8, 1, encInfo->fptr_stego_image);
  }

  return e_success;
}

Status do_encoding(EncodeInfo *encInfo) {
  /* perform encoding */
  if (check_capacity(encInfo) != e_success) {
    fprintf(stderr, "ERROR: Data cannot be written into image\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Valid capacity\n");

  if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) !=
      e_success) {
    fprintf(stderr, "ERROR: Failed to copy header\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Copied bmp header\n");

  if (encode_magic_string(MAGIC_STRING, encInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to encode magic string\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Encoded magic string\n");

  char *extn = strrchr(encInfo->secret_fname, '.');
  if (extn == NULL)
    return e_failure;
  strcpy(encInfo->extn_secret_file, extn);

  if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),
                                   encInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to encode secret file extension size.\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Encoded secret file extension size\n");

  if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) !=
      e_success) {
    fprintf(stderr, "ERROR: Failed to encode secret file extension.\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Encoded secret file extension\n");

  if (encode_secret_file_size(encInfo->size_secret_file, encInfo) !=
      e_success) {
    fprintf(stderr, "ERROR: Failed to encode secret file size\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Encoded secret file size\n");

  if (encode_secret_file_data(encInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to encode secret data\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Encoded secret file data\n");

  if (copy_remaining_img_data(encInfo) != e_success) {
    fprintf(stderr, "ERROR: Failed to copy remaining data\n");
    return e_failure;
  }
  fprintf(stdout, "INFO: Copied remaining data\n");

  return e_success;
}

uint get_file_size(FILE *fptr) {

  fseek(fptr, 0, SEEK_END);
  uint size = ftell(fptr);
  return size;
}

Status encode_secret_file_extn_size(uint file_size, EncodeInfo *encInfo) {
  return encode_size_to_lsb(file_size, encInfo);
}

Status encode_size_to_lsb(uint size, EncodeInfo *encInfo) {
  /* encode file size to lsb */
  char str[32];
  fread(str, 32, 1, encInfo->fptr_src_image);

  for (uint i = 0; i < 32; i++) {
    str[i] = (str[i] & 0xFE) | ((size >> i) & 1);
  }
  fwrite(str, 32, 1, encInfo->fptr_stego_image);

  return e_success;
}

Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo) {
  encode_data_to_image(file_extn, strlen(file_extn), encInfo);
  return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo) {
  encode_size_to_lsb(file_size, encInfo);
  return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo) {
  /* encoding data to image */
  char data[encInfo->size_secret_file];
  rewind(encInfo->fptr_secret);
  fread(data, encInfo->size_secret_file, 1, encInfo->fptr_secret);
  return encode_data_to_image(data, encInfo->size_secret_file, encInfo);
}

Status copy_remaining_img_data(EncodeInfo *encInfo) {
  /*writing remaining bytes 2KB at a time */
  char buffer[2048];
  size_t bytes_read;

  while ((bytes_read =
              fread(buffer, 1, sizeof(buffer), encInfo->fptr_src_image)) > 0) {
    fwrite(buffer, 1, bytes_read, encInfo->fptr_stego_image);
  }

  return e_success;
}
