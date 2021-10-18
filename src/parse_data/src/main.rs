use std::path::Path;
use std::fs::File;
use std::io::Write;
use std::env;

mod data_file;

pub const TASK_OPS: usize = 1048576;
pub const OPS_CNT: usize = 65;

fn write_one_op_data(data: &[u64]) {
    let path = Path::new("./data.txt");
    let mut file = File::create(path).unwrap();
    let mut cnt = 0;
    for &i in data {
        file.write(i.to_string().as_bytes()).unwrap();
        if i > 2000 {
            println!("{}", i);
            cnt += 1;
        }
        file.write(&[b'\n']).unwrap();
    }
    println!("cnt: {}", cnt);
}

fn read_one_op(ops: u64) {
    let path = Path::new("/dev/pmem0.1");
    // data_file::create(path);
    let mut data = vec![0u64; TASK_OPS].into_boxed_slice();
    let one_op: u64 = (TASK_OPS * std::mem::size_of::<u64>()) as u64;
    data_file::read(path, &mut *data, ops * one_op, one_op);

    write_one_op_data(&data);
}

fn read_all() {
    let data_path = Path::new("/dev/pmem0.1");
    let csv_path = Path::new("./all-data.txt");
    let data_per_op = 1000;
    let mut data = vec![];
    let one_op: u64 = (TASK_OPS * std::mem::size_of::<u64>()) as u64;
    let data_size = (data_per_op * std::mem::size_of::<u64>()) as u64;
    for ops in 1..=OPS_CNT {
        let mut op_data = vec![0u64; data_per_op + 1].into_boxed_slice();
        op_data[0] = ops as u64;
        data_file::read(data_path, &mut op_data[1..], ops as u64 * one_op, data_size);
        data.push(op_data);
    }
    let mut wtr = csv::Writer::from_path(csv_path).unwrap();
    for i in 0..=data_per_op {
        let mut row = vec![];
        for op in &data {
            row.push(op[i]);
        }
        wtr.write_record(row.iter().map(|x| x.to_string())).unwrap();
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        println!("Usage: parse_data ops/all");
        return;
    }

    let ops = args[1].clone();
    if ops == "all" {
        read_all();
    } else {
        let ops: u64 = args[1].parse().unwrap();
        if ops > (OPS_CNT as u64) {
            println!("Maximum ops is {}!", OPS_CNT);
            return;
        }
        read_one_op(ops);
    }
}
