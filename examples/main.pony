"""
Pony examples tests

This package will eventually test all examples. At the moment we only
use them to type check.
"""

use "ponytest"

// Include all example packages here, even if they don't have tests, so
// that we load them to perform syntax and type checks.

// Commented packages are broken at the moment. We have to fix them and
// then keep the tests green.

// The FFI examples below are commented out beccause they may
// not link correctly due to one or more missing libraries that
// our naive Makefile does not know about.

use ex_circle = "circle"
use ex_commandline = "commandline"
use ex_counter = "counter"
use ex_dbg = "dbg"
// No compilable Pony code inside: use ex_dtrace = "dtrace"
use ex_echo = "echo"
// use ex_ffi_callbacks = "ffi-callbacks"
// use ex_ffi_struct = "ffi-struct"
use ex_files = "files"
use ex_gups_basic = "gups_basic"
use ex_gups_opt = "gups_opt"
use ex_helloworld = "helloworld"
use ex_httpget = "httpget"
use ex_httpserver = "httpserver"
use ex_ifdef = "ifdef"
use ex_lambda = "lambda"
use ex_mailbox = "mailbox"
use ex_mandelbrot = "mandelbrot"
use ex_message_ubench = "message-ubench"
use ex_mixed = "mixed"
use ex_n_body = "n-body"
use ex_net = "net"
use ex_overload = "overload"
use ex_printargs = "printargs"
use ex_producer_consumer = "producer-consumer"
use ex_readline = "readline"
use ex_ring = "ring"
use ex_spreader = "spreader"
use ex_timers = "timers"
use ex_under_pressure = "under_pressure"
use ex_yield = "yield"


actor Main
  new create(env: Env) =>
    env.out.write("All examples compile!\n")
