/*
 * simple_regex.c
 *
 * Simle regex library.
 *
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 *
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Header$
 */

/*
 * Some notes:
 *
 * - This is a very simple regex library (read: return a match if some string
 *   matches some regex).  It is probably not POSIX (if there are a POSIX or
 *   other standard) compatible.
 *
 * - I primarily wrote it to _not_ use glibc type regex functions, in case we
 *   might want to use it in code that have to be linked agaist klibc, etc.
 *
 * - It really is not optimized in any way yet.
 *
 * - Supported operators are:
 *
 *     '.', '?', '*', '+'  - So called 'wildcards'
 *     '[a-z]', '[^a-z]'   - Basic 'lists'.  Note that 'a-z' just specify that
 *                           it supports basic lists as well as sequences ..
 *                           The '^' is for an inverted list of course.
 *     '^', '$'            - The 'from start' and 'to end' operators.  If these
 *                           are not used at the start ('^') or end ('$') of the
 *                           regex, they will be treated as normal characters
 *                           (this of course exclude the use of '^' in a 'list').
 *
 * - If an invalid argument was passed, the functions returns 0 with
 *   'regex_data->match == 0' (no error with no match) rather than -1.  It may
 *   not be consistant with other practices, but I personally do not feel it is
 *   a critical error for these types of functions, and there are debugging you
 *   can enable to verify that there are no such issues.
 *   
 * - __somefunction() is usually a helper function for somefunction().  I guess
 *   recursion might be an alternative, but I try to avoid it.
 *
 * - In general if we are matching a 'wildcard' ('*', '+' or '?'), a 'word'
 *   (read: some part of the regex that do not contain a 'wildcard' or 'list')
 *   will have a greater 'weight' than the 'wildcard'.  This means that we
 *   will only continue to evaluate the 'wildcard' until the following 'word'
 *   (if any) matches.  Currently this do not hold true for a 'list' not
 *   followed by a 'wildcard' - I might fix this in future.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcscripts/rcutil.h"

/* Macro to check if a regex_data_t pointer is valid */
#define CHECK_REGEX_DATA_P(_regex_data, _on_error) \
 do { \
   if ((NULL == _regex_data) \
       || (NULL == _regex_data->data) \
       /* We do not check for this, as it might still \
	* provide a match ('*' or '?' wildcard) */ \
       /* || (0 == strlen(_regex_data->data)) */ \
       || (NULL == _regex_data->regex) \
       || (0 == strlen(_regex_data->regex))) \
     { \
       rc_errno_set (EINVAL); \
       DBG_MSG ("Invalid argument passed!\n"); \
       goto _on_error; \
     } \
 } while (0)

static size_t get_word (const char *regex, char **r_word);
static int match_word (regex_data_t * regex_data);
static size_t get_list_size (const char *regex);
static size_t get_list (const char *regex, char **r_list);
static int __match_list (regex_data_t * regex_data);
static int match_list (regex_data_t * regex_data);
static size_t get_wildcard (const char *regex, char *r_wildcard);
static int __match_wildcard (regex_data_t * regex_data,
			     int (*match_func) (regex_data_t * regex_data),
			     const char *regex);
static int match_wildcard (regex_data_t * regex_data);
static int __match (regex_data_t * regex_data);

/*
 * Return values for match_* functions
 *
 *  0		- There was no error.  If there was a match, regex_data->match
 *  		- will be > 0 (this is the definitive check - if not true, the
 *  		- other values of the struct may be bogus), regex_data->count
 *  		- will be the amount of data that was matched (might be 0 for
 *  		- some wildcards), and regex_data->r_count will be > 0.
 *
 *  -1		- An error occured.  Check errno for more info.
 *
 */

size_t
get_word (const char *regex, char **r_word)
{
  char *r_list;
  char *str_ptr;
  size_t count = 0;
  size_t tmp_count;

  if (!check_arg_str (regex))
    return 0;

  *r_word = xmalloc (strlen (regex) + 1);
  if (NULL == r_word)
    return 0;

  str_ptr = *r_word;

  while (strlen (regex) > 0)
    {
      switch (regex[0])
	{
	case '*':
	case '+':
	case '?':
	  /* If its a wildcard, backup one step */
	  *(--str_ptr) = '\0';
	  count--;
	  return count;
	case '[':
	  tmp_count = get_list (regex, &r_list);
	  free (r_list);
	  /* In theory should not happen, but you never know
	   * what may happen in future ... */
	  if (-1 == tmp_count)
	    goto error;

	  /* Bail if we have a list */
	  if (tmp_count > 0)
	    {
	      str_ptr[0] = '\0';
	      return count;
	    }
	default:
	  *str_ptr++ = *regex++;
	  count++;
	  break;
	}
    }

  str_ptr[0] = '\0';

  return count;

error:
  free (*r_word);

  return -1;
}

