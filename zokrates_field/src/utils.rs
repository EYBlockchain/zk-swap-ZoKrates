use num_bigint::{BigInt};
use num_traits::{One, Zero};

/// Calculates the gcd using an iterative implementation of the extended euclidian algorithm.
/// Returning `(d, s, t)` so that `d = s * a + t * b`
///
/// # Arguments
/// * `a` - First number as `BigInt`
/// * `b` - Second number as `BigInt`
pub fn extended_euclid(a: &BigInt, b: &BigInt) -> (BigInt, BigInt, BigInt) {
    let (mut s, mut old_s) = (BigInt::zero(), BigInt::one());
    let (mut t, mut old_t) = (BigInt::one(), BigInt::zero());
    let (mut r, mut old_r) = (b.clone(), a.clone());
    while !&r.is_zero() {
        let quotient = &old_r / &r;
        let tmp_r = old_r.clone();
        old_r = r.clone();
        r = &tmp_r - &quotient * &r;
        let tmp_s = old_s.clone();
        old_s = s.clone();
        s = &tmp_s - &quotient * &s;
        let tmp_t = old_t.clone();
        old_t = t.clone();
        t = &tmp_t - &quotient * &t;
    }
    return (old_r, old_s, old_t);
}

