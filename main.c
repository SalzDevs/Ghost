#include "stdio.h" 
#include "string.h"
#include "assert.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
  TOK_FUNC,
  TOK_INT,
  TOK_IF,
  TOK_FOR,
  TOK_BREAK,
  TOK_RETURN,
  TOK_NAME,
  TOK_NUMBER,
  TOK_ASSIGN, 
  TOK_EQ,   
  TOK_PLUS, 
  TOK_GT,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_SEMI,
  TOK_COMMA,
  TOK_COMM,
  TOK_EOF,
  TOK_ERR,
} TokenType;

typedef struct {
  TokenType type;
  // Ownership: text is malloc'd or NULL. Caller owns it and must free().
  char* text;
} Token ;

typedef struct {
  char* src;
  int pos;
  int row;
  int col;
  // 1-based start of the last token produced (set by next_token)
  int tok_row;
  int tok_col;
} Lexer;

typedef struct {
  char* type;
  char* name;
}Param;

typedef struct {
  char* name;
  Param args[8];
  int nargs;
  char* return_types[8];
  int nreturns;
} FuncDef;

typedef struct {
  Lexer* lex;
  // always contains the next token (lookahead);
  Token cur;
} Parser;

static void advance(Lexer* lexer) {
  if (lexer->src[lexer->pos] == '\n') {
    lexer->row++;
    lexer->col = 1;
  } else {
    lexer->col++;
  }
  lexer->pos++;
}

static Token next_token_raw(Lexer* lexer);

Token next_token(Lexer* lexer) {
  assert(lexer != NULL);
  
  // skip comments (treat them like white spaces)
  while (1) {
    while (isspace((unsigned char)lexer->src[lexer->pos]))
      advance(lexer);
    if (lexer->src[lexer->pos] == '/' &&
        lexer->src[lexer->pos + 1] == '/') {
      while (lexer->src[lexer->pos] != '\0' &&
             lexer->src[lexer->pos] != '\n')
        advance(lexer);
      continue;
    }
    break;
  }

  lexer->tok_row = lexer->row;
  lexer->tok_col = lexer->col;
  return next_token_raw(lexer);
}

static Token next_token_raw(Lexer* lexer) {
  assert(lexer != NULL);

  if (lexer->src[lexer->pos] == '\0')
    return (Token){.type = TOK_EOF, .text = NULL};

  char c = lexer->src[lexer->pos];

  // word -> keyword or NAME
  if (isalpha((unsigned char)c) || c == '_') {
    int start = lexer->pos;
    while (isalnum((unsigned char)lexer->src[lexer->pos]) ||
           lexer->src[lexer->pos] == '_')
      advance(lexer);
    int len = lexer->pos - start;
    char *buf = malloc(len + 1);
    memcpy(buf, &lexer->src[start], len);
    buf[len] = '\0';
    if (strcmp(buf, "func") == 0) return (Token){TOK_FUNC, buf};
    if (strcmp(buf, "int") == 0) return (Token){TOK_INT, buf};
    if (strcmp(buf, "if") == 0) return (Token){TOK_IF, buf};
    if (strcmp(buf, "for") == 0) return (Token){TOK_FOR, buf};
    if (strcmp(buf, "break") == 0) return (Token){TOK_BREAK, buf};
    if (strcmp(buf, "return") == 0) return (Token){TOK_RETURN, buf};
    return (Token){TOK_NAME, buf};
  }

  // number
  if (isdigit((unsigned char)c)) {
    int start = lexer->pos;
    while (isdigit((unsigned char)lexer->src[lexer->pos]))
      advance(lexer);
    int len = lexer->pos - start;
    char *buf = malloc(len + 1);
    memcpy(buf, &lexer->src[start], len);
    buf[len] = '\0';
    return (Token){TOK_NUMBER, buf};
  }

  // == vs =
  if (c == '=') {
    if (lexer->src[lexer->pos + 1] == '=') {
      advance(lexer);
      advance(lexer);
      return (Token){TOK_EQ, NULL};
    }
    advance(lexer);
    return (Token){TOK_ASSIGN, NULL};
  }

  // single chars (no text needed; type says it all)
  advance(lexer);
  switch (c) {
    case '+': return (Token){TOK_PLUS, NULL};
    case '>': return (Token){TOK_GT, NULL};
    case '(': return (Token){TOK_LPAREN, NULL};
    case ')': return (Token){TOK_RPAREN, NULL};
    case '{': return (Token){TOK_LBRACE, NULL};
    case '}': return (Token){TOK_RBRACE, NULL};
    case ';': return (Token){TOK_SEMI, NULL};
    case ',': return (Token){TOK_COMMA, NULL};
  }

  char *buf = malloc(2);
  buf[0] = c;
  buf[1] = '\0';
  return (Token){TOK_ERR, buf};
}

const char *tok_name(TokenType t) {
  switch (t) {
    case TOK_FUNC: return "FUNC";
    case TOK_INT: return "INT";
    case TOK_IF: return "IF";
    case TOK_FOR: return "FOR";
    case TOK_BREAK: return "BREAK";
    case TOK_RETURN: return "RETURN";
    case TOK_NAME: return "NAME";
    case TOK_NUMBER: return "NUMBER";
    case TOK_ASSIGN: return "ASSIGN";
    case TOK_EQ: return "EQ";
    case TOK_PLUS: return "PLUS";
    case TOK_GT: return "GT";
    case TOK_LPAREN: return "LPAREN";
    case TOK_RPAREN: return "RPAREN";
    case TOK_LBRACE: return "LBRACE";
    case TOK_RBRACE: return "RBRACE";
    case TOK_SEMI: return "SEMI";
    case TOK_COMMA: return "COMMA";
    case TOK_COMM: return "COMM";
    case TOK_EOF: return "EOF";
    case TOK_ERR: return "ERR";
    default: return "OTHER";
  }
}

