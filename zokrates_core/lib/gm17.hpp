/**
 * @file gm17.hpp
 * @author Jacob Eberhardt <jacob.eberhardt@tu-berlin.de
 * @author Dennis Kuhnert <dennis.kuhnert@campus.tu-berlin.de>
 * @date 2017
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool _gm17_setup(const uint8_t* A,
            const uint8_t* B,
            const uint8_t* C,
            int A_len,
            int B_len,
            int C_len,
            int constraints,
            int variables,
            int inputs,
            const char* pk_path,
            const char* vk_path
          );

bool _gm17_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _gm17_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

bool _gm17_mnt4_setup(const uint8_t* A,
            const uint8_t* B,
            const uint8_t* C,
            int A_len,
            int B_len,
            int C_len,
            int constraints,
            int variables,
            int inputs,
            const char* pk_path,
            const char* vk_path
          );

bool _gm17_mnt4_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _gm17_mnt4_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

bool _gm17_mnt6_setup(const uint8_t* A,
            const uint8_t* B,
            const uint8_t* C,
            int A_len,
            int B_len,
            int C_len,
            int constraints,
            int variables,
            int inputs,
            const char* pk_path,
            const char* vk_path
          );

bool _gm17_mnt6_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _gm17_mnt6_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

bool _gm17_mnt4753_setup(const uint8_t* A,
            const uint8_t* B,
            const uint8_t* C,
            int A_len,
            int B_len,
            int C_len,
            int constraints,
            int variables,
            int inputs,
            const char* pk_path,
            const char* vk_path
          );

bool _gm17_mnt4753_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _gm17_mnt4753_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

bool _gm17_mnt6753_setup(const uint8_t* A,
            const uint8_t* B,
            const uint8_t* C,
            int A_len,
            int B_len,
            int C_len,
            int constraints,
            int variables,
            int inputs,
            const char* pk_path,
            const char* vk_path
          );

bool _gm17_mnt6753_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _gm17_mnt6753_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

#ifdef __cplusplus
} // extern "C"
#endif