int
match_word (regex_data_t * regex_data)
{
  char *data_p = regex_data->data;
  char *r_word = NULL, *r_word_p;
  size_t count = 0;

  CHECK_REGEX_DATA_P (regex_data, exit);

  count = get_word (regex_data->regex, &r_word);
  if (-1 == count)
    goto error;
  if (0 == count)
    goto exit;
  r_word_p = r_word;

  while ((strlen (data_p) > 0) && (strlen (r_word_p) > 0))
    {
      /* If 'r_word' is not 100% part of 'string', we do not have
       * a match.  If its a '.', it matches no matter what. */
      if ((data_p[0] != r_word_p[0]) && ('.' != r_word_p[0]))
	{
	  count = 0;
	  goto exit;
	}

      data_p++;
      r_word_p++;
    }

  /* If 'string' is shorter than 'r_word', we do not have a match */
  if ((0 == strlen (data_p)) && (0 < strlen (r_word_p)))
    {
      count = 0;
      goto exit;
    }

exit:
  /* Fill in our structure */
  if (0 == count)
    regex_data->match = REGEX_NO_MATCH;
  else if (strlen (regex_data->data) == count)
    regex_data->match = REGEX_FULL_MATCH;
  else
    regex_data->match = REGEX_PARTIAL_MATCH;
  if (regex_data->match != REGEX_NO_MATCH)
    regex_data->where = regex_data->data;
  else
    regex_data->where = NULL;
  regex_data->count = count;
  regex_data->r_count = count;

  free (r_word);
  return 0;

error:
  regex_data->match = REGEX_NO_MATCH;

  free (r_word);
  return -1;
}

size_t
get_list_size (const char *regex)
{
  size_t count = 0;

  if (!check_arg_str (regex))
    return 0;

  if ('[' != regex[0])
    {
      rc_errno_set (EINVAL);
      DBG_MSG ("Invalid argument passed!\n");
      return 0;
    }

  regex++;

  while ((strlen (regex) > 0) && (']' != regex[0]))
    {
      /* We have a sequence (x-y) */
      if (('-' == regex[0])
	  && (']' != regex[1])
	  && (strlen (regex) >= 2) && (regex[-1] < regex[1]))
	{
	  /* Add current + diff in sequence */
	  count += regex[1] - regex[-1];
	  /* Take care of '-' and next char */
	  regex += 2;
	}
      else
	{
	  regex++;
	  count++;
	}
    }

  return count;
}

size_t
get_list (const char *regex, char **r_list)
{
  char *buf = NULL;
  size_t count = 0;
  size_t size;

  if (!check_arg_str (regex))
    return 0;

  /* Bail if we do not have a list.  Do not add debugging, as
   * it is very noisy (used a lot when we call match_list() in
   * __match() and match() to test for list matching) */
  if ('[' != regex[0])
    return 0;

  size = get_list_size (regex);
  if (0 == size)
    {
      /* Should not be an issue, but just in case */
      DBG_MSG ("0 returned by get_list_size.\n");
      return 0;
    }

  *r_list = xmalloc (size + 1);
  if (NULL == *r_list)
    return -1;

  buf = *r_list;

  /* Take care of '[' */
  regex++;
  count++;

  while ((strlen (regex) > 0) && (']' != regex[0]))
    {
      /* We have a sequence (x-y) */
      if (('-' == regex[0])
	  && (']' != regex[1])
	  && (strlen (regex) >= 2) && (regex[-1] < regex[1]))
	{
	  /* Fill in missing chars in sequence */
	  while (buf[-1] < regex[1])
	    {
	      buf[0] = (char) (buf[-1] + 1);
	      buf++;
	      /* We do not increase count */
	    }
	  /* Take care of '-' and next char */
	  count += 2;
	  regex += 2;
	}
      else
	{
	  *buf++ = *regex++;
	  count++;
	}
    }

  buf[0] = '\0';
  /* Take care of ']' */
  count++;

  /* We do not have a list as it does not end in ']' */
  if (']' != regex[0])
    {
      count = 0;
      free (*r_list);
    }

  return count;
}

/* If the first is the '^' character, everything but the list is matched
 * NOTE:  We only evaluate _ONE_ data character at a time!! */
