/***************************************************************************************
 ** Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

/* oprand type,  enum 从256开始避免污染操作符的ASCII码. */
enum {
  TK_NOTYPE = 256,
  TK_uDECNUM,
  TK_DECNUM,
  TK_HEXNUM, // number
  TK_REGNAME,
  TK_NEQUAL,
  TK_LOGICAND, // logical operator
  TK_DEREF,
  /* TODO: Add more token types */

};

typedef enum {
  EVAL_SUCCESS = 0,
  EVAL_BAD_EXPR,
  EVAL_DIV_ZERO,
  EVAL_ILLEGAL_PARENTHESES, // parentheses don't match up
  EVAL_ILLEGAL_OP,          // operator not in {+,-,*,/}.
  EVAL_DEREF_INVALID,
  EVAL_ILLEGAL_ARITY,
  EVAL_ILLEGAL_REGNAME,
  /* can add more eval status */
} EvalStatus;

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     * (当需要转义时，请注意语言也会进行一次转义，正则表达式也进行元字符的转义)
     * {"\\d+", TK_DECNUM}
     * POSIX兼容正则表达式不支持\d（regex.h默认是POSIX.2库，不支持PCRE）
     */
    {" +", TK_NOTYPE},                  // spaces
    {"\\+", '+'},                       // plus
    {"==", '='},                        // equal
    {"!=", TK_NEQUAL},                  // not equal to
    {"-", '-'},                         // minus
    {"\\*", '*'},                       // times
    {"/", '/'},                         // division
    {"&&", TK_LOGICAND},                // logical and
    {"0[Xx][[:digit:]]+", TK_HEXNUM},   // hexical number
    {"[[:digit:]]+[uU]", TK_uDECNUM},   // unsigned decial number
    {"[[:digit:]]+", TK_DECNUM},        // decial number
    {"\\$[\\$[:alnum:]]+", TK_REGNAME}, // reg name
    {"\\(", '('},                       // left bracket
    {"\\)", ')'},                       // right bracket
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex,
                  REG_EXTENDED); // compile string to regex_t object
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

#define MAX_TOKENS_LEN 65536
typedef struct token {
  int type;
  char str[32];
} Token;

// static Token tokens[32] __attribute__((used)) = {};
/* for testing long expr */
static Token tokens[MAX_TOKENS_LEN] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Token numbers over MAX_TOKENS_LEN, panic */
    if (nr_token > MAX_TOKENS_LEN) { // for testing expr-gen, make it larger
      panic("The tokens of expression are more than max array length %d",
            MAX_TOKENS_LEN);
    }

    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        /* Some token only need it's type */
        case '=':
        /* PASS THROUGH */
        case '+':
        /* PASS THROUGH */
        case '-':
        /* PASS THROUGH */
        case '*':
        /* PASS THROUGH */
        case '/':
        /* PASS THROUGH */
        case TK_NEQUAL:
        /* PASS THROUGH */
        case TK_LOGICAND:
        /* PASS THROUGH */
        case '(':
        /* PASS THROUGH */
        case ')':
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;

        /* Some token need record it's value*/
        case TK_DECNUM:
          /* PASS THROUGH */
        case TK_HEXNUM:
          /* PASS THROUGH */
        case TK_uDECNUM:
          /* PASS THROUGH */
        case TK_REGNAME:
          tokens[nr_token].type = rules[i].token_type;
          if (substr_len >= 32) {
            /* src length greater or equal to des, manual truncate token and
             * append '\0' */
            strncpy(tokens[nr_token].str, e + position - substr_len,
                    31); // May lose token's suffix,
            tokens[nr_token].str[31] = '\0';
            Log("The length of numeric token is %d, over maxsize 32, truncated "
                "value are: %s",
                substr_len, tokens[nr_token].str);

          } else {
            strncpy(tokens[nr_token].str, e + position - substr_len, 32);
          }
          nr_token++;
          break;

        /* Token like space don't need any operation */
        default:
          break;
        }
        break; // match once
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

#define MAX_OP_TYPE 255 + 100
static bool computable __attribute__((used)) = true;

/*  op_rank is smaller, priority is higher */
static struct rank {
  int op_type;
  int op_rank;
  int arity;
} ranks[] = {
    {TK_DEREF, 1, 1}, // deference  operand
    {'*', 2, 2},         
	{'/', 2, 2}, 
	{'+', 3, 2},
    {'-', 3, 2},         
	{'=', 4, 2}, //  ==
    {TK_NEQUAL, 4, 2},   // !=
    {TK_LOGICAND, 5, 2},  // &&
                                      // TODO: add more rank of operator.
};