void parser_advance(Parser* p) {
  free(p->cur.text);
  p->cur = next_token(p->lex);
}

void free_funcdef(FuncDef* f) {
  free(f->name);
  for (int i = 0; i < f->nargs; i++) {
    free(f->args[i].type);
    free(f->args[i].name);
  }
  for (int i = 0; i < f->nreturns; i++) free(f->return_types[i]);
  f->name = NULL; f->nargs = 0; f->nreturns = 0;
}

int parse_func(Parser* p, FuncDef* out) {
  out->name = NULL; out->nargs = 0; out->nreturns = 0;

  if (p->cur.type != TOK_FUNC) return 0;
  parser_advance(p);

  if (p->cur.type != TOK_NAME) return 0;
  out->name = strdup(p->cur.text);
  parser_advance(p);

  if (p->cur.type == TOK_LPAREN) {
    parser_advance(p);
    while (p->cur.type != TOK_RPAREN && p->cur.type != TOK_EOF) {
      if (out->nargs >= 8) { free_funcdef(out); return 0; }
      if (p->cur.type != TOK_INT && p->cur.type != TOK_NAME) {
        free_funcdef(out);
        return 0;
      }
      char *t = strdup(p->cur.text);
      parser_advance(p);
      if (p->cur.type != TOK_NAME) {
        free(t);
        free_funcdef(out);
        return 0;
      }
      char *n = strdup(p->cur.text);
      parser_advance(p);
      out->args[out->nargs].type = t;
      out->args[out->nargs].name = n;
      out->nargs++;
      if (p->cur.type == TOK_COMMA) { parser_advance(p); continue; }
      break;
    }
    if (p->cur.type != TOK_RPAREN) { free_funcdef(out); return 0; }
    parser_advance(p);
  }

  if (p->cur.type == TOK_LPAREN) {
    parser_advance(p);
    while (p->cur.type != TOK_RPAREN && p->cur.type != TOK_EOF) {
      if (out->nreturns >= 8) { free_funcdef(out); return 0; }
      if (p->cur.type != TOK_INT && p->cur.type != TOK_NAME) {
        free_funcdef(out);
        return 0;
      }
      out->return_types[out->nreturns++] = strdup(p->cur.text);
      parser_advance(p);
      if (p->cur.type == TOK_COMMA) { parser_advance(p); continue; }
      break;
    }
    if (p->cur.type != TOK_RPAREN) { free_funcdef(out); return 0; }
    parser_advance(p);
  }

  if (p->cur.type != TOK_LBRACE) {
    return 0;
  }
  parser_advance(p);
  
  int num_tokens_body = 1;
  int depth = 1;
  while (p->cur.type != TOK_EOF) {
    if (p->cur.type == TOK_LBRACE) {
      depth++;
    }
    if (p->cur.type==TOK_RBRACE) {
      depth--;
      if (depth==0) {
        break;
      }
    }
    printf("Reading body! Current Number of Tokens in the Body {%d} Token {%s} depth {%d}\n", num_tokens_body, p->cur.text, depth);
    num_tokens_body++;
    parser_advance(p);
  }
  printf("Reading body! Current Number of Tokens in the Body {%d} Token {%s} depth {%d}\n", num_tokens_body, p->cur.text, depth);

  if (p->cur.type != TOK_RBRACE) { free_funcdef(out) ; return 0; }

  return 1;
}

char* loadFile(const char* filename) {
  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    printf("Not able to open the file.\n");
    return NULL;
  }
  
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *src = malloc(n + 1);
  fread(src, 1, n, f);
  src[n] = '\0';
  fclose(f);

  return src;
}

int main(){
  const char *path = "example.ghost";
  char* src = loadFile(path);
  if (src == NULL) {
    return 1;
  }

  int err_count = 0;
  Lexer lex = {src, 0, 1, 1, 1, 1};


  Parser p = {&lex, next_token(&lex)};

  for (;;) {
    if (p.cur.type == TOK_EOF) break;
    if (p.cur.type == TOK_FUNC) {
      FuncDef fd;
      if (parse_func(&p, &fd)) {
        printf("func %s nargs=%d nreturns=%d\n", fd.name, fd.nargs, fd.nreturns);
        free_funcdef(&fd);
        parser_advance(&p); // eat closing RBRACE left by parse_func
      } else {
        fprintf(stderr, "parse error at (Line %d Column: %d)\n", lex.tok_row, lex.tok_col);
        err_count++;
        parser_advance(&p);
      }
      continue;
    }
    if (p.cur.type == TOK_ERR) {
      fprintf(stderr, "error at (Line %d Column: %d): unexpected character: '%s'\n", lex.tok_row, lex.tok_col, p.cur.text);
      err_count++;
    } else {
      fprintf(stderr, "error at (Line %d Column: %d): unexpected %s\n", lex.tok_row, lex.tok_col, tok_name(p.cur.type));
      err_count++;
    }
    parser_advance(&p);
  }
  free(p.cur.text);
  free(src);

  return err_count ? 1 : 0;  
}
