#include "options.h"

#include "../libponyc/ponyc.h"
#include "../libponyc/ast/parserapi.h"
#include "../libponyc/ast/bnfprint.h"
#include "../libponyc/pkg/package.h"
#include "../libponyc/pkg/buildflagset.h"
#include "../libponyc/ast/stringtab.h"
#include "../libponyc/ast/treecheck.h"
#include "../libponyc/options/options.h"
#include "../libponyc/pass/pass.h"
#include <platform.h>
#include "../libponyrt/options/options.h"
#include "../libponyrt/mem/pool.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define MAYBE_UNUSED(x) (void)x

#if !defined(PONY_DEFAULT_SSL)
// This default value is defined in packages/crypto and packages/net/ssl
#define PONY_DEFAULT_SSL "openssl_0.9.0"
#endif

enum
{
  OPT_VERSION,
  OPT_HELP,
  OPT_DEBUG,
  OPT_BUILDFLAG,
  OPT_STRIP,
  OPT_PATHS,
  OPT_OUTPUT,
  OPT_BIN_NAME,
  OPT_LIBRARY,
  OPT_RUNTIMEBC,
  OPT_PIC,
  OPT_NOPIC,
  OPT_DOCS,
  OPT_DOCS_PUBLIC,

  OPT_SAFE,
  OPT_CPU,
  OPT_FEATURES,
  OPT_TRIPLE,
  OPT_STATS,
  OPT_LINK_ARCH,
  OPT_LINKER,

  OPT_VERBOSE,
  OPT_PASSES,
  OPT_AST,
  OPT_ASTPACKAGE,
  OPT_TRACE,
  OPT_WIDTH,
  OPT_IMMERR,
  OPT_VERIFY,
  OPT_FILENAMES,
  OPT_CHECKTREE,
  OPT_EXTFUN,
  OPT_SIMPLEBUILTIN,
  OPT_LINT_LLVM,

  OPT_BNF,
  OPT_ANTLR,
  OPT_ANTLRRAW
};

static opt_arg_t std_args[] =
{
  {"version", 'v', OPT_ARG_NONE, OPT_VERSION},
  {"help", 'h', OPT_ARG_NONE, OPT_HELP},
  {"debug", 'd', OPT_ARG_NONE, OPT_DEBUG},
  {"define", 'D', OPT_ARG_REQUIRED, OPT_BUILDFLAG},
  {"strip", 's', OPT_ARG_NONE, OPT_STRIP},
  {"path", 'p', OPT_ARG_REQUIRED, OPT_PATHS},
  {"output", 'o', OPT_ARG_REQUIRED, OPT_OUTPUT},
  {"bin-name", 'b', OPT_ARG_REQUIRED, OPT_BIN_NAME},
  {"library", 'l', OPT_ARG_NONE, OPT_LIBRARY},
  {"runtimebc", '\0', OPT_ARG_NONE, OPT_RUNTIMEBC},
  {"pic", '\0', OPT_ARG_NONE, OPT_PIC},
  {"nopic", '\0', OPT_ARG_NONE, OPT_NOPIC},
  {"docs", 'g', OPT_ARG_NONE, OPT_DOCS},
  {"docs-public", '\0', OPT_ARG_NONE, OPT_DOCS_PUBLIC},

  {"safe", '\0', OPT_ARG_OPTIONAL, OPT_SAFE},
  {"cpu", '\0', OPT_ARG_REQUIRED, OPT_CPU},
  {"features", '\0', OPT_ARG_REQUIRED, OPT_FEATURES},
  {"triple", '\0', OPT_ARG_REQUIRED, OPT_TRIPLE},
  {"stats", '\0', OPT_ARG_NONE, OPT_STATS},
  {"link-arch", '\0', OPT_ARG_REQUIRED, OPT_LINK_ARCH},
  {"linker", '\0', OPT_ARG_REQUIRED, OPT_LINKER},

  {"verbose", 'V', OPT_ARG_REQUIRED, OPT_VERBOSE},
  {"pass", 'r', OPT_ARG_REQUIRED, OPT_PASSES},
  {"ast", 'a', OPT_ARG_NONE, OPT_AST},
  {"astpackage", '\0', OPT_ARG_NONE, OPT_ASTPACKAGE},
  {"trace", 't', OPT_ARG_NONE, OPT_TRACE},
  {"width", 'w', OPT_ARG_REQUIRED, OPT_WIDTH},
  {"immerr", '\0', OPT_ARG_NONE, OPT_IMMERR},
  {"verify", '\0', OPT_ARG_NONE, OPT_VERIFY},
  {"files", '\0', OPT_ARG_NONE, OPT_FILENAMES},
  {"checktree", '\0', OPT_ARG_NONE, OPT_CHECKTREE},
  {"extfun", '\0', OPT_ARG_NONE, OPT_EXTFUN},
  {"simplebuiltin", '\0', OPT_ARG_NONE, OPT_SIMPLEBUILTIN},
  {"lint-llvm", '\0', OPT_ARG_NONE, OPT_LINT_LLVM},

  {"bnf", '\0', OPT_ARG_NONE, OPT_BNF},
  {"antlr", '\0', OPT_ARG_NONE, OPT_ANTLR},
  {"antlrraw", '\0', OPT_ARG_NONE, OPT_ANTLRRAW},
  OPT_ARGS_FINISH
};