#define NR_RANKS ARRLEN(ranks)

static int ranks_map[MAX_OP_TYPE] = {0};
static int aritys_map[MAX_OP_TYPE] = {0};

/* check whether given expresionn consist of paired parentheses */
static bool check_parentheses_match(int left, int right) {
  char stack[MAX_TOKENS_LEN] = {}; // simulate stack
  int sp = 0;                      // stack pointer

  int i = left;
  while (i <= right) {
    /* left parenthesis pop in */
    if (tokens[i].type == '(') {
      stack[sp++] = '(';
    } else if (tokens[i].type == ')') {
      /* stack empty or sp is not refer to  leftparenthesis*/
      if (!sp || stack[--sp] != '(') {
        return false;
      }
    }
    i++;
  }
  /* there are redundant parenthesis, expr illegal  */
  if (sp != 0) {
    return false;
  }

  return true;
}

static bool check_exist_parenthesis(int left, int right) {
  int i = 0;
  for (i = left; i <= right; i++) {
    if (tokens[i].type == '(' || tokens[i].type == ')') {
      return true;
    }
  }
  return false;
}

/*check whether expression surrounded by a pair of removable parentheses and
 * whether expression computable*/
static bool check_parentheses(int left, int right) {
  /* don't exist any parentheses */
  if (check_exist_parenthesis(left, right) == false) {
    computable = true;
    return false;
  }
  /* illegal expression: parentheses don't match */
  if (check_parentheses_match(left, right) == false) {
    computable = false;
    return false;
  }
  /* There are not surrounded parentheses or not removable */
  else if (tokens[left].type != '(' || tokens[right].type != ')' ||
           check_parentheses_match(left + 1, right - 1) == false) {
    computable = true;
    return false;
  }
  /* surrounded by a pair of match parentheses, can savely remove them */
  computable = true;
  return true;
}

/* init ranks_map */
static void init_rank() {
  for (int i = 0; i < NR_RANKS; i++) {
    ranks_map[ranks[i].op_type] = ranks[i].op_rank;
    aritys_map[ranks[i].op_type] = ranks[i].arity;
  }
}

/* check op priority in tokens[cur] and tokens[last] */
static int check_op_priority(int cur, int last) {
  /* priority of rank(tokens[cur]) is higher than rank(tokens[last]), return 1.
   *							  equal to, return 0.
   *							  lower than
   *rank(tokens[last]), return -1. PS:The smaller the ranks is, the higher the
   *priority is.
   */
  int cur_op = tokens[cur].type;
  int last_op = tokens[last].type;
  if (ranks_map[cur_op] < ranks_map[last_op])
    return 1; // higher
  else if (ranks_map[cur_op] == ranks_map[last_op])
    return 0; // equal
  else
    return -1; // smaller
}

/* check whether token is operator */
static bool check_token_isop(int index) {
  if (tokens[index].type == '+' || tokens[index].type == '-' ||
      tokens[index].type == '*' || tokens[index].type == '/' ||
      tokens[index].type == '=' || tokens[index].type == TK_NEQUAL ||
      tokens[index].type == TK_LOGICAND || tokens[index].type == TK_DEREF) {
    return true;
  }
  return false;
}

/* return the index of main operator, index start from zero */
static int find_mainop(int left, int right) {
  /* in this if branch, expression won't surround by a pair of matched
   * parentheses so, only when bracket depth == 0 we try to update it example:
   * (a+b)*c
   * if we record op when op_idx=-1 and don't care bracket_depth, the maipop
   * turn out to be '+'by judging priority, which is wrong answer.
   */

  int bracket_depth = 0;
  int op_idx = -1;

  for (int i = left; i <= right; i++) {
    if (tokens[i].type == '(') {
      bracket_depth++;
    } else if (tokens[i].type == ')') {
      bracket_depth--;
    }
    /* any op in brack won't be the mainop, no matter what priority it is */
    if (check_token_isop(i) && bracket_depth == 0) {
      /* first time meet op or op priority smaller or equal than previous
       * recorded op, update op index.
       */
      if (op_idx == -1 || check_op_priority(i, op_idx) <= 0) {
        op_idx = i;
      }
    }
  }
  return op_idx;
}

uint8_t *guest_to_host(paddr_t paddr);
word_t isa_reg_str2val(const char *s, bool *success);

word_t deref_addr(uint32_t addr, EvalStatus *status) {
  if (addr < CONFIG_MBASE || addr - CONFIG_MBASE > CONFIG_MSIZE) {
    *status = EVAL_DEREF_INVALID;
    return -1;
  }

  uint8_t *start = guest_to_host(addr);
  word_t value;
  memcpy(&value, start, sizeof(word_t));
  return value;
}

