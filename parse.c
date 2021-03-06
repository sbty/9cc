#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
/**
 * Tokenizer
 * */

// 次のトークンが期待している記号のときにはトークンを１つ進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
  {
    return false;
  }
  token = token->next;
  return true;
}

Token *consume_ident()
{
  if (token->kind != TK_IDENT)
  {
    return 0;
  }
  Token *ident_token = token;
  token = token->next;
  return ident_token;
}

// 次のトークンが期待している記号のときには、トークンを１つ進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
  {
    printf("expect, '%s'.", op);
    error_at(token->str, op);
  }
  token = token->next;
}

// 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
// それ以外の場合はエラーを報告する。
int expect_number()
{
  if (token->kind != TK_NUM)
  {
    error_at(token->str, "Not a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}
bool at_eof()
{
  return token->kind == TK_EOF;
}

bool starts_with(const char *pre, const char *str)
{
  size_t lenpre = strlen(pre),
         lenstr = strlen(str);
  return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;
  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }
    if (starts_with("==", p) ||
        starts_with("!=", p) ||
        starts_with(">=", p) ||
        starts_with("<=", p))
    {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p = p + 2;
      continue;
    }
    switch (*p)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '(':
    case ')':
    case '<':
    case '>':
    case '=':
    case ';':
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }
    default:
    {
      // Do nothing
    }
    }

    if ('a' <= *p && *p <= 'z')
    {
      cur = new_token(TK_IDENT, cur, p++, 1);
      continue;
    }

    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error_at(token->str, "Can't tokenize");
  }
  new_token(TK_EOF, cur, p, 1);
  return head.next;
}
/**
 * Parser
 * */

/**
 * function implementation
 * */
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *primary()
{
  if (consume("("))
  {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok)
  {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }

  return new_node_num(expect_number());
}

Node *mul()
{
  Node *node = unary();
  for (;;)
  {
    if (consume("*"))
    {
      node = new_node(ND_MUL, node, unary());
    }
    else if (consume("/"))
    {
      node = new_node(ND_DIV, node, unary());
    }
    else
    {
      return node;
    }
  }
}

Node *unary()
{
  if (consume("+"))
  {
    return primary();
  }
  if (consume("-"))
  {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  return primary();
}

Node *add()
{
  Node *node = mul();
  for (;;)
  {
    if (consume("+"))
    {
      node = new_node(ND_ADD, node, mul());
    }
    else if (consume("-"))
    {
      node = new_node(ND_SUB, node, mul());
    }
    else
    {
      return node;
    }
  }
}

Node *relational()
{
  Node *node = add();
  for (;;)
  {
    if (consume("<"))
    {
      node = new_node(ND_LT, node, add());
    }
    else if (consume(">"))
    {
      node = new_node(ND_LT, add(), node);
    }
    else if (consume("<="))
    {
      node = new_node(ND_LE, node, add());
    }
    else if (consume(">="))
    {
      node = new_node(ND_LE, add(), node);
    }
    else
    {
      return node;
    }
  }
}

Node *equality()
{
  Node *node = relational();
  for (;;)
  {
    if (consume("=="))
    {
      node = new_node(ND_EQ, node, relational());
    }
    else if (consume("!="))
    {
      node = new_node(ND_NE, node, relational());
    }
    else
    {
      return node;
    }
  }
}

Node *assign()
{
  Node *node = equality();

  if (consume("="))
  {
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

Node *expr()
{
  return assign();
}

Node *stmt()
{
  Node *node = expr();
  expect(";");
  return node;
}

void program()
{
  int i = 0;
  while (!at_eof())
  {
    code[i++] = stmt();
  }
  code[i] = NULL;
}