opt_arg_t* ponyc_opt_std_args(void) {
  return std_args;
}

static void usage(void)
{
  printf("%s\n%s\n%s\n%s\n%s\n%s", // for complying with -Woverlength-strings
    "ponyc [OPTIONS] <package directory>\n"
    ,
    "The package directory defaults to the current directory.\n"
    ,
    "Options:\n"
    "  --version, -v    Print the version of the compiler and exit.\n"
    "  --help, -h       Print this help text and exit.\n"
    "  --debug, -d      Don't optimise the output.\n"
    "  --define, -D     Define the specified build flag.\n"
    "    =name\n"
    "  --strip, -s      Strip debug info.\n"
    "  --path, -p       Add an additional search path.\n"
    "    =path          Used to find packages and libraries.\n"
    "  --output, -o     Write output to this directory.\n"
    "    =path          Defaults to the current directory.\n"
    "  --bin-name, -b   Name of executable binary.\n"
    "    =name          Defaults to name of the directory.\n"
    "  --library, -l    Generate a C-API compatible static library.\n"
    "  --runtimebc      Compile with the LLVM bitcode file for the runtime.\n"
    "  --pic            Compile using position independent code.\n"
    "  --nopic          Don't compile using position independent code.\n"
    "  --docs, -g       Generate code documentation.\n"
    "  --docs-public    Generate code documentation for public types only.\n"
    ,
    "Rarely needed options:\n"
    "  --safe           Allow only the listed packages to use C FFI.\n"
    "    =package       With no packages listed, only builtin is allowed.\n"
    "  --cpu            Set the target CPU.\n"
    "    =name          Default is the host CPU.\n"
    "  --features       CPU features to enable or disable.\n"
    "    =+this,-that   Use + to enable, - to disable.\n"
    "                   Defaults to detecting all CPU features from the host.\n"
    "  --triple         Set the target triple.\n"
    "    =name          Defaults to the host triple.\n"
    "  --stats          Print some compiler stats.\n"
    "  --link-arch      Set the linking architecture.\n"
    "    =name          Default is the host architecture.\n"
    "  --linker         Set the linker command to use.\n"
    "    =name          Default is the compiler used to compile ponyc.\n"
    "  --define, -D     Define which openssl version to use default is " PONY_DEFAULT_SSL "\n"
    "    =openssl_1.1.0\n"
    "    =openssl_0.9.0\n"
    ,
    "Debugging options:\n"
    "  --verbose, -V    Verbosity level.\n"
    "    =0             Only print errors.\n"
    "    =1             Print info on compiler stages.\n"
    "    =2             More detailed compilation information.\n"
    "    =3             External tool command lines.\n"
    "    =4             Very low-level detail.\n"
    PASS_HELP
    "  --ast, -a        Output an abstract syntax tree for the whole program.\n"
    "  --astpackage     Output an abstract syntax tree for the main package.\n"
    "  --trace, -t      Enable parse trace.\n"
    "  --width, -w      Width to target when printing the AST.\n"
    "    =columns       Defaults to the terminal width.\n"
    "  --immerr         Report errors immediately rather than deferring.\n"
    "  --checktree      Verify AST well-formedness.\n"
    "  --verify         Verify LLVM IR.\n"
    "  --extfun         Set function default linkage to external.\n"
    "  --simplebuiltin  Use a minimal builtin package.\n"
    "  --files          Print source file names as each is processed.\n"
    "  --bnf            Print out the Pony grammar as human readable BNF.\n"
    "  --antlr          Print out the Pony grammar as an ANTLR file.\n"
    "  --lint-llvm      Run the LLVM linting pass on generated IR.\n"
    ,
    "Runtime options for Pony programs (not for use with ponyc):\n"
    "  --ponythreads    Use N scheduler threads. Defaults to the number of\n"
    "                   cores (not hyperthreads) available.\n"
    "  --ponyminthreads Minimum number of active scheduler threads allowed.\n"
    "                   Defaults to 0.\n"
    "  --ponycdmin      Defer cycle detection until 2^N actors have blocked.\n"
    "                   Defaults to 2^4.\n"
    "  --ponycdmax      Always cycle detect when 2^N actors have blocked.\n"
    "                   Defaults to 2^18.\n"
    "  --ponycdconf     Send cycle detection CNF messages in groups of 2^N.\n"
    "                   Defaults to 2^6.\n"
    "  --ponygcinitial  Defer garbage collection until an actor is using at\n"
    "                   least 2^N bytes. Defaults to 2^14.\n"
    "  --ponygcfactor   After GC, an actor will next be GC'd at a heap memory\n"
    "                   usage N times its current value. This is a floating\n"
    "                   point value. Defaults to 2.0.\n"
    "  --ponynoyield    Do not yield the CPU when no work is available.\n"
    "  --ponynoblock    Do not send block messages to the cycle detector.\n"
    "  --ponynopin      Do not pin scheduler threads or the ASIO thread, even\n"
    "                   if --ponypinasio is set.\n"
    "  --ponypinasio    Pin the ASIO thread to a CPU the way scheduler\n"
    "                   threads are pinned to CPUs.\n"
    "  --ponyversion    Print the version of the compiler and exit.\n"
    );
}

