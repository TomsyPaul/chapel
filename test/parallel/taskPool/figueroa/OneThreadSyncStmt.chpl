config const n = 10;
var total: sync int = 0;

def foo (x) {
    total += x;
}

sync {
  for i in 1..n do
    begin foo(i);
}

writeln ("total is ", total);
