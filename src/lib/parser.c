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

static void tfp_expect_raise_err(bool *errptr, const char* ptr, enum tfp_token_types tok, enum tfp_token_types expected_tok, size_t l, const char* extra) {
  if (errptr) {
    *errptr = true;
    return;
  }
  const char* p = extra == NULL ? "": extra;
  const char *tr = strlen(ptr) > 10 ? "..." : "";

  if ( tok == expected_tok || expected_tok == tfp_tok_dummy || tok == tfp_tok_invalid ) {
    tf_print_err("Parse error: %s\n\tat `%.10s%s`\n\t    ^", p, ptr, tr);
  }
  else {
    tf_print_err(
      "Parse error: Expected %s, got %s\n\tat `%.10s%s`\n\t    ^",
      tfp_token_type_name(expected_tok), tfp_token_type_name(tok), ptr, tr
    );
  }
    
  while (l && --l)
    putchar('~');

  putchar('\n');
}

static uint8_t tfp_parse_time__hr(unsigned long hr)
{
  if (hr > 23)
  {
    tf_print_warn("hr > 23 is not allowed, falling back to 0\n");
    hr = 0;
  }
  return (uint8_t)hr;
}

static uint8_t tfp_parse_time__min(unsigned long min)
{
  if (min > 59)
  {
    tf_print_warn("min > 59 is not allowed, falling back to 0\n");
    min = 0;
  }
  return (uint8_t)min;
}

struct tf_atm tfp_parse_time_atm(const char** ptr, bool *errptr)
{
  struct tf_atm t = {0};
  struct tfp_token tok;
  const char* p = *ptr;

  // start with a number is always atm-time
  if (tfp_match(&p, tfp_tok_unsigned, &tok)) {
    t = tfp_parse_time_atm_basic(ptr, errptr);
    return t;
  }
  // special keyword : now, hardcode for now!
  else if ( tfp_match(ptr, tfp_tok_now, &tok)) {
    t.clock.h = 21;
    t.clock.m = 5;
    return t;
  }
  // to-do
  // else if ( tfp_match(&p, tfp_tok_ellipses, &tok)) {
  // *ptr = p;
  // }
  
  if (errptr) {
    *errptr = true;
    return t;
  }

  return t;
}

tf_time tfp_parse_time(const char **ptr, bool *errptr)
{
  struct tfp_token tok;
  struct tf_atm t_atm = {0};
  struct tf_atm t_dur = {0};
  const char* p = *ptr;
  tf_time t = {0};

  bool err = false;
  // start time
  t_atm = tfp_parse_time_atm(&p, &err);
  if (err) {
    tfp_expect_raise_err(
      errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy, strlen(*ptr),
      "Invalid format"
    );
    return t;
  }
  *ptr = p;
  
  t.start = t_atm;
  
  // reset error just in case
  err = false;
  
  // looking for operator
  // match `-`
  if ( tfp_match(ptr, tfp_tok_minus, &tok) ) {
    p = *ptr;
    t_atm = tfp_parse_time_atm(&p, &err);
    // if matched normal atm_time with no error,
    // then expect eof
    if (!err) {
      tfp_expect(&p, tfp_tok_eof, NULL, errptr);
      if (errptr && *errptr) return t;
      t.end = t_atm;
      *ptr = p;
      return t;
    };
    p = *ptr;
    err = false;
    
    // if err, try to match duration
    t_dur = tfp_parse_time_dur(&p, &err);
    if (err) {
      tfp_expect_raise_err(
        errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy, strlen(*ptr),
        "Expected either time or duration after `-`"
      );
      return t;
    } else {
      tfp_expect(&p, tfp_tok_eof, NULL, errptr);
      if (errptr && *errptr) return t;
    }
    uint8_t m = t.start.clock.m;
    uint8_t h = t.start.clock.h;
    if ( t_dur.clock.h > h || (t_dur.clock.m > m && h == 0) ) {
      tfp_expect_raise_err(
        errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy,
        strlen(*ptr), "Invalid operation, out of bounds"
      );
      return t;
    }
    *ptr = p;
    if ( t_dur.clock.m > m ) {
      m += 60;
      h -= 1;
    }
    m -= t_dur.clock.m;
    h -= t_dur.clock.h;
    t.end.clock.h = h;
    t.end.clock.m = m;
    return t;
  }
  // match `+`
  else if ( tfp_match(ptr, tfp_tok_plus, &tok) ) {
    p = *ptr;
    t_dur = tfp_parse_time_dur(&p, errptr);
    if (errptr && *errptr) return t;
    uint8_t h = t.start.clock.h;
    uint8_t m = t.start.clock.m;
    m += t_dur.clock.m;
    h += m / 60;
    m %= 60;
    h += t_dur.clock.h;
    if ( h > 23 ) {
      tfp_expect_raise_err(
        errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy,
        strlen(*ptr), "Invalid operation, out of bounds"
      );
      return t;
    } else {
      tfp_expect(&p, tfp_tok_eof, NULL, errptr);
      if (errptr && *errptr) return t;
    }
    *ptr = p;
    t.end.clock.h = h;
    t.end.clock.m = m;
    return t;
  }
  // to-do
  // else if ( tfp_match(ptr, tfp_tok_ellipses) ) {
  // }
  tfp_expect_raise_err(
    errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy,
    strlen(*ptr), "Invalid format"
  );
  return t;
}

