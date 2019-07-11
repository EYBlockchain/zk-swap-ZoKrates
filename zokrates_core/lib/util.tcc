#pragma once

/**
 * @file wraplibsnark.cpp
 * @author Jacob Eberhardt <jacob.eberhardt@tu-berlin.de
 * @author Dennis Kuhnert <dennis.kuhnert@campus.tu-berlin.de>
 * @date 2017
 */

#include <fstream>
#include <iostream>
#include <cassert>
#include <iomanip>
#include <libff/algebra/fields/field_utils.hpp>

using namespace std;

// TODO check it (should crash when not verified)
const mp_size_t mp_limb_t_size = 8;

template<mp_size_t n>
libff::bigint<n> libsnarkBigintFromBytes(const uint8_t* _x)
{
  libff::bigint<n> x;

  for (unsigned i = 0; i < n; i++) {
    for (unsigned j = 0; j < 8; j++) {
      x.data[n - 1 - i] |= uint64_t(_x[i * 8 + j]) << (8 * (7-j));
    }
  }

  return x;
}

template<mp_size_t n>
std::string HexStringFromLibsnarkBigint(libff::bigint<n> _x) {
  uint8_t x[mp_limb_t_size * n];
  for (unsigned i = 0; i < n; i++)
    for (unsigned j = 0; j < 8; j++)
      x[i * 8 + j] = uint8_t(uint64_t(_x.data[n - 1 - i]) >> (8 * (7 - j)));

  std::stringstream ss;
  ss << std::setfill('0');
  for (unsigned i = 0; i<mp_limb_t_size * n; i++) {
    ss << std::hex << std::setw(2) << (int)x[i];
  }
  return ss.str();
}

template<mp_size_t n>
std::string outputInputAsHex(libff::bigint<n> _x){
  return "\"0x" + HexStringFromLibsnarkBigint<n>(_x) + "\"";
}

template<mp_size_t n, typename G1T>
std::string outputPointG1AffineAsHex(G1T _p)
{
  G1T aff = _p;
  aff.to_affine_coordinates();
  if constexpr (!std::is_same<G1T, libff::alt_bn128_G1>::value) {
    return
      "0x" +
      HexStringFromLibsnarkBigint<n>(aff.X().as_bigint()) +
      ", 0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y().as_bigint());
  } else {
    return
      "0x" +
      HexStringFromLibsnarkBigint<n>(aff.X.as_bigint()) +
      ", 0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y.as_bigint());
  }
}

template<mp_size_t n, typename G1T>
std::string outputPointG1AffineAsHexJson(G1T _p)
{
  G1T aff = _p;
  aff.to_affine_coordinates();
  if constexpr (!std::is_same<G1T, libff::alt_bn128_G1>::value) {
    return
      "[\"0x" +
      HexStringFromLibsnarkBigint<n>(aff.X().as_bigint()) +
      "\", \"0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y().as_bigint())+"\"]";
  } else {
    return
      "[\"0x" +
      HexStringFromLibsnarkBigint<n>(aff.X.as_bigint()) +
      "\", \"0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y.as_bigint())+"\"]";
  }
}

template<mp_size_t n, typename G2T>
std::string outputPointG2AffineAsHex(G2T _p)
{
  G2T aff = _p;
  aff.to_affine_coordinates();
  if constexpr (!std::is_same<G2T, libff::alt_bn128_G2>::value) {
    return
      "[0x" +
      HexStringFromLibsnarkBigint<n>(aff.X().c1.as_bigint()) + ", 0x" +
      HexStringFromLibsnarkBigint<n>(aff.X().c0.as_bigint()) + "], [0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y().c1.as_bigint()) + ", 0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y().c0.as_bigint()) + "]";
  } else {
    return
      "[0x" +
      HexStringFromLibsnarkBigint<n>(aff.X.c1.as_bigint()) + ", 0x" +
      HexStringFromLibsnarkBigint<n>(aff.X.c0.as_bigint()) + "], [0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y.c1.as_bigint()) + ", 0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y.c0.as_bigint()) + "]";
  }
}

template<mp_size_t n, typename G2T>
std::string outputPointG2AffineAsHexJson(G2T _p)
{
  G2T aff = _p;
  aff.to_affine_coordinates();
  if constexpr (!std::is_same<G2T, libff::alt_bn128_G2>::value) {
    return
      "[[\"0x" +
      HexStringFromLibsnarkBigint<n>(aff.X().c1.as_bigint()) + "\", \"0x" +
      HexStringFromLibsnarkBigint<n>(aff.X().c0.as_bigint()) + "\"], [\"0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y().c1.as_bigint()) + "\", \"0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y().c0.as_bigint()) + "\"]]";
  } else {
    return
      "[[\"0x" +
      HexStringFromLibsnarkBigint<n>(aff.X.c1.as_bigint()) + "\", \"0x" +
      HexStringFromLibsnarkBigint<n>(aff.X.c0.as_bigint()) + "\"], [\"0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y.c1.as_bigint()) + "\", \"0x" +
      HexStringFromLibsnarkBigint<n>(aff.Y.c0.as_bigint()) + "\"]]";
  }
}

template<typename T>
void writeToFile(std::string path, T& obj) {
  std::stringstream ss;
  ss << obj;
  std::ofstream fh;
  fh.open(path, std::ios::binary);
  ss.rdbuf()->pubseekpos(0, std::ios_base::out);
  fh << ss.rdbuf();
  fh.flush();
  fh.close();
}

template<typename T>
T loadFromFile(std::string path) {
  std::stringstream ss;
  std::ifstream fh(path, std::ios::binary);

  assert(fh.is_open());

  ss << fh.rdbuf();
  fh.close();

  ss.rdbuf()->pubseekpos(0, std::ios_base::in);

  T obj;
  ss >> obj;

  return obj;
}
