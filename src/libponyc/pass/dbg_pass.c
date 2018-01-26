#include "dbg_pass.h"
#include "../ast/id.h"
#include "../pass/expr.h"
#include "../../libponyrt/mem/pool.h"
#include "ponyassert.h"
#include <string.h>

void dbg_pass_fprint(FILE* fp, const char* fun_name, const char* ex, int rv,
    const char* s)
{
  if(rv == RV_IGNORE)
    if(s == NULL)
      fprintf(fp, "%s:%s\n", fun_name, ex);
    else
      fprintf(fp, "%s:%s %s\n", fun_name, ex, s);
  else
    if(s == NULL)
      fprintf(fp, "%s:%s rv=%d\n", fun_name, ex, rv);
    else
      fprintf(fp, "%s:%s rv=%d %s\n", fun_name, ex, rv, s);
}

void dbg_pass_ast_fprint_and_parents(FILE* fp, const char* fun_name,
    const char* ex, int rv, const char* ast_name, ast_t* ast, const char* s,
    int number, int width)
{

  for(int i=0; (ast != NULL) && (i < number); i++)
  {
    if(rv == RV_IGNORE)
      if(s == NULL)
        fprintf(fp, "%s:%s %s[%d]: ", fun_name, ex, ast_name, -i);
      else
        fprintf(fp, "%s:%s %s %s[%d]: ", fun_name, ex, s, ast_name, -i);
    else
      if(s == NULL)
        fprintf(fp, "%s:%s %sv=%d %s[%d]: ", fun_name, ex,
          ((strcmp("-",ex) == 0) ? "r" : ""), rv, ast_name, -i);
      else
        fprintf(fp, "%s:%s %sv=%d %s %s[%d]: ", fun_name, ex,
          ((strcmp("-",ex) == 0) ? "r" : ""), rv, s, ast_name, -i);
    ast_fprint(fp, ast, width);
    ast = ast_parent(ast);
  }
}

void dbg_pass_ast_fprint_siblings(FILE* fp, const char* fun_name,
    const char* ast_name, ast_t* ast, int width)
{
  ast_t* parent = ast_parent(ast);
  ast_t* child = ast_child(parent);
  for(int i=0; (child != NULL); i++)
  {
    fprintf(fp, "%s %s[%d]: ", fun_name, ast_name, i);
    ast_fprint(fp, child, width);
    child = ast_sibling(child);
  }
}
