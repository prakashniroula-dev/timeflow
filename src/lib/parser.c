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
#include <time.h>

static const char *tfp_token_type_name(enum tfp_token_types type);

static void tfp_expect_raise_err(const char **errptr, const char* ptr, enum tfp_token_types tok, enum tfp_token_types expected_tok, size_t l, const char* extra, const char* extra2) {
  const char* p = extra == NULL ? "": extra;
  const char* p2 = extra2 == NULL ? "": extra2;
  const char* psep = extra2 == NULL ? "": ": ";
  const char *tr = strlen(ptr) > 10 ? "..." : "";

  if ( tok == expected_tok || expected_tok == tfp_tok_dummy || tok == tfp_tok_invalid ) {
    if (errptr) {
      *errptr = p;
      return;
    }
    tf_print_err("Parse error: %s%s%s\n\tat `%.10s%s`\n\t    ^", p, psep, p2, ptr, tr);
  }
  else {
    if (errptr) {
      *errptr = "Token mismatch";
      return;
    }
    tf_print_err("Parse error: Expected %s, got %s\n\tat `%.10s%s`\n\t", tfp_token_type_name(expected_tok), tfp_token_type_name(tok), ptr, tr);
  }
   
  while (l && --l)
    putchar('~');

  putchar('\n');
}

static uint8_t tfp_parse_time__hr(unsigned long hr, const char** errptr)
{
  if (hr < 24) return (uint8_t) hr;
  const char* err = "hr > 23 is not allowed\n";
  if ( errptr ) {
    *errptr = err;
  } else {
    tf_print_warn("%s", err);
  }
  return 0;
}

static uint8_t tfp_parse_time__min(unsigned long min, const char** errptr)
{
  if (min < 60) return (uint8_t) min;
  const char* err = "min > 59 is not allowed\n";
  if ( errptr ) {
    *errptr = err;
  } else {
    tf_print_warn("%s", err);
  }
  return 0;
}

static struct tf_atm tfp_invalid_atm() {
  struct tf_atm t = {0};
  t.type = tf_time_invalid;
  return t;
}

static tf_time tfp_invalid_time() {
  tf_time t = {.start = tfp_invalid_atm(), .end = tfp_invalid_atm()};
  return t;
}

struct tf_atm tfp_parse_time_atm(const char** ptr, const char **errptr)
{
  struct tf_atm t = {0};
  struct tfp_token tok;
  const char* p = *ptr;

  if ( errptr ) {
    *errptr = NULL;
  }

  // start with a number is always atm-time (basic absolute time)
  if (tfp_match(ptr, tfp_tok_unsigned, &tok)) {
    *ptr = p;
    t = tfp_parse_time_atm_basic(ptr, errptr);
    return t;
  }
  // special keyword : now, hardcode for now!
  else if ( tfp_match(ptr, tfp_tok_now, &tok)) {
    time_t t_raw;
    struct tm *t_loc;
    time(&t_raw);
    t_loc = localtime(&t_raw);
    // Extract hour and minute as integers
    t.clock.h = t_loc->tm_hour;
    t.clock.m = t_loc->tm_min;
    return t;
  }

  return tfp_invalid_atm();
}

