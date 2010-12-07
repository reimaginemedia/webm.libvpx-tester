/*
*  Copyright (c) 2010 The VP8 project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/


#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "args.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#if defined(__GNUC__) && __GNUC__
extern void die(const char *fmt, ...) __attribute__((noreturn));
#else
extern void die(const char *fmt, ...);
#endif


struct arg arg_init(char **argv)
{
    struct arg a;

    a.argv      = argv;
    a.argv_step = 1;
    a.name      = NULL;
    a.val       = NULL;
    a.def       = NULL;
    return a;
}

int arg_match(struct arg *arg_, const struct arg_def *def, char **argv)
{
    struct arg arg;

    if (!argv[0] || argv[0][0] != '-')
        return 0;

    arg = arg_init(argv);

    if (def->short_name
        && strlen(arg.argv[0]) == strlen(def->short_name) + 1
        && !strcmp(arg.argv[0] + 1, def->short_name))
    {

        arg.name = arg.argv[0] + 1;
        arg.val = def->has_val ? arg.argv[1] : NULL;
        arg.argv_step = def->has_val ? 2 : 1;
    }
    else if (def->long_name)
    {
        int name_len = strlen(def->long_name);

        if (strlen(arg.argv[0]) >= name_len + 2
            && arg.argv[0][1] == '-'
            && !strncmp(arg.argv[0] + 2, def->long_name, name_len)
            && (arg.argv[0][name_len+2] == '='
                || arg.argv[0][name_len+2] == '\0'))
        {

            arg.name = arg.argv[0] + 2;
            arg.val = arg.name[name_len] == '=' ? arg.name + name_len + 1 : NULL;
            arg.argv_step = 1;
        }
    }

    if (arg.name && !arg.val && def->has_val)
        die("Error: option %s requires argument.\n", arg.name);

    if (arg.name && arg.val && !def->has_val)
        die("Error: option %s requires no argument.\n", arg.name);

    if (arg.name
        && (arg.val || !def->has_val))
    {
        arg.def = def;
        *arg_ = arg;
        return 1;
    }

    return 0;
}


const char *arg_next(struct arg *arg)
{
    if (arg->argv[0])
        arg->argv += arg->argv_step;

    return *arg->argv;
}


char **argv_dup(int argc, const char **argv)
{
    char **new_argv = (char **)malloc((argc + 1) * sizeof(*argv));

    memcpy(new_argv, argv, argc * sizeof(*argv));
    new_argv[argc] = NULL;
    return new_argv;
}


void arg_show_usage(FILE *fp, const struct arg_def *const *defs)
{
    char option_text[40] = {0};

    for (; *defs; defs++)
    {
        const struct arg_def *def = *defs;
        const char *short_val = def->has_val ? " <arg>" : "";
        const char *long_val = def->has_val ? "=<arg>" : "";

        if (def->short_name && def->long_name)
            snprintf(option_text, 37, "-%s%s, --%s%s",
                     def->short_name, short_val,
                     def->long_name, long_val);
        else if (def->short_name)
            snprintf(option_text, 37, "-%s%s",
                     def->short_name, short_val);
        else if (def->long_name)
            snprintf(option_text, 37, "          --%s%s",
                     def->long_name, long_val);

        fprintf(fp, "  %-37s\t%s\n", option_text, def->desc);
    }
}


unsigned int arg_parse_uint(const struct arg *arg)
{
    long int   rawval;
    char      *endptr;

    rawval = strtol(arg->val, &endptr, 10);

    if (arg->val[0] != '\0' && endptr[0] == '\0')
    {
        if (rawval >= 0 && rawval <= UINT_MAX)
            return rawval;

        die("Option %s: Value %ld out of range for unsigned int\n",
            arg->name, rawval);
    }

    die("Option %s: Invalid character '%c'\n", arg->name, *endptr);
    return 0;
}


int arg_parse_int(const struct arg *arg)
{
    long int   rawval;
    char      *endptr;

    rawval = strtol(arg->val, &endptr, 10);

    if (arg->val[0] != '\0' && endptr[0] == '\0')
    {
        if (rawval >= INT_MIN && rawval <= INT_MAX)
            return rawval;

        die("Option %s: Value %ld out of range for signed int\n",
            arg->name, rawval);
    }

    die("Option %s: Invalid character '%c'\n", arg->name, *endptr);
    return 0;
}


struct vpx_rational
{
    int num; /**< fraction numerator */
    int den; /**< fraction denominator */
};
struct vpx_rational arg_parse_rational(const struct arg *arg)
{
    long int             rawval;
    char                *endptr;
    struct vpx_rational  rat;

    /* parse numerator */
    rawval = strtol(arg->val, &endptr, 10);

    if (arg->val[0] != '\0' && endptr[0] == '/')
    {
        if (rawval >= INT_MIN && rawval <= INT_MAX)
            rat.num = rawval;
        else die("Option %s: Value %ld out of range for signed int\n",
                     arg->name, rawval);
    }
    else die("Option %s: Expected / at '%c'\n", arg->name, *endptr);

    /* parse denominator */
    rawval = strtol(endptr + 1, &endptr, 10);

    if (arg->val[0] != '\0' && endptr[0] == '\0')
    {
        if (rawval >= INT_MIN && rawval <= INT_MAX)
            rat.den = rawval;
        else die("Option %s: Value %ld out of range for signed int\n",
                     arg->name, rawval);
    }
    else die("Option %s: Invalid character '%c'\n", arg->name, *endptr);

    return rat;
}
