use lazy_static::lazy_static;

use std::convert::From;
use std::fmt;
use std::fmt::{Debug, Display};
use std::ops::{Add, Div, Mul, Sub};

use num_bigint::{BigInt, BigUint, Sign, ToBigInt};
use num_integer::Integer;
use num_traits::{One, Zero};

use pairing::bn256::Bn256;

use serde_derive::{Deserialize, Serialize};

use super::field::{Pow, Field};
use super::utils;

lazy_static! {
    static ref P: BigInt = BigInt::parse_bytes(
        b"21888242871839275222246405745257275088548364400416034343698204186575808495617",
        10
    )
    .unwrap();
}

#[derive(PartialEq, PartialOrd, Clone, Eq, Ord, Hash, Serialize, Deserialize)]
pub struct BN128 {
    value: BigInt,
}

impl Field for BN128 {
    type BellmanEngine = Bn256;

    fn into_byte_vector(&self) -> Vec<u8> {
        match self.value.to_biguint() {
            Option::Some(val) => val.to_bytes_le(),
            Option::None => panic!("Should never happen."),
        }
    }

    fn from_byte_vector(bytes: Vec<u8>) -> Self {
        let uval = BigUint::from_bytes_le(bytes.as_slice());
        BN128 {
            value: BigInt::from_biguint(Sign::Plus, uval),
        }
    }

    fn to_dec_string(&self) -> String {
        self.value.to_str_radix(10)
    }

    fn inverse_mul(&self) -> Self {
        let (b, s, _) = utils::extended_euclid(&self.value, &*P);
        assert_eq!(b, BigInt::one());
        BN128 {
            value: &s - s.div_floor(&*P) * &*P,
        }
    }
    fn min_value() -> Self {
        BN128 {
            value: ToBigInt::to_bigint(&0).unwrap(),
        }
    }
    fn max_value() -> Self {
        BN128 {
            value: &*P - ToBigInt::to_bigint(&1).unwrap(),
        }
    }
    fn get_required_bits() -> usize {
        (*P).bits()
    }
    fn try_from_dec_str<'a>(s: &'a str) -> Result<Self, ()> {
        let x = BigInt::parse_bytes(s.as_bytes(), 10).ok_or(())?;
        Ok(BN128 {
            value: &x - x.div_floor(&*P) * &*P,
        })
    }
    fn to_compact_dec_string(&self) -> String {
        // values up to (p-1)/2 included are represented as positive, values between (p+1)/2 and p-1 as represented as negative by subtracting p
        if self.value <= BN128::max_value().value / 2 {
            format!("{}", self.value.to_str_radix(10))
        } else {
            format!(
                "({})",
                (&self.value - (BN128::max_value().value + BigInt::one())).to_str_radix(10)
            )
        }
    }
}

impl Default for BN128 {
    fn default() -> Self {
        BN128 {
            value: BigInt::default(),
        }
    }
}

impl Display for BN128 {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.value.to_str_radix(10))
    }
}

impl Debug for BN128 {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.value.to_str_radix(10))
    }
}

impl From<i32> for BN128 {
    fn from(num: i32) -> Self {
        let x = ToBigInt::to_bigint(&num).unwrap();
        BN128 {
            value: &x - x.div_floor(&*P) * &*P,
        }
    }
}

impl From<u32> for BN128 {
    fn from(num: u32) -> Self {
        let x = ToBigInt::to_bigint(&num).unwrap();
        BN128 {
            value: &x - x.div_floor(&*P) * &*P,
        }
    }
}

impl From<usize> for BN128 {
    fn from(num: usize) -> Self {
        let x = ToBigInt::to_bigint(&num).unwrap();
        BN128 {
            value: &x - x.div_floor(&*P) * &*P,
        }
    }
}

impl Zero for BN128 {
    fn zero() -> BN128 {
        BN128 {
            value: ToBigInt::to_bigint(&0).unwrap(),
        }
    }
    fn is_zero(&self) -> bool {
        self.value == ToBigInt::to_bigint(&0).unwrap()
    }
}

impl One for BN128 {
    fn one() -> BN128 {
        BN128 {
            value: ToBigInt::to_bigint(&1).unwrap(),
        }
    }
}

impl Add<BN128> for BN128 {
    type Output = BN128;

    fn add(self, other: Self) -> Self {
        BN128 {
            value: (self.value + other.value) % &*P,
        }
    }
}

impl<'a> Add<&'a BN128> for BN128 {
    type Output = BN128;

    fn add(self, other: &Self) -> Self {
        BN128 {
            value: (self.value + other.value.clone()) % &*P,
        }
    }
}

impl Sub<BN128> for BN128 {
    type Output = BN128;

    fn sub(self, other: Self) -> Self {
        let x = self.value - other.value;
        BN128 {
            value: &x - x.div_floor(&*P) * &*P,
        }
    }
}

impl<'a> Sub<&'a BN128> for BN128 {
    type Output = BN128;

    fn sub(self, other: &Self) -> Self {
        let x = self.value - other.value.clone();
        BN128 {
            value: &x - x.div_floor(&*P) * &*P,
        }
    }
}

impl Mul<BN128> for BN128 {
    type Output = BN128;

    fn mul(self, other: Self) -> Self {
        BN128 {
            value: (self.value * other.value) % &*P,
        }
    }
}

impl<'a> Mul<&'a BN128> for BN128 {
    type Output = BN128;

    fn mul(self, other: &Self) -> Self {
        BN128 {
            value: (self.value * other.value.clone()) % &*P,
        }
    }
}

impl Div<BN128> for BN128 {
    type Output = BN128;

    fn div(self, other: Self) -> Self {
        self * other.inverse_mul()
    }
}

impl<'a> Div<&'a BN128> for BN128 {
    type Output = BN128;

    fn div(self, other: &Self) -> Self {
        self / other.clone()
    }
}

impl Pow<usize> for BN128 {
    type Output = BN128;

    fn pow(self, exp: usize) -> Self {
        let mut res = BN128::from(1);
        for _ in 0..exp {
            res = res * &self;
        }
        res
    }
}

impl Pow<BN128> for BN128 {
    type Output = BN128;

    fn pow(self, exp: Self) -> Self {
        let mut res = BN128::one();
        let mut current = BN128::zero();
        loop {
            if current >= exp {
                return res;
            }
            res = res * &self;
            current = current + BN128::one();
        }
    }
}

impl<'a> Pow<&'a BN128> for BN128 {
    type Output = BN128;

