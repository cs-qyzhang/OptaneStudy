use std::path::Path;

mod data_file;

pub const TASK_OPS: usize = 1024;

fn main() {
    let path = Path::new("./data.txt");
    data_file::create(path);
    let mut data = [0u64; TASK_OPS];
    data_file::read_one_op(path, &mut data);

    for i in data {
        println!("{}", i);
    }
}
