#if !defined(TIMEFLOW_PARSER_TYPES_H)
#define TIMEFLOW_PARSER_TYPES_H

#include <tf_types.h>
#include <stdlib.h>
#include <stdbool.h>

// tfp = timeflow parser

enum tfp_token_types
{
  /* Basic */
  
  tfp_tok_dummy = -2,
  tfp_tok_eof = -1,
  tfp_tok_invalid = 0,
  
  tfp_tok_unsigned,
  tfp_tok_integer,
  
  tfp_tok_string,

  tfp_tok_minus,
  tfp_tok_plus,

  /* Time related */
  
  // keywords
  tfp_tok_now,
  tfp_tok_allday,

  // symbols
  tfp_tok_colon,
  tfp_tok_timedur_h,
  tfp_tok_timedur_m,
  tfp_tok_ellipses,

  /* Date related */

  // keywords
  tfp_tok_today,
  tfp_tok_tomorrow,
  tfp_tok_yesterday,
  tfp_tok_literal_day,
  tfp_tok_literal_week,
  tfp_tok_literal_month,
  tfp_tok_literal_year,

  // symbols
  tfp_tok_fw_slash,
  tfp_tok_dayname,
  tfp_tok_datedur_y,
  tfp_tok_datedur_m,
  tfp_tok_datedur_d,
  tfp_tok_datedur_w,

  /* Extra (Tasks related) */
  tfp_tok_view,
  tfp_tok_taskview_quick,
  tfp_tok_taskview
};

union tfp_tok_data {
  bool _null;
  long _int;
  unsigned long _unsigned;
  char _char;
  const char* _string;
  struct tf_atm _dur;
  struct tf_atm _atm;
};

struct tfp_token {
  enum tfp_token_types type;
  union tfp_tok_data data;
};

/* Basic lexers */
struct tfp_token tfp_lex_keyword(const char** ptr);
struct tfp_token tfp_lex_number(const char** ptr);
struct tfp_token tfp_lex_symbol(const char** ptr);

void tfp_skip_whitespace(const char** ptr);
struct tfp_token tfp_next_token(const char** ptr);

/* Expect / match token type */
bool tfp_expect(const char** ptr, enum tfp_token_types type, struct tfp_token* dest, bool *errptr);
bool tfp_match(const char** ptr, enum tfp_token_types type, struct tfp_token* dest);

/* Parsers */
struct tf_atm tfp_parse_time_atm(const char** ptr, bool* err_ptr);
struct tf_atm tfp_parse_time_atm_basic(const char** ptr, bool* err_ptr);
struct tf_atm tfp_parse_time_dur(const char** ptr, bool* err_ptr);
tf_time tfp_parse_time(const char** ptr, bool* err_ptr);

#endif // TIMEFLOW_PARSER_TYPES_H
