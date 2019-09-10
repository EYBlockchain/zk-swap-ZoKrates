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
#include "libff/algebra/curves/bls12_377/bls12_377_pp.hpp"

// contains required interfaces and types (keypair, proof, generator, prover, verifier)
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

#include "util.tcc"
// contains aggregation circuit
#include "aggregator.tcc"

typedef long integer_coeff_t;

using namespace std;
using namespace libsnark;

namespace pghr13 {

//takes input and puts it into constraint system
template<mp_size_t N, typename ppT>
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
    uint8_t variable_value[N*mp_limb_t_size];
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
      libff::bigint<N> value = libsnarkBigintFromBytes<N>(A_vvmap[A_id].variable_value);
      if (!value.is_zero())
        lin_comb_A.add_term(A_vvmap[A_id].variable_id, value);
      A_id++;
    }
    while (B_id < B_len && B_vvmap[B_id].constraint_id == row) {
      libff::bigint<N> value = libsnarkBigintFromBytes<N>(B_vvmap[B_id].variable_value);
      if (!value.is_zero())
        lin_comb_B.add_term(B_vvmap[B_id].variable_id, value);
      B_id++;
    }
    while (C_id < C_len && C_vvmap[C_id].constraint_id == row) {
      libff::bigint<N> value = libsnarkBigintFromBytes<N>(C_vvmap[C_id].variable_value);
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

template<mp_size_t N, typename ppT, typename G1T, typename G2T>
void serializeVerificationKeyToFile(r1cs_ppzksnark_verification_key<ppT> vk, const char* vk_path) {
  std::stringstream ss;

  unsigned icLength = vk.encoded_IC_query.rest.indices.size() + 1;

  ss << "\t\tvk.A = " << outputPointG2AffineAsHex<N, G2T>(vk.alphaA_g2) << endl;
  ss << "\t\tvk.B = " << outputPointG1AffineAsHex<N, G1T>(vk.alphaB_g1) << endl;
  ss << "\t\tvk.C = " << outputPointG2AffineAsHex<N, G2T>(vk.alphaC_g2) << endl;
  ss << "\t\tvk.gamma = " << outputPointG2AffineAsHex<N, G2T>(vk.gamma_g2) << endl;
  ss << "\t\tvk.gammaBeta1 = " << outputPointG1AffineAsHex<N, G1T>(vk.gamma_beta_g1) << endl;
  ss << "\t\tvk.gammaBeta2 = " << outputPointG2AffineAsHex<N, G2T>(vk.gamma_beta_g2) << endl;
  ss << "\t\tvk.Z = " << outputPointG2AffineAsHex<N, G2T>(vk.rC_Z_g2) << endl;
  ss << "\t\tvk.IC.len() = " << icLength << endl;
  ss << "\t\tvk.IC[0] = " << outputPointG1AffineAsHex<N, G1T>(vk.encoded_IC_query.first) << endl;
  for (size_t i = 1; i < icLength; ++i)
  {
                  auto vkICi = outputPointG1AffineAsHex<N, G1T>(vk.encoded_IC_query.rest.values[i - 1]);
                  ss << "\t\tvk.IC[" << i << "] = " << vkICi << endl;
  }

  std::ofstream fh;
  fh.open(vk_path, std::ios::binary);
  ss.rdbuf()->pubseekpos(0, std::ios_base::out);
  fh << ss.rdbuf();
  fh.flush();
  fh.close();
}

template<mp_size_t N, typename ppT, typename G1T, typename G2T>
void exportProof(r1cs_ppzksnark_proof<ppT> proof, const char* proof_path, const r1cs_primary_input<libff::Fr<ppT>> input) {
  //create JSON file
  std::stringstream ss;
  ss << "{" << "\n";
  ss << "\t\"proof\":" << "\n";
  ss << "\t{" << "\n";
  ss << "\t\t\"A\":" <<outputPointG1AffineAsHexJson<N, G1T>(proof.g_A.g) << ",\n";
  ss << "\t\t\"A_p\":" <<outputPointG1AffineAsHexJson<N, G1T>(proof.g_A.h) << ",\n";
  ss << "\t\t\"B\":" << "\n";
  ss << "\t\t\t" << outputPointG2AffineAsHexJson<N, G2T>(proof.g_B.g) << ",\n";
  ss << "\t\t\n";
  ss << "\t\t\"B_p\":" <<outputPointG1AffineAsHexJson<N, G1T>(proof.g_B.h) << ",\n";
  ss << "\t\t\"C\":" <<outputPointG1AffineAsHexJson<N, G1T>(proof.g_C.g) << ",\n";
  ss << "\t\t\"C_p\":" <<outputPointG1AffineAsHexJson<N, G1T>(proof.g_C.h) << ",\n";
  ss << "\t\t\"H\":" <<outputPointG1AffineAsHexJson<N, G1T>(proof.g_H) << ",\n";
  ss << "\t\t\"K\":" <<outputPointG1AffineAsHexJson<N, G1T>(proof.g_K) << "\n";
  ss << "\t}," << "\n";
  //add input to json
  ss << "\t\"input\":" << "[";
  for (unsigned int i = 0; i < input.size(); i++) {
    if(i!=0){
      ss << ",";
    }
    ss << outputInputAsHex<N>(input[i].as_bigint());
  }
  ss << "]" << "\n";
  ss << "}" << "\n";
  std::string s = ss.str();
  //write json string to proof_path
  writeToFile(proof_path, s);
}

template<mp_size_t N, typename ppT, typename G1T, typename G2T>
bool setup(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs, const char* pk_path, const char* vk_path)
{
  auto cs = createConstraintSystem<N, ppT>(A, B, C, A_len, B_len, C_len, constraints, variables, inputs);
  assert(cs.num_variables() >= (unsigned)inputs);
  assert(cs.num_inputs() == (unsigned)inputs);
  assert(cs.num_constraints() == (unsigned)constraints);
  auto keypair = r1cs_ppzksnark_generator<ppT>(cs);

  serializeProvingKeyToFile<ppT>(keypair.pk, pk_path);

  serializeVerificationKeyToFile<N, ppT, G1T, G2T>(keypair.vk, vk_path);
  // serialize vk in raw format (easy verify)
  string raw_vk_path = string(vk_path).append(".raw");
  writeToFile(raw_vk_path, keypair.vk);

  return true;
}

template<mp_size_t N, typename ppT, typename G1T, typename G2T>
bool generate_proof(const char* pk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length, const uint8_t* private_inputs, int private_inputs_length)
{
  auto pk = deserializeProvingKeyFromFile<ppT>(pk_path);
  r1cs_variable_assignment<libff::Fr<ppT> > full_variable_assignment;
  for (int i = 1; i < public_inputs_length; i++) {
    full_variable_assignment.push_back(libff::Fr<ppT>(libsnarkBigintFromBytes<N>(public_inputs + i*N*mp_limb_t_size)));
  }
  for (int i = 0; i < private_inputs_length; i++) {
    full_variable_assignment.push_back(libff::Fr<ppT>(libsnarkBigintFromBytes<N>(private_inputs + i*N*mp_limb_t_size)));
  }
  r1cs_primary_input<libff::Fr<ppT>> primary_input(full_variable_assignment.begin(), full_variable_assignment.begin() + public_inputs_length-1);
  r1cs_auxiliary_input<libff::Fr<ppT>> auxiliary_input(full_variable_assignment.begin() + public_inputs_length-1, full_variable_assignment.end());
  auto proof = r1cs_ppzksnark_prover<ppT>(pk, primary_input, auxiliary_input);

  exportProof<N, ppT, G1T, G2T>(proof, proof_path, primary_input);
  // serialize proof in raw format (easy verify)
  string raw_proof_path = string(proof_path).append(".raw");
  writeToFile(raw_proof_path, proof);
  // serialize primary input in raw format (easy verify)
  string raw_input_path = string(proof_path).append(".input.raw");
  writeVectorToFile(raw_input_path, primary_input);

  return true;
}

template<typename ppT>
bool verify_proof(const char* vk_path, const char* proof_path)
{
  string raw_vk_path = string(vk_path).append(".raw");
  auto vk = loadFromFile<r1cs_ppzksnark_verification_key<ppT>>(raw_vk_path);

  string raw_proof_path = string(proof_path).append(".raw");
  auto proof = loadFromFile<r1cs_ppzksnark_proof<ppT>>(raw_proof_path);

  string raw_input_path = string(proof_path).append(".input.raw");
  auto input = loadVectorFromFile<libff::Fr<ppT>>(raw_input_path);

  return r1cs_ppzksnark_verifier_strong_IC<ppT>(vk, input, proof);
}

template<typename ppT_F, mp_size_t N, typename ppT, typename G1T, typename G2T>
bool batch(
    const char *vk_1_path, const char *proof_1_path,
    const char *vk_2_path, const char *proof_2_path,
    const char *agg_vk_path, const char *agg_proof_path)
{
  // vks
  std::vector<r1cs_ppzksnark_verification_key<ppT_F>> vks;
  {
    string raw_vk_path = string(vk_1_path).append(".raw");
    auto vk = loadFromFile<r1cs_ppzksnark_verification_key<ppT_F>>(raw_vk_path);
    vks.emplace_back(vk);
  }
  {
    string raw_vk_path = string(vk_2_path).append(".raw");
    auto vk = loadFromFile<r1cs_ppzksnark_verification_key<ppT_F>>(raw_vk_path);
    vks.emplace_back(vk);
  }

  // inputs
  std::vector<r1cs_primary_input<libff::Fr<ppT_F>>> inputs;
  {
    string raw_input_path = string(proof_1_path).append(".input.raw");
    auto input = loadVectorFromFile<libff::Fr<ppT_F>>(raw_input_path);
    inputs.emplace_back(input);
  }
  {
    string raw_input_path = string(proof_2_path).append(".input.raw");
    auto input = loadVectorFromFile<libff::Fr<ppT_F>>(raw_input_path);
    inputs.emplace_back(input);
  }

  // proofs
  std::vector<r1cs_ppzksnark_proof<ppT_F>> proofs;
  {
    string raw_proof_path = string(proof_1_path).append(".raw");
    auto proof = loadFromFile<r1cs_ppzksnark_proof<ppT_F>>(raw_proof_path);
    proofs.emplace_back(proof);
  }
  {
    string raw_proof_path = string(proof_2_path).append(".raw");
    auto proof = loadFromFile<r1cs_ppzksnark_proof<ppT_F>>(raw_proof_path);
    proofs.emplace_back(proof);
  }

  aggregator<ppT_F, ppT> agg(2, inputs[0].size());
  agg.generate_r1cs_constraints();
  r1cs_ppzksnark_keypair<ppT> keypair = r1cs_ppzksnark_generator<ppT>(agg.pb.get_constraint_system());
  agg.generate_r1cs_witness(vks, inputs, proofs);
  auto primary_input = agg.pb.primary_input();
  auto auxiliary_input = agg.pb.auxiliary_input();
  r1cs_ppzksnark_proof<ppT> proof = r1cs_ppzksnark_prover<ppT>(keypair.pk, primary_input, auxiliary_input);

  serializeVerificationKeyToFile<N, ppT, G1T, G2T>(keypair.vk, agg_vk_path);
  // serialize vk in raw format (easy verify)
  string raw_agg_vk_path = string(agg_vk_path).append(".raw");
  writeToFile(raw_agg_vk_path, keypair.vk);

  exportProof<N, ppT, G1T, G2T>(proof, agg_proof_path, primary_input);
  // serialize proof in raw format (easy verify)
  string raw_agg_proof_path = string(agg_proof_path).append(".raw");
  writeToFile(raw_agg_proof_path, proof);
  // serialize primary input in raw format (easy verify)
  string raw_agg_input_path = string(agg_proof_path).append(".input.raw");
  writeVectorToFile(raw_agg_input_path, primary_input);

  return true;
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

bool _pghr13_verify_proof(const char* vk_path, const char* proof_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::alt_bn128_pp::init_public_params();
  return pghr13::verify_proof<libff::alt_bn128_pp>(vk_path, proof_path);
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

bool _pghr13_mnt4_verify_proof(const char* vk_path, const char* proof_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt4_pp::init_public_params();
  return pghr13::verify_proof<libff::mnt4_pp>(vk_path, proof_path);
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

bool _pghr13_mnt6_verify_proof(const char* vk_path, const char* proof_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt6_pp::init_public_params();
  return pghr13::verify_proof<libff::mnt6_pp>(vk_path, proof_path);
}

bool _pghr13_mnt4_mnt6_batch(
    const char *vk_1_path, const char *proof_1_path,
    const char *vk_2_path, const char *proof_2_path,
    const char *agg_vk_path, const char *agg_proof_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt4_pp::init_public_params();
  libff::mnt6_pp::init_public_params();
  return pghr13::batch<libff::mnt4_pp, libff::mnt6_r_limbs, libff::mnt6_pp, libff::mnt6_G1, libff::mnt6_G2>(vk_1_path, proof_1_path, vk_2_path, proof_2_path, agg_vk_path, agg_proof_path);
}

bool _pghr13_mnt6_mnt4_batch(
    const char *vk_1_path, const char *proof_1_path,
    const char *vk_2_path, const char *proof_2_path,
    const char *agg_vk_path, const char *agg_proof_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::mnt6_pp::init_public_params();
  libff::mnt4_pp::init_public_params();
  return pghr13::batch<libff::mnt6_pp, libff::mnt4_r_limbs, libff::mnt4_pp, libff::mnt4_G1, libff::mnt4_G2>(vk_1_path, proof_1_path, vk_2_path, proof_2_path, agg_vk_path, agg_proof_path);
}

bool _pghr13_bls12_377_setup(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs, const char* pk_path, const char* vk_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::bls12_377_pp::init_public_params();
  return pghr13::setup<libff::bls12_377_r_limbs, libff::bls12_377_pp, libff::bls12_377_G1, libff::bls12_377_G2>(A, B, C, A_len, B_len, C_len, constraints, variables, inputs, pk_path, vk_path);
}

bool _pghr13_bls12_377_generate_proof(const char* pk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length, const uint8_t* private_inputs, int private_inputs_length)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::bls12_377_pp::init_public_params();
  return pghr13::generate_proof<libff::bls12_377_r_limbs, libff::bls12_377_pp, libff::bls12_377_G1, libff::bls12_377_G2>(pk_path, proof_path, public_inputs, public_inputs_length, private_inputs, private_inputs_length);
}

bool _pghr13_bls12_377_verify_proof(const char* vk_path, const char* proof_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::bls12_377_pp::init_public_params();
  return pghr13::verify_proof<libff::bls12_377_pp>(vk_path, proof_path);
}
