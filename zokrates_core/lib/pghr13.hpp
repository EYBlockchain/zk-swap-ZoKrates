#pragma once

/**
 * @file pghr13.hpp
 * @author Jacob Eberhardt <jacob.eberhardt@tu-berlin.de
 * @author Dennis Kuhnert <dennis.kuhnert@campus.tu-berlin.de>
 * @date 2017
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool _pghr13_setup(const uint8_t* A,
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

bool _pghr13_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _pghr13_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

bool _pghr13_mnt4_setup(const uint8_t* A,
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

bool _pghr13_mnt4_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _pghr13_mnt4_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

bool _pghr13_mnt6_setup(const uint8_t* A,
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

bool _pghr13_mnt6_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _pghr13_mnt6_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

bool _pghr13_mnt4_mnt6_batch(
    const char *vk_1_path, const char *proof_1_path,
    const char *vk_2_path, const char *proof_2_path,
    const char *agg_vk_path, const char *agg_proof_path
    );

bool _pghr13_mnt6_mnt4_batch(
    const char *vk_1_path, const char *proof_1_path,
    const char *vk_2_path, const char *proof_2_path,
    const char *agg_vk_path, const char *agg_proof_path
    );

bool _pghr13_bls12_377_setup(const uint8_t* A,
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

bool _pghr13_bls12_377_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _pghr13_bls12_377_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

bool _pghr13_sw6_setup(const uint8_t* A,
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

bool _pghr13_sw6_generate_proof(const char* pk_path,
            const char* proof_path,
            const uint8_t* public_inputs,
            int public_inputs_length,
            const uint8_t* private_inputs,
            int private_inputs_length
          );

bool _pghr13_sw6_verify_proof(
        const char* vk_path,
        const char* proof_path
        );

#ifdef __cplusplus
} // extern "C"
#endif
