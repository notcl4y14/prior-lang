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

```sh
mkdir build && cd build/
cmake .. && cmake --build .
```

## Testing (developers)

```sh
# Build test
cmake --build . -t build_tests
# Run test
cmake --build . -t test
```

Use `assert` from `assert.h` to write tests. CTest output from CMake is vague and `stderr` and `stdout` are intercepted, so run the individual test executables in `build` (for example `build/test_lexer`) to find out which part of the test (or which assert) triggered the failure.