#define OPENSSL_LEADER "openssl_"
static const char* valid_openssl_flags[] =
  { OPENSSL_LEADER "1.1.0", OPENSSL_LEADER "0.9.0", NULL };


static bool is_openssl_flag(const char* name)
{
  return 0 == strncmp(OPENSSL_LEADER, name, strlen(OPENSSL_LEADER));
}

static bool validate_openssl_flag(const char* name)
{
  for(const char** next = valid_openssl_flags; *next != NULL; next++)
  {
    if(0 == strcmp(*next, name))
      return true;
  }
  return false;
}

/**
 * Handle special cases of options like compile time defaults
 *
 * return CONTINUE if no errors else an ponyc_opt_process_t EXIT_XXX code.
 */
static ponyc_opt_process_t special_opt_processing(pass_opt_t *opt)
{
  // Suppress compiler errors due to conditional compilation
  MAYBE_UNUSED(opt);

#if defined(PONY_DEFAULT_PIC)
  #if (PONY_DEFAULT_PIC == true) || (PONY_DEFAULT_PIC == false)
    opt->pic = PONY_DEFAULT_PIC;
  #else
    #error "PONY_DEFAULT_PIC must be true or false"
  #endif
#endif

#if defined(USE_SCHEDULER_SCALING_PTHREADS)
  // Defined "scheduler_scaling_pthreads" so that SIGUSR2 is made available for
  // use by the signals package when not using signals for scheduler scaling
  define_build_flag("scheduler_scaling_pthreads");
#endif

  define_build_flag(PONY_DEFAULT_SSL);

  return CONTINUE;
}


