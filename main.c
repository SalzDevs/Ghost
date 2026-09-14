#include "stdio.h" 
#include "string.h"

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
  TOK_COMMA
} TokenType;

typedef struct {
  TokenType type;
  char* text;
  int col;
  int row;
} Token ;

int main(){
  printf("Hello world!\n"); 
}
