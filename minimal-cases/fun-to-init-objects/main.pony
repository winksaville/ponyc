class TestFunInit
  var a_u8: U8

  new make() =>
    init_A()

  fun ref init_A() =>
    a_u8 = 21

actor Main
  new create(env: Env) =>
    var c: TestFunInit ref = TestFunInit.make()
    env.out.print("c.a_u8=" + c.a_u8.string())
