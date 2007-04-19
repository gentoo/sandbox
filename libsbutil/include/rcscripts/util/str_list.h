/*
 * str_list.h
 *
 * String list macros.
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

#ifndef __RC_STR_LIST_H__
#define __RC_STR_LIST_H__

/* Add a new item to a string list.  If the pointer to the list is NULL,
 * allocate enough memory for the amount of entries needed.  Ditto for
 * when it already exists, but we add one more entry than it can
 * contain.  The list is NULL terminated.
 * NOTE: _only_ memory for the list are allocated, and not for the items - that
 *       should be done by relevant code (unlike str_list_del_item() that will
 *       free the memory) */
#define str_list_add_item(_string_list, _item, _error) \
 do { \
   char **_tmp_p; \
   int _i = 0; \
   if (!check_str (_item)) \
     { \
       goto _error; \
     } \
   while ((NULL != _string_list) && (NULL != _string_list[_i])) \
     { \
       _i++; \
     } \
   /* Amount of entries + new + terminator */ \
   _tmp_p = xrealloc (_string_list, sizeof (char *) * (_i + 2)); \
   if (NULL == _tmp_p) \
     { \
       goto _error; \
     } \
   _string_list = _tmp_p; \
   _string_list[_i] = _item; \
   /* Terminator */ \
   _string_list[_i+1] = NULL; \
 } while (0)

/* Same as above, just makes a copy of the item, so calling code do not need
 * to do this. */
#define str_list_add_item_copy(_string_list, _item, _error) \
 do { \
   char *_tmp_str; \
   char **_tmp_p; \
   int _i = 0; \
   if (!check_str (_item)) \
     { \
       goto _error; \
     } \
   while ((NULL != _string_list) && (NULL != _string_list[_i])) \
     { \
       _i++; \
     } \
   /* Amount of entries + new + terminator */ \
   _tmp_p = xrealloc (_string_list, sizeof (char *) * (_i + 2)); \
   if (NULL == _tmp_p) \
     { \
       goto _error; \
     } \
   _string_list = _tmp_p; \
   _tmp_str = xstrndup (_item, strlen (_item)); \
   if (NULL == _tmp_str) \
     { \
       goto _error; \
     } \
   _string_list[_i] = _tmp_str; \
   /* Terminator */ \
   _string_list[_i+1] = NULL; \
 } while (0)

/* Add a new item to a string list (foundamental the same as above), but make
 * sure we have all the items alphabetically sorted. */
#define str_list_add_item_sorted(_string_list, _item, _error) \
 do { \
   char **_tmp_p; \
   char *_str_p1; \
   char *_str_p2; \
   int _i = 0; \
   if (!check_str (_item)) \
     { \
       goto _error; \
     } \
   while ((NULL != _string_list) && (NULL != _string_list[_i])) \
     { \
       _i++; \
     } \
   /* Amount of entries + new + terminator */ \
   _tmp_p = xrealloc (_string_list, sizeof (char *) * (_i + 2)); \
   if (NULL == _tmp_p) \
     { \
       goto _error; \
     } \
   _string_list = _tmp_p; \
   if (0 == _i) \
     { \
       /* Needed so that the end NULL will propagate
	* (iow, make sure our 'NULL != _str_p1' test below
	*  do not fail) */ \
       _string_list[_i] = NULL; \
     } \
   /* Actual terminator that needs adding */ \
   _string_list[_i+1] = NULL; \
   _i = 0; \
   /* See where we should insert the new item to have it all \
    * alphabetically sorted */ \
   while (NULL != _string_list[_i]) \
     { \
       if (strcmp (_string_list[_i], _item) > 0) \
	 { \
           break; \
         } \
       _i++; \
     } \
   /* Now just insert the new item, and shift the rest one over.
    * '_str_p2' is temporary storage to swap the indexes in a loop,
    * and 'str_p1' is used to store the old value across the loop */ \
   _str_p1 = _string_list[_i]; \
   _string_list[_i] = _item; \
    do { \
     _i++;\
     _str_p2 = _string_list[_i]; \
     _string_list[_i] = _str_p1; \
     _str_p1 = _str_p2; \
    } while (NULL != _str_p1); \
 } while (0)

