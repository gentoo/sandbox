/*
 * simple_regex.h
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

#ifndef __RC_SIMPLE_REGEX_H__
#define __RC_SIMPLE_REGEX_H__

#define REGEX_NO_MATCH 0	/* We have no match */
#define REGEX_PARTIAL_MATCH 1	/* Some of the string matches the regex */
#define REGEX_FULL_MATCH 2	/* The whole string matches the regex */

/* Macro to fill in .data and .regex */
#define FILL_REGEX_DATA(_regex_data, _string, _regex) \
 do { \
   _regex_data.data = _string; \
   _regex_data.regex = _regex; \
 } while (0)

/* Fill in _regex_data with _data and _regex, on failure goto _error */
#define DO_REGEX(_regex_data, _data, _regex, _error) \
 do { \
   FILL_REGEX_DATA(_regex_data, _data, _regex); \
   if (-1 == match(&_regex_data)) \
     { \
       DBG_MSG("Could not do regex match!\n"); \
       goto _error; \
     } \
 } while (0)

/* Evaluate to true if we have some kind of match */
#define REGEX_MATCH(_regex_data) \
 ((REGEX_FULL_MATCH == _regex_data.match) \
  || (REGEX_PARTIAL_MATCH == _regex_data.match))

/* Same as above, but for use when _regex_data is a pointer */
#define REGEX_MATCH_P(_regex_data) \
 ((REGEX_FULL_MATCH == _regex_data->match) \
  || (REGEX_PARTIAL_MATCH == _regex_data->match))

typedef struct
{
  char *data;			/* String to perform regex operation on */
  char *regex;			/* String containing regex to use */
  int match;			/* Will be set if there was a match.  Check
				 * REGEX_*_MATCH above for possible values */
  char *where;			/* Pointer to where match starts in data */
  size_t count;			/* Count characters from data matched by regex */
  size_t r_count;		/* Count characters of regex used for match.  This
				 * should normally be the lenght of regex, but might
				 * not be for some internal functions ... */
} regex_data_t;

/*
 * Return:
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
int match (regex_data_t * regex_data);

#endif /* __RC_SIMPLE_REGEX_H__ */