/* eval expression from left to right */
word_t eval(int left, int right, EvalStatus *status) {
  if (left > right) {
    /* bad expression */
    *status = EVAL_BAD_EXPR;
    return -1;
  }
  /* single token, should be a number or reg */
  else if (left == right) {
    /*  regname */
    if (tokens[left].type == TK_REGNAME) {
      bool success = true;
      uint32_t reg_value = (uint32_t)isa_reg_str2val(
          tokens[left].str + 1, &success); // ignore the prefix $
      if (success)
        return reg_value;
      *status = EVAL_ILLEGAL_REGNAME;
      return -1;
    }
    /* number */
    return (uint32_t)strtoul(
        tokens[left].str, NULL,
        0); // assume all input are uint32_t, so choose strtoul
  }
  /* legal parentheses, meanwhile leftmost and rightmost are matched, remove
     them  */
  else if (check_parentheses(left, right) == true) {
    return eval(left + 1, right - 1, status);
  }
  /* not surrounded or surrouned but not matched */
  else if (computable == true) {
    int op_idx = find_mainop(left, right);
    /* Binary operand */
    if (aritys_map[tokens[op_idx].type] == 2) {
      int val1 = eval(left, op_idx - 1, status);
      int val2 = eval(op_idx + 1, right, status);

      if (*status != EVAL_SUCCESS)
        return -1; // illegal expression, status show the reason.

      switch (tokens[op_idx].type) {
      case '+':
        return (uint32_t)val1 + (uint32_t)val2;
      case '-':
        return (uint32_t)val1 - (uint32_t)val2;
      case '*':
        return (uint32_t)val1 * (uint32_t)val2;
      case '/':
        if (val2 == 0) {
          *status = EVAL_DIV_ZERO;
          return -1;
        }
        return (uint32_t)val1 / (uint32_t)val2;
      case '=':
        return val1 == val2;
      case TK_LOGICAND:
        return val1 && val2;
      case TK_NEQUAL:
        return val1 != val2;
      default:
        *status = EVAL_ILLEGAL_OP;
        return -1;
      }
    }
    /* Unary operand */
    else if (aritys_map[tokens[op_idx].type] == 1) {

      int val = eval(op_idx + 1, right, status); // 右结合，所以是op_idx+1

      if (*status != EVAL_SUCCESS)
        return -1;

      switch (tokens[op_idx].type) {
      case TK_DEREF:
        return (uint32_t)deref_addr(val, status);
      default:
        *status = EVAL_ILLEGAL_OP;
        return -1;
      }
    }

    *status = EVAL_ILLEGAL_ARITY;
    return -1;
  }
  /* Not surrounded by parentheses and they don't match up */
  *status = EVAL_ILLEGAL_PARENTHESES;
  return -1;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return -1;
  }

  /* TODO: Insert codes to evaluate the expression. */

  EvalStatus status = EVAL_SUCCESS;
  word_t result;

  if (nr_token == 0) {
    printf("Empty expression");
    *success = false;
    return -1;
  }

  /* Token '*'  counld be dereference  */
  for (int i = 0; i < nr_token; i++) {
    /* in the begining or the previous token is operator */
    if (tokens[i].type == '*' && (i == 0 || check_token_isop(i - 1))) {
      tokens[i].type = TK_DEREF;
    }
  }

  init_rank();
  result = eval(0, nr_token - 1, &status);

  switch (status) {
  case EVAL_SUCCESS:
    *success = true;
    return result;
  case EVAL_BAD_EXPR:
    printf("Command p receive bad expression\n");
    *success = false;
    return -1;
  case EVAL_ILLEGAL_PARENTHESES:
    printf("Expression do not have match parenthesis\n");
    *success = false;
    return -1;
  case EVAL_ILLEGAL_OP:
    printf("Exists undefined operator, which is not one of +,-,*,/\n ");
    *success = false;
    return -1;
  case EVAL_DIV_ZERO:
    printf("Division by zero\n");
    *success = false;
    return -1;
  case EVAL_DEREF_INVALID:
    printf("Dereference an invalid address\n");
    *success = false;
    return -1;
  case EVAL_ILLEGAL_ARITY:
    printf("Token arity illegal\n");
    *success = false;
    return -1;
  case EVAL_ILLEGAL_REGNAME:
    printf("Illegal reg name\n");
    *success = false;
    return -1;
  default:
    printf("Undefined behavior\n");
    *success = false;
    return -1;
  }
  return 0;
}
