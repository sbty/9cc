// tokenの種類
typedef enum
{
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token
{
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum
{
  ND_ADD,    // +
  ND_SUB,    // -
  ND_DIV,    // /
  ND_MUL,    // *
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_NUM,    // 整数
  ND_ASSIGN, // =
  ND_LVAR,   // ローカル変数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node
{
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺 left hand side
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合
  int offset;    // kingがND_LVARの場合
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

void gen(Node *node);

Token *tokenize();

void program();
Node *assign();
Node *stmt();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

extern Token *token;
extern char *user_input;
extern Node *code[100];