/* Delete one entry from the string list, and shift the rest down if the entry
 * was not at the end.  For now we do not resize the amount of entries the
 * string list can contain, and free the memory for the matching item */
#define str_list_del_item(_string_list, _item, _error) \
 do { \
   int _i = 0; \
   if (!check_str (_item)) \
     { \
       goto _error; \
     } \
   if (NULL == _string_list) \
     { \
       rc_errno_set (EINVAL); \
       DBG_MSG ("Invalid string list passed!\n"); \
       goto _error; \
     } \
   while (NULL != _string_list[_i]) \
     { \
       if (0 == strcmp (_item, _string_list[_i])) \
	 { \
	   break; \
	 } \
       else \
	 { \
	   _i++; \
	 } \
     } \
   if (NULL == _string_list[_i]) \
     { \
       rc_errno_set (EINVAL); \
       DBG_MSG ("Invalid string list item passed!\n"); \
       goto _error; \
     } \
   free (_string_list[_i]); \
   /* Shift all the following items one forward */ \
   do { \
     _string_list[_i] = _string_list[_i+1]; \
     /* This stupidity is to shutup gcc */ \
     _i++; \
   } while (NULL != _string_list[_i]); \
 } while (0)

/* Step through each entry in the string list, setting '_pos' to the
 * beginning of the entry.  '_counter' is used by the macro as index,
 * but should not be used by code as index (or if really needed, then
 * it should usually by +1 from what you expect, and should only be
 * used in the scope of the macro) */
#define str_list_for_each_item(_string_list, _pos, _counter) \
 if ((NULL != _string_list) && (0 == (_counter = 0))) \
   while (NULL != (_pos = _string_list[_counter++]))

/* Same as above (with the same warning about '_counter').  Now we just
 * have '_next' that are also used for indexing.  Once again rather refrain
 * from using it if not absolutely needed.  The major difference to above,
 * is that it should be safe from having the item removed from under you. */
#define str_list_for_each_item_safe(_string_list, _pos, _next, _counter) \
 if ((NULL != _string_list) && (0 == (_counter = 0))) \
   /* First part of the while checks if this is the
    * first loop, and if so setup _pos and _next
    * and increment _counter */ \
   while ((((0 == _counter) \
	    && (NULL != (_pos = _string_list[_counter])) \
	    && (_pos != (_next = _string_list[++_counter]))) \
	  /* Second part is when it is not the first loop
	   * and _pos was not removed from under us.  We
	   * just increment _counter, and setup _pos and
	   * _next */ \
	  || ((0 != _counter) \
	      && (_pos == _string_list[_counter-1]) \
	      && (_next == _string_list[_counter]) \
	      && (NULL != (_pos = _string_list[_counter])) \
	      && (_pos != (_next = _string_list[++_counter]))) \
	  /* Last part is when _pos was removed from under
	   * us.  We basically just setup _pos and _next,
	   * but leave _counter alone */ \
	  || ((0 != _counter) \
	      && (_pos != _string_list[_counter-1]) \
	      && (_next == _string_list[_counter-1]) \
	      && (NULL != (_pos = _string_list[_counter-1])) \
	      && (_pos != (_next = _string_list[_counter])))))

/* Just free the whole string list */
#define str_list_free(_string_list) \
 do { \
   if (NULL != _string_list) \
     { \
       int _i = 0; \
       while (NULL != _string_list[_i]) \
	 { \
	   free (_string_list[_i]); \
	   _string_list[_i++] = NULL; \
	 } \
       free (_string_list); \
       _string_list = NULL; \
     } \
 } while (0)

#endif /* __RC_STR_LIST_H__ */
