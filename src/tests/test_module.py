print("Hello from recomp incbin test module");

fib_holder_0 = 0
fib_holder_1 = 1
def fibonacci_step():
    global fib_holder_0, fib_holder_1
    fib_next = fib_holder_0 + fib_holder_1
    fib_holder_0 = fib_holder_1
    fib_holder_1 = fib_next