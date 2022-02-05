/**
 * The created sub-thread will run concurrently withe the Main thread.
 * However, when the Main thread exits, the sub-thread will exit too.
 * 
 * We can use a join function to wait for the sub-thread.
 * 
 * https://doc.rust-lang.org/book/ch16-01-threads.html
 * 
 * 
 */


use std::thread;
use std::time::Duration;

fn main() {
    let handle = thread::spawn(|| {
        for i in 1..10 {
            println!("hi number {} from the spawned thread!", i);
            thread::sleep(Duration::from_millis(1));
        }
    });

    for i in 1..5 {
        println!("hi number {} from the main thread!", i);
        thread::sleep(Duration::from_millis(1));
    }

	// wait for the created sub-thread
	// Or it will exit when the main thread exits.
	handle.join().unwrap();
}