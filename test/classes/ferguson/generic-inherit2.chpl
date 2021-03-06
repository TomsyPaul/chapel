class Parent {
  type t;
  var x:t;
  proc parent_method() {
    writeln(x);
  }
  proc overridden_method() {
    writeln(x);
  }

}

class Child : Parent {
  var y:t;
  proc overridden_method() {
    writeln(x,y);
  }
  proc child_method() {
    writeln(y);
  }
}

writeln("Parent(int)");
var p = new borrowed Parent(int, 1);
p.parent_method();
p.overridden_method();

writeln("Child(int)");
var c = new borrowed Child(int, 1, 2);
c.parent_method();
c.overridden_method();
c.child_method();

writeln("Dynamic Child(int)");
var pc:Parent(int) = c;
pc.parent_method();
pc.overridden_method();
