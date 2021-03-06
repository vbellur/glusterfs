/*
  Copyright (c) 2010-2011 Gluster, Inc. <http://www.gluster.com>
  This file is part of GlusterFS.

  GlusterFS is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License,
  or (at your option) any later version.

  GlusterFS is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <fnmatch.h>

#include "xlator.h"
#include "defaults.h"

#define GF_OPTION_LIST_EMPTY(_opt) (_opt->value[0] == NULL)


static int
xlator_option_validate_path (xlator_t *xl, const char *key, const char *value,
                             volume_option_t *opt, char **op_errstr)
{
        int   ret = -1;
        char  errstr[256];

        if (strstr (value, "../")) {
                snprintf (errstr, 256,
                          "invalid path given '%s'",
                          value);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

                /* Make sure the given path is valid */
        if (value[0] != '/') {
                snprintf (errstr, 256,
                          "option %s %s: '%s' is not an "
                          "absolute path name",
                          key, value, value);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_int (xlator_t *xl, const char *key, const char *value,
                            volume_option_t *opt, char **op_errstr)
{
        long long inputll = 0;
        int       ret = -1;
        char      errstr[256];

        /* Check the range */
        if (gf_string2longlong (value, &inputll) != 0) {
                snprintf (errstr, 256,
                          "invalid number format \"%s\" in option \"%s\"",
                          value, key);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        if ((opt->min == 0) && (opt->max == 0)) {
                gf_log (xl->name, GF_LOG_DEBUG,
                        "no range check required for 'option %s %s'",
                        key, value);
                ret = 0;
                goto out;
        }

        if ((inputll < opt->min) || (inputll > opt->max)) {
                snprintf (errstr, 256,
                          "'%lld' in 'option %s %s' is out of range "
                          "[%"PRId64" - %"PRId64"]",
                          inputll, key, value, opt->min, opt->max);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_sizet (xlator_t *xl, const char *key, const char *value,
                              volume_option_t *opt, char **op_errstr)
{
        uint64_t  size = 0;
        int       ret = -1;
        char      errstr[256];

        /* Check the range */
        if (gf_string2bytesize (value, &size) != 0) {
                snprintf (errstr, 256,
                          "invalid number format \"%s\" in option \"%s\"",
                          value, key);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        if ((opt->min == 0) && (opt->max == 0)) {
                gf_log (xl->name, GF_LOG_DEBUG,
                        "no range check required for 'option %s %s'",
                        key, value);
                ret = 0;
                goto out;
        }

        if ((size < opt->min) || (size > opt->max)) {
                snprintf (errstr, 256,
                          "'%"PRId64"' in 'option %s %s' is out of range "
                          "[%"PRId64" - %"PRId64"]",
                          size, key, value, opt->min, opt->max);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_bool (xlator_t *xl, const char *key, const char *value,
                             volume_option_t *opt, char **op_errstr)
{
        int          ret = -1;
        char         errstr[256];
        gf_boolean_t bool;


        /* Check if the value is one of
           '0|1|on|off|no|yes|true|false|enable|disable' */

        if (gf_string2boolean (value, &bool) != 0) {
                snprintf (errstr, 256,
                          "option %s %s: '%s' is not a valid boolean value",
                          key, value, value);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_xlator (xlator_t *xl, const char *key, const char *value,
                               volume_option_t *opt, char **op_errstr)
{
        int          ret = -1;
        char         errstr[256];
        xlator_t    *xlopt = NULL;


        /* Check if the value is one of the xlators */
        xlopt = xl;
        while (xlopt->prev)
                xlopt = xlopt->prev;

        while (xlopt) {
                if (strcmp (value, xlopt->name) == 0) {
                        ret = 0;
                        break;
                }
                xlopt = xlopt->next;
        }

        if (!xlopt) {
                snprintf (errstr, 256,
                          "option %s %s: '%s' is not a valid volume name",
                          key, value, value);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_str (xlator_t *xl, const char *key, const char *value,
                            volume_option_t *opt, char **op_errstr)
{
        int          ret = -1;
        int          i = 0;
        char         errstr[256];
        char         given_array[4096] = {0,};

        /* Check if the '*str' is valid */
        if (GF_OPTION_LIST_EMPTY(opt)) {
                ret = 0;
                goto out;
        }

        for (i = 0; (i < ZR_OPTION_MAX_ARRAY_SIZE) && opt->value[i]; i++) {
 #ifdef  GF_DARWIN_HOST_OS
                if (fnmatch (opt->value[i], value, 0) == 0) {
                        ret = 0;
                        break;
                }
 #else
                if (fnmatch (opt->value[i], value, FNM_EXTMATCH) == 0) {
                        ret = 0;
                        break;
                }
 #endif
        }

        if ((i <= ZR_OPTION_MAX_ARRAY_SIZE) && (!opt->value[i])) {
                /* enter here only if
                 * 1. reached end of opt->value array and haven't
                 *    validated input
                 *                      OR
                 * 2. valid input list is less than
                 *    ZR_OPTION_MAX_ARRAY_SIZE and input has not
                 *    matched all possible input values.
                 */

                for (i = 0; (i < ZR_OPTION_MAX_ARRAY_SIZE) && opt->value[i];) {
                        strcat (given_array, opt->value[i]);
                        if (((++i) < ZR_OPTION_MAX_ARRAY_SIZE) &&
                           (opt->value[i]))
                                strcat (given_array, ", ");
                        else
                                strcat (given_array, ".");
                }
                snprintf (errstr, 256,
                          "option %s %s: '%s' is not valid "
                          "(possible options are %s)",
                          key, value, value, given_array);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_percent (xlator_t *xl, const char *key, const char *value,
                                volume_option_t *opt, char **op_errstr)
{
        int          ret = -1;
        char         errstr[256];
        uint32_t     percent = 0;


        /* Check if the value is valid percentage */
        if (gf_string2percent (value, &percent) != 0) {
                snprintf (errstr, 256,
                          "invalid percent format \"%s\" in \"option %s\"",
                          value, key);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        if ((percent < 0) || (percent > 100)) {
                snprintf (errstr, 256,
                          "'%d' in 'option %s %s' is out of range [0 - 100]",
                          percent, key, value);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


/* TODO: clean this up */
static int
xlator_option_validate_percent_or_sizet (xlator_t *xl, const char *key,
                                         const char *value,
                                         volume_option_t *opt, char **op_errstr)
{
        int          ret = -1;
        char         errstr[256];
        uint32_t     percent = 0;
        uint64_t     size = 0;

        /* Check if the value is valid percentage */
        if (gf_string2percent (value, &percent) == 0) {
                if (percent > 100) {
                        gf_log (xl->name, GF_LOG_DEBUG,
                                "value given was greater than 100, "
                                "assuming this is actually a size");
                        if (gf_string2bytesize (value, &size) == 0) {
                                        /* Check the range */
                                if ((opt->min == 0) && (opt->max == 0)) {
                                        gf_log (xl->name, GF_LOG_DEBUG,
                                                "no range check rquired for "
                                                "'option %s %s'",
                                                key, value);
                                                // It is a size
                                                ret = 0;
                                                goto out;
                                }
                                if ((size < opt->min) || (size > opt->max)) {
                                        snprintf (errstr, 256,
                                                  "'%"PRId64"' in 'option %s %s' "
                                                  "is out of range [%"PRId64" - "
                                                  "%"PRId64"]",
                                                  size, key, value,
                                                  opt->min, opt->max);
                                        gf_log (xl->name, GF_LOG_ERROR, "%s",
                                                errstr);
                                        goto out;
                                }
                                // It is a size
                                ret = 0;
                                goto out;
                        } else {
                                // It's not a percent or size
                                snprintf (errstr, 256,
                                          "invalid number format \"%s\" "
                                          "in \"option %s\"",
                                          value, key);
                                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                                goto out;
                        }
                }
                // It is a percent
                ret = 0;
                goto out;
        } else {
                if (gf_string2bytesize (value, &size) == 0) {
                        /* Check the range */
                        if ((opt->min == 0) && (opt->max == 0)) {
                                gf_log (xl->name, GF_LOG_DEBUG,
                                        "no range check required for "
                                        "'option %s %s'",
                                        key, value);
                                // It is a size
                                ret = 0;
                                goto out;
                        }
                        if ((size < opt->min) || (size > opt->max)) {
                                snprintf (errstr, 256,
                                          "'%"PRId64"' in 'option %s %s'"
                                          " is out of range [%"PRId64" -"
                                          " %"PRId64"]",
                                          size, key, value, opt->min, opt->max);
                                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                                goto out;
                        }
                } else {
                        // It's not a percent or size
                        snprintf (errstr, 256,
                                  "invalid number format \"%s\" in \"option %s\"",
                                  value, key);
                        gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                        goto out;
                }
                //It is a size
                ret = 0;
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_time (xlator_t *xl, const char *key, const char *value,
                             volume_option_t *opt, char **op_errstr)
{
        int          ret = -1;
        char         errstr[256];
        uint32_t     input_time = 0;

                /* Check if the value is valid percentage */
        if (gf_string2time (value, &input_time) != 0) {
                snprintf (errstr, 256,
                          "invalid time format \"%s\" in "
                          "\"option %s\"",
                          value, key);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        if ((opt->min == 0) && (opt->max == 0)) {
                gf_log (xl->name, GF_LOG_DEBUG,
                        "no range check required for "
                        "'option %s %s'",
                        key, value);
                ret = 0;
                goto out;
        }

        if ((input_time < opt->min) || (input_time > opt->max)) {
                snprintf (errstr, 256,
                          "'%"PRIu32"' in 'option %s %s' is "
                          "out of range [%"PRId64" - %"PRId64"]",
                          input_time, key, value,
                          opt->min, opt->max);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_double (xlator_t *xl, const char *key, const char *value,
                               volume_option_t *opt, char **op_errstr)
{
        int          ret = -1;
        char         errstr[256];
        double       val = 0.0;

        /* Check if the value is valid double */
        if (gf_string2double (value, &val) != 0) {
                snprintf (errstr, 256,
                          "invalid double \"%s\" in \"option %s\"",
                          value, key);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        if (val < 0.0) {
                snprintf (errstr, 256,
                          "invalid double \"%s\" in \"option %s\"",
                          value, key);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                goto out;
        }

        if ((opt->min == 0) && (opt->max == 0)) {
                gf_log (xl->name, GF_LOG_DEBUG,
                        "no range check required for 'option %s %s'",
                        key, value);
                ret = 0;
                goto out;
        }

        ret = 0;
out:
        if (ret && op_errstr)
                *op_errstr = gf_strdup (errstr);
        return ret;
}


static int
xlator_option_validate_addr (xlator_t *xl, const char *key, const char *value,
                             volume_option_t *opt, char **op_errstr)
{
        int          ret = -1;
        char         errstr[256];

        if (!valid_internet_address ((char *)value)) {
                snprintf (errstr, 256,
                          "internet address '%s' does not conform to standards.",
                          value);
                gf_log (xl->name, GF_LOG_ERROR, "%s", errstr);
                if (op_errstr)
                        *op_errstr = gf_strdup (errstr);
        }

        ret = 0;

        return ret;
}


static int
xlator_option_validate_any (xlator_t *xl, const char *key, const char *value,
                            volume_option_t *opt, char **op_errstr)
{
        return 0;
}


typedef int (xlator_option_validator_t) (xlator_t *xl, const char *key,
                                         const char *value,
                                         volume_option_t *opt, char **operrstr);

int
xlator_option_validate (xlator_t *xl, char *key, char *value,
                        volume_option_t *opt, char **op_errstr)
{
        int       ret = -1;
        xlator_option_validator_t *validate;
        xlator_option_validator_t *validators[] = {
                [GF_OPTION_TYPE_PATH]        = xlator_option_validate_path,
                [GF_OPTION_TYPE_INT]         = xlator_option_validate_int,
                [GF_OPTION_TYPE_SIZET]       = xlator_option_validate_sizet,
                [GF_OPTION_TYPE_BOOL]        = xlator_option_validate_bool,
                [GF_OPTION_TYPE_XLATOR]      = xlator_option_validate_xlator,
                [GF_OPTION_TYPE_STR]         = xlator_option_validate_str,
                [GF_OPTION_TYPE_PERCENT]     = xlator_option_validate_percent,
                [GF_OPTION_TYPE_PERCENT_OR_SIZET] =
                xlator_option_validate_percent_or_sizet,
                [GF_OPTION_TYPE_TIME]        = xlator_option_validate_time,
                [GF_OPTION_TYPE_DOUBLE]      = xlator_option_validate_double,
                [GF_OPTION_TYPE_INTERNET_ADDRESS] = xlator_option_validate_addr,
                [GF_OPTION_TYPE_ANY]         = xlator_option_validate_any,
                [GF_OPTION_TYPE_MAX]         = NULL,
        };

        if (opt->type < 0 || opt->type >= GF_OPTION_TYPE_MAX) {
                gf_log (xl->name, GF_LOG_ERROR,
                        "unknown option type '%d'", opt->type);
                goto out;
        }

        validate = validators[opt->type];

        ret = validate (xl, key, value, opt, op_errstr);
out:
        return ret;
}


static volume_option_t *
xlator_volume_option_get_list (volume_opt_list_t *vol_list, const char *key)
{
        volume_option_t         *opt = NULL;
        volume_opt_list_t       *opt_list = NULL;
        volume_option_t         *found = NULL;
        int                      index = 0;
        int                      i = 0;
        char                    *cmp_key = NULL;

        if (!vol_list->given_opt) {
                opt_list = list_entry (vol_list->list.next, volume_opt_list_t,
                                       list);
                opt = opt_list->given_opt;
        } else
                opt = vol_list->given_opt;

        for (index = 0; opt[index].key && opt[index].key[0]; index++) {
                for (i = 0; i < ZR_VOLUME_MAX_NUM_KEY; i++) {
                        cmp_key = opt[index].key[i];
                        if (!cmp_key)
                                break;
                        if (fnmatch (cmp_key, key, FNM_NOESCAPE) == 0) {
                                found = &opt[index];
                                goto out;
                        }
                }
        }
out:
        return found;
}


volume_option_t *
xlator_volume_option_get (xlator_t *xl, const char *key)
{
        volume_opt_list_t       *vol_list = NULL;
        volume_option_t         *found = NULL;

        list_for_each_entry (vol_list, &xl->volume_options, list) {
                found = xlator_volume_option_get_list (vol_list, key);
                if (found)
                        break;
        }

        return found;
}


static void
xl_opt_validate (dict_t *dict, char *key, data_t *value, void *data)
{
        xlator_t          *xl = NULL;
        volume_opt_list_t *vol_opt = NULL;
        volume_option_t   *opt = NULL;
        int                ret = 0;
        char              *errstr = NULL;

        struct {
                xlator_t           *this;
                volume_opt_list_t  *vol_opt;
                char               *errstr;
        } *stub;

        stub = data;
        xl = stub->this;
        vol_opt = stub->vol_opt;

        opt = xlator_volume_option_get_list (vol_opt, key);
        if (!opt)
                return;

        ret = xlator_option_validate (xl, key, value->data, opt, &errstr);
        if (errstr)
                /* possible small leak of previously set stub->errstr */
                stub->errstr = errstr;

        if (fnmatch (opt->key[0], key, FNM_NOESCAPE) != 0) {
                gf_log (xl->name, GF_LOG_WARNING, "option '%s' is deprecated, "
                        "preferred is '%s', continuing with correction",
                        key, opt->key[0]);
                dict_set (dict, opt->key[0], value);
                dict_del (dict, key);
        }
        return;
}


int
xlator_options_validate_list (xlator_t *xl, dict_t *options,
                              volume_opt_list_t *vol_opt, char **op_errstr)
{
        int ret = 0;
        struct {
                xlator_t           *this;
                volume_opt_list_t  *vol_opt;
                char               *errstr;
        } stub;

        stub.this = xl;
        stub.vol_opt = vol_opt;
        stub.errstr = NULL;

        dict_foreach (options, xl_opt_validate, &stub);
        if (stub.errstr) {
                ret = -1;
                if (op_errstr)
                        *op_errstr = stub.errstr;
        }

        return ret;
}


int
xlator_options_validate (xlator_t *xl, dict_t *options, char **op_errstr)
{
        int                ret     = 0;
        volume_opt_list_t *vol_opt = NULL;


        if (!xl) {
                gf_log (THIS->name, GF_LOG_DEBUG, "'this' not a valid ptr");
                ret = -1;
                goto out;
        }

        if (list_empty (&xl->volume_options))
                goto out;

        list_for_each_entry (vol_opt, &xl->volume_options, list) {
                ret = xlator_options_validate_list (xl, options, vol_opt,
                                                    op_errstr);
        }
out:
        return ret;
}


int
xlator_validate_rec (xlator_t *xlator, char **op_errstr)
{
        int            ret  = -1;
        xlator_list_t *trav = NULL;
        xlator_t      *old_THIS = NULL;

        GF_VALIDATE_OR_GOTO ("xlator", xlator, out);

        trav = xlator->children;

        while (trav) {
                if (xlator_validate_rec (trav->xlator, op_errstr)) {
                        gf_log ("xlator", GF_LOG_WARNING, "validate_rec failed");
                        goto out;
                }

                trav = trav->next;
        }

        if (xlator_dynload (xlator))
                gf_log (xlator->name, GF_LOG_DEBUG, "Did not load the symbols");

        old_THIS = THIS;
        THIS = xlator;

        /* Need this here, as this graph has not yet called init() */
        if (!xlator->mem_acct.num_types) {
                if (!xlator->mem_acct_init)
                        xlator->mem_acct_init = default_mem_acct_init;
                xlator->mem_acct_init (xlator);
        }

        ret = xlator_options_validate (xlator, xlator->options, op_errstr);
        THIS = old_THIS;

        if (ret) {
                gf_log (xlator->name, GF_LOG_INFO, "%s", *op_errstr);
                goto out;
        }

        gf_log (xlator->name, GF_LOG_DEBUG, "Validated options");

        ret = 0;
out:
        return ret;
}


int
graph_reconf_validateopt (glusterfs_graph_t *graph, char **op_errstr)
{
        xlator_t *xlator = NULL;
        int       ret = -1;

        GF_ASSERT (graph);

        xlator = graph->first;

        ret = xlator_validate_rec (xlator, op_errstr);

        return ret;
}


static int
xlator_reconfigure_rec (xlator_t *old_xl, xlator_t *new_xl)
{
        xlator_list_t *trav1    = NULL;
        xlator_list_t *trav2    = NULL;
        int32_t        ret      = -1;
        xlator_t      *old_THIS = NULL;

        GF_VALIDATE_OR_GOTO ("xlator", old_xl, out);
        GF_VALIDATE_OR_GOTO ("xlator", new_xl, out);

        trav1 = old_xl->children;
        trav2 = new_xl->children;

        while (trav1 && trav2) {
                ret = xlator_reconfigure_rec (trav1->xlator, trav2->xlator);
                if (ret)
                        goto out;

                gf_log (trav1->xlator->name, GF_LOG_DEBUG, "reconfigured");

                trav1 = trav1->next;
                trav2 = trav2->next;
        }

        if (old_xl->reconfigure) {
                old_THIS = THIS;
                THIS = old_xl;

                ret = old_xl->reconfigure (old_xl, new_xl->options);

                THIS = old_THIS;

                if (ret)
                        goto out;
        } else {
                gf_log (old_xl->name, GF_LOG_DEBUG, "No reconfigure() found");
        }

        ret = 0;
out:
        return ret;
}


int
xlator_tree_reconfigure (xlator_t *old_xl, xlator_t *new_xl)
{
        xlator_t *new_top = NULL;
        xlator_t *old_top = NULL;

        GF_ASSERT (old_xl);
        GF_ASSERT (new_xl);

        old_top = old_xl;
        new_top = new_xl;

        return xlator_reconfigure_rec (old_top, new_top);
}


int
xlator_option_info_list (volume_opt_list_t *list, char *key,
                         char **def_val, char **descr)
{
        int                     ret = -1;
        volume_option_t         *opt = NULL;


        opt = xlator_volume_option_get_list (list, key);
        if (!opt)
                goto out;

        if (def_val)
                *def_val = opt->default_value;
        if (descr)
                *descr = opt->description;

        ret = 0;
out:
        return ret;
}


static int
not_null (char *in, char **out)
{
        if (!in || !out)
                return -1;

        *out = in;
        return 0;
}


static int
xl_by_name (char *in, xlator_t **out)
{
        xlator_t  *xl = NULL;

        xl = xlator_search_by_name (THIS, in);

        if (!xl)
                return -1;
        *out = xl;
        return 0;
}


static int
pc_or_size (char *in, uint64_t *out)
{
        uint32_t  pc = 0;
        int       ret = 0;

        if (gf_string2percent (in, &pc) == 0) {
                if (pc > 100) {
                        ret = gf_string2bytesize (in, out);
                } else {
                        *out = pc;
                }
        } else {
                ret = gf_string2bytesize (in, out);
        }
        return ret;
}


DEFINE_INIT_OPT(char *, str, not_null);
DEFINE_INIT_OPT(uint64_t, uint64, gf_string2uint64);
DEFINE_INIT_OPT(int64_t, int64, gf_string2int64);
DEFINE_INIT_OPT(uint32_t, uint32, gf_string2uint32);
DEFINE_INIT_OPT(int32_t, int32, gf_string2int32);
DEFINE_INIT_OPT(uint64_t, size, gf_string2bytesize);
DEFINE_INIT_OPT(uint32_t, percent, gf_string2percent);
DEFINE_INIT_OPT(uint64_t, percent_or_size, pc_or_size);
DEFINE_INIT_OPT(gf_boolean_t, bool, gf_string2boolean);
DEFINE_INIT_OPT(xlator_t *, xlator, xl_by_name);
DEFINE_INIT_OPT(char *, path, not_null);



DEFINE_RECONF_OPT(char *, str, not_null);
DEFINE_RECONF_OPT(uint64_t, uint64, gf_string2uint64);
DEFINE_RECONF_OPT(int64_t, int64, gf_string2int64);
DEFINE_RECONF_OPT(uint32_t, uint32, gf_string2uint32);
DEFINE_RECONF_OPT(int32_t, int32, gf_string2int32);
DEFINE_RECONF_OPT(uint64_t, size, gf_string2bytesize);
DEFINE_RECONF_OPT(uint32_t, percent, gf_string2percent);
DEFINE_RECONF_OPT(uint64_t, percent_or_size, pc_or_size);
DEFINE_RECONF_OPT(gf_boolean_t, bool, gf_string2boolean);
DEFINE_RECONF_OPT(xlator_t *, xlator, xl_by_name);
DEFINE_RECONF_OPT(char *, path, not_null);