struct tf_atm tfp_parse_time_dur(const char **ptr, bool *errptr)
{
  struct tfp_token tok;
  struct tf_atm t_atm = {0};

  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr))
    return t_atm;

  if (tfp_match(ptr, tfp_tok_timedur_h, NULL))
  {
    t_atm.clock.h = tfp_parse_time__hr(tok.data._unsigned);
    if (!tfp_match(ptr, tfp_tok_unsigned, &tok))
      return t_atm;
  }

  if (!tfp_expect(ptr, tfp_tok_timedur_m, NULL, errptr))
    return t_atm;

  t_atm.clock.m = tfp_parse_time__min(tok.data._unsigned);
  return t_atm;
}

struct tf_atm tfp_parse_time_atm_basic(const char **ptr, bool *errptr)
{
  struct tfp_token tok;
  struct tf_atm t_atm = {0};

  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr))
    return t_atm;

  t_atm.clock.h = tfp_parse_time__hr(tok.data._unsigned);

  // expect a colon in between
  if (!tfp_expect(ptr, tfp_tok_colon, &tok, errptr))
    return t_atm;

  tfp_skip_whitespace(ptr);
  bool single_digit = **ptr == '0';

  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr))
    return t_atm;

  t_atm.clock.m = tfp_parse_time__min(tok.data._unsigned);

  if (!single_digit && t_atm.clock.m < 10)
  {
    if (errptr)
    {
      *errptr = true;
    }
    else
    {
      tf_print_warn("minutes should be 2 digits");
    }
  }

  return t_atm;
}


bool tfp_match(const char **ptr, enum tfp_token_types type, struct tfp_token *dest)
{
  const char *start = *ptr;
  struct tfp_token tok = tfp_next_token(ptr);
  if (tok.type != type)
  {
    *ptr = start;
    return false;
  }
  if (dest)
  {
    *dest = tok;
  }
  return true;
}

bool tfp_expect(const char **ptr, enum tfp_token_types type, struct tfp_token *dest, bool *errptr)
{
  const char *start = *ptr;

  struct tfp_token tok = tfp_next_token(ptr);

  if (tok.type == type)
  {
    if (dest)
    {
      *dest = tok;
    }
    return true;
  }

  const char *end = *ptr;

  *ptr = start;
  tfp_expect_raise_err(errptr, start, tok.type, type, end - start, tok.data._string);
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