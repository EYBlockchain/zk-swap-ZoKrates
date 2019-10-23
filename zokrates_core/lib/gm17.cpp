/**
 * @file wraplibsnark.cpp
 * @author Jacob Eberhardt <jacob.eberhardt@tu-berlin.de
 * @author Dennis Kuhnert <dennis.kuhnert@campus.tu-berlin.de>
 * @date 2017
 */

#include "gm17.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <iomanip>

// contains definition of alt_bn128 ec public parameters
#include "libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp"
// contains required interfaces and types (keypair, proof, generator, prover, verifier)
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_se_ppzksnark/r1cs_se_ppzksnark.hpp>

#include "util.tcc"

typedef long integer_coeff_t;

using namespace std;
using namespace libsnark;

namespace gm17 {

//takes input and puts it into constraint system
template<mp_size_t R, typename ppT>
r1cs_se_ppzksnark_constraint_system<ppT> createConstraintSystem(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs)
{
  r1cs_se_ppzksnark_constraint_system<ppT> cs;
  cs.primary_input_size = inputs;
  cs.auxiliary_input_size = variables - inputs - 1; // ~one not included

  cout << "num variables: " << variables <<endl;
  cout << "num constraints: " << constraints <<endl;
  cout << "num inputs: " << inputs <<endl;

  struct VariableValueMapping {
    int constraint_id;
    int variable_id;
    uint8_t variable_value[R*mp_limb_t_size];
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
      libff::bigint<R> value = libsnarkBigintFromBytes<R>(A_vvmap[A_id].variable_value);
      if (!value.is_zero())
        lin_comb_A.add_term(A_vvmap[A_id].variable_id, value);
      A_id++;
    }
    while (B_id < B_len && B_vvmap[B_id].constraint_id == row) {
      libff::bigint<R> value = libsnarkBigintFromBytes<R>(B_vvmap[B_id].variable_value);
      if (!value.is_zero())
        lin_comb_B.add_term(B_vvmap[B_id].variable_id, value);
      B_id++;
    }
    while (C_id < C_len && C_vvmap[C_id].constraint_id == row) {
      libff::bigint<R> value = libsnarkBigintFromBytes<R>(C_vvmap[C_id].variable_value);
      if (!value.is_zero())
        lin_comb_C.add_term(C_vvmap[C_id].variable_id, value);
      C_id++;
    }

    cs.add_constraint(r1cs_constraint<libff::Fr<ppT> >(lin_comb_A, lin_comb_B, lin_comb_C));
  }

  return cs;
}

template<typename ppT>
void serializeProvingKeyToFile(r1cs_se_ppzksnark_proving_key<ppT> pk, const char* pk_path){
  writeToFile(pk_path, pk);
}

template<typename ppT>
r1cs_se_ppzksnark_proving_key<ppT> deserializeProvingKeyFromFile(const char* pk_path){
  return loadFromFile<r1cs_se_ppzksnark_proving_key<ppT>>(pk_path);
}

template<mp_size_t Q, typename ppT, typename G1T, typename G2T>
void serializeVerificationKeyToFile(r1cs_se_ppzksnark_verification_key<ppT> vk, const char* vk_path){
  std::stringstream ss;

  unsigned queryLength = vk.query.size();

  ss << "\t\tvk.H = " << outputPointG2AffineAsHex<Q, G2T>(vk.H) << endl;
  ss << "\t\tvk.Galpha = " << outputPointG1AffineAsHex<Q, G1T>(vk.G_alpha) << endl;
  ss << "\t\tvk.Hbeta = " << outputPointG2AffineAsHex<Q, G2T>(vk.H_beta) << endl;
  ss << "\t\tvk.Ggamma = " << outputPointG1AffineAsHex<Q, G1T>(vk.G_gamma) << endl;
  ss << "\t\tvk.Hgamma = " << outputPointG2AffineAsHex<Q, G2T>(vk.H_gamma) << endl;
  ss << "\t\tvk.query.len() = " << queryLength << endl;
  for (mp_size_t i = 0; i < queryLength; ++i)
  {
      auto vkqueryi = outputPointG1AffineAsHex<Q, G1T>(vk.query[i]);
      ss << "\t\tvk.query[" << i << "] = " << vkqueryi << endl;
  }

  std::ofstream fh;
  fh.open(vk_path, std::ios::binary);
  ss.rdbuf()->pubseekpos(0, std::ios_base::out);
  fh << ss.rdbuf();
  fh.flush();
  fh.close();
}

