/**
 * Move : means the transfer of ownership.
 * Only one alias can get the ownership of a variable. 
 * 
 * "=" works as move.
 * 
 */

pub struct Foo {
    value: u8,
}

fn main() {
    let foo = Foo { value: 42 };
    let bar = foo;  // move: transfer the data ownership of Foo, from foo to bar.

    //println!("{}", foo.value); // compilation error: use of moved value: `foo.value`
    println!("{}", bar.value);
}