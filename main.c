#include <stdarg.h>
#include <stdio.h>

#include "9cc.h"

// 入力プログラムを格納
char *user_input;

// 現在見ているトークン
Token *token;

Node *code[100];

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n"); //26*8

  for (int i = 0; code[i]; i++)
  {
    gen(code[i]);

    // A value left as the result of statement at stack
    // So pop it to prevent overflow the stack
    printf(" pop rax\n");
  }

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n"); //26*8

  return 0;
}
