/**
 * @file krypt.h 
 * @author Aethiopis II ben Zahab
 * 
 * @brief Contains various encryption/decryption algorithims commonly used, esp in SSL.
 * @version 0.1
 * @date 2024-01-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef KRYPT_H
#define KRYPT_H



//=====================================================================================|
//          INCLUDES
//=====================================================================================|
#include "basics.h"




//=====================================================================================|
//          PROTOTYPES
//=====================================================================================|
void Base64_Encode(const unsigned char *input, int len, unsigned char *output);
int Base64_Decode(const unsigned char *input, int len, unsigned char *output);



#endif