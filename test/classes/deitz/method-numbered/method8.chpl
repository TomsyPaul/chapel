class foo {
  var x : int = 12;
  proc bar(x : int) {
    writeln(x);
  }
}

var f : foo = new unmanaged foo();

f.x = 12;
f.bar(13);
f.bar(f.x);

delete f;
