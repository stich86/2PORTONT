/* test_conf_parser_ov4.c test */

/* test all kinds of options and the conf file parser */
/* differently from test_conf_parser_ov.c, first scan the conf file and
   then the command line; however, if options were specified in the
   conf file and also in the command line we get an error (uses
   check_ambiguity parameter */

#include <stdlib.h>
#include <stdio.h>

#include "test_conf_parser_cmd.h"

static struct my_args_info args_info;

int
main (int argc, char **argv)
{
  int result = 0;
  struct test_conf_parser_cmd_parser_params *params;
  
  /* initialize the parameters structure */
  params = test_conf_parser_cmd_parser_params_create();
  
  /* 
     initialize args_info, but don't check for required options
     NOTICE: the other fields are initialized to their default values
  */
  params->check_required = 0;

  /* call the config file parser */
  if (test_conf_parser_cmd_parser_config_file
      ("../../tests/test_conf2.conf", &args_info, params) != 0) {
    result = 1;
    goto stop;
  }

  /* 
     check ambiguity with config file options,
     do not initialize args_info, check for required options.
  */
  params->initialize = 0;
  params->check_ambiguity = 1;
  params->check_required = 1;

  /* call the command line parser */
  if (test_conf_parser_cmd_parser_ext (argc, argv, &args_info, params) != 0) {
    result = 1;
    goto stop;
  }

  printf ("value of required: %s\n", args_info.required_arg);
  printf ("value of string: %s\n", args_info.string_arg);
  printf ("value of no-short_given: %d\n", args_info.no_short_given);
  printf ("value of int: %d\n", args_info.int_arg);
  printf ("value of float: %f\n", args_info.float_arg);

 stop:
  /* deallocate structures */
  test_conf_parser_cmd_parser_free (&args_info);
  free (params);

  return result;
}