template<mp_size_t Q, mp_size_t R, typename ppT, typename G1T, typename G2T>
void exportProof(r1cs_se_ppzksnark_proof<ppT> proof, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length){
    //create JSON file
    std::stringstream ss;
    ss << "{" << "\n";
      ss << "\t\"proof\":" << "\n";
        ss << "\t{" << "\n";
          ss << "\t\t\"A\":" <<outputPointG1AffineAsHexJson<Q, G1T>(proof.A) << ",\n";
          ss << "\t\t\"B\":" << "\n";
            ss << "\t\t\t" << outputPointG2AffineAsHexJson<Q, G2T>(proof.B) << ",\n";
          ss << "\t\t\n";
          ss << "\t\t\"C\":" <<outputPointG1AffineAsHexJson<Q, G1T>(proof.C) << ",\n";
        ss << "\t}," << "\n";
      //add input to json
      ss << "\t\"input\":" << "[";
      for (int i = 1; i < public_inputs_length; i++) {
        if(i!=1){
          ss << ",";
        }
        ss << outputInputAsHex<R>(libsnarkBigintFromBytes<R>(public_inputs + i*R*mp_limb_t_size));
      }
      ss << "]" << "\n";
    ss << "}" << "\n";

    std::string s = ss.str();
    //write json string to proof_path
    writeToFile(proof_path, s);
}

template<mp_size_t Q, mp_size_t R, typename ppT, typename G1T, typename G2T>
bool setup(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs, const char* pk_path, const char* vk_path)
{
  auto cs = gm17::createConstraintSystem<R, ppT>(A, B, C, A_len, B_len, C_len, constraints, variables, inputs);
  assert(cs.num_variables() >= (unsigned)inputs);
  assert(cs.num_inputs() == (unsigned)inputs);
  assert(cs.num_constraints() == (unsigned)constraints);
  auto keypair = r1cs_se_ppzksnark_generator<libff::alt_bn128_pp>(cs);
  gm17::serializeProvingKeyToFile<ppT>(keypair.pk, pk_path);
  gm17::serializeVerificationKeyToFile<Q, ppT, G1T, G2T>(keypair.vk, vk_path);
  return true;
}

template<mp_size_t Q, mp_size_t R, typename ppT, typename G1T, typename G2T>
bool generate_proof(const char* pk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length, const uint8_t* private_inputs, int private_inputs_length)
{
  auto pk = gm17::deserializeProvingKeyFromFile<ppT>(pk_path);
  r1cs_variable_assignment<libff::Fr<libff::alt_bn128_pp> > full_variable_assignment;
  for (int i = 1; i < public_inputs_length; i++) {
    full_variable_assignment.push_back(libff::Fr<libff::alt_bn128_pp>(libsnarkBigintFromBytes<R>(public_inputs + i*R*mp_limb_t_size)));
  }
  for (int i = 0; i < private_inputs_length; i++) {
    full_variable_assignment.push_back(libff::Fr<libff::alt_bn128_pp>(libsnarkBigintFromBytes<R>(private_inputs + i*R*mp_limb_t_size)));
  }
  r1cs_primary_input<libff::Fr<libff::alt_bn128_pp>> primary_input(full_variable_assignment.begin(), full_variable_assignment.begin() + public_inputs_length-1);
  r1cs_primary_input<libff::Fr<libff::alt_bn128_pp>> auxiliary_input(full_variable_assignment.begin() + public_inputs_length-1, full_variable_assignment.end());
  auto proof = r1cs_se_ppzksnark_prover<libff::alt_bn128_pp>(pk, primary_input, auxiliary_input);
  gm17::exportProof<Q, R, ppT, G1T, G2T>(proof, proof_path, public_inputs, public_inputs_length);
  return true;
}

}

bool _gm17_setup(const uint8_t* A, const uint8_t* B, const uint8_t* C, int A_len, int B_len, int C_len, int constraints, int variables, int inputs, const char* pk_path, const char* vk_path)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::alt_bn128_pp::init_public_params();
  return gm17::setup<libff::alt_bn128_q_limbs, libff::alt_bn128_r_limbs, libff::alt_bn128_pp, libff::alt_bn128_G1, libff::alt_bn128_G2>(A, B, C, A_len, B_len, C_len, constraints, variables, inputs, pk_path, vk_path);
}

bool _gm17_generate_proof(const char* pk_path, const char* proof_path, const uint8_t* public_inputs, int public_inputs_length, const uint8_t* private_inputs, int private_inputs_length)
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
  libff::alt_bn128_pp::init_public_params();
  return gm17::generate_proof<libff::alt_bn128_q_limbs, libff::alt_bn128_r_limbs, libff::alt_bn128_pp, libff::alt_bn128_G1, libff::alt_bn128_G2>(pk_path, proof_path, public_inputs, public_inputs_length, private_inputs, private_inputs_length);
}
