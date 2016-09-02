/**
   @file    hood.c
   @brief   dictionary object
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hood.h"

/** Minimum dictionary size to start with */
#define DICT_MIN_SZ     8

/** Define this to:
    0 for no debugging
    1 for moderate debugging
    2 for heavy debugging
 */
#define DEBUG           2

#define DICT_BIGSZ      65536

#define dict_hash   dict_hash_murmur

/* forward definition */
static int dict_resize(dict * d);

/** replacement for strdup() which is not always provided by libc */
static char * xstrdup(char * s)
{
    char * t ;
    if (!s)
        return NULL ;

    t = malloc(strlen(s)+1) ;
    if (t) {
        strcpy(t,s);
    }
    return t ;
}


/* Murmurhash on 64 bits */
static uint64_t dict_hash_murmur(char * key)
{
    uint64_t m = 0xc6a4a7935bd1e995;
    int r = 47;
    int len ;
    uint64_t seed = 0;
    uint64_t h, k ;
    uint64_t * data ;
    uint64_t * end ;
    unsigned char * data2 ;

    if (!key)
        return 0 ;

    len = (int)strlen(key);
    h = seed ^ (len * m);
    data = (uint64_t *) key ;
    end = data + (len/8);

    while (data != end) {
        k = *data++;
        k *= m; 
        k ^= k >> r; 
        k *= m; 
        h ^= k;
        h *= m; 
    }
    data2 = (unsigned char*)data;
    switch (len & 7) {
        case 7: h ^= (uint64_t)data2[6] << 48;
        case 6: h ^= (uint64_t)data2[5] << 40;
        case 5: h ^= (uint64_t)data2[4] << 32;
        case 4: h ^= (uint64_t)data2[3] << 24;
        case 3: h ^= (uint64_t)data2[2] << 16;
        case 2: h ^= (uint64_t)data2[1] << 8;
        case 1: h ^= (uint64_t)data2[0];
                h *= m;
    }
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    return h;
}

/** Add an item to a dictionary by copying key/val into the dict. */
int dict_add(dict * d, char * key, char * val)
{
    uint64_t  hash ;
    uint64_t  pos ;
    int       dib ;
    char   *  loc_key ;
    char   *  loc_val ;

    if (!d || !key)
        return -1 ;

#if DEBUG>2
    printf("dict_add[%s][%s]\n", key, val ? val : "UNDEF");
#endif

    hash = dict_hash(key);
    pos = hash % d->size;
    dib = 0 ;
    while (1) {
        if (d->table[pos].key == NULL) {
            d->table[pos].key = xstrdup(key);
            d->table[pos].val = xstrdup(val);
            d->table[pos].dib = dib ;
            d->table[pos].hash = hash;
            d->used++ ;
            break ;
        }
        if (d->table[pos].dib < dib) {
            loc_key = d->table[pos].key ;
            loc_val = d->table[pos].val ;

            d->table[pos].key = xstrdup(key);
            d->table[pos].val = xstrdup(val);
            d->table[pos].dib = dib ;
            d->table[pos].hash = hash ;
            dict_add(d, loc_key, loc_val);
            free(loc_key);
            free(loc_val);
            break ;
        }
        dib++;
        pos = (pos+1) % d->size ;
    }
    /* Resize if needed */
    if (3 * d->used > 2 * d->size) {
        dict_resize(d);
    }
    return 0;
}

/** Resize a dictionary */
static int dict_resize(dict * d)
{
    unsigned newsz ;
    unsigned factor ;
    dict  *  nd ;
    int      i ;

    newsz = d->size ;
    factor = (d->size > DICT_BIGSZ) ? 2 : 4 ;
    while (newsz <= (factor * d->used)) {
        newsz *= 2 ;
    }
    if (newsz==d->size) {
        return 0 ;
    }

#if DEBUG>2
    printf("resizing %d to %d\n", d->size, newsz);
#endif
    nd = dict_new(newsz);
    for (i=0 ; i<d->size ; i++) {
        if (d->table[i].key!=NULL) {
            dict_add(nd, d->table[i].key, d->table[i].val);
            free(d->table[i].key);
            if (d->table[i].val) {
                free(d->table[i].val);
            }
        }
    }
    d->size = nd->size ;
    d->used = nd->used ;
    free(d->table);
    d->table = nd->table ;
    return 0 ;
}

/** Allocate a new dict */
dict * dict_new(int sz)
{
    dict * d ;

    d = calloc(1, sizeof(dict));
    if (!d)
        return NULL;
    d->size  = sz>0 ? sz : DICT_MIN_SZ ;
    d->used  = 0 ;
    d->table = calloc(d->size, sizeof(keypair));
    return d ;
}

/** Deallocate a dict */
void dict_free(dict * d)
{
    int i ;
    
    if (!d)
        return ;

    if (d->table==NULL) {
        free(d);
        return ;
    }

    for (i=0 ; i<d->size ; i++) {
        if (d->table[i].key) {
            free(d->table[i].key);
            if (d->table[i].val) {
                free(d->table[i].val);
            }
        }
    }
    free(d->table);
    free(d);
    return ;
}

