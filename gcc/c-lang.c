/* Language-specific hook definitions for C front end.
   Copyright (C) 1991, 1995, 1997, 1998,
   1999, 2000 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#include "config.h"
#include "system.h"
#include "tree.h"
#include "function.h"
#include "input.h"
#include "toplev.h"
#include "diagnostic.h"
#include "output.h"
#include "flags.h"
#include "ggc.h"
#include "rtl.h"
#include "expr.h"
#include "c-tree.h"
#include "c-lex.h"

static int c_tree_printer PARAMS ((output_buffer *));

#if USE_CPPLIB
#include "cpplib.h"
extern cpp_reader  parse_in;
#endif

/* Each of the functions defined here
   is an alternative to a function in objc-actions.c.  */
   
int
lang_decode_option (argc, argv)
     int argc;
     char **argv;
{
  return c_decode_option (argc, argv);
}

void
lang_init_options ()
{
#if USE_CPPLIB
  cpp_init ();
  cpp_reader_init (&parse_in);
#endif
  /* Mark as "unspecified".  */
  flag_bounds_check = -1;
}

void
lang_init ()
{
  /* If still "unspecified", make it match -fbounded-pointers.  */
  if (flag_bounds_check < 0)
    flag_bounds_check = flag_bounded_pointers;

  /* If still unspecified, make it match pedantic && -std=c99.  */
  if (mesg_implicit_function_declaration < 0)
    {
      if (pedantic && flag_isoc99)
	mesg_implicit_function_declaration = flag_pedantic_errors ? 2 : 1;
      else
	mesg_implicit_function_declaration = 0;
    }

  /* the beginning of the file is a new line; check for # */
  /* With luck, we discover the real source file's name from that
     and put it in input_filename.  */
#if !USE_CPPLIB
  ungetc (check_newline (), finput);
#endif

  save_lang_status = &push_c_function_context;
  restore_lang_status = &pop_c_function_context;
  mark_lang_status = &mark_c_function_context;
  lang_expand_expr = &c_expand_expr;
  lang_safe_from_p = &c_safe_from_p;
  lang_printer = &c_tree_printer;
  lang_expand_decl_stmt = &c_expand_decl_stmt;

  c_parse_init ();
}

void
lang_finish ()
{
}

const char *
lang_identify ()
{
  return "c";
}

void
print_lang_statistics ()
{
}

/* used by print-tree.c */

void
lang_print_xnode (file, node, indent)
     FILE *file ATTRIBUTE_UNUSED;
     tree node ATTRIBUTE_UNUSED;
     int indent ATTRIBUTE_UNUSED;
{
}

/* Used by c-lex.c, but only for objc.  */

tree
lookup_interface (arg)
     tree arg ATTRIBUTE_UNUSED;
{
  return 0;
}

tree
is_class_name (arg)
    tree arg ATTRIBUTE_UNUSED;
{
  return 0;
}

void
maybe_objc_check_decl (decl)
     tree decl ATTRIBUTE_UNUSED;
{
}

int
maybe_objc_comptypes (lhs, rhs, reflexive)
     tree lhs ATTRIBUTE_UNUSED;
     tree rhs ATTRIBUTE_UNUSED;
     int reflexive ATTRIBUTE_UNUSED;
{
  return -1;
}

tree
maybe_objc_method_name (decl)
    tree decl ATTRIBUTE_UNUSED;
{
  return 0;
}

tree
maybe_building_objc_message_expr ()
{
  return 0;
}

int
recognize_objc_keyword ()
{
  return 0;
}

/* Used by c-typeck.c (build_external_ref), but only for objc.  */

tree
lookup_objc_ivar (id)
     tree id ATTRIBUTE_UNUSED;
{
  return 0;
}

#if !defined(ASM_OUTPUT_CONSTRUCTOR) || !defined(ASM_OUTPUT_DESTRUCTOR)
extern tree static_ctors;
extern tree static_dtors;

static tree start_cdtor		PARAMS ((int));
static void finish_cdtor	PARAMS ((tree));

static tree
start_cdtor (method_type)
     int method_type;
{
  tree fnname = get_file_function_name (method_type);
  tree void_list_node_1 = build_tree_list (NULL_TREE, void_type_node);
  tree body;

  start_function (void_list_node_1,
		  build_parse_node (CALL_EXPR, fnname, 
				    tree_cons (NULL_TREE, NULL_TREE, 
					       void_list_node_1),
				    NULL_TREE),
		  NULL_TREE, NULL_TREE);
  store_parm_decls ();

  current_function_cannot_inline
    = "static constructors and destructors cannot be inlined";

  body = c_begin_compound_stmt ();

  pushlevel (0);
  clear_last_expr ();
  add_scope_stmt (/*begin_p=*/1, /*partial_p=*/0);

  return body;
}

static void
finish_cdtor (body)
     tree body;
{
  tree scope;
  tree block;

  scope = add_scope_stmt (/*begin_p=*/0, /*partial_p=*/0);
  block = poplevel (0, 0, 0); 
  SCOPE_STMT_BLOCK (TREE_PURPOSE (scope)) = block;
  SCOPE_STMT_BLOCK (TREE_VALUE (scope)) = block;

  RECHAIN_STMTS (body, COMPOUND_BODY (body)); 

  finish_function (0);
}
#endif

/* Called at end of parsing, but before end-of-file processing.  */

void
finish_file ()
{
#ifndef ASM_OUTPUT_CONSTRUCTOR
  if (static_ctors)
    {
      tree body = start_cdtor ('I');

      for (; static_ctors; static_ctors = TREE_CHAIN (static_ctors))
	c_expand_expr_stmt (build_function_call (TREE_VALUE (static_ctors),
						 NULL_TREE));

      finish_cdtor (body);
    }
#endif
#ifndef ASM_OUTPUT_DESTRUCTOR
  if (static_dtors)
    {
      tree body = start_cdtor ('D');

      for (; static_dtors; static_dtors = TREE_CHAIN (static_dtors))
	c_expand_expr_stmt (build_function_call (TREE_VALUE (static_dtors),
						 NULL_TREE));

      finish_cdtor (body);
    }
#endif
  
  if (back_end_hook)
    (*back_end_hook) (getdecls ());
}

/* Called during diagnostic message formatting process to print a
   source-level entity onto BUFFER.  The meaning of the format specifiers
   is as follows:
   %D: a general decl,
   %F: a function declaration,
   %T: a type.

   These format specifiers form a subset of the format specifiers set used
   by the C++ front-end.
   Please notice when called, the `%' part was already skipped by the
   diagnostic machinery.  */
static int
c_tree_printer (buffer)
     output_buffer *buffer;
{
  tree t = va_arg (output_buffer_format_args (buffer), tree);

  switch (*output_buffer_text_cursor (buffer))
    {
    case 'D':
    case 'F':
    case 'T':
      {
        const char *n = DECL_NAME (t)
          ? (*decl_printable_name) (t, 2)
          : "({anonymous})";
        output_add_string (buffer, n);
      }
      return 1;

    default:
      return 0;
    }
}
