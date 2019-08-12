/**
 * @file wraplibsnark.cpp
 * @author Jacob Eberhardt <jacob.eberhardt@tu-berlin.de
 * @author Dennis Kuhnert <dennis.kuhnert@campus.tu-berlin.de>
 * @date 2017
 */

#include "pghr13.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <iomanip>

// contains definition of alt_bn128 ec public parameters
#include "libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp"
#include "libff/algebra/curves/mnt/mnt4/mnt4_pp.hpp"
#include "libff/algebra/curves/mnt/mnt6/mnt6_pp.hpp"
// contains required interfaces and types (keypair, proof, generator, prover, verifier)
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

#include "util.tcc"

typedef long integer_coeff_t;

using namespace std;
using namespace libsnark;

namespace pghr13 {

//takes input and puts it into constraint system
template<mp_size_t n, typename ppT>
r1cs_ppzksnark_constraint_system<ppT> createConstraintSystem(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs)
{
  r1cs_ppzksnark_constraint_system<ppT> cs;
  cs.primary_input_size = inputs;
  cs.auxiliary_input_size = variables - inputs - 1; // ~one not included

  cout << "num variables: " << variables <<endl;
  cout << "num constraints: " << constraints <<endl;
  cout << "num inputs: " << inputs <<endl;

  struct VariableValueMapping {
    int constraint_id;
    int variable_id;
    uint8_t variable_value[n*mp_limb_t_size];
  };
  const VariableValueMapping* A_vvmap = (VariableValueMapping*) A;
  const VariableValueMapping* B_vvmap = (VariableValueMapping*) B;
  const VariableValueMapping* C_vvmap = (VariableValueMapping*) C;

  int A_id = 0;
  int B_id = 0;
  int C_id = 0;

  for (int row = 0; row < constraints; row++) {
    linear_combination<libff::Fr<ppT> > lin_comb_A, lin_comb_B, lin_comb_C;

    while (A_id < A_len && A_vvmap[A_id].constraint_id == row) {
      libff::bigint<n> value = libsnarkBigintFromBytes<n>(A_vvmap[A_id].variable_value);
      if (!value.is_zero())
        lin_comb_A.add_term(A_vvmap[A_id].variable_id, value);
      A_id++;
    }
    while (B_id < B_len && B_vvmap[B_id].constraint_id == row) {
      libff::bigint<n> value = libsnarkBigintFromBytes<n>(B_vvmap[B_id].variable_value);
      if (!value.is_zero())
        lin_comb_B.add_term(B_vvmap[B_id].variable_id, value);
      B_id++;
    }
    while (C_id < C_len && C_vvmap[C_id].constraint_id == row) {
      libff::bigint<n> value = libsnarkBigintFromBytes<n>(C_vvmap[C_id].variable_value);
      if (!value.is_zero())
        lin_comb_C.add_term(C_vvmap[C_id].variable_id, value);
      C_id++;
    }

    cs.add_constraint(r1cs_constraint<libff::Fr<ppT> >(lin_comb_A, lin_comb_B, lin_comb_C));
  }

  return cs;
}

template<typename ppT>
void serializeProvingKeyToFile(r1cs_ppzksnark_proving_key<ppT> pk, const char* pk_path) {
  writeToFile(pk_path, pk);
}

template<typename ppT>
r1cs_ppzksnark_proving_key<ppT> deserializeProvingKeyFromFile(const char* pk_path) {
  return loadFromFile<r1cs_ppzksnark_proving_key<ppT>>(pk_path);
}

template<mp_size_t n, typename ppT, typename G1T, typename G2T>
void serializeVerificationKeyToFile(r1cs_ppzksnark_verification_key<ppT> vk, const char* vk_path) {
  string raw_vk_path = string(vk_path).append(".raw");
  writeToFile(raw_vk_path, vk);

  std::stringstream ss;

  unsigned icLength = vk.encoded_IC_query.rest.indices.size() + 1;

  ss << "\t\tvk.A = " << outputPointG2AffineAsHex<n, G2T>(vk.alphaA_g2) << endl;
  ss << "\t\tvk.B = " << outputPointG1AffineAsHex<n, G1T>(vk.alphaB_g1) << endl;
  ss << "\t\tvk.C = " << outputPointG2AffineAsHex<n, G2T>(vk.alphaC_g2) << endl;
  ss << "\t\tvk.gamma = " << outputPointG2AffineAsHex<n, G2T>(vk.gamma_g2) << endl;
  ss << "\t\tvk.gammaBeta1 = " << outputPointG1AffineAsHex<n, G1T>(vk.gamma_beta_g1) << endl;
  ss << "\t\tvk.gammaBeta2 = " << outputPointG2AffineAsHex<n, G2T>(vk.gamma_beta_g2) << endl;
  ss << "\t\tvk.Z = " << outputPointG2AffineAsHex<n, G2T>(vk.rC_Z_g2) << endl;
  ss << "\t\tvk.IC.len() = " << icLength << endl;
  ss << "\t\tvk.IC[0] = " << outputPointG1AffineAsHex<n, G1T>(vk.encoded_IC_query.first) << endl;
  for (size_t i = 1; i < icLength; ++i)
  {
                  auto vkICi = outputPointG1AffineAsHex<n, G1T>(vk.encoded_IC_query.rest.values[i - 1]);
                  ss << "\t\tvk.IC[" << i << "] = " << vkICi << endl;
  }

  std::ofstream fh;
  fh.open(vk_path, std::ios::binary);
  ss.rdbuf()->pubseekpos(0, std::ios_base::out);
  fh << ss.rdbuf();
  fh.flush();
  fh.close();
}

template<mp_size_t n, typename ppT, typename G1T, typename G2T>
void exportProof(r1cs_ppzksnark_proof<ppT> proof, const char* proof_path, const uint8_t* public_inputs,
            int public_inputs_length){
                string raw_proof_path = string(proof_path).append(".raw");
                writeToFile(raw_proof_path, proof);

                //create JSON file
                std::stringstream ss;
                ss << "{" << "\n";
                  ss << "\t\"proof\":" << "\n";
                    ss << "\t{" << "\n";
                      ss << "\t\t\"A\":" <<outputPointG1AffineAsHexJson<n, G1T>(proof.g_A.g) << ",\n";
                      ss << "\t\t\"A_p\":" <<outputPointG1AffineAsHexJson<n, G1T>(proof.g_A.h) << ",\n";
                      ss << "\t\t\"B\":" << "\n";
                        ss << "\t\t\t" << outputPointG2AffineAsHexJson<n, G2T>(proof.g_B.g) << ",\n";
                      ss << "\t\t\n";
                      ss << "\t\t\"B_p\":" <<outputPointG1AffineAsHexJson<n, G1T>(proof.g_B.h) << ",\n";
                      ss << "\t\t\"C\":" <<outputPointG1AffineAsHexJson<n, G1T>(proof.g_C.g) << ",\n";
                      ss << "\t\t\"C_p\":" <<outputPointG1AffineAsHexJson<n, G1T>(proof.g_C.h) << ",\n";
                      ss << "\t\t\"H\":" <<outputPointG1AffineAsHexJson<n, G1T>(proof.g_H) << ",\n";
                      ss << "\t\t\"K\":" <<outputPointG1AffineAsHexJson<n, G1T>(proof.g_K) << "\n";
                    ss << "\t}," << "\n";
                  //add input to json
                  ss << "\t\"input\":" << "[";
                  for (int i = 1; i < public_inputs_length; i++) {
                    if(i!=1){
                      ss << ",";
                    }
                    ss << outputInputAsHex<n>(libsnarkBigintFromBytes<n>(public_inputs + i*n*mp_limb_t_size));
                  }
                  ss << "]" << "\n";
                ss << "}" << "\n";

                std::string s = ss.str();
                //write json string to proof_path
                writeToFile(proof_path, s);
}

template<mp_size_t n, typename ppT, typename G1T, typename G2T>
bool setup(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs, const char* pk_path, const char* vk_path)
{
  auto cs = createConstraintSystem<n, ppT>(A, B, C, A_len, B_len, C_len, constraints, variables, inputs);
  assert(cs.num_variables() >= (unsigned)inputs);
  assert(cs.num_inputs() == (unsigned)inputs);
  assert(cs.num_constraints() == (unsigned)constraints);
  auto keypair = r1cs_ppzksnark_generator<ppT>(cs);
  serializeProvingKeyToFile<ppT>(keypair.pk, pk_path);
  serializeVerificationKeyToFile<n, ppT, G1T, G2T>(keypair.vk, vk_path);
  return true;
}

template<mp_size_t n, typename ppT, typename G1T, typename G2T>
bool generate_proof(const char* pk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length, const uint8_t* private_inputs, int private_inputs_length)
{
  auto pk = deserializeProvingKeyFromFile<ppT>(pk_path);
  r1cs_variable_assignment<libff::Fr<ppT> > full_variable_assignment;
  for (int i = 1; i < public_inputs_length; i++) {
    full_variable_assignment.push_back(libff::Fr<ppT>(libsnarkBigintFromBytes<n>(public_inputs + i*n*mp_limb_t_size)));
  }
  for (int i = 0; i < private_inputs_length; i++) {
    full_variable_assignment.push_back(libff::Fr<ppT>(libsnarkBigintFromBytes<n>(private_inputs + i*n*mp_limb_t_size)));
  }
  r1cs_primary_input<libff::Fr<ppT>> primary_input(full_variable_assignment.begin(), full_variable_assignment.begin() + public_inputs_length-1);
  r1cs_primary_input<libff::Fr<ppT>> auxiliary_input(full_variable_assignment.begin() + public_inputs_length-1, full_variable_assignment.end());
  auto proof = r1cs_ppzksnark_prover<ppT>(pk, primary_input, auxiliary_input);
  exportProof<n, ppT, G1T, G2T>(proof, proof_path, public_inputs, public_inputs_length);
  return true;
}

template<mp_size_t n, typename ppT, typename G1T, typename G2T>
bool verify_proof(const char* vk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length) {
  {
    string raw_vk_path = string(vk_path).append(".raw");
    auto vk = loadFromFile<r1cs_ppzksnark_verification_key<ppT>>(raw_vk_path);

    r1cs_variable_assignment<libff::Fr<ppT> > public_variable_assignment;
    for (int i = 1; i < public_inputs_length; i++) {
      public_variable_assignment.push_back(libff::Fr<ppT>(libsnarkBigintFromBytes<n>(public_inputs + i*n*mp_limb_t_size)));
    }
    r1cs_primary_input<libff::Fr<ppT>> prim(public_variable_assignment.begin(), public_variable_assignment.begin() + public_inputs_length-1);

    string raw_proof_path = string(proof_path).append(".raw");
    auto proof = loadFromFile<r1cs_ppzksnark_proof<ppT>>(raw_proof_path);

    return r1cs_ppzksnark_verifier_strong_IC<ppT>(vk, prim, proof);
  }
}

}

