extern crate libc;

use self::libc::c_char;
use std::ffi::CString;

extern "C" {
    fn _pghr13_mnt4_mnt6_batch(
        vk_1_path: *const c_char,
        proof_1_path: *const c_char,
        vk_2_path: *const c_char,
        proof_2_path: *const c_char,
        agg_vk_path: *const c_char,
        agg_proof_path: *const c_char,
    ) -> bool;
    fn _pghr13_mnt6_mnt4_batch(
        vk_1_path: *const c_char,
        proof_1_path: *const c_char,
        vk_2_path: *const c_char,
        proof_2_path: *const c_char,
        agg_vk_path: *const c_char,
        agg_proof_path: *const c_char,
    ) -> bool;
/*    fn _pghr13_mnt4753_mnt6753_batch(
        vk_1_path: *const c_char,
        proof_1_path: *const c_char,
        vk_2_path: *const c_char,
        proof_2_path: *const c_char,
        agg_vk_path: *const c_char,
        agg_proof_path: *const c_char,
    ) -> bool;
    fn _pghr13_mnt6753_mnt4753_batch(
        vk_1_path: *const c_char,
        proof_1_path: *const c_char,
        vk_2_path: *const c_char,
        proof_2_path: *const c_char,
        agg_vk_path: *const c_char,
        agg_proof_path: *const c_char,
    ) -> bool;*/
}

pub fn batch(
    from_curve: &str,
    to_curve: &str,
    vk_1_path: &str,
    proof_1_path: &str,
    vk_2_path: &str,
    proof_2_path: &str,
    agg_vk_path: &str,
    agg_proof_path: &str,
) -> bool {
    let vk_1_path_cstring = CString::new(vk_1_path).unwrap();
    let proof_1_path_cstring = CString::new(proof_1_path).unwrap();
    let vk_2_path_cstring = CString::new(vk_2_path).unwrap();
    let proof_2_path_cstring = CString::new(proof_2_path).unwrap();
    let agg_vk_path_cstring = CString::new(agg_vk_path).unwrap();
    let agg_proof_path_cstring = CString::new(agg_proof_path).unwrap();
    match (from_curve, to_curve) {
        ("MNT4", "MNT6") => unsafe {
            _pghr13_mnt4_mnt6_batch(
                vk_1_path_cstring.as_ptr(),
                proof_1_path_cstring.as_ptr(),
                vk_2_path_cstring.as_ptr(),
                proof_2_path_cstring.as_ptr(),
                agg_vk_path_cstring.as_ptr(),
                agg_proof_path_cstring.as_ptr(),
            )
        },
        ("MNT6", "MNT4") => unsafe {
            _pghr13_mnt6_mnt4_batch(
                vk_1_path_cstring.as_ptr(),
                proof_1_path_cstring.as_ptr(),
                vk_2_path_cstring.as_ptr(),
                proof_2_path_cstring.as_ptr(),
                agg_vk_path_cstring.as_ptr(),
                agg_proof_path_cstring.as_ptr(),
            )
        },
        /*("MNT4753", "MNT6753") => unsafe {
            _pghr13_mnt4753_mnt6753_batch(
                vk_1_path_cstring.as_ptr(),
                proof_1_path_cstring.as_ptr(),
                vk_2_path_cstring.as_ptr(),
                proof_2_path_cstring.as_ptr(),
                agg_vk_path_cstring.as_ptr(),
                agg_proof_path_cstring.as_ptr(),
            )
        },
        ("MNT6753", "MNT4753") => unsafe {
            _pghr13_mnt6753_mnt4753_batch(
                vk_1_path_cstring.as_ptr(),
                proof_1_path_cstring.as_ptr(),
                vk_2_path_cstring.as_ptr(),
                proof_2_path_cstring.as_ptr(),
                agg_vk_path_cstring.as_ptr(),
                agg_proof_path_cstring.as_ptr(),
            )
        },*/ 
        _ => panic!("Not supported batch"),
    }
}
