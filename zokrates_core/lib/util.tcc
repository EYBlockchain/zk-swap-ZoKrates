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

template<mp_size_t N>
libff::bigint<N> libsnarkBigintFromBytes(const uint8_t* _x)
{
  libff::bigint<N> x;

  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < 8; j++) {
      x.data[N - 1 - i] |= uint64_t(_x[i * 8 + j]) << (8 * (7-j));
    }
  }

  return x;
}

template<mp_size_t N>
std::string HexStringFromLibsnarkBigint(libff::bigint<N> _x) {
  uint8_t x[mp_limb_t_size * N];
  for (unsigned i = 0; i < N; i++)
    for (unsigned j = 0; j < 8; j++)
      x[i * 8 + j] = uint8_t(uint64_t(_x.data[N - 1 - i]) >> (8 * (7 - j)));

  std::stringstream ss;
  ss << std::setfill('0');
  for (unsigned i = 0; i<mp_limb_t_size * N; i++) {
    ss << std::hex << std::setw(2) << (int)x[i];
  }
  return ss.str();
}

template<mp_size_t N>
std::string outputInputAsHex(libff::bigint<N> _x){
  return "\"0x" + HexStringFromLibsnarkBigint<N>(_x) + "\"";
}

template<mp_size_t Q, typename G1T>
std::string outputPointG1AffineAsHex(G1T _p)
{
  G1T aff = _p;
  aff.to_affine_coordinates();
  if constexpr (std::is_same<G1T, libff::mnt4_G1>::value || std::is_same<G1T, libff::mnt6_G1>::value) {
    return
      "0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X().as_bigint()) +
      ", 0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y().as_bigint());
  } else {
    return
      "0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X.as_bigint()) +
      ", 0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y.as_bigint());
  }
}

template<mp_size_t Q, typename G1T>
std::string outputPointG1AffineAsHexJson(G1T _p)
{
  G1T aff = _p;
  aff.to_affine_coordinates();
  if constexpr (std::is_same<G1T, libff::mnt4_G1>::value || std::is_same<G1T, libff::mnt6_G1>::value) {
    return
      "[\"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X().as_bigint()) +
      "\", \"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y().as_bigint())+"\"]";
  } else {
    return
      "[\"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X.as_bigint()) +
      "\", \"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y.as_bigint())+"\"]";
  }
}

template<mp_size_t Q, typename G2T>
std::string outputPointG2AffineAsHex(G2T _p)
{
  G2T aff = _p;
  aff.to_affine_coordinates();
  if constexpr (std::is_same<G2T, libff::mnt4_G2>::value || std::is_same<G2T, libff::mnt6_G2>::value) {
    return
      "[0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X().c1.as_bigint()) + ", 0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X().c0.as_bigint()) + "], [0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y().c1.as_bigint()) + ", 0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y().c0.as_bigint()) + "]";
  } else {
    return
      "[0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X.c1.as_bigint()) + ", 0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X.c0.as_bigint()) + "], [0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y.c1.as_bigint()) + ", 0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y.c0.as_bigint()) + "]";
  }
}

template<mp_size_t Q, typename G2T>
std::string outputPointG2AffineAsHexJson(G2T _p)
{
  G2T aff = _p;
  aff.to_affine_coordinates();
  if constexpr (std::is_same<G2T, libff::mnt4_G2>::value || std::is_same<G2T, libff::mnt6_G2>::value) {
    return
      "[[\"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X().c1.as_bigint()) + "\", \"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X().c0.as_bigint()) + "\"], [\"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y().c1.as_bigint()) + "\", \"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y().c0.as_bigint()) + "\"]]";
  } else {
    return
      "[[\"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X.c1.as_bigint()) + "\", \"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.X.c0.as_bigint()) + "\"], [\"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y.c1.as_bigint()) + "\", \"0x" +
      HexStringFromLibsnarkBigint<Q>(aff.Y.c0.as_bigint()) + "\"]]";
  }
}

template<typename T>
void writeVectorToFile(std::string path, std::vector<T> &v) {
  // vector<T> to ss
  std::stringstream ss;
  ss << v.size() << "\n";
  for (const T& t : v)
  {
    ss << t << OUTPUT_NEWLINE;
  }
  // ss to file
  std::ofstream fh;
  fh.open(path, std::ios::binary);
  ss.rdbuf()->pubseekpos(0, std::ios_base::out);
  fh << ss.rdbuf();
  fh.flush();
  fh.close();
}

template<typename T>
std::vector<T> loadVectorFromFile(std::string path)
{
  // file to ss
  std::stringstream ss;
  std::ifstream fh(path, std::ios::binary);
  assert(fh.is_open());
  ss << fh.rdbuf();
  fh.close();
  // ss to vector<T>
  ss.rdbuf()->pubseekpos(0, std::ios_base::in);
  std::vector<T> v;
  size_t size;
  ss >> size;
  char c;
  ss.read(&c, 1);
  v.resize(0);
  for (size_t i = 0; i < size; ++i)
  {
    T elt;
    ss >> elt;
    char c;
    ss.read(&c, 1);
    v.push_back(elt);
  }
  return v;
}

template<typename T>
void writeToFile(std::string path, T& obj) {
  // T to ss
  std::stringstream ss;
  ss << obj;
  // ss to file
  std::ofstream fh;
  fh.open(path, std::ios::binary);
  ss.rdbuf()->pubseekpos(0, std::ios_base::out);
  fh << ss.rdbuf();
  fh.flush();
  fh.close();
}

template<typename T>
T loadFromFile(std::string path) {
  // file to ss
  std::stringstream ss;
  std::ifstream fh(path, std::ios::binary);
  assert(fh.is_open());
  ss << fh.rdbuf();
  fh.close();
  // ss to T
  ss.rdbuf()->pubseekpos(0, std::ios_base::in);
  T obj;
  ss >> obj;
  return obj;
}
