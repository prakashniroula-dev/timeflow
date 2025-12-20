/**
 * parser.c
 * Parsers for use in timeflow
 *
 * Author : github.com/prakashniroula-dev
 */

#include <parser.h>
#include <tf_types.h>
#include <string.h>
#include <stdlib.h>
#include <tf_io.h>

static const char *tfp_token_type_name(enum tfp_token_types type);

static uint8_t tfp_parse_time__hr(unsigned long hr) {
  if ( hr > 23 ) {
    tf_print_warn("hr > 23 is not allowed, falling back to 0\n");
    hr = 0;
  }
  return (uint8_t)hr;
}

static uint8_t tfp_parse_time__min(unsigned long min) {
  if ( min > 59 ) {
    tf_print_warn("min > 59 is not allowed, falling back to 0\n");
    min = 0;
  }
  return (uint8_t)min;
}

struct tf_atm tfp_parse_time_dur(const char **ptr, bool *errptr)
{
  struct tfp_token tok;
  struct tf_atm t_atm = {0};

  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr)) return t_atm;

  if (tfp_match(ptr, tfp_tok_timedur_h, NULL)) {
    t_atm.clock.h = tfp_parse_time__hr(tok.data._unsigned);
    if (!tfp_match(ptr, tfp_tok_unsigned, &tok)) return t_atm;
  }

  if (!tfp_expect(ptr, tfp_tok_timedur_m, &tok, errptr)) return t_atm;

  t_atm.clock.m = tfp_parse_time__min(tok.data._unsigned);
  return t_atm;
}

struct tf_atm tfp_parse_time_atm(const char **ptr, bool *errptr)
{
  struct tfp_token tok;
  struct tf_atm t_atm = {0};

  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr)) return t_atm;

  t_atm.clock.h = tfp_parse_time__hr(tok.data._unsigned);
  
  // expect a colon in between
  if (!tfp_expect(ptr, tfp_tok_colon, &tok, errptr)) return t_atm;
  
  tfp_skip_whitespace(ptr);
  bool single_digit = **ptr == '0';
  
  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr)) return t_atm;

  t_atm.clock.m = tfp_parse_time__min(tok.data._unsigned);

  if (!single_digit && t_atm.clock.m < 10) {
    if (errptr) {
      *errptr = true;
    } else {
      tf_print_warn("minutes should be 2 digits");
    }
  }

  return t_atm;
}

bool tfp_match(const char **ptr, enum tfp_token_types type, struct tfp_token *dest)
{
  const char* start = *ptr;
  struct tfp_token tok = tfp_next_token(ptr);
  if (tok.type != type) {
    *ptr = start;
    return false;
  }
  if ( dest ) {
    *dest = tok;
  }
  return true;
}

bool tfp_expect(const char **ptr, enum tfp_token_types type, struct tfp_token *dest, bool* errptr)
{
  const char *start = *ptr;

  struct tfp_token tok = tfp_next_token(ptr);

  if (tok.type == type)
  {
    if ( dest ) {
      *dest = tok;
    }
    return true;
  }

  const char *end = *ptr;

  *ptr = start;
  if (errptr) {
    *errptr = false;
    return false;
  }

  const char *tr = strlen(start) > 10 ? "..." : "";

  if (tok.type == tfp_tok_invalid)
  {
    tf_print_err("Parse error - %s\n\tat `%.10s%s`\n\t    ^", tok.data._string, start, tr);

    // print '~' upto where it's parsed
    {
      size_t l = (end - start);
      l = l > 10 ? 10 : l;
      while (--l)
        putchar('~');
    }

    putchar('\n');
    return false;
  }

  const char *expected = tfp_token_type_name(type);
  const char *got = tfp_token_type_name(tok.type);
  tf_print_err(
      "Token mismatch - Expected %s, got %s\n\tat `%.10s%s`\n\t    ^",
      expected, got, start, tr);

  // print '~' upto where it's parsed
  {
    size_t l = (end - start);
    l = l > 10 ? 10 : l;
    while (--l)
      putchar('~');
  }

  putchar('\n');
  return false;
}

static const char *tfp_token_type_name(enum tfp_token_types type)
{
  switch (type)
  {
  case tfp_tok_invalid:
    return "invalid token";
  case tfp_tok_eof:
    return "end of file";
  case tfp_tok_unsigned:
    return "positive integer";
  case tfp_tok_integer:
    return "integer";
  case tfp_tok_string:
    return "string";
  case tfp_tok_minus:
    return "-";
  case tfp_tok_plus:
    return "+";
  case tfp_tok_now:
    return "\"now\"";
  case tfp_tok_allday:
    return "\"all day\"";
  case tfp_tok_colon:
    return ":";
  case tfp_tok_timedur_h:
    return "\'h\'";
  case tfp_tok_timedur_m:
    return "\'m\'";
  case tfp_tok_ellipses:
    return "(...)";
  case tfp_tok_today:
    return "\"today\"";
  case tfp_tok_tomorrow:
    return "\"tomorrow\"";
  case tfp_tok_yesterday:
    return "\"yesterday\"";
  case tfp_tok_literal_day:
    return "\"day\"";
  case tfp_tok_literal_week:
    return "\"week\"";
  case tfp_tok_literal_month:
    return "\"month\"";
  case tfp_tok_literal_year:
    return "\"year\"";
  case tfp_tok_fw_slash:
    return "/";
  case tfp_tok_dayname:
    return "day (sun/mon/../sat)";
  case tfp_tok_datedur_d:
    return "\'d\'";
  case tfp_tok_datedur_m:
    return "\'mo\'";
  case tfp_tok_datedur_y:
    return "\'y\'";
  case tfp_tok_datedur_w:
    return "\'w\'";
  case tfp_tok_taskview_quick:
    return "\"current\" | \"prev\" | \"next\"";
  case tfp_tok_taskview:
    return "\"task <date>\"";
  default:
    break;
  }
  return "\?\?<unknown token>\?\?";
}