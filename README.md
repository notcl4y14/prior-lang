# Prior

The Prior programming language. Prior is a metaprogrammable statically-typed system programming language that enables the use of runtime modifications (A.K.A Runtime Metaprogramming) while being on the near level as C.

Currently the language does NOT have a compiler until the language's lexical, syntactical, semantical and logical structures are proven to be stable. For now the language is run by an interpreter. When the time comes, the development of the actual compiler (assembly generation) will begin.

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
