mod bn128;
mod mnt;
mod mnt753;
mod bls12_377;
mod sw6;
mod edwards;
#[cfg(feature = "libsnark")]
mod batch;

use std::fs::File;
use zokrates_field::field::FieldPrime;

pub use self::bn128::G16;
#[cfg(feature = "libsnark")]
pub use self::bn128::GM17;
#[cfg(feature = "libsnark")]
pub use self::bn128::PGHR13;
#[cfg(feature = "libsnark")]
pub use self::bls12_377::PGHR13_BLS12;
#[cfg(feature = "libsnark")]
pub use self::mnt::PGHR13_MNT4;
#[cfg(feature = "libsnark")]
pub use self::mnt::PGHR13_MNT6;
#[cfg(feature = "libsnark")]
pub use self::mnt753::PGHR13_MNT4753;
#[cfg(feature = "libsnark")]
pub use self::sw6::PGHR13_SW6;
#[cfg(feature = "libsnark")]
pub use self::edwards::PGHR13_EDWARDS;
#[cfg(feature = "libsnark")]
pub use self::batch::*;

use crate::ir;
use std::io::BufReader;

pub trait ProofSystem {
    fn setup(&self, program: ir::Prog<FieldPrime>, pk_path: &str, vk_path: &str);

    fn generate_proof(
        &self,
        program: ir::Prog<FieldPrime>,
        witness: ir::Witness<FieldPrime>,
        pk_path: &str,
        proof_path: &str,
    ) -> bool;

    fn verify_proof(
        &self,
        vk_path: &str,
        proof_path: &str,
    ) -> bool;

    fn export_solidity_verifier(&self, reader: BufReader<File>) -> String;
}