int
__match_list (regex_data_t * regex_data)
{
  regex_data_t tmp_data;
  char *data_p = regex_data->data;
  char *list_p = regex_data->regex;
  char test_regex[2] = { '\0', '\0' };
  int invert = 0;
  int lmatch;
  int retval;

  CHECK_REGEX_DATA_P (regex_data, failed);

  if ('^' == list_p[0])
    {
      /* We need to invert the match */
      invert = 1;
      /* Make sure '^' is not part of our list */
      list_p++;
    }

  if (invert)
    /* All should be a match if not in the list */
    lmatch = 1;
  else
    /* We only have a match if in the list */
    lmatch = 0;

  while (strlen (list_p) > 0)
    {
      test_regex[0] = list_p[0];

      FILL_REGEX_DATA (tmp_data, data_p, test_regex);
      retval = match_word (&tmp_data);
      if (-1 == retval)
	goto error;

      if (REGEX_MATCH (tmp_data))
	{
	  if (invert)
	    /* If we exclude the list from
	     * characters we try to match, we
	     * have a match until one of the
	     * list is found. */
	    lmatch = 0;
	  else
	    /* If not, we have to keep looking
	     * until one from the list match
	     * before we have a match */
	    lmatch = 1;
	  break;
	}
      list_p++;
    }

  /* Fill in our structure */
  if (lmatch)
    {
      regex_data->match = REGEX_PARTIAL_MATCH;
      regex_data->where = regex_data->data;
      regex_data->count = 1;
      /* This one is more cosmetic, as match_list() will
       * do the right thing */
      regex_data->r_count = 0;	/* strlen(regex_data->regex); */
    }
  else
    {
failed:
      regex_data->match = REGEX_NO_MATCH;
      regex_data->where = NULL;
      regex_data->count = 0;
      regex_data->r_count = 0;
    }

  return 0;

error:
  regex_data->match = REGEX_NO_MATCH;

  return -1;
}

int
match_list (regex_data_t * regex_data)
{
  regex_data_t tmp_data;
  char *data_p = regex_data->data;
  char *list_p = regex_data->regex;
  char *r_list = NULL;
  size_t r_count = 0;
  int retval;

  CHECK_REGEX_DATA_P (regex_data, failed);

  r_count = get_list (list_p, &r_list);
  if (-1 == r_count)
    goto error;
  if (0 == r_count)
    goto failed;

  FILL_REGEX_DATA (tmp_data, data_p, &list_p[r_count - 1]);
  retval = __match_wildcard (&tmp_data, __match_list, r_list);
  if (-1 == retval)
    goto error;
  if (REGEX_MATCH (tmp_data))
    {
      /* This should be 2 ('word' + 'wildcard'), so just remove
       * the wildcard */
      tmp_data.r_count--;
      goto exit;
    }

  FILL_REGEX_DATA (tmp_data, data_p, r_list);
  retval = __match_list (&tmp_data);
  if (-1 == retval)
    goto error;
  if (REGEX_MATCH (tmp_data))
    goto exit;

failed:
  /* We will fill in regex_data below */
  tmp_data.match = REGEX_NO_MATCH;
  tmp_data.where = NULL;
  tmp_data.count = 0;
  tmp_data.r_count = 0;

exit:
  /* Fill in our structure */
  regex_data->match = tmp_data.match;
  regex_data->where = tmp_data.where;
  regex_data->count = tmp_data.count;
  if (regex_data->match != REGEX_NO_MATCH)
    /* tmp_data.r_count for __match_wildcard will take care of the
     * wildcard, and tmp_data.r_count for __match_list will be 0 */
    regex_data->r_count = r_count + tmp_data.r_count;
  else
    regex_data->r_count = 0;

  free (r_list);
  return 0;

error:
  regex_data->match = REGEX_NO_MATCH;

  free (r_list);
  return -1;
}

size_t
get_wildcard (const char *regex, char *r_wildcard)
{
  if (!check_arg_str (regex))
    return 0;

  r_wildcard[0] = regex[0];
  r_wildcard[2] = '\0';

  switch (regex[1])
    {
    case '*':
    case '+':
    case '?':
      r_wildcard[1] = regex[1];
      break;
    default:
      r_wildcard[0] = '\0';
      return 0;
    }

  return strlen (r_wildcard);
}

