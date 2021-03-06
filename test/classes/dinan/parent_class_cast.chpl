class P {
    proc f() return "";
}

class C: P {
    proc f() return "C";
}

class D: P {
    proc f() return "D";
}

// OK: var ps: [1..2] P = (C():P, D():P);
var ps: [1..2] P = (new borrowed C(), new borrowed D());

for i in ps do
    writeln(i.f());
