## TODO:

- [x] Tokens
  - [x] Add Positions to Tokens
- [ ] AST
  - [ ] Combine Update Expression with Unary Expression if needed
  - [x] Implement `else if` and `else`
  - [x] Make `if`, `while` statements use Expression instead of just Block
  - [x] Array type expression `[]type`
  - [x] Array expression `[1, 2, 3, ...]`
  - [x] Implement error system
  - [x] Implement `break` and `continue`
  - [x] Switch statement `switch (<expr>) <switch-block>`
  - [x] Implement enum
  - [ ] Switch from memory arena to allocation tree to allow AST optimizations
  - [ ] Implement casts
- [ ] Semantics Check
  - [x] Implement error system
  - [ ] Implement auto-cast
- [ ] Optimizer
  - [ ] Comptime binary expression solver
  - [ ] Comptime unary expression solver

- [ ] Bytecode
  - [ ] Make the Bytecode Structure compatible with assembly to allow Assembly Generation
    - [ ] Implement registers
- [ ] Assembly Generator
