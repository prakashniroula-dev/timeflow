/**
 * lexer.c
 * lexers for use in timeflow
 *
 * Author : github.com/prakashniroula-dev
 */

#include <tf_types.h>
#include <parser.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


#define tfpm_match_str(ptr, len, literal) (len == sizeof(literal) - 1 && strncmp(literal, ptr, sizeof(literal) - 1) == 0)
#define tfpm_tok_type(tok) ((struct tfp_token){.type = tok, .data._null = true})

void tfp_skip_whitespace(const char **ptr)
{
  while (**ptr && isspace((unsigned char)**ptr)) {
    (*ptr)++;
  }
}

struct tfp_token tfp_next_token(const char **ptr)
{
  tfp_skip_whitespace(ptr);

  unsigned char c = **ptr;

  if (c == '\0')
    return tfpm_tok_type(tfp_tok_eof);

  if (isdigit(c) || (c == '-' && isdigit((unsigned char)*(*ptr+1))))
    return tfp_lex_number(ptr);
  
  if (isalpha(c))
    return tfp_lex_keyword(ptr);

  return tfp_lex_symbol(ptr);
}


struct tfp_token tfp_lex_number(const char **ptr)
{
  char* rp = NULL;
  struct tfp_token tok = {tfp_tok_unsigned, .data._null = false};
  
  errno = 0;
  if (**ptr == '-') {
    tok.type = tfp_tok_integer;
    tok.data._int = strtol(*ptr, &rp, 10);
  } else {
    tok.data._unsigned = strtoul(*ptr, &rp, 10);
  }

  if (errno == ERANGE)
   return (struct tfp_token) {tfp_tok_invalid, .data._string = "Number overflow"};

  if (*ptr == rp)
    return (struct tfp_token) {tfp_tok_invalid, .data._string = "Digit not present when expected.."};

  *ptr = rp;
  return tok;
}


struct tfp_token tfp_lex_symbol(const char **ptr) 
{
  const char *start = *ptr;
  
  if (tfpm_match_str(start, 3, "...")) {
    (*ptr) = start + 3;
    return tfpm_tok_type(tfp_tok_ellipses);
  }
  
  (*ptr)++;
  switch (*start)
  {
    case '+': return tfpm_tok_type(tfp_tok_plus);
    case '-': return tfpm_tok_type(tfp_tok_minus);
    case '/': return tfpm_tok_type(tfp_tok_fw_slash);
    case ':': return tfpm_tok_type(tfp_tok_colon);
    default: break;
  }

  return (struct tfp_token){tfp_tok_invalid, .data._string = "Invalid symbol"};
}


struct tfp_token tfp_lex_keyword(const char **ptr)
{

  const char *start = *ptr;

  while (isalpha((unsigned char)**ptr))
    (*ptr)++;

  size_t len = *ptr - start;

  // Time keywords
  if (tfpm_match_str(start, len, "now"))
    return tfpm_tok_type(tfp_tok_now);

  if (tfpm_match_str(start, len, "allday"))
    return tfpm_tok_type(tfp_tok_allday);

  if (tfpm_match_str(start, len, "h"))
    return tfpm_tok_type(tfp_tok_timedur_h);

  if (tfpm_match_str(start, len, "m"))
    return tfpm_tok_type(tfp_tok_timedur_m);

  // Date keywords
  if (tfpm_match_str(start, len, "today"))
    return tfpm_tok_type(tfp_tok_today);

  if (tfpm_match_str(start, len, "tomorrow"))
    return tfpm_tok_type(tfp_tok_tomorrow);

  if (tfpm_match_str(start, len, "yesterday"))
    return tfpm_tok_type(tfp_tok_yesterday);

  if (tfpm_match_str(start, len, "day"))
    return tfpm_tok_type(tfp_tok_literal_day);

  if (tfpm_match_str(start, len, "month"))
    return tfpm_tok_type(tfp_tok_literal_month);

  if (tfpm_match_str(start, len, "year"))
    return tfpm_tok_type(tfp_tok_literal_year);

  if (tfpm_match_str(start, len, "d"))
    return tfpm_tok_type(tfp_tok_datedur_d);

  if (tfpm_match_str(start, len, "mo"))
    return tfpm_tok_type(tfp_tok_datedur_m);

  if (tfpm_match_str(start, len, "y"))
    return tfpm_tok_type(tfp_tok_datedur_y);

  if (tfpm_match_str(start, len, "w"))
    return tfpm_tok_type(tfp_tok_datedur_w);

  // Extra( timeflow specific keywords )
  if (tfpm_match_str(start, len, "list"))
    return (struct tfp_token){tfp_tok_view, .data._string = "list"};

  if (tfpm_match_str(start, len, "grid"))
    return (struct tfp_token){tfp_tok_view, .data._string = "grid"};

  if (tfpm_match_str(start, len, "week"))
    return (struct tfp_token){tfp_tok_view, .data._string = "week"};

  if (tfpm_match_str(start, len, "current"))
    return (struct tfp_token){tfp_tok_taskview_quick, .data._string = "current"};

  if (tfpm_match_str(start, len, "next"))
    return (struct tfp_token){tfp_tok_taskview_quick, .data._string = "next"};

  if (tfpm_match_str(start, len, "prev"))
    return (struct tfp_token){tfp_tok_taskview_quick, .data._string = "prev"};

  if (tfpm_match_str(start, len, "task"))
    return tfpm_tok_type(tfp_tok_taskview);

  return (struct tfp_token){.type = tfp_tok_invalid, .data._string = "Invalid keyword"};
}