/** Internal find function */
static int dict_find(dict * d, char * key)
{
    uint64_t  hash ;
    uint64_t  pos ;
    int       found ;

    if (!d || !key)
        return -1 ;

    hash = dict_hash(key);
    pos  = hash % d->size ;
    found = -1 ;

    while (1) {
        if (d->table[pos].key == NULL) {
            break ;
        }
        if (d->table[pos].hash == hash &&
            !strcmp(key, d->table[pos].key)) {
            found = pos ;
            break ;
        }
        pos = (pos+1)%d->size;
    }
    return found ;
}

/** Get an item from a dict */
char * dict_get(dict * d, char * key, char * defval)
{
    int       pos ;

    if (!d || !key)
        return defval ;

    pos = dict_find(d, key);
    return (pos<0) ? defval : d->table[pos].val ;
}

/** Delete an item in a dict */
int dict_del(dict * d, char * key)
{
    int pos, npos ;

    if (!d || !key) {
        return 0 ;
    }

    pos = dict_find(d, key);
    if (pos<0) {
        return 0 ;
    }

    /* Delete found key pair */
    if (d->table[pos].key) {
        free(d->table[pos].key);
        d->table[pos].key = NULL ;
    }
    if (d->table[pos].val) {
        free(d->table[pos].val);
        d->table[pos].val = NULL ;
    }
    d->used-- ;

    /* Backtrack on later key pairs */
    while (1) {
        npos = (pos+1) % d->size ;
        if (d->table[npos].key == NULL) {
            /* Stop backtracking */
            break ;
        }

        if (d->table[npos].dib == 0) {
            /* dib==0 is a stop point */
            break ;
        }

        /* Copy next pos into current pos */
        d->table[pos].key  = d->table[npos].key ;
        d->table[pos].val  = d->table[npos].val ;
        d->table[pos].dib  = d->table[npos].dib - 1 ;
        d->table[pos].hash = d->table[npos].hash ;

        d->table[npos].key = NULL ;
        d->table[npos].val = NULL ;
        /* Switch to next position */
        pos = npos ;
    }
    return 1 ;
}

/** Dump a dict to a file pointer */
void dict_dump(dict * d, FILE * out)
{
    int    i ;

    if (!d || !out)
        return ;

    fprintf(out, "# size=%d used=%d\n", d->size, d->used);
    for (i=0 ; i<d->size ; i++) {
        if (d->table[i].key!=NULL) {
            fprintf(out, "[% 6d] %20s: %s\n", i,
                    d->table[i].key,
                    d->table[i].val ? d->table[i].val : "UNDEF");
        }
    }
    return ;
}

/*---------------------------------------------------------------------------
                                Test main
 ---------------------------------------------------------------------------*/
#ifdef MAIN

#include <time.h>
#include <sys/time.h>

#define ALIGN   "%15s: %6.4f\n"
#define NKEYS   10*1024*1024

double epoch_double()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + (t.tv_usec * 1.0) / 1000000.0;
}

int main(int argc, char * argv[])
{
    dict * d ;
    double t1, t2 ;
    int i ;
    int nkeys ;
    char * buffer ;
    char * val ;

    nkeys = (argc>1) ? (int)atoi(argv[1]) : NKEYS ;
    printf("%15s: %d\n", "values", nkeys);
    buffer = malloc(9 * nkeys);

    d = dict_new(0);
    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        sprintf(buffer + i * 9, "%08x", i);
    }
    t2 = epoch_double();
    printf(ALIGN, "initialization", t2 - t1);

    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        dict_add(d, buffer + i*9, buffer +i*9);
    }
    t2 = epoch_double();
    printf(ALIGN, "adding", t2 - t1);

    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        val = dict_get(d, buffer + i*9, "UNDEF");
#if DEBUG>0
        if (val && strcmp(val, buffer+i*9)) {
            printf("-> WRONG got[%s] exp[%s]\n", val, buffer+i*9);
        }
#endif
    }
    t2 = epoch_double();
    printf(ALIGN, "lookup", t2 - t1);


    if (nkeys<100)
        dict_dump(d, stdout);

    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        dict_del(d, buffer + i*9);
    }
    t2 = epoch_double();
    printf(ALIGN, "delete", t2 - t1);

    /* dict_dump(d, stdout); */
    t1 = epoch_double();
    for(i = 0; i < nkeys; i++) {
        dict_add(d, buffer + i*9, buffer +i*9);
    }
    t2 = epoch_double();
    printf(ALIGN, "adding", t2 - t1);
    
    t1 = epoch_double();
    dict_free(d);
    t2 = epoch_double();
    printf(ALIGN, "free", t2 - t1);

    free(buffer);
    return 0 ;

}
#endif
