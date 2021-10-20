use std::path::Path;
use std::fs::File;
use std::io::{Write, Seek, Read};
use std::env;
use std::convert::TryInto;

mod data;

#[derive(Default, Debug)]
pub struct LatencyTasks {
    task_cnt: usize,
    repeat_cnt: usize,
    name: Vec<String>
}

fn write_one_op_data(data: &[u64]) {
    let path = Path::new("./data.csv");
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

fn read_one_op(ops: usize, tasks: &LatencyTasks) {
    let path = Path::new("/dev/pmem0.1");
    // data_file::create(path);
    let mut data = vec![0u64; tasks.repeat_cnt].into_boxed_slice();
    let one_op: u64 = (tasks.repeat_cnt * std::mem::size_of::<u64>()) as u64;
    data::read(path, &mut *data, ops as u64 * one_op, one_op);

    write_one_op_data(&data);
}

fn read_all(tasks: &LatencyTasks) {
    let data_path = Path::new("/dev/pmem0.1");
    let csv_path = Path::new("./all-data.csv");
    let data_per_op = 1000;
    let mut data = vec![];
    let one_op: u64 = (tasks.repeat_cnt * std::mem::size_of::<u64>()) as u64;
    let data_size = (data_per_op * std::mem::size_of::<u64>()) as u64;
    for ops in 0..tasks.task_cnt {
        let mut op_data = vec![0u64; data_per_op].into_boxed_slice();
        data::read(data_path, &mut op_data, (ops as u64 + 2) * one_op, data_size);
        data.push(op_data);
    }
    let mut wtr = csv::Writer::from_path(csv_path).unwrap();
    wtr.write_record(&tasks.name).unwrap();
    for i in 0..data_per_op {
        let mut row = vec![];
        for op in &data {
            row.push(op[i]);
        }
        wtr.write_record(row.iter().map(|x| x.to_string())).unwrap();
    }
}

fn validate_args(args: &Vec<String>) -> (bool, usize) {
    if args.len() < 2 {
        println!("Usage: parse_data ops/all");
        return (false, 0);
    }

    let ops = args[1].clone();
    if ops == "all" {
        (true, 0)
    } else {
        match ops.parse::<usize>() {
            Ok(n) => (true, n),
            Err(err) => {
                println!("Error parsing command line arguments: {:?}", err);
                (false, 0)
            }
        }
    }
}

fn read_latency_tasks_info(file: &mut File) -> LatencyTasks {
    let mut tasks = LatencyTasks::default();
    let mut data = vec![0u8; 64].into_boxed_slice();

    file.rewind().unwrap();
    file.read(&mut data[..16]).unwrap();
    tasks.task_cnt = u64::from_le_bytes(data[..8].try_into().unwrap()) as usize;
    tasks.repeat_cnt = u64::from_le_bytes(data[8..16].try_into().unwrap()) as usize;
    let mut readed = 0;
    while readed < tasks.task_cnt {
        file.read_exact(&mut data).unwrap();
        let end = data.iter().position(|&x| x == 0).unwrap();
        tasks.name.push(String::from_utf8(data[..end].to_vec().clone()).unwrap());
        readed += 1;
    }
    println!("{:?}", tasks);
    tasks
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let (valid, ops) = validate_args(&args);
    if !valid {
        return;
    }

    let data_path = Path::new("/dev/pmem0.1");
    let mut data_file = File::open(data_path).unwrap();
    let tasks = read_latency_tasks_info(&mut data_file);
    if ops > tasks.task_cnt {
        println!("Maximum ops is {}!", tasks.task_cnt);
        return;
    }

    if ops == 0 {
        read_all(&tasks);
    } else {
        read_one_op(ops, &tasks);
    }
}
