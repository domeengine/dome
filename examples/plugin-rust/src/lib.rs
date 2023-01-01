use std::ffi::CString;

// Check: https://doc.rust-lang.org/stable/std/ffi/struct.CString.html#method.into_raw
// Check: https://docs.rust-embedded.org/book/interoperability/rust-with-c.html
#[no_mangle]
pub extern "C" fn rust_function() -> *mut i8 {
  let message = String::from("Hello, world! from Rust Function!");
  let c_str = CString::new(message).unwrap();
  return c_str.into_raw();
}