int
__match_wildcard (regex_data_t * regex_data,
		  int (*match_func) (regex_data_t * regex_data),
		  const char *regex)
{
  regex_data_t tmp_data;
  char *data_p = regex_data->data;
  char *wildcard_p = regex_data->regex;
  char r_wildcard[3];
  size_t count = 0;
  size_t r_count = 0;
  int is_match = 0;
  int retval;

  CHECK_REGEX_DATA_P (regex_data, exit);

  if (NULL == match_func)
    {
      rc_errno_set (EINVAL);
      DBG_MSG ("NULL match_func was passed!\n");
      goto exit;
    }

  r_count = get_wildcard (wildcard_p, r_wildcard);
  if (0 == r_count)
    goto exit;

  FILL_REGEX_DATA (tmp_data, data_p, (char *) regex);
  retval = match_func (&tmp_data);
  if (-1 == retval)
    goto error;

  switch (r_wildcard[1])
    {
    case '*':
    case '?':
      /* '*' and '?' always matches */
      is_match = 1;
    case '+':
      /* We need to match all of them */
      do
	{
	  /* If we have at least one match for '+', or none
	   * for '*' or '?', check if we have a word or list match.
	   * We do this because a word weights more than a wildcard */
	  if ((strlen (wildcard_p) > 2)
	      && ((count > 0)
		  || ('*' == r_wildcard[1])
		  || ('?' == r_wildcard[1])))
	    {
	      regex_data_t tmp_data2;
#if 0
	      printf ("data_p = %s, wildcard_p = %s\n", data_p, wildcard_p);
#endif

	      FILL_REGEX_DATA (tmp_data2, data_p, &wildcard_p[2]);
	      retval = match (&tmp_data2);
	      if (-1 == retval)
		goto error;

	      if (
		  /* '.' might be a special case ... */
		  /* ('.' != wildcard_p[2]) && */
		  ((REGEX_MATCH (tmp_data2))
		   && (REGEX_FULL_MATCH == tmp_data2.match)))
		{
		  goto exit;
		}
	    }

	  if (REGEX_MATCH (tmp_data))
	    {
	      data_p += tmp_data.count;
	      count += tmp_data.count;
	      is_match = 1;

	      FILL_REGEX_DATA (tmp_data, data_p, (char *) regex);
	      retval = match_func (&tmp_data);
	      if (-1 == retval)
		goto error;
	    }
	  /* Only once for '?' */
	}
      while ((REGEX_MATCH (tmp_data)) && ('?' != r_wildcard[1]));

      break;
    default:
      /* No wildcard */
      break;
    }

exit:
  /* Fill in our structure */
  /* We can still have a match ('*' and '?'), although count == 0 */
  if ((0 == count) && (0 == is_match))
    regex_data->match = REGEX_NO_MATCH;
  else if (strlen (regex_data->data) == count)
    regex_data->match = REGEX_FULL_MATCH;
  else
    regex_data->match = REGEX_PARTIAL_MATCH;
  if (regex_data->match != REGEX_NO_MATCH)
    regex_data->where = regex_data->data;
  else
    regex_data->where = NULL;
  regex_data->count = count;
  regex_data->r_count = r_count;

  return 0;

error:
  regex_data->match = REGEX_NO_MATCH;

  return -1;
}

int
match_wildcard (regex_data_t * regex_data)
{
  regex_data_t tmp_data;
  char *data_p = regex_data->data;
  char *wildcard_p = regex_data->regex;
  char r_wildcard[3];
  size_t r_count;
  int retval;

  CHECK_REGEX_DATA_P (regex_data, failed);

  /* Invalid wildcard - we need a character + a regex operator */
  if (strlen (wildcard_p) < 2)
    goto failed;

  r_count = get_wildcard (wildcard_p, r_wildcard);
  if (0 == r_count)
    goto failed;

  /* Needed so that match_word() will not bail if it sees the wildcard */
  r_wildcard[1] = '\0';

  FILL_REGEX_DATA (tmp_data, data_p, wildcard_p);
  retval = __match_wildcard (&tmp_data, match_word, r_wildcard);
  if (-1 == retval)
    goto error;
  if (REGEX_MATCH (tmp_data))
    goto exit;

failed:
  /* We will fill in regex_data below */
  tmp_data.match = REGEX_NO_MATCH;
  tmp_data.where = NULL;
  tmp_data.count = 0;
  tmp_data.r_count = 0;

exit:
  /* Fill in our structure */
  regex_data->match = tmp_data.match;
  regex_data->where = tmp_data.where;
  regex_data->count = tmp_data.count;
  regex_data->r_count = tmp_data.r_count;

  return 0;

error:
  regex_data->match = REGEX_NO_MATCH;

  return -1;
}

