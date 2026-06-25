# Prior

The Prior programming language. Prior is a metaprogrammable statically-typed system programming language that enables the use of runtime modifications (A.K.A Runtime Metaprogramming) while also designed to be *yet another improvement to the C programming language*.

The current state of the program is only a basic bytecode generator and a VM. Assembly Generation is planned.

## Hello World

```rs
import "std.io";

fn main (): void {
    const string: []c8 = "World"
    std.io.println("Hello ${string}!");
}
```

## How to build

Use `bash build.sh` to build the program. No CMake yet, apologies.
