/*
 * Name: Emmanuel J
 * Reg. No: 26011f_039
 * Date: 30-7-2026
 * Description: Steganography project
 */
#include "decode.h"
#include "encode.h"
#include "types.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

  // Check command-line operation type (-e or -d)
  int res = check_operation_type(argc, argv);

  if (res == e_encode) {
    /* encoding StegoImage */
    EncodeInfo encInfo;
    fprintf(stdout, "INFO: Encoding is selected\n");

    if (read_and_validate_encode_args(argv, &encInfo) == e_success) {
      fprintf(stdout, "INFO: Read and validate encode args is successful\n");

      // Open source image, secret file, and stego image
      if (open_files(&encInfo) == e_failure) {
        fprintf(stderr, "ERROR: open_files function failed\n");
        return 1;
      } else {
        fprintf(stdout, "SUCCESS: open_files function completed\n");

        // Perform complete encoding workflow
        if (do_encoding(&encInfo) == e_success) {
          fprintf(stdout, "INFO: ## Encoding successful ##\n");
        } else {
          fprintf(stderr, "ERROR: Encoding failed\n");
          return 1;
        }
      }
    } else {
      fprintf(stderr, "ERROR: Read and validate encode args failed\n");
      return 1;
    }

  } else if (res == e_decode) {
    /* Decoding StegoImage */
    DecodeInfo decInfo;
    fprintf(stdout, "INFO: Decoding is selected\n");

    if (read_and_validate_decode_args(argv, &decInfo) == e_success) {
      fprintf(stdout, "INFO: Read and validate decode args is successful\n");

      // Perform complete decoding workflow
      if (do_decoding(&decInfo) == e_success) {
        fprintf(stdout, "INFO: ## Decoding successful ##\n");
      } else {
        fprintf(stderr, "ERROR: Decoding failed\n");
        return 1;
      }
    } else {
      fprintf(stderr, "ERROR: Read and validate decode args failed\n");
      return 1;
    }

  } else {
    // Usage guidelines
    fprintf(stderr, "ERROR: Invalid operation\n");
    fprintf(stderr, "Usage for Encoding: ");
    fprintf(stderr, "%s -e <Image.bmp> <secret.txt> [Output.bmp]\n", argv[0]);
    fprintf(stderr, "Usage for Decoding: ");
    fprintf(stderr,
            "Usage for Decoding: %s -d <StegoImage.bmp> [OutputFilename.txt]\n",
            argv[0]);
    return 1;
  }

  return 0;
}