int
__match (regex_data_t * regex_data)
{
  regex_data_t tmp_data;
  char *data_p = regex_data->data;
  char *regex_p = regex_data->regex;
  size_t count = 0;
  size_t r_count = 0;
  int rmatch = 0;
  int retval;

  CHECK_REGEX_DATA_P (regex_data, failed);

  while (strlen (regex_p) > 0)
    {
#if 0
      printf ("data_p = '%s', regex_p = '%s'\n", data_p, regex_p);
#endif

      FILL_REGEX_DATA (tmp_data, data_p, regex_p);
      retval = match_list (&tmp_data);
      if (-1 == retval)
	goto error;
      if (REGEX_MATCH (tmp_data))
	goto have_match;

      FILL_REGEX_DATA (tmp_data, data_p, regex_p);
      retval = match_wildcard (&tmp_data);
      if (-1 == retval)
	goto error;
      if (REGEX_MATCH (tmp_data))
	goto have_match;

      FILL_REGEX_DATA (tmp_data, data_p, regex_p);
      retval = match_word (&tmp_data);
      if (-1 == retval)
	goto error;
      if (REGEX_MATCH (tmp_data))
	goto have_match;

      break;

have_match:
      data_p += tmp_data.count;
      count += tmp_data.count;
      regex_p += tmp_data.r_count;
      r_count += tmp_data.r_count;
      rmatch = 1;

      /* Check that we do not go out of bounds */
      if (((data_p - regex_data->data) > strlen (regex_data->data))
	  || ((regex_p - regex_data->regex) > strlen (regex_data->regex)))
	goto failed;
    }

  /* We could not match the whole regex (data too short?) */
  if (0 != strlen (regex_p))
    goto failed;

  goto exit;

failed:
  /* We will fill in regex_data below */
  count = 0;
  r_count = 0;
  rmatch = 0;

exit:
  /* Fill in our structure */
  /* We can still have a match ('*' and '?'), although count == 0 */
  if ((0 == count) && (0 == rmatch))
    regex_data->match = REGEX_NO_MATCH;
  else if (strlen (regex_data->data) == count)
    regex_data->match = REGEX_FULL_MATCH;
  else
    regex_data->match = REGEX_PARTIAL_MATCH;
  if (regex_data->match != REGEX_NO_MATCH)
    regex_data->where = regex_data->data;
  else
    regex_data->where = NULL;
  regex_data->count = count;
  regex_data->r_count = r_count;

  return 0;

error:
  regex_data->match = REGEX_NO_MATCH;

  return -1;
}

int
match (regex_data_t * regex_data)
{
  regex_data_t tmp_data;
  char *data_p = regex_data->data;
  char *regex_p;
  char *buf = NULL;
  int from_start = 0;
  int to_end = 0;
  int retval;

  CHECK_REGEX_DATA_P (regex_data, failed);

  /* We might be modifying regex_p, so make a copy */
  buf = xstrndup (regex_data->regex, strlen (regex_data->regex));
  if (NULL == buf)
    goto error;

  regex_p = buf;

  /* Should we only match from the start? */
  if ('^' == regex_p[0])
    {
      regex_p++;
      from_start = 1;
    }

  /* Should we match up to the end? */
  if ('$' == regex_p[strlen (regex_p) - 1])
    {
      regex_p[strlen (regex_p) - 1] = '\0';
      to_end = 1;
    }

  do
    {
      FILL_REGEX_DATA (tmp_data, data_p, regex_p);
      retval = __match (&tmp_data);
      if (-1 == retval)
	goto error;
    }
  while ((strlen (data_p++) > 0)
	 && (!REGEX_MATCH (tmp_data)) && (0 == from_start));

  /* Compensate for above extra inc */
  data_p--;

  /* Fill in our structure */
  if (REGEX_MATCH (tmp_data))
    {
      /* Check if we had an '$' at the end of the regex, and
       * verify that we still have a match */
      if ((1 == to_end) && (tmp_data.count != strlen (data_p)))
	{
	  goto failed;
	}

      if ((data_p == regex_data->data)
	  && (tmp_data.match == REGEX_FULL_MATCH))
	regex_data->match = REGEX_FULL_MATCH;
      else
	regex_data->match = REGEX_PARTIAL_MATCH;
      regex_data->where = data_p;
      regex_data->count = tmp_data.count;
      regex_data->r_count = tmp_data.r_count;
      if (1 == from_start)
	regex_data->r_count++;
      if (1 == to_end)
	regex_data->r_count++;
    }
  else
    {
failed:
      regex_data->match = REGEX_NO_MATCH;
      regex_data->where = NULL;
      regex_data->count = 0;
      regex_data->r_count = 0;
    }

  free (buf);

  return 0;

error:
  regex_data->match = REGEX_NO_MATCH;
  free (buf);

  return -1;
}

