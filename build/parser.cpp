/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 5 "src/parser.y"
 
  // C/C++ 头文件与全局声明
  #include <cstdio>
  // yylex 在 lexer.l 里生成
  int yylex(void);        /* 声明即可，别加 extern "C" */
  extern FILE* yyin;

  //Program* g_root = nullptr;
  extern int yylineno;
  void yyerror(const char *s) {
    std::fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
  }

#line 85 "D:/Compiler/Compiler/build/parser.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NUMBER = 3,                     /* NUMBER  */
  YYSYMBOL_IDENTIFIER = 4,                 /* IDENTIFIER  */
  YYSYMBOL_PLUS = 5,                       /* PLUS  */
  YYSYMBOL_MINUS = 6,                      /* MINUS  */
  YYSYMBOL_MULTIPLY = 7,                   /* MULTIPLY  */
  YYSYMBOL_DIVIDE = 8,                     /* DIVIDE  */
  YYSYMBOL_PERCENT = 9,                    /* PERCENT  */
  YYSYMBOL_EXCLAPOINT = 10,                /* EXCLAPOINT  */
  YYSYMBOL_EQ = 11,                        /* EQ  */
  YYSYMBOL_NEQ = 12,                       /* NEQ  */
  YYSYMBOL_LE = 13,                        /* LE  */
  YYSYMBOL_GE = 14,                        /* GE  */
  YYSYMBOL_LT = 15,                        /* LT  */
  YYSYMBOL_GT = 16,                        /* GT  */
  YYSYMBOL_OR = 17,                        /* OR  */
  YYSYMBOL_AND = 18,                       /* AND  */
  YYSYMBOL_ASSIGN = 19,                    /* ASSIGN  */
  YYSYMBOL_LPAREN = 20,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 21,                    /* RPAREN  */
  YYSYMBOL_SEMI = 22,                      /* SEMI  */
  YYSYMBOL_LBRACE = 23,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 24,                    /* RBRACE  */
  YYSYMBOL_COMMA = 25,                     /* COMMA  */
  YYSYMBOL_INT = 26,                       /* INT  */
  YYSYMBOL_VOID = 27,                      /* VOID  */
  YYSYMBOL_IF = 28,                        /* IF  */
  YYSYMBOL_ELSE = 29,                      /* ELSE  */
  YYSYMBOL_WHILE = 30,                     /* WHILE  */
  YYSYMBOL_BREAK = 31,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 32,                  /* CONTINUE  */
  YYSYMBOL_RETURN = 33,                    /* RETURN  */
  YYSYMBOL_ERROR = 34,                     /* ERROR  */
  YYSYMBOL_LOWER_THAN_ELSE = 35,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_YYACCEPT = 36,                  /* $accept  */
  YYSYMBOL_program = 37,                   /* program  */
  YYSYMBOL_FuncDef_list = 38,              /* FuncDef_list  */
  YYSYMBOL_statement = 39,                 /* statement  */
  YYSYMBOL_statement_list = 40,            /* statement_list  */
  YYSYMBOL_Block = 41,                     /* Block  */
  YYSYMBOL_FuncDef = 42,                   /* FuncDef  */
  YYSYMBOL_return_type = 43,               /* return_type  */
  YYSYMBOL_param_list_opt = 44,            /* param_list_opt  */
  YYSYMBOL_param_list = 45,                /* param_list  */
  YYSYMBOL_param = 46,                     /* param  */
  YYSYMBOL_expression = 47,                /* expression  */
  YYSYMBOL_LOrexpr = 48,                   /* LOrexpr  */
  YYSYMBOL_LAndexpr = 49,                  /* LAndexpr  */
  YYSYMBOL_Relexpr = 50,                   /* Relexpr  */
  YYSYMBOL_Addexpr = 51,                   /* Addexpr  */
  YYSYMBOL_Mulexpr = 52,                   /* Mulexpr  */
  YYSYMBOL_Unaryexpr = 53,                 /* Unaryexpr  */
  YYSYMBOL_Primaryexpr = 54,               /* Primaryexpr  */
  YYSYMBOL_expr_list_opt = 55,             /* expr_list_opt  */
  YYSYMBOL_expr_list = 56                  /* expr_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   120

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  36
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  21
/* YYNRULES -- Number of rules.  */
#define YYNRULES  58
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  108

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   290


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    83,    83,    93,    98,   107,   108,   109,   110,   114,
     119,   123,   127,   131,   133,   134,   135,   138,   139,   143,
     156,   168,   169,   173,   174,   178,   183,   191,   201,   205,
     206,   210,   211,   215,   216,   217,   218,   219,   220,   221,
     225,   226,   227,   231,   232,   233,   234,   238,   239,   240,
     241,   245,   250,   254,   258,   268,   269,   273,   278
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NUMBER", "IDENTIFIER",
  "PLUS", "MINUS", "MULTIPLY", "DIVIDE", "PERCENT", "EXCLAPOINT", "EQ",
  "NEQ", "LE", "GE", "LT", "GT", "OR", "AND", "ASSIGN", "LPAREN", "RPAREN",
  "SEMI", "LBRACE", "RBRACE", "COMMA", "INT", "VOID", "IF", "ELSE",
  "WHILE", "BREAK", "CONTINUE", "RETURN", "ERROR", "LOWER_THAN_ELSE",
  "$accept", "program", "FuncDef_list", "statement", "statement_list",
  "Block", "FuncDef", "return_type", "param_list_opt", "param_list",
  "param", "expression", "LOrexpr", "LAndexpr", "Relexpr", "Addexpr",
  "Mulexpr", "Unaryexpr", "Primaryexpr", "expr_list_opt", "expr_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-97)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -20,   -97,   -97,     9,   -20,   -97,    14,   -97,   -97,     2,
       3,    28,    15,    13,   -97,   -97,    24,     3,   -97,   -97,
     -97,    11,   -97,    26,    20,    20,    20,    20,   -97,   -97,
      47,    39,    41,    53,    55,    81,   -97,   -97,    57,    71,
      56,    51,    52,     4,   -97,   -97,    20,    20,    69,   -97,
     -97,   -97,    77,    80,    20,    20,   -97,   -97,   -97,    68,
     -97,    20,    20,    20,    20,    20,    20,    20,    20,    20,
      20,    20,    20,    20,    78,   -97,    83,    82,   -97,    20,
      84,    85,   -97,    56,    51,    52,    52,    52,    52,    52,
      52,     4,     4,   -97,   -97,   -97,   -97,   -97,    20,    86,
      50,    50,   -97,   -97,    73,   -97,    50,   -97
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    21,    22,     0,     2,     3,     0,     1,     4,     0,
      23,     0,     0,    24,    26,    27,     0,     0,    17,    20,
      25,     0,    52,    51,     0,     0,     0,     0,     6,    19,
       0,     0,     0,     0,     0,     0,    18,     5,     0,    28,
      29,    31,    33,    40,    43,    47,     0,    55,    51,    48,
      49,    50,     0,     0,     0,     0,    13,    14,    15,     0,
       7,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    58,     0,    56,    53,     0,
       0,     0,    16,    30,    32,    38,    39,    36,    37,    34,
      35,    41,    42,    44,    45,    46,     8,    54,     0,     0,
       0,     0,    57,     9,    10,    12,     0,    11
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -97,   -97,   -97,   -96,   -97,    93,   106,   -97,   -97,   -97,
      94,   -27,   -97,    54,    58,    29,    -1,   -23,   -97,   -97,
     -97
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     3,     4,    36,    21,    37,     5,     6,    12,    13,
      14,    38,    39,    40,    41,    42,    43,    44,    45,    76,
      77
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      52,    49,    50,    51,   104,   105,     1,     2,    59,     7,
     107,    71,    72,    73,    22,    23,    24,    25,     9,    74,
      75,    26,    10,    22,    48,    24,    25,    80,    81,    11,
      26,    27,    15,    28,    18,    29,    16,    30,    17,    31,
      27,    32,    33,    34,    35,    46,    47,    18,    93,    94,
      95,    53,    99,    22,    23,    24,    25,    69,    70,    54,
      26,    55,    63,    64,    65,    66,    67,    68,    91,    92,
      27,   102,    28,    18,    62,    56,    30,    57,    31,    60,
      32,    33,    34,    35,    22,    48,    24,    25,    61,    47,
      82,    26,    85,    86,    87,    88,    89,    90,    78,    79,
      96,    27,   106,    58,    97,   100,   101,    98,   103,    19,
       8,    20,     0,     0,     0,    83,     0,     0,     0,     0,
      84
};

