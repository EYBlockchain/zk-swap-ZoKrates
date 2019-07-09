use std::fs::File;
use zokrates_field::Field;

mod utils;
#[cfg(feature = "libsnark")]
mod pghr13;
#[cfg(feature = "libsnark")]
mod pghr13_mnt4;
#[cfg(feature = "libsnark")]
mod pghr13_mnt6;
// mod g16
#[cfg(feature = "libsnark")]
mod gm17;

#[cfg(feature = "libsnark")]
pub use self::pghr13::PGHR13;
#[cfg(feature = "libsnark")]
pub use self::pghr13_mnt4::PGHR13_MNT4;
#[cfg(feature = "libsnark")]
pub use self::pghr13_mnt6::PGHR13_MNT6;
// pub use self::g16::G16;
#[cfg(feature = "libsnark")]
pub use self::gm17::GM17;

use crate::ir;
use std::io::BufReader;

pub trait ProofSystem {
    type F: Field;

    fn setup(&self, program: ir::Prog<Self::F>, pk_path: &str, vk_path: &str);

    fn generate_proof(
        &self,
        program: ir::Prog<Self::F>,
        witness: ir::Witness<Self::F>,
        pk_path: &str,
        proof_path: &str,
    ) -> bool;

    fn export_solidity_verifier(&self, reader: BufReader<File>) -> String;
}
