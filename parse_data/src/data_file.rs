use std::fs::File;
use std::io::{Seek, SeekFrom, Read, Write};
use std::path::Path;
use super::TASK_OPS;

pub fn create(path: &Path) {
    let mut file = File::create(path).unwrap();
    for i in 0u64..2*TASK_OPS as u64 {
        let write_size = file.write(&i.to_le_bytes()).unwrap();
        assert_eq!(write_size, std::mem::size_of_val(&i));
    }
}

pub fn read_one_op(path: &Path, data: &mut [u64; TASK_OPS]) -> usize {
    let mut file = File::open(path).unwrap();

    file.seek(SeekFrom::Start((TASK_OPS * std::mem::size_of::<u64>()) as u64)).unwrap();

    let data_size = std::mem::size_of_val(data);
    let mut data_u8 = unsafe {
        std::slice::from_raw_parts_mut(
            data.as_mut_ptr() as *mut u8,
            data_size)
    };

    println!("data size: {}", std::mem::size_of_val(data_u8));
    let mut readed_size: usize = 0;
    while readed_size < data_size {
        let read_size = file.read(data_u8).unwrap();
        readed_size += read_size;
        data_u8 = &mut data_u8[read_size..];
        println!("{}, {}", read_size, std::mem::size_of_val(data_u8));
    }
    readed_size
}