tf_time tfp_parse_time(const char **ptr, const char **errptr)
{
  struct tfp_token tok = {0};
  struct tf_atm t_atm = {0};
  struct tf_atm t_dur = {0};
  const char* start = *ptr;
  const char* err = NULL;
  tf_time t = {0};

  // check for allday ( complete time )
  if (tfp_match(ptr, tfp_tok_allday, NULL)) {
    if (!tfp_expect(ptr, tfp_tok_eof, NULL, errptr)) return t;
    t.end.clock.h = 23;
    t.end.clock.m = 59;
    return t;
  }

  // start time
  t_atm = tfp_parse_time_atm(ptr, &err);
  if (t_atm.type == tf_time_invalid && !(tfp_match(ptr, tfp_tok_ellipses, &tok))) {
    *ptr = start;
    tfp_expect_raise_err(
      errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy, strlen(*ptr),
      "Invalid format", err
    );
    return tfp_invalid_time();
  }
  t.start = t_atm;
  
  // if eof, then it's a point time, so set end to null
  if (t_atm.type != tf_time_invalid && tfp_match(ptr, tfp_tok_eof, NULL)) {
    t.end.type = tf_time_null;
    return t;
  }
  

  
  // first ellipses = dynamic prev time
  // if first is ellipses, next time can be directly supplied
  // so check for that too
  if ( tok.type == tfp_tok_ellipses ) {
    t.start.type = tf_dtime_prev; // dynamic time
    start = *ptr;
    t_atm = tfp_parse_time_atm(ptr, &err);
    if ( t_atm.type != tf_time_invalid ) {
      t.end = t_atm;
      return t;
    }
    *ptr = start;
  }
  
  // looking for operator
  // match `-`
  start = *ptr;
  if ( tfp_match(ptr, tfp_tok_minus, &tok) ) {
    start = *ptr;

    // try matching ellipses to allow dynamic time (alt-format)
    if ( tfp_match(ptr, tfp_tok_ellipses, NULL) ) {
      if (!tfp_expect(ptr, tfp_tok_eof, NULL, errptr)) return tfp_invalid_time();
      t.end.type = tf_dtime_next;
      return t;
    }

    t_atm = tfp_parse_time_atm(ptr, &err);
    // if matched normal atm_time with no error, then expect eof
    if (t_atm.type != tf_time_invalid) {
      if(!tfp_expect(ptr, tfp_tok_eof, NULL, errptr)) return tfp_invalid_time();
      t.end = t_atm;
      return t;
    };
    *ptr = start;
    err = NULL;
    
    // if err, try to match duration
    t_dur = tfp_parse_time_dur(ptr, &err);
    if (t_dur.type == tf_time_invalid) {
      *ptr = start;
      tfp_expect_raise_err(
        errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy, strlen(*ptr),
        "Expected either time or duration after `-`", err
      );
      return tfp_invalid_time();
    } else {
      if(!tfp_expect(ptr, tfp_tok_eof, NULL, errptr)) return tfp_invalid_time();
    }
    // Account for dynamic times ( case: ... - <duration> )
    if ( t.start.type == tf_dtime_prev ) {
      t.end.type = tf_dtime_minus_dur;
      t.end.clock = t_dur.clock;
      return t;
    }
    uint8_t m = t.start.clock.m;
    uint8_t h = t.start.clock.h;
    t.end.clock.h = h;
    t.end.clock.m = m;
    if ( t_dur.clock.h > h || (t_dur.clock.m > m && h <= t_dur.clock.h) ) {
      *ptr = start;
      tfp_expect_raise_err(
        errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy,
        strlen(*ptr), "Invalid operation, out of bounds", NULL
      );
      return tfp_invalid_time();
    }
    if ( t_dur.clock.m > m ) {
      m += 60;
      h -= 1;
    }
    m -= t_dur.clock.m;
    h -= t_dur.clock.h;
    t.start.clock.h = h;
    t.start.clock.m = m;
    return t;
  }
  // match `+`
  else if ( tfp_match(ptr, tfp_tok_plus, &tok) ) {
    start = *ptr;
    t_dur = tfp_parse_time_dur(ptr, errptr);
    if (errptr && *errptr) {
      *ptr = start;
      return tfp_invalid_time();
    };
    // Account for dynamic times ( case: ... + <duration> )
    if ( t.start.type == tf_dtime_prev ) {
      t.end.type = tf_dtime_plus_dur;
      t.end.clock = t_dur.clock;
      return t;
    }
    uint8_t h = t.start.clock.h;
    uint8_t m = t.start.clock.m;
    m += t_dur.clock.m;
    h += m / 60;
    m %= 60;
    h += t_dur.clock.h;
    if ( h > 23 ) {
      *ptr = start;
      tfp_expect_raise_err(
        errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy,
        strlen(*ptr), "Invalid operation, out of bounds", NULL
      );
      return tfp_invalid_time();
    } else {
      tfp_expect(ptr, tfp_tok_eof, NULL, errptr);
      if (errptr && *errptr) {
        *ptr = start;
        return tfp_invalid_time();
      };
    }
    t.end.clock.h = h;
    t.end.clock.m = m;
    return t;
  }
  // check for ellipses ( end without middle operator )
  // indicates to next block's start
  else if ( tfp_match(ptr, tfp_tok_ellipses, NULL)) {
    start = *ptr;
    if (!tfp_expect(ptr, tfp_tok_eof, NULL, errptr)) return t;
    t.end.type = tf_dtime_next;
    return t;
  }
  *ptr = start;
  tfp_expect_raise_err(
    errptr, *ptr, tfp_tok_dummy, tfp_tok_dummy,
    strlen(*ptr), "Invalid format", *errptr
  );
  return t;
}

struct tf_atm tfp_parse_time_dur(const char **ptr, const char **errptr)
{
  struct tfp_token tok;
  struct tf_atm t_atm = {0};
  if ( errptr ) {
    *errptr = NULL;
  }
  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr))
    return tfp_invalid_atm();

  if (tfp_match(ptr, tfp_tok_timedur_h, NULL))
  {
    t_atm.clock.h = tfp_parse_time__hr(tok.data._unsigned, errptr);
    if (!tfp_match(ptr, tfp_tok_unsigned, &tok))
      return t_atm;
  }

  if (!tfp_expect(ptr, tfp_tok_timedur_m, NULL, errptr))
    return tfp_invalid_atm();

  t_atm.clock.m = tfp_parse_time__min(tok.data._unsigned, errptr);
  return t_atm;
}

struct tf_atm tfp_parse_time_atm_basic(const char **ptr, const char **errptr)
{
  if ( errptr ) {
    *errptr = NULL;
  }
  struct tfp_token tok;
  struct tf_atm t_atm = {0};
  const char* err = NULL;

  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr))
    return tfp_invalid_atm();

  t_atm.clock.h = tfp_parse_time__hr(tok.data._unsigned, &err);
  if (err) {
    if (errptr) *errptr = err;
    return tfp_invalid_atm();
  }

  // expect a colon in between
  if (!tfp_expect(ptr, tfp_tok_colon, &tok, errptr))
    return tfp_invalid_atm();

  tfp_skip_whitespace(ptr);
  // if there is a leading 0, then it's a single digit
  bool single_digit = **ptr == '0';

  if (!tfp_expect(ptr, tfp_tok_unsigned, &tok, errptr))
    return tfp_invalid_atm();

  t_atm.clock.m = tfp_parse_time__min(tok.data._unsigned, &err);
  if (err) {
    if (errptr) *errptr = err;
    return tfp_invalid_atm();
  }

  if (!single_digit && t_atm.clock.m < 10)
  {
    const char* err = "Minutes should be 2 digits";
    if (errptr)
    {
      *errptr = err;
      return tfp_invalid_atm();
    }
    tf_print_warn("%s", err);
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

bool tfp_expect(const char **ptr, enum tfp_token_types type, struct tfp_token *dest, const char **errptr)
{
  if ( errptr ) {
    *errptr = NULL;
  }
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
  tfp_expect_raise_err(errptr, start, tok.type, type, end - start, tok.data._string, NULL);
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