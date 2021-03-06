extern crate libc;

use std::ffi::CString;
use self::libc::{c_char, c_int};
use ir;
use proof_system::bn128::utils::libsnark::{prepare_setup, prepare_generate_proof};
use proof_system::bn128::utils::solidity::{SOLIDITY_G2_ADDITION_LIB, SOLIDITY_PAIRING_LIB};
use proof_system::ProofSystem;

use regex::Regex;
use std::fs::File;
use std::io::{BufRead, BufReader};

use zokrates_field::field::FieldPrime;

pub struct PGHR13 {}

impl PGHR13 {
    pub fn new() -> PGHR13 {
        PGHR13 {}
    }
}

extern "C" {
    fn _pghr13_setup(
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

    fn _pghr13_generate_proof(
        pk_path: *const c_char,
        proof_path: *const c_char,
        public_inputs: *const u8,
        public_inputs_length: c_int,
        private_inputs: *const u8,
        private_inputs_length: c_int,
    ) -> bool;

    fn _pghr13_verify_proof(
        vk_path: *const c_char,
        proof_path: *const c_char,
    ) -> bool;
}

impl ProofSystem for PGHR13 {
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
            _pghr13_setup(
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

        println!(
            "{:?}",
            (pk_path_cstring.clone(), proof_path_cstring.clone(),)
        );

        unsafe {
            _pghr13_generate_proof(
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
            _pghr13_verify_proof(
                vk_path_cstring.as_ptr(),
                proof_path_cstring.as_ptr(),
            )
        }
    }

    fn export_solidity_verifier(&self, reader: BufReader<File>) -> String {
        let mut lines = reader.lines();

        let mut template_text = String::from(CONTRACT_TEMPLATE);
        let ic_template = String::from("vk.IC[index] = Pairing.G1Point(points);"); //copy this for each entry

        //replace things in template
        let vk_regex = Regex::new(r#"(<%vk_[^i%]*%>)"#).unwrap();
        let vk_ic_len_regex = Regex::new(r#"(<%vk_ic_length%>)"#).unwrap();
        let vk_ic_index_regex = Regex::new(r#"index"#).unwrap();
        let vk_ic_points_regex = Regex::new(r#"points"#).unwrap();
        let vk_ic_repeat_regex = Regex::new(r#"(<%vk_ic_pts%>)"#).unwrap();
        let vk_input_len_regex = Regex::new(r#"(<%vk_input_length%>)"#).unwrap();

        for _ in 0..7 {
            let current_line: String = lines
                .next()
                .expect("Unexpected end of file in verification key!")
                .unwrap();
            let current_line_split: Vec<&str> = current_line.split("=").collect();
            assert_eq!(current_line_split.len(), 2);
            template_text = vk_regex
                .replace(template_text.as_str(), current_line_split[1].trim())
                .into_owned();
        }

        let current_line: String = lines
            .next()
            .expect("Unexpected end of file in verification key!")
            .unwrap();
        let current_line_split: Vec<&str> = current_line.split("=").collect();
        assert_eq!(current_line_split.len(), 2);
        let ic_count: i32 = current_line_split[1].trim().parse().unwrap();

        template_text = vk_ic_len_regex
            .replace(template_text.as_str(), format!("{}", ic_count).as_str())
            .into_owned();
        template_text = vk_input_len_regex
            .replace(template_text.as_str(), format!("{}", ic_count - 1).as_str())
            .into_owned();

        let mut ic_repeat_text = String::new();
        for x in 0..ic_count {
            let mut curr_template = ic_template.clone();
            let current_line: String = lines
                .next()
                .expect("Unexpected end of file in verification key!")
                .unwrap();
            let current_line_split: Vec<&str> = current_line.split("=").collect();
            assert_eq!(current_line_split.len(), 2);
            curr_template = vk_ic_index_regex
                .replace(curr_template.as_str(), format!("{}", x).as_str())
                .into_owned();
            curr_template = vk_ic_points_regex
                .replace(curr_template.as_str(), current_line_split[1].trim())
                .into_owned();
            ic_repeat_text.push_str(curr_template.as_str());
            if x < ic_count - 1 {
                ic_repeat_text.push_str("\n        ");
            }
        }
        template_text = vk_ic_repeat_regex
            .replace(template_text.as_str(), ic_repeat_text.as_str())
            .into_owned();

        let re = Regex::new(r"(?P<v>0[xX][0-9a-fA-F]{64})").unwrap();
        template_text = re.replace_all(&template_text, "uint256($v)").to_string();

        format!(
            "{}{}{}",
            SOLIDITY_G2_ADDITION_LIB, SOLIDITY_PAIRING_LIB, template_text
        )
    }
}

const CONTRACT_TEMPLATE: &str = r#"contract Verifier {
    using Pairing for *;
    struct VerifyingKey {
        Pairing.G2Point A;
        Pairing.G1Point B;
        Pairing.G2Point C;
        Pairing.G2Point gamma;
        Pairing.G1Point gammaBeta1;
        Pairing.G2Point gammaBeta2;
        Pairing.G2Point Z;
        Pairing.G1Point[] IC;
    }
    struct Proof {
        Pairing.G1Point A;
        Pairing.G1Point A_p;
        Pairing.G2Point B;
        Pairing.G1Point B_p;
        Pairing.G1Point C;
        Pairing.G1Point C_p;
        Pairing.G1Point K;
        Pairing.G1Point H;
    }
    function verifyingKey() pure internal returns (VerifyingKey memory vk) {
        vk.A = Pairing.G2Point(<%vk_a%>);
        vk.B = Pairing.G1Point(<%vk_b%>);
        vk.C = Pairing.G2Point(<%vk_c%>);
        vk.gamma = Pairing.G2Point(<%vk_g%>);
        vk.gammaBeta1 = Pairing.G1Point(<%vk_gb1%>);
        vk.gammaBeta2 = Pairing.G2Point(<%vk_gb2%>);
        vk.Z = Pairing.G2Point(<%vk_z%>);
        vk.IC = new Pairing.G1Point[](<%vk_ic_length%>);
        <%vk_ic_pts%>
    }
    function verify(uint[] memory input, Proof memory proof) internal returns (uint) {
        VerifyingKey memory vk = verifyingKey();
        require(input.length + 1 == vk.IC.length);
        // Compute the linear combination vk_x
        Pairing.G1Point memory vk_x = Pairing.G1Point(0, 0);
        for (uint i = 0; i < input.length; i++)
            vk_x = Pairing.addition(vk_x, Pairing.scalar_mul(vk.IC[i + 1], input[i]));
        vk_x = Pairing.addition(vk_x, vk.IC[0]);
        if (!Pairing.pairingProd2(proof.A, vk.A, Pairing.negate(proof.A_p), Pairing.P2())) return 1;
        if (!Pairing.pairingProd2(vk.B, proof.B, Pairing.negate(proof.B_p), Pairing.P2())) return 2;
        if (!Pairing.pairingProd2(proof.C, vk.C, Pairing.negate(proof.C_p), Pairing.P2())) return 3;
        if (!Pairing.pairingProd3(
            proof.K, vk.gamma,
            Pairing.negate(Pairing.addition(vk_x, Pairing.addition(proof.A, proof.C))), vk.gammaBeta2,
            Pairing.negate(vk.gammaBeta1), proof.B
        )) return 4;
        if (!Pairing.pairingProd3(
                Pairing.addition(vk_x, proof.A), proof.B,
                Pairing.negate(proof.H), vk.Z,
                Pairing.negate(proof.C), Pairing.P2()
        )) return 5;
        return 0;
    }
    event Verified(string s);
    function verifyTx(
            uint[2] memory a,
            uint[2] memory a_p,
            uint[2][2] memory b,
            uint[2] memory b_p,
            uint[2] memory c,
            uint[2] memory c_p,
            uint[2] memory h,
            uint[2] memory k,
            uint[<%vk_input_length%>] memory input
        ) public returns (bool r) {
        Proof memory proof;
        proof.A = Pairing.G1Point(a[0], a[1]);
        proof.A_p = Pairing.G1Point(a_p[0], a_p[1]);
        proof.B = Pairing.G2Point([b[0][0], b[0][1]], [b[1][0], b[1][1]]);
        proof.B_p = Pairing.G1Point(b_p[0], b_p[1]);
        proof.C = Pairing.G1Point(c[0], c[1]);
        proof.C_p = Pairing.G1Point(c_p[0], c_p[1]);
        proof.H = Pairing.G1Point(h[0], h[1]);
        proof.K = Pairing.G1Point(k[0], k[1]);
        uint[] memory inputValues = new uint[](input.length);
        for(uint i = 0; i < input.length; i++){
            inputValues[i] = input[i];
        }
        if (verify(inputValues, proof) == 0) {
            emit Verified("Transaction successfully verified.");
            return true;
        } else {
            return false;
        }
    }
}
"#;
