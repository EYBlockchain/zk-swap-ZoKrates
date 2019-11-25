extern crate libc;

use std::ffi::CString;
use self::libc::{c_char, c_int};
use ir;
use proof_system::mnt::utils::libsnark::{prepare_setup, prepare_generate_proof};
use proof_system::ProofSystem;

use std::fs::File;
use std::io::BufReader;

use zokrates_field::field::FieldPrime;

extern "C" {
    fn _gm17_mnt6_setup(
        A: *const u8,
        B: *const u8,
        C: *const u8,
        A_len: c_int,
        B_len: c_int,
        C_len: c_int,
        constraints: c_int,
        variables: c_int,
        inputs: c_int,
        pk_path: *const c_char,
        vk_path: *const c_char,
    ) -> bool;

    fn _gm17_mnt6_generate_proof(
        pk_path: *const c_char,
        proof_path: *const c_char,
        public_inputs: *const u8,
        public_inputs_length: c_int,
        private_inputs: *const u8,
        private_inputs_length: c_int,
    ) -> bool;

    fn _gm17_mnt6_verify_proof(
        vk_path: *const c_char,
        proof_path: *const c_char,
    ) -> bool;
}

pub struct GM17_MNT6 {}

impl GM17_MNT6 {
    pub fn new() -> GM17_MNT6 {
        GM17_MNT6 {}
    }
}

impl ProofSystem for GM17_MNT6 {
    fn setup(&self, program: ir::Prog<FieldPrime>, pk_path: &str, vk_path: &str) {
        let (
            a_arr,
            b_arr,
            c_arr,
            a_vec,
            b_vec,
            c_vec,
            num_constraints,
            num_variables,
            num_inputs,
            pk_path_cstring,
            vk_path_cstring,
        ) = prepare_setup(program, pk_path, vk_path);

        unsafe {
            _gm17_mnt6_setup(
                a_arr.as_ptr(),
                b_arr.as_ptr(),
                c_arr.as_ptr(),
                a_vec.len() as i32,
                b_vec.len() as i32,
                c_vec.len() as i32,
                num_constraints as i32,
                num_variables as i32,
                num_inputs as i32,
                pk_path_cstring.as_ptr(),
                vk_path_cstring.as_ptr(),
            );
        }
    }

    fn generate_proof(
        &self,
        program: ir::Prog<FieldPrime>,
        witness: ir::Witness<FieldPrime>,
        pk_path: &str,
        proof_path: &str,
    ) -> bool {
        let (
            pk_path_cstring,
            proof_path_cstring,
            public_inputs_arr,
            public_inputs_length,
            private_inputs_arr,
            private_inputs_length,
        ) = prepare_generate_proof(program, witness, pk_path, proof_path);

        unsafe {
            _gm17_mnt6_generate_proof(
                pk_path_cstring.as_ptr(),
                proof_path_cstring.as_ptr(),
                public_inputs_arr[0].as_ptr(),
                public_inputs_length as i32,
                private_inputs_arr[0].as_ptr(),
                private_inputs_length as i32,
            )
        }
    }

    fn verify_proof(
        &self,
        vk_path: &str,
        proof_path: &str,
    ) -> bool {
        let vk_path_cstring = CString::new(vk_path).unwrap();
        let proof_path_cstring = CString::new(proof_path).unwrap();
        unsafe {
            _gm17_mnt6_verify_proof(
                vk_path_cstring.as_ptr(),
                proof_path_cstring.as_ptr(),
            )
        }
    }

    fn export_solidity_verifier(&self, _: BufReader<File>) -> String {
        panic!("Not implemented");
    }

}
