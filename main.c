#include "stdio.h" 
#include "string.h"
#include "assert.h"
#include <ctype.h>
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
  TOK_EOF
} TokenType;

typedef struct {
  TokenType type;
  char* text;
} Token ;

typedef struct {
  char* src;
  int pos;
} Lexer;

Token next_token(Lexer* lexer) {
  assert(lexer != NULL);
  
  // skip comments (treat then like white spaces)
  while (1) {
    while (isspace((unsigned char)lexer->src[lexer->pos]))
      lexer->pos++;
    if (lexer->src[lexer->pos] == '/' &&
        lexer->src[lexer->pos + 1] == '/') {
      while (lexer->src[lexer->pos] != '\0' &&
             lexer->src[lexer->pos] != '\n')
        lexer->pos++;
      continue;
    }
    break;
  }

  if (lexer->src[lexer->pos] == '\0')
    return (Token){.type = TOK_EOF, .text = ""};

  char c = lexer->src[lexer->pos];

  // word -> keyword or NAME
  if (isalpha((unsigned char)c) || c == '_') {
    int start = lexer->pos;
    while (isalnum((unsigned char)lexer->src[lexer->pos]) ||
           lexer->src[lexer->pos] == '_')
      lexer->pos++;
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
      lexer->pos++;
    int len = lexer->pos - start;
    char *buf = malloc(len + 1);
    memcpy(buf, &lexer->src[start], len);
    buf[len] = '\0';
    return (Token){TOK_NUMBER, buf};
  }

  // == vs =
  if (c == '=') {
    if (lexer->src[lexer->pos + 1] == '=') {
      lexer->pos += 2;
      return (Token){TOK_EQ, "="}; 
    }
    lexer->pos++;
    return (Token){TOK_ASSIGN, "="};
  }

  // single chars
  lexer->pos++;
  switch (c) {
    case '+': return (Token){TOK_PLUS, "+"};
    case '>': return (Token){TOK_GT, ">"};
    case '(': return (Token){TOK_LPAREN, "("};
    case ')': return (Token){TOK_RPAREN, ")"};
    case '{': return (Token){TOK_LBRACE, "{"};
    case '}': return (Token){TOK_RBRACE, "}"};
    case ';': return (Token){TOK_SEMI, ";"};
    case ',': return (Token){TOK_COMMA, ","};
  }

  return next_token(lexer);
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
    default: return "OTHER";
  }
}

char* loadFile(const char* filename) {
  const char *path = "example.ghost";
  FILE *f = fopen(path, "rb");
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

  Lexer lex = {src, 0};
  for (;;) {
    Token t = next_token(&lex);
    printf("%s %s\n", tok_name(t.type), t.text);
    if (t.type == TOK_EOF) break;
  }
  free(src);
}
