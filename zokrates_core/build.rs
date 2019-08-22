#[cfg(feature = "libsnark")]
extern crate cc;
#[cfg(feature = "libsnark")]
extern crate cmake;

fn main() {
    #[cfg(feature = "libsnark")]
    {
        use std::env;
        use std::path::PathBuf;

        // submodule path
        let libsnark_source_path = &PathBuf::from(env::var("OUT_DIR").unwrap())
            .parent().unwrap()
            .parent().unwrap()
            .parent().unwrap()
            .parent().unwrap()
            .parent().unwrap()
            .join("deps")
            .join("libsnark");

        // build libsnark
        let libsnark = cmake::Config::new(libsnark_source_path)
            .define("WITH_PROCPS", "OFF")
            .define("USE_PT_COMPRESSION", "OFF")
            .define("MONTGOMERY_OUTPUT", "ON")
            .define("BINARY_OUTPUT", "ON")
            .build();

        // build backends
        cc::Build::new()
            .cpp(true)
            .debug(cfg!(debug_assertions))
            .flag("-std=c++11")
            .include(libsnark_source_path)
            .include(libsnark_source_path.join("depends/libff"))
            .include(libsnark_source_path.join("depends/libfqfft"))
            .file("lib/gm17.cpp")
            .file("lib/pghr13.cpp")
            .compile("libwraplibsnark.a");

        println!(
            "cargo:rustc-link-search=native={}",
            libsnark.join("lib").display()
        );

        println!("cargo:rustc-link-lib=gmp");
        println!("cargo:rustc-link-lib=gmpxx");
        println!("cargo:rustc-link-lib=ssl");
        println!("cargo:rustc-link-lib=crypto");

        #[cfg(debug_assertions)]
        {
            println!("cargo:rustc-link-lib=static=snarkd");
            println!("cargo:rustc-link-lib=static=ffd");
        }
        #[cfg(not(debug_assertions))]
        {
            println!("cargo:rustc-link-lib=static=snark");
            println!("cargo:rustc-link-lib=static=ff");
        }
    }
}