static const yytype_int8 yycheck[] =
{
      27,    24,    25,    26,   100,   101,    26,    27,    35,     0,
     106,     7,     8,     9,     3,     4,     5,     6,     4,    46,
      47,    10,    20,     3,     4,     5,     6,    54,    55,    26,
      10,    20,     4,    22,    23,    24,    21,    26,    25,    28,
      20,    30,    31,    32,    33,    19,    20,    23,    71,    72,
      73,     4,    79,     3,     4,     5,     6,     5,     6,    20,
      10,    20,    11,    12,    13,    14,    15,    16,    69,    70,
      20,    98,    22,    23,    18,    22,    26,    22,    28,    22,
      30,    31,    32,    33,     3,     4,     5,     6,    17,    20,
      22,    10,    63,    64,    65,    66,    67,    68,    21,    19,
      22,    20,    29,    22,    21,    21,    21,    25,    22,    16,
       4,    17,    -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,
      62
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    26,    27,    37,    38,    42,    43,     0,    42,     4,
      20,    26,    44,    45,    46,     4,    21,    25,    23,    41,
      46,    40,     3,     4,     5,     6,    10,    20,    22,    24,
      26,    28,    30,    31,    32,    33,    39,    41,    47,    48,
      49,    50,    51,    52,    53,    54,    19,    20,     4,    53,
      53,    53,    47,     4,    20,    20,    22,    22,    22,    47,
      22,    17,    18,    11,    12,    13,    14,    15,    16,     5,
       6,     7,     8,     9,    47,    47,    55,    56,    21,    19,
      47,    47,    22,    49,    50,    51,    51,    51,    51,    51,
      51,    52,    52,    53,    53,    53,    22,    21,    25,    47,
      21,    21,    47,    22,    39,    39,    29,    39
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    36,    37,    38,    38,    39,    39,    39,    39,    39,
      39,    39,    39,    39,    39,    39,    39,    40,    40,    41,
      42,    43,    43,    44,    44,    45,    45,    46,    47,    48,
      48,    49,    49,    50,    50,    50,    50,    50,    50,    50,
      51,    51,    51,    52,    52,    52,    52,    53,    53,    53,
      53,    54,    54,    54,    54,    55,    55,    56,    56
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     2,     4,     5,
       5,     7,     5,     2,     2,     2,     3,     0,     2,     3,
       6,     1,     1,     0,     1,     3,     1,     2,     1,     1,
       3,     1,     3,     1,     3,     3,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     3,     3,     1,     2,     2,
       2,     1,     1,     3,     4,     0,     1,     3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_IDENTIFIER: /* IDENTIFIER  */
#line 72 "src/parser.y"
            { free(((*yyvaluep).strval)); }
#line 938 "D:/Compiler/Compiler/build/parser.cpp"
        break;

    case YYSYMBOL_FuncDef_list: /* FuncDef_list  */
#line 73 "src/parser.y"
            { delete ((*yyvaluep).funcs); }
#line 944 "D:/Compiler/Compiler/build/parser.cpp"
        break;

    case YYSYMBOL_statement_list: /* statement_list  */
#line 73 "src/parser.y"
            { delete ((*yyvaluep).stmts); }
#line 950 "D:/Compiler/Compiler/build/parser.cpp"
        break;

    case YYSYMBOL_param_list_opt: /* param_list_opt  */
#line 73 "src/parser.y"
            { delete ((*yyvaluep).params); }
#line 956 "D:/Compiler/Compiler/build/parser.cpp"
        break;

    case YYSYMBOL_param_list: /* param_list  */
#line 73 "src/parser.y"
            { delete ((*yyvaluep).params); }
#line 962 "D:/Compiler/Compiler/build/parser.cpp"
        break;

    case YYSYMBOL_expr_list_opt: /* expr_list_opt  */
#line 73 "src/parser.y"
            { delete ((*yyvaluep).args); }
#line 968 "D:/Compiler/Compiler/build/parser.cpp"
        break;

    case YYSYMBOL_expr_list: /* expr_list  */
#line 73 "src/parser.y"
            { delete ((*yyvaluep).args); }
#line 974 "D:/Compiler/Compiler/build/parser.cpp"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: FuncDef_list  */
#line 84 "src/parser.y"
      {
      Program* p = arena_make<Program>();
      if ((yyvsp[0].funcs)) { p->funcs.swap(*(yyvsp[0].funcs)); delete (yyvsp[0].funcs); }
      (yyval.program) = p;
      g_root = p;
    }
#line 1249 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 3: /* FuncDef_list: FuncDef  */
#line 94 "src/parser.y"
      {
      (yyval.funcs) = new std::vector<FuncDef*>();
      (yyval.funcs)->push_back((yyvsp[0].func));
    }
#line 1258 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 4: /* FuncDef_list: FuncDef_list FuncDef  */
#line 99 "src/parser.y"
    {
      (yyval.funcs) = (yyvsp[-1].funcs);//$$为最左的FuncDef规约得到的FuncDef_list
      (yyval.funcs)->push_back((yyvsp[0].func));//压入
    }
#line 1267 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 5: /* statement: Block  */
#line 107 "src/parser.y"
            { (yyval.stmt) = (yyvsp[0].block); }
#line 1273 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 6: /* statement: SEMI  */
#line 108 "src/parser.y"
            { (yyval.stmt) = nullptr; }
#line 1279 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 7: /* statement: expression SEMI  */
#line 109 "src/parser.y"
                       { (yyval.stmt) = arena_make<ExprStmt>((yyvsp[-1].expr)); }
#line 1285 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 8: /* statement: IDENTIFIER ASSIGN expression SEMI  */
#line 110 "src/parser.y"
                                        {
      (yyval.stmt) = arena_make<AssignStmt>(std::string((yyvsp[-3].strval)), (yyvsp[-1].expr));
      free((yyvsp[-3].strval));
    }
#line 1294 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 9: /* statement: INT IDENTIFIER ASSIGN expression SEMI  */
#line 115 "src/parser.y"
     {
      /* 局部变量类型固定为 int */
      (yyval.stmt) = arena_make<DeclStmt>(std::string((yyvsp[-3].strval)), (yyvsp[-1].expr)); free((yyvsp[-3].strval));
    }
#line 1303 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 10: /* statement: IF LPAREN expression RPAREN statement  */
#line 120 "src/parser.y"
     {
      (yyval.stmt) = arena_make<IfStmt>((yyvsp[-2].expr), (yyvsp[0].stmt), nullptr);
    }
#line 1311 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 11: /* statement: IF LPAREN expression RPAREN statement ELSE statement  */
#line 124 "src/parser.y"
     {
      (yyval.stmt) = arena_make<IfStmt>((yyvsp[-4].expr), (yyvsp[-2].stmt), (yyvsp[0].stmt));
    }
#line 1319 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 12: /* statement: WHILE LPAREN expression RPAREN statement  */
#line 128 "src/parser.y"
     {
      (yyval.stmt) = arena_make<WhileStmt>((yyvsp[-2].expr), (yyvsp[0].stmt));
    }
#line 1327 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 13: /* statement: BREAK SEMI  */
#line 132 "src/parser.y"
      { (yyval.stmt) = arena_make<BreakStmt>(); }
#line 1333 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 14: /* statement: CONTINUE SEMI  */
#line 133 "src/parser.y"
                     { (yyval.stmt) = arena_make<ContinueStmt>(); }
#line 1339 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 15: /* statement: RETURN SEMI  */
#line 134 "src/parser.y"
                   { (yyval.stmt) = arena_make<ReturnStmt>(nullptr); }
#line 1345 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 16: /* statement: RETURN expression SEMI  */
#line 135 "src/parser.y"
                               { (yyval.stmt) = arena_make<ReturnStmt>((yyvsp[-1].expr)); }
#line 1351 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 17: /* statement_list: %empty  */
#line 138 "src/parser.y"
          { (yyval.stmts) = new std::vector<Stmt*>(); }
#line 1357 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 18: /* statement_list: statement_list statement  */
#line 139 "src/parser.y"
                              { (yyval.stmts) = (yyvsp[-1].stmts); if ((yyvsp[0].stmt)) (yyval.stmts)->push_back((yyvsp[0].stmt)); }
#line 1363 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 19: /* Block: LBRACE statement_list RBRACE  */
#line 144 "src/parser.y"
      {
      Block* b = arena_make<Block>();
      if ((yyvsp[-1].stmts)) {
        for (auto* s : *(yyvsp[-1].stmts)) if (s) b->stmts.push_back(s);
        delete (yyvsp[-1].stmts);
      }
      (yyval.block) = b;
    }
#line 1376 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 20: /* FuncDef: return_type IDENTIFIER LPAREN param_list_opt RPAREN Block  */
#line 157 "src/parser.y"
      {
      FuncDef* f = arena_make<FuncDef>();
      f->ret = (yyvsp[-5].type_val);
      f->name = std::string((yyvsp[-4].strval)); free((yyvsp[-4].strval));
      if ((yyvsp[-2].params)) { f->params.swap(*(yyvsp[-2].params)); delete (yyvsp[-2].params); }
      f->body = (yyvsp[0].block);
      (yyval.func) = f;
    }
#line 1389 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 21: /* return_type: INT  */
#line 168 "src/parser.y"
         { (yyval.type_val) = TypeKind::TY_INT; }
#line 1395 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 22: /* return_type: VOID  */
#line 169 "src/parser.y"
          { (yyval.type_val) = TypeKind::TY_VOID; }
#line 1401 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 23: /* param_list_opt: %empty  */
#line 173 "src/parser.y"
            { (yyval.params) = new std::vector<Param*>(); }
#line 1407 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 24: /* param_list_opt: param_list  */
#line 174 "src/parser.y"
                { (yyval.params) = (yyvsp[0].params); }
#line 1413 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 25: /* param_list: param_list COMMA param  */
#line 179 "src/parser.y"
      {
      (yyval.params) = (yyvsp[-2].params);
      (yyval.params)->push_back((yyvsp[0].param));
    }
#line 1422 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 26: /* param_list: param  */
#line 184 "src/parser.y"
    {
      (yyval.params) = new std::vector<Param*>();
      (yyval.params)->push_back((yyvsp[0].param));
    }
#line 1431 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 27: /* param: INT IDENTIFIER  */
#line 192 "src/parser.y"
      {
      Param* p = arena_make<Param>();
      p->type_val = TypeKind::TY_INT;
      p->name = std::string((yyvsp[0].strval)); free((yyvsp[0].strval));
      (yyval.param) = p;
    }
#line 1442 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 28: /* expression: LOrexpr  */
#line 201 "src/parser.y"
             { (yyval.expr) = (yyvsp[0].expr); }
#line 1448 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 29: /* LOrexpr: LAndexpr  */
#line 205 "src/parser.y"
               { (yyval.expr) = (yyvsp[0].expr); }
#line 1454 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 30: /* LOrexpr: LOrexpr OR LAndexpr  */
#line 206 "src/parser.y"
                         { (yyval.expr) = arena_make<BinaryExpr>(BinOp::LOr,  (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1460 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 31: /* LAndexpr: Relexpr  */
#line 210 "src/parser.y"
             { (yyval.expr) = (yyvsp[0].expr); }
#line 1466 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 32: /* LAndexpr: LAndexpr AND Relexpr  */
#line 211 "src/parser.y"
                          { (yyval.expr) = arena_make<BinaryExpr>(BinOp::LAnd, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1472 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 33: /* Relexpr: Addexpr  */
#line 215 "src/parser.y"
             { (yyval.expr) = (yyvsp[0].expr); }
#line 1478 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 34: /* Relexpr: Relexpr LT Addexpr  */
#line 216 "src/parser.y"
                         { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Lt,  (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1484 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 35: /* Relexpr: Relexpr GT Addexpr  */
#line 217 "src/parser.y"
                         { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Gt,  (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1490 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 36: /* Relexpr: Relexpr LE Addexpr  */
#line 218 "src/parser.y"
                         { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Le,  (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1496 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 37: /* Relexpr: Relexpr GE Addexpr  */
#line 219 "src/parser.y"
                         { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Ge,  (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1502 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 38: /* Relexpr: Relexpr EQ Addexpr  */
#line 220 "src/parser.y"
                         { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Eq,  (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1508 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 39: /* Relexpr: Relexpr NEQ Addexpr  */
#line 221 "src/parser.y"
                          { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Neq, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1514 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 40: /* Addexpr: Mulexpr  */
#line 225 "src/parser.y"
             { (yyval.expr) = (yyvsp[0].expr); }
#line 1520 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 41: /* Addexpr: Addexpr PLUS Mulexpr  */
#line 226 "src/parser.y"
                          { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Add, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1526 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 42: /* Addexpr: Addexpr MINUS Mulexpr  */
#line 227 "src/parser.y"
                           { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Sub, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1532 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 43: /* Mulexpr: Unaryexpr  */
#line 231 "src/parser.y"
               { (yyval.expr) = (yyvsp[0].expr); }
#line 1538 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 44: /* Mulexpr: Mulexpr MULTIPLY Unaryexpr  */
#line 232 "src/parser.y"
                                 { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Mul, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1544 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 45: /* Mulexpr: Mulexpr DIVIDE Unaryexpr  */
#line 233 "src/parser.y"
                               { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Div, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1550 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 46: /* Mulexpr: Mulexpr PERCENT Unaryexpr  */
#line 234 "src/parser.y"
                                { (yyval.expr) = arena_make<BinaryExpr>(BinOp::Mod, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1556 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 47: /* Unaryexpr: Primaryexpr  */
#line 238 "src/parser.y"
                  { (yyval.expr) = (yyvsp[0].expr); }
#line 1562 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 48: /* Unaryexpr: PLUS Unaryexpr  */
#line 239 "src/parser.y"
                     { (yyval.expr) = arena_make<UnaryExpr>(UnOp::Pos, (yyvsp[0].expr)); }
#line 1568 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 49: /* Unaryexpr: MINUS Unaryexpr  */
#line 240 "src/parser.y"
                      { (yyval.expr) = arena_make<UnaryExpr>(UnOp::Neg, (yyvsp[0].expr)); }
#line 1574 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 50: /* Unaryexpr: EXCLAPOINT Unaryexpr  */
#line 241 "src/parser.y"
                           { (yyval.expr) = arena_make<UnaryExpr>(UnOp::Not, (yyvsp[0].expr)); }
#line 1580 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 51: /* Primaryexpr: IDENTIFIER  */
#line 246 "src/parser.y"
      {
      (yyval.expr) = arena_make<VarExpr>(std::string((yyvsp[0].strval))); free((yyvsp[0].strval));
    }
#line 1588 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 52: /* Primaryexpr: NUMBER  */
#line 251 "src/parser.y"
    {
      (yyval.expr) = arena_make<IntLiteral>((yyvsp[0].intval));
    }
#line 1596 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 53: /* Primaryexpr: LPAREN expression RPAREN  */
#line 255 "src/parser.y"
    {
      (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1604 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 54: /* Primaryexpr: IDENTIFIER LPAREN expr_list_opt RPAREN  */
#line 259 "src/parser.y"
    {
      CallExpr* c = arena_make<CallExpr>();
      c->callee = std::string((yyvsp[-3].strval)); free((yyvsp[-3].strval));
      if ((yyvsp[-1].args)) { c->args.swap(*(yyvsp[-1].args)); delete (yyvsp[-1].args); }
      (yyval.expr) = c;
    }
#line 1615 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 55: /* expr_list_opt: %empty  */
#line 268 "src/parser.y"
            { (yyval.args) = new std::vector<Expr*>(); }
#line 1621 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 56: /* expr_list_opt: expr_list  */
#line 269 "src/parser.y"
               { (yyval.args) = (yyvsp[0].args); }
#line 1627 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 57: /* expr_list: expr_list COMMA expression  */
#line 274 "src/parser.y"
      {
      (yyval.args) = (yyvsp[-2].args);
      (yyval.args)->push_back((yyvsp[0].expr));
    }
#line 1636 "D:/Compiler/Compiler/build/parser.cpp"
    break;

  case 58: /* expr_list: expression  */
#line 279 "src/parser.y"
    {
      (yyval.args) = new std::vector<Expr*>();
      (yyval.args)->push_back((yyvsp[0].expr));
    }
#line 1645 "D:/Compiler/Compiler/build/parser.cpp"
    break;


#line 1649 "D:/Compiler/Compiler/build/parser.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 284 "src/parser.y"


/* （可选）提供一个入口 main 给单独测试用 */
/* 
int main() {
  return yyparse();
}
*/