    fn pow(self, exp: &'a Self) -> Self {
        let mut res = BN128::one();
        let mut current = BN128::zero();
        loop {
            if &current >= exp {
                return res;
            }
            res = res * &self;
            current = current + BN128::one();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    impl<'a> From<&'a str> for BN128 {
        fn from(s: &'a str) -> Self {
            BN128::try_from_dec_str(s).unwrap()
        }
    }

    #[cfg(test)]
    mod field_prime {
        use super::*;
        use bincode::{deserialize, serialize, Infinite};

        #[test]
        fn positive_number() {
            assert_eq!(
                "1234245612".parse::<BigInt>().unwrap(),
                BN128::from("1234245612").value
            );
        }

        #[test]
        fn negative_number() {
            assert_eq!(
                P.checked_sub(&"12".parse::<BigInt>().unwrap()).unwrap(),
                BN128::from("-12").value
            );
        }

        #[test]
        fn addition() {
            assert_eq!(
                "65484493".parse::<BigInt>().unwrap(),
                (BN128::from("65416358") + BN128::from("68135")).value
            );
            assert_eq!(
                "65484493".parse::<BigInt>().unwrap(),
                (BN128::from("65416358") + &BN128::from("68135")).value
            );
        }

        #[test]
        fn addition_negative_small() {
            assert_eq!(
                "3".parse::<BigInt>().unwrap(),
                (BN128::from("5") + BN128::from("-2")).value
            );
            assert_eq!(
                "3".parse::<BigInt>().unwrap(),
                (BN128::from("5") + &BN128::from("-2")).value
            );
        }

        #[test]
        fn addition_negative() {
            assert_eq!(
                "65348223".parse::<BigInt>().unwrap(),
                (BN128::from("65416358") + BN128::from("-68135")).value
            );
            assert_eq!(
                "65348223".parse::<BigInt>().unwrap(),
                (BN128::from("65416358") + &BN128::from("-68135")).value
            );
        }

        #[test]
        fn subtraction() {
            assert_eq!(
                "65348223".parse::<BigInt>().unwrap(),
                (BN128::from("65416358") - BN128::from("68135")).value
            );
            assert_eq!(
                "65348223".parse::<BigInt>().unwrap(),
                (BN128::from("65416358") - &BN128::from("68135")).value
            );
        }

        #[test]
        fn subtraction_negative() {
            assert_eq!(
                "65484493".parse::<BigInt>().unwrap(),
                (BN128::from("65416358") - BN128::from("-68135")).value
            );
            assert_eq!(
                "65484493".parse::<BigInt>().unwrap(),
                (BN128::from("65416358") - &BN128::from("-68135")).value
            );
        }

        #[test]
        fn subtraction_overflow() {
            assert_eq!(
                "21888242871839275222246405745257275088548364400416034343698204186575743147394"
                    .parse::<BigInt>()
                    .unwrap(),
                (BN128::from("68135") - BN128::from("65416358")).value
            );
            assert_eq!(
                "21888242871839275222246405745257275088548364400416034343698204186575743147394"
                    .parse::<BigInt>()
                    .unwrap(),
                (BN128::from("68135") - &BN128::from("65416358")).value
            );
        }

        #[test]
        fn multiplication() {
            assert_eq!(
                "13472".parse::<BigInt>().unwrap(),
                (BN128::from("32") * BN128::from("421")).value
            );
            assert_eq!(
                "13472".parse::<BigInt>().unwrap(),
                (BN128::from("32") * &BN128::from("421")).value
            );
        }

        #[test]
        fn multiplication_negative() {
            assert_eq!(
                "21888242871839275222246405745257275088548364400416034343698204186575808014369"
                    .parse::<BigInt>()
                    .unwrap(),
                (BN128::from("54") * BN128::from("-8912")).value
            );
            assert_eq!(
                "21888242871839275222246405745257275088548364400416034343698204186575808014369"
                    .parse::<BigInt>()
                    .unwrap(),
                (BN128::from("54") * &BN128::from("-8912")).value
            );
        }

        #[test]
        fn multiplication_two_negative() {
            assert_eq!(
                "648".parse::<BigInt>().unwrap(),
                (BN128::from("-54") * BN128::from("-12")).value
            );
            assert_eq!(
                "648".parse::<BigInt>().unwrap(),
                (BN128::from("-54") * &BN128::from("-12")).value
            );
        }

        #[test]
        fn multiplication_overflow() {
            assert_eq!(
                "6042471409729479866150380306128222617399890671095126975526159292198160466142"
                    .parse::<BigInt>()
                    .unwrap(),
                (BN128::from(
                    "21888242871839225222246405785257275088694311157297823662689037894645225727"
                ) * BN128::from("218882428715392752222464057432572755886923"))
                .value
            );
            assert_eq!(
                "6042471409729479866150380306128222617399890671095126975526159292198160466142"
                    .parse::<BigInt>()
                    .unwrap(),
                (BN128::from(
                    "21888242871839225222246405785257275088694311157297823662689037894645225727"
                ) * &BN128::from("218882428715392752222464057432572755886923"))
                    .value
            );
        }

        #[test]
        fn division() {
            assert_eq!(
                BN128::from(4),
                BN128::from(48) / BN128::from(12)
            );
            assert_eq!(
                BN128::from(4),
                BN128::from(48) / &BN128::from(12)
            );
        }

        #[test]
        fn division_negative() {
            let res = BN128::from(-54) / BN128::from(12);
            assert_eq!(BN128::from(-54), BN128::from(12) * res);
        }

        #[test]
        fn division_two_negative() {
            let res = BN128::from(-12) / BN128::from(-85);
            assert_eq!(BN128::from(-12), BN128::from(-85) * res);
        }

        #[test]
        fn pow_small() {
            assert_eq!(
                "8".parse::<BigInt>().unwrap(),
                (BN128::from("2").pow(BN128::from("3"))).value
            );
            assert_eq!(
                "8".parse::<BigInt>().unwrap(),
                (BN128::from("2").pow(&BN128::from("3"))).value
            );
        }

        #[test]
        fn pow_usize() {
            assert_eq!(
                "614787626176508399616".parse::<BigInt>().unwrap(),
                (BN128::from("54").pow(12)).value
            );
        }

        #[test]
        fn pow() {
            assert_eq!(
                "614787626176508399616".parse::<BigInt>().unwrap(),
                (BN128::from("54").pow(BN128::from("12"))).value
            );
            assert_eq!(
                "614787626176508399616".parse::<BigInt>().unwrap(),
                (BN128::from("54").pow(&BN128::from("12"))).value
            );
        }

        #[test]
        fn pow_negative() {
            assert_eq!(
                "21888242871839275222246405745257275088548364400416034343686819230535502784513"
                    .parse::<BigInt>()
                    .unwrap(),
                (BN128::from("-54").pow(BN128::from("11"))).value
            );
            assert_eq!(
                "21888242871839275222246405745257275088548364400416034343686819230535502784513"
                    .parse::<BigInt>()
                    .unwrap(),
                (BN128::from("-54").pow(&BN128::from("11"))).value
            );
        }

        #[test]
        fn serde_ser_deser() {
            let serialized = &serialize(&BN128::from("11"), Infinite).unwrap();
            let deserialized = deserialize(serialized).unwrap();
            assert_eq!(BN128::from("11"), deserialized);
        }

        #[test]
        fn serde_json_ser_deser() {
            let serialized = serde_json::to_string(&BN128::from("11")).unwrap();
            let deserialized = serde_json::from_str(&serialized).unwrap();
            assert_eq!(BN128::from("11"), deserialized);
        }

        #[test]
        fn bytes_ser_deser() {
            let fp = BN128::from("101");
            let bv = fp.into_byte_vector();
            assert_eq!(fp, BN128::from_byte_vector(bv));
        }

        #[test]
        fn dec_string_ser_deser() {
            let fp = BN128::from("101");
            let bv = fp.to_dec_string();
            assert_eq!(fp, BN128::try_from_dec_str(&bv).unwrap());
        }

        #[test]
        fn compact_representation() {
            let one = BN128::from(1);
            assert_eq!("1", &one.to_compact_dec_string());
            let minus_one = BN128::from(0) - one;
            assert_eq!("(-1)", &minus_one.to_compact_dec_string());
            // (p-1)/2 -> positive notation
            let p_minus_one_over_two =
                (BN128::from(0) - BN128::from(1)) / BN128::from(2);
            assert_eq!(
                "10944121435919637611123202872628637544274182200208017171849102093287904247808",
                &p_minus_one_over_two.to_compact_dec_string()
            );
            // (p-1)/2 + 1 -> negative notation (p-1)/2 + 1 - p == (-p+1)/2
            let p_minus_one_over_two_plus_one = ((BN128::from(0) - BN128::from(1))
                / BN128::from(2))
                + BN128::from(1);
            assert_eq!(
                "(-10944121435919637611123202872628637544274182200208017171849102093287904247808)",
                &p_minus_one_over_two_plus_one.to_compact_dec_string()
            );
        }
    }

    #[test]
    fn bigint_assertions() {
        let x = BigInt::parse_bytes(b"65", 10).unwrap();
        assert_eq!(&x + &x, BigInt::parse_bytes(b"130", 10).unwrap());
        assert_eq!(
            "1".parse::<BigInt>().unwrap(),
            "3".parse::<BigInt>()
                .unwrap()
                .div_floor(&"2".parse::<BigInt>().unwrap())
        );
        assert_eq!(
            "-2".parse::<BigInt>().unwrap(),
            "-3".parse::<BigInt>()
                .unwrap()
                .div_floor(&"2".parse::<BigInt>().unwrap())
        );
    }

    #[test]
    fn test_extended_euclid() {
        assert_eq!(
            (
                ToBigInt::to_bigint(&1).unwrap(),
                ToBigInt::to_bigint(&-9).unwrap(),
                ToBigInt::to_bigint(&47).unwrap()
            ),
            utils::extended_euclid(
                &ToBigInt::to_bigint(&120).unwrap(),
                &ToBigInt::to_bigint(&23).unwrap()
            )
        );
        assert_eq!(
            (
                ToBigInt::to_bigint(&2).unwrap(),
                ToBigInt::to_bigint(&2).unwrap(),
                ToBigInt::to_bigint(&-11).unwrap()
            ),
            utils::extended_euclid(
                &ToBigInt::to_bigint(&122).unwrap(),
                &ToBigInt::to_bigint(&22).unwrap()
            )
        );
        assert_eq!(
            (
                ToBigInt::to_bigint(&2).unwrap(),
                ToBigInt::to_bigint(&-9).unwrap(),
                ToBigInt::to_bigint(&47).unwrap()
            ),
            utils::extended_euclid(
                &ToBigInt::to_bigint(&240).unwrap(),
                &ToBigInt::to_bigint(&46).unwrap()
            )
        );
        let (b, s, _) = utils::extended_euclid(&ToBigInt::to_bigint(&253).unwrap(), &*P);
        assert_eq!(b, BigInt::one());
        let s_field = BN128 {
            value: &s - s.div_floor(&*P) * &*P,
        };
        assert_eq!(
            BN128::from(
                "12717674712096337777352654721552646000065650461901806515903699665717959876900"
            ),
            s_field
        );
    }

    mod bellman {
        use super::*;

        use ff::Field as FField;

        extern crate rand;
        use pairing::bn256::Fr;
        use rand::{thread_rng, Rng};
        use Field;

        #[test]
        fn fr_to_field_to_fr() {
            let rng = &mut thread_rng();
            for _ in 0..1000 {
                let a: Fr = rng.gen();
                assert_eq!(BN128::from_bellman(a).into_bellman(), a);
            }
        }

        #[test]
        fn field_to_fr_to_field() {
            // use Fr to get a random element
            let rng = &mut thread_rng();
            for _ in 0..1000 {
                let a: Fr = rng.gen();
                // now test idempotence
                let a = BN128::from_bellman(a);
                assert_eq!(BN128::from_bellman(a.clone().into_bellman()), a);
            }
        }

        #[test]
        fn one() {
            let a = BN128::from(1);

            assert_eq!(a.into_bellman(), Fr::one());
        }

        #[test]
        fn zero() {
            let a = BN128::from(0);

            assert_eq!(a.into_bellman(), Fr::zero());
        }

        #[test]
        fn minus_one() {
            let mut a: Fr = Fr::one();
            a.negate();
            assert_eq!(BN128::from_bellman(a), BN128::from(-1));
        }

        #[test]
        fn add() {
            let rng = &mut thread_rng();

            let mut a: Fr = rng.gen();
            let b: Fr = rng.gen();

            let aa = BN128::from_bellman(a);
            let bb = BN128::from_bellman(b);
            let cc = aa + bb;

            a.add_assign(&b);

            assert_eq!(BN128::from_bellman(a), cc);
        }
    }

}
