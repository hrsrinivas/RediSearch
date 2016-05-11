#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__

#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include "types.h"
#include "util/khash.h"
#include "varint.h"

/* Represents a token found in a document */
typedef struct  {
    // token string
    const char *s;
    // token string length
    size_t len;
    
    // position in the document - this is written to the inverted index
    u_int pos;
    
    // the token's score
    u_short score;
    
    // Field id - used later for filtering.
    // NOT IMPLEMENTED YET
    u_char fieldId;
} Token;


// A TokenFunc handles tokens in a tokenizer, for example aggregates them, or builds the query tree
typedef int(*TokenFunc)(void *ctx, Token t);

// A NormalizeFunc converts a raw token to the normalized form in which it will be stored
typedef char*(*NormalizeFunc)(char*, size_t*);

//! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~
#define DEFAULT_SEPARATORS " \t,./(){}[]:;/\\~!@#$%^&*-_=+|'`\"<>?";
#define QUERY_SEPARATORS " \t,./{}[]:;/\\~!@#$%^&*-_=+()|'<>?";
static const char *stopwords[] =  {
            "a", "is", "the", "an", "and", "are", "as", "at", "be", "but", "by",
            "for", "if", "in", "into", "it",
            "no", "not", "of", "on", "or", "such",
            "that", "their", "then", "there", "these",
            "they", "this", "to", "was", "will", "with", NULL
    };
    
// TODO: Optimize this with trie or something...    
int isStopword(const char *w);

typedef struct {
    const char *text;
    char **pos;
    const char *separators;
    u_short fieldScore;
    u_char fieldId;
    TokenFunc tokenFunc;
    void *tokenFuncCtx;
    NormalizeFunc normalize;
    
} TokenizerCtx;


/* The actual tokenizing process runner */
int _tokenize(TokenizerCtx *ctx);

/** The extenral API. Tokenize text, and create tokens with the given score and fieldId.
TokenFunc is a callback that will be called for each token found */
int tokenize(const char *text, u_short score, u_char fieldId, void *ctx, TokenFunc f);

/** A simple text normalizer that convertes all tokens to lowercase and removes accents. 
Does NOT normalize unicode */
char*DefaultNormalize(char *s, size_t *len);


/* A query-specific tokenizer, that reads symbols like quots, pipes, etc */
typedef struct {
    const char *text;
    size_t len;
    char *pos;
    const char *separators;
    NormalizeFunc normalize;
    
} QueryTokenizer;

/* Quer tokenizer token type */
typedef enum {
    T_WORD,
    T_QUOTE,
    T_AND,
    T_OR,
    T_END
} QueryTokenType;

/* A token in the process of parsing a query. Unlike the document tokenizer,  it
works iteratively and is not callback based.  */
typedef struct {
    const char *s;
    size_t len;
    QueryTokenType type;
} QueryToken;

/* Create a new query tokenizer. There is no need to free anything in the object */
QueryTokenizer NewQueryTokenizer(char *text, size_t len);

/* Read the next token from the tokenizer. If tit has reached the end of the 
query text, it will return a token with type T_END and null content.

Note: The token's text might not be null terminated, so use the len variable */
QueryToken QueryTokenizer_Next(QueryTokenizer *t);

/* Returns 1 if the tokenizer can read more tokens from the query text */
int QueryTokenizer_HasNext(QueryTokenizer *);

#endif