# Example DOME Rust Plugin

This example is similar to the [traditional plugin](https://github.com/domeengine/dome/tree/main/examples/plugin). But
we are calling a Rust function to log a message.

![console](https://user-images.githubusercontent.com/292738/210174087-e0ea398f-c71d-423d-8350-956340f88f45.png)

- *test.c*:

```c
void alertMethod(WrenVM* vm) {
  // ...
  core->log(ctx, "%s\n", rust_function());
  // ...
}
```

Is important to include the generated static lib in the compilation step. `libexample.a`

```Makefile
test.dylib: test.c
	gcc -dynamiclib -o test.dylib -I../../include test.c libexample.a -undefined dynamic_lookup
```