ponyc_opt_process_t ponyc_opt_process(opt_state_t* s, pass_opt_t* opt,
       /*OUT*/ bool* print_program_ast,
       /*OUT*/ bool* print_package_ast)
{
  ponyc_opt_process_t exit_code = CONTINUE;
  int id;
  *print_program_ast = false;
  *print_package_ast = false;

  exit_code = special_opt_processing(opt);
  if(exit_code != CONTINUE)
    return exit_code;
  
  while((id = ponyint_opt_next(s)) != -1)
  {
    switch(id)
    {
      case OPT_VERSION:
        printf("%s\n", PONY_VERSION_STR);
        printf("Defaults: pic=%s ssl=%s\n", opt->pic ? "true" : "false",
            PONY_DEFAULT_SSL);
        return EXIT_0;

      case OPT_HELP:
        usage();
        return EXIT_0;
      case OPT_DEBUG: opt->release = false; break;
      case OPT_BUILDFLAG:
        if(is_openssl_flag(s->arg_val))
        {
          if(validate_openssl_flag(s->arg_val))
          { // User wants to add an openssl_flag,
            // remove any existing openssl_flags.
            remove_build_flags(valid_openssl_flags);
          } else {
            printf("Error: %s is invalid openssl flag, expecting one of:\n",
                s->arg_val);
            for(const char** next=valid_openssl_flags; *next != NULL; next++)
              printf("       %s\n", *next);
            return EXIT_255;
          }
        }
        define_build_flag(s->arg_val);
        break;
      case OPT_STRIP: opt->strip_debug = true; break;
      case OPT_PATHS: package_add_paths(s->arg_val, opt); break;
      case OPT_OUTPUT: opt->output = s->arg_val; break;
      case OPT_BIN_NAME: opt->bin_name = s->arg_val; break;
      case OPT_LIBRARY: opt->library = true; break;
      case OPT_RUNTIMEBC: opt->runtimebc = true; break;
      case OPT_PIC: opt->pic = true; break;
      case OPT_NOPIC: opt->pic = false; break;
      case OPT_DOCS:
        {
          opt->docs = true;
          opt->docs_private = true;
        }
        break;
      case OPT_DOCS_PUBLIC:
        {
          opt->docs = true;
          opt->docs_private = false;
        }
        break;
      case OPT_SAFE:
        if(!package_add_safe(s->arg_val, opt))
        {
          printf("Error adding safe packages: %s\n", s->arg_val);
          exit_code = EXIT_255;
        }
        break;

      case OPT_CPU: opt->cpu = s->arg_val; break;
      case OPT_FEATURES: opt->features = s->arg_val; break;
      case OPT_TRIPLE: opt->triple = s->arg_val; break;
      case OPT_STATS: opt->print_stats = true; break;
      case OPT_LINK_ARCH: opt->link_arch = s->arg_val; break;
      case OPT_LINKER: opt->linker = s->arg_val; break;

      case OPT_AST: *print_program_ast = true; break;
      case OPT_ASTPACKAGE: *print_package_ast = true; break;
      case OPT_TRACE: opt->parse_trace = true; break;
      case OPT_WIDTH: opt->ast_print_width = atoi(s->arg_val); break;
      case OPT_IMMERR: errors_set_immediate(opt->check.errors, true); break;
      case OPT_VERIFY: opt->verify = true; break;
      case OPT_EXTFUN: opt->extfun = true; break;
      case OPT_SIMPLEBUILTIN: opt->simple_builtin = true; break;
      case OPT_FILENAMES: opt->print_filenames = true; break;
      case OPT_CHECKTREE: opt->check_tree = true; break;
      case OPT_LINT_LLVM: opt->lint_llvm = true; break;

      case OPT_BNF: print_grammar(false, true); return EXIT_0;
      case OPT_ANTLR: print_grammar(true, true); return EXIT_0;
      case OPT_ANTLRRAW: print_grammar(true, false); return EXIT_0;

      case OPT_VERBOSE:
        {
          int v = atoi(s->arg_val);
          if(v >= 0 && v <= 4)
          {
            opt->verbosity = (verbosity_level)v;
          } else {
            printf("Verbosity must be 0..4, %d is invalid\n", v);
            exit_code = EXIT_255;
          }
        }
        break;

      case OPT_PASSES:
        if(!limit_passes(opt, s->arg_val))
        {
          printf("Invalid pass=%s it should be one of:\n%s", s->arg_val,
             "  parse,syntax,sugar,scope,import,name,flatten,traits,docs,\n"
             "  refer,expr,verify,final,reach,paint,ir,bitcode,asm,obj,all\n");
          exit_code = EXIT_255;
        }
        break;

      default:
        printf("BUG: unprocessed option id %d\n", id);
        return EXIT_255;
    }
  }

  for(int i = 1; i < (*s->argc); i++)
  {
    if(s->argv[i][0] == '-')
    {
      printf("Unrecognised option: %s\n", s->argv[i]);
      exit_code = EXIT_255;
    }
  }

  return exit_code;
}
