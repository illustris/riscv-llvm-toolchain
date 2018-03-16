extern "C"
void g() {};
void f() __attribute__((alias("g")));
