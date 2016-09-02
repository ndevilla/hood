/*-------------------------------------------------------------------------*/
/**
   @file    hood.h
   @author  N. Devillard
   @brief   Dictionary implemented along Robin Hood hashing
*/
/*--------------------------------------------------------------------------*/
#ifndef _HOOD_H_
#define _HOOD_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/** Keypair: holds a key/value pair. Key must be a hashable C string */
typedef struct _keypair_ {
    char    * key ;
    char    * val ;
    uint64_t  hash ;
    int       dib ;
} keypair ;

/** Dict is the top type needed for clients of the dict object */
typedef struct _dict_ {
    unsigned  used ;
    unsigned  size ;
    keypair * table ;
} dict ;

/**
  @brief    Allocate a new dictionary object
  @return   Newly allocated dict, to be freed with dict_free()
 */
dict * dict_new(int sz);


/**
  @brief    Deallocate a dictionary object
  @param    d   dict to deallocate
  @return   void
 */
void   dict_free(dict * d);

/**
  @brief    Add an item to a dictionary
  @param    d       dict to add to
  @param    key     Key for the element to be inserted
  @param    val     Value to associate to the key, may be NULL
  @return   0 if Ok, something else in case of error

  Insert an element into a dictionary. If an element already exists with
  the same key, it is overwritten and the previous associated data are freed.
 */
int    dict_add(dict * d, char * key, char * val);

/**
  @brief    Get an item from a dictionary
  @param    d       dict to get item from
  @param    key     Key to look for
  @param    defval  Value to return if key is not found in dict
  @return   Element found, or defval
 */
char * dict_get(dict * d, char * key, char * defval);

/**
  @brief    Delete an item in a dictionary
  @param    d       dict where item is to be deleted
  @param    key     Key to look for
  @return   Number of actually deleted items. Should be 0 or 1
 */
int dict_del(dict * d, char * key);

/**
  @brief    Dump dict contents to an opened file pointer
  @param    d       dict to dump
  @param    out     File to output data to

  Dump the contents of a dictionary to an opened file pointer.
  It is Ok to pass 'stdout' or 'stderr' as file pointers.

  This function is mostly meant for debugging purposes.
 */
void   dict_dump(dict * d, FILE * out);

#endif

