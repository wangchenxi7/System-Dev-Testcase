/**
 * Two threads increase a shared counter via the Channel.
 * 
 */

use std::thread;
use std::sync::mpsc;
use std::time::Duration;

fn main() {
    let (tx, rx) = mpsc::channel();
    

	// Thread#1, send the ownership of its created variable, val
	thread::spawn(move || {
        let val = String::from("hi");

		// [?] Will this sleep cause problems for the receiver?
		// NO. The receiver will wait for the message.
		println!("Preparing to send ownership of variable: val");
		thread::sleep(Duration::from_millis(2000));
        
		tx.send(val).unwrap();

		// the code below will not prevent the exiting of main thread.
		// The main thread needs a join function to wait for the exit of the sub-thread.
		thread::sleep(Duration::from_millis(2000));
		println!("Ownership sent.");
    });

	// Let main thread receive the ownership of the variable, val
	// rx.recv() will block the main thread’s execution and wait until a value is sent down the channel. 
	let received = rx.recv().unwrap();

	//let received = String::from("nothing");

    println!("Got: {}", received);
}