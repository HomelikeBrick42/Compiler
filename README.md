# Compiler

## Variables

```c
// This is how you declare a variable
a: int = 5;
```

```c
// You can omit the type if it can be infered from the value
b := 5;
```

```c
// Or just have the type and have it default initialized
c: float;
```

```c
// Constants
PI :: 3.14;
```

```c
// You can still specify a type with constants
constant: float : 5;
```

## Procedures

```c
// A procedure looks like this
(a: int, b: float) -> int {
    return 5;
};
```

```c
// Usally you assign it to a constant like this
square :: (n: int) -> int {
    return n * n;
};
```

```c
// You call procedures like you would in c
value := square(5);
```

```c
// If you omit the body it becomes a procedure type
func_type :: (a: int, b: float) -> int;
```
