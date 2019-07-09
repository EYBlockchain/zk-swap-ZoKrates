use std::convert::From;
use std::fmt::{Debug, Display};
use std::hash::Hash;
use std::ops::{Add, Div, Mul, Sub};

use num_traits::{One, Zero};

use pairing::Engine;
use pairing::ff::ScalarEngine;

pub trait Pow<RHS> {
    type Output;
    fn pow(self, _: RHS) -> Self::Output;
}

pub trait Field:
    From<i32>
    + From<u32>
    + From<usize>
    + Zero
    + One
    + Clone
    + PartialEq
    + Eq
    + Hash
    + PartialOrd
    + Display
    + Debug
    + Add<Self, Output = Self>
    + for<'a> Add<&'a Self, Output = Self>
    + Sub<Self, Output = Self>
    + for<'a> Sub<&'a Self, Output = Self>
    + Mul<Self, Output = Self>
    + for<'a> Mul<&'a Self, Output = Self>
    + Div<Self, Output = Self>
    + for<'a> Div<&'a Self, Output = Self>
    + Pow<usize, Output = Self>
    + Pow<Self, Output = Self>
    + for<'a> Pow<&'a Self, Output = Self>
{
    /// An associated type to be able to operate with Bellman ff traits
    type BellmanEngine: Engine;

    fn from_bellman(e: <Self::BellmanEngine as ScalarEngine>::Fr) -> Self {
        use ff::{PrimeField, PrimeFieldRepr};
        let mut res: Vec<u8> = vec![];
        e.into_repr().write_le(&mut res).unwrap();
        Self::from_byte_vector(res)
    }

    fn into_bellman(self) -> <Self::BellmanEngine as ScalarEngine>::Fr {
        use ff::PrimeField;
        let s = self.to_dec_string();
        <Self::BellmanEngine as ScalarEngine>::Fr::from_str(&s).unwrap()
    }

    /// Returns this `Field`'s contents as little-endian byte vector
    fn into_byte_vector(&self) -> Vec<u8>;
    /// Returns an element of this `Field` from a little-endian byte vector
    fn from_byte_vector(_: Vec<u8>) -> Self;
    /// Returns this `Field`'s contents as decimal string
    fn to_dec_string(&self) -> String;
    /// Returns the multiplicative inverse, i.e.: self * self.inverse_mul() = Self::one()
    fn inverse_mul(&self) -> Self;
    /// Returns the smallest value that can be represented by this field type.
    fn min_value() -> Self;
    /// Returns the largest value that can be represented by this field type.
    fn max_value() -> Self;
    /// Returns the number of required bits to represent this field type.
    fn get_required_bits() -> usize;
    /// Tries to parse a string into this representation
    fn try_from_dec_str<'a>(s: &'a str) -> Result<Self, ()>;
    /// Returns a decimal string representing a the member of the equivalence class of this `Field` in Z/pZ
    /// which lies in [-(p-1)/2, (p-1)/2]
    fn to_compact_dec_string(&self) -> String;
}
