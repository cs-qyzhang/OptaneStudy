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

pub fn read(path: &Path, data: &mut [u64], pos: u64, cnt: u64) -> u64 {
    let mut file = File::open(path).unwrap();

    file.seek(SeekFrom::Start(pos)).unwrap();

    let mut data_u8 = unsafe {
        std::slice::from_raw_parts_mut(
            data.as_mut_ptr() as *mut u8,
            std::mem::size_of_val(data))
    };

    let mut readed_size: u64 = 0;
    while readed_size < cnt {
        let read_size = file.read(data_u8).unwrap();
        readed_size += read_size as u64;
        data_u8 = &mut data_u8[read_size..];
    }
    readed_size
}