bool _pghr13_setup(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs, const char* pk_path, const char* vk_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::alt_bn128_pp::init_public_params();
  return pghr13::setup<libff::alt_bn128_r_limbs, libff::alt_bn128_pp, libff::alt_bn128_G1, libff::alt_bn128_G2>(A, B, C, A_len, B_len, C_len, constraints, variables, inputs, pk_path, vk_path);
}

bool _pghr13_generate_proof(const char* pk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length, const uint8_t* private_inputs, int private_inputs_length)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::alt_bn128_pp::init_public_params();
  return pghr13::generate_proof<libff::alt_bn128_r_limbs, libff::alt_bn128_pp, libff::alt_bn128_G1, libff::alt_bn128_G2>(pk_path, proof_path, public_inputs, public_inputs_length, private_inputs, private_inputs_length);
}

bool _pghr13_verify_proof(const char* vk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::alt_bn128_pp::init_public_params();
  return pghr13::verify_proof<libff::alt_bn128_r_limbs, libff::alt_bn128_pp, libff::alt_bn128_G1, libff::alt_bn128_G2>(vk_path, proof_path, public_inputs, public_inputs_length);
}

bool _pghr13_mnt4_setup(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs, const char* pk_path, const char* vk_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt4_pp::init_public_params();
  return pghr13::setup<libff::mnt4_r_limbs, libff::mnt4_pp, libff::mnt4_G1, libff::mnt4_G2>(A, B, C, A_len, B_len, C_len, constraints, variables, inputs, pk_path, vk_path);
}

bool _pghr13_mnt4_generate_proof(const char* pk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length, const uint8_t* private_inputs, int private_inputs_length)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt4_pp::init_public_params();
  return pghr13::generate_proof<libff::mnt4_r_limbs, libff::mnt4_pp, libff::mnt4_G1, libff::mnt4_G2>(pk_path, proof_path, public_inputs, public_inputs_length, private_inputs, private_inputs_length);
}

bool _pghr13_mnt4_verify_proof(const char* vk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt4_pp::init_public_params();
  return pghr13::verify_proof<libff::mnt4_r_limbs, libff::mnt4_pp, libff::mnt4_G1, libff::mnt4_G2>(vk_path, proof_path, public_inputs, public_inputs_length);
}

bool _pghr13_mnt6_setup(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs, const char* pk_path, const char* vk_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt6_pp::init_public_params();
  return pghr13::setup<libff::mnt6_r_limbs, libff::mnt6_pp, libff::mnt6_G1, libff::mnt6_G2>(A, B, C, A_len, B_len, C_len, constraints, variables, inputs, pk_path, vk_path);
}

bool _pghr13_mnt6_generate_proof(const char* pk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length, const uint8_t* private_inputs, int private_inputs_length)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt6_pp::init_public_params();
  return pghr13::generate_proof<libff::mnt6_r_limbs, libff::mnt6_pp, libff::mnt6_G1, libff::mnt6_G2>(pk_path, proof_path, public_inputs, public_inputs_length, private_inputs, private_inputs_length);
}

bool _pghr13_mnt6_verify_proof(const char* vk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt6_pp::init_public_params();
  return pghr13::verify_proof<libff::mnt6_r_limbs, libff::mnt6_pp, libff::mnt6_G1, libff::mnt6_G2>(vk_path, proof_path, public_inputs, public_inputs_length);
}
