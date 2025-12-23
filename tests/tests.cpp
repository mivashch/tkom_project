#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "ast_dump.h"

using namespace minilang;
using namespace minilang::ast;

static std::string parseAndDump(const std::string& src)
{
    Lexer lx(makeStringSource(src));
    Parser p(lx);
    std::unique_ptr<Program> program = p.parseProgram();
    if (!program) {
        return "ERROR: " + (p.lastError().value_or("<unknown>"));
    }
    std::stringstream result;
    AstPrinter printer(result);
    printer.dump(*program);
    return result.str();
}

static void expectAST(const std::string& src, const std::string& expected)
{
    std::string out = parseAndDump(src);
    EXPECT_EQ(out, expected);
}

TEST(ParserTest, LiteralInteger) {
    expectAST("42;",
R"(Program:
  ExprStmt:
    Literal(42)
)");
}

TEST(ParserTest, LiteralFloat) {
    expectAST("3.14;",
R"(Program:
  ExprStmt:
    Literal(3.14)
)");
}

TEST(ParserTest, LiteralString) {
    expectAST("\"hello\";",
R"(Program:
  ExprStmt:
    Literal("hello")
)");
}

TEST(ParserTest, LiteralBool) {
    expectAST("true;",
R"(Program:
  ExprStmt:
    Literal(true)
)");
}

TEST(ParserTest, UnaryMinus) {
    expectAST("-x;",
R"(Program:
  ExprStmt:
    Unary(-)
      Identifier(x)
)");
}

TEST(ParserTest, BinaryAddMultiplyPrecedence) {
    expectAST("a + b * c;",
R"(Program:
  ExprStmt:
    Binary('+')
      Identifier(a)
      Binary('*')
        Identifier(b)
        Identifier(c)
)");
}

TEST(ParserTest, ComplexPrecedence) {
    expectAST("a && b || c;",
R"(Program:
  ExprStmt:
    Binary('||')
      Binary('&&')
        Identifier(a)
        Identifier(b)
      Identifier(c)
)");
}

TEST(ParserTest, CustomOperators) {
    expectAST("x &*& y =>> z;",
R"(Program:
  ExprStmt:
    Binary('=>>')
      Binary('&*&')
        Identifier(x)
        Identifier(y)
      Identifier(z)
)");
}

TEST(ParserTest, SimpleCall) {
    expectAST("f(1,2);",
R"(Program:
  ExprStmt:
    Call:
      Callee:
        Identifier(f)
      Args:
        Literal(1)
        Literal(2)
)");
}

TEST(ParserTest, NestedCall) {
    expectAST("f(g(1), h(2,3));",
R"(Program:
  ExprStmt:
    Call:
      Callee:
        Identifier(f)
      Args:
        Call:
          Callee:
            Identifier(g)
          Args:
            Literal(1)
        Call:
          Callee:
            Identifier(h)
          Args:
            Literal(2)
            Literal(3)
)");
}

TEST(ParserTest, VarDeclSimple) {
    expectAST("x = 10;",
R"(Program:
  ExprStmt:
    Assign(x)
      Literal(10)
)");
}

TEST(ParserTest, ConstVarDecl) {
    expectAST("const y = 2;",
R"(Program:
  VarDecl(const y)
    Literal(2)
)");
}

TEST(ParserTest, AssignStmt) {
    expectAST("x = y;",
R"(Program:
  ExprStmt:
    Assign(x)
      Identifier(y)
)");
}

TEST(ParserTest, FuncDeclNoArgs) {
    expectAST("fun int foo() { return 1; }",
R"(Program:
  FuncDecl(int foo())
    Block:
      Return:
        Literal(1)
)");
}

TEST(ParserTest, FuncDeclWithArgs) {
    expectAST("fun float add(a:int, b:float) { return a + b; }",
R"(Program:
  FuncDecl(float add(a:int, b:float))
    Block:
      Return:
        Binary('+')
          Identifier(a)
          Identifier(b)
)");
}

TEST(ParserTest, IfSimple) {
    expectAST("if (x) { y = 1; }",
R"(Program:
  If:
    Cond:
      Identifier(x)
    Then:
      Block:
        ExprStmt:
          Assign(y)
            Literal(1)
)");
}

TEST(ParserTest, IfElse) {
    expectAST("if (x) { y = 1; } else { y = 2; }",
R"(Program:
  If:
    Cond:
      Identifier(x)
    Then:
      Block:
        ExprStmt:
          Assign(y)
            Literal(1)
    Else:
      Block:
        ExprStmt:
          Assign(y)
            Literal(2)
)");
}

TEST(ParserTest, ForLoop) {
  expectAST(
      "for (i = 0; i < 10; i = i + 1) { x = x + i; }",
R"(Program:
  For:
    Init:
      Assign(i)
        Literal(0)
    Cond:
      Binary('<')
        Identifier(i)
        Literal(10)
    Post:
      Assign(i)
        Binary('+')
          Identifier(i)
          Literal(1)
    Body:
      Block:
        ExprStmt:
          Assign(x)
            Binary('+')
              Identifier(x)
              Identifier(i)
)"
  );
}


TEST(ParserTest, BlockMultiple) {
    expectAST("{ a = 1; b = 2; }",
R"(Program:
  Block:
    ExprStmt:
      Assign(a)
        Literal(1)
    ExprStmt:
      Assign(b)
        Literal(2)
)");
}

TEST(ParserTest, ParenthesisPrecedence) {
    expectAST("(a + b) * c;",
R"(Program:
  ExprStmt:
    Binary('*')
      Binary('+')
        Identifier(a)
        Identifier(b)
      Identifier(c)
)");
}

TEST(ParserTest, ErrorMissingSemicolon) {
  EXPECT_THROW({
      parseAndDump("x = 1");
  }, ParseError);
}

TEST(ParserTest, ErrorBadExpr) {
  EXPECT_THROW({
    parseAndDump("x = * 10;");
}, ParseError);
}

TEST(ParserTest, ErrorBadFunction) {
  EXPECT_THROW({
  parseAndDump("fun int f( { }");
}, ParseError);
}

TEST(ParserTest, ComparisonOperators) {
  expectAST("a == b; a != b; a < b; a <= b; a > b; a >= b;",
R"(Program:
  ExprStmt:
    Binary('==')
      Identifier(a)
      Identifier(b)
  ExprStmt:
    Binary('!=')
      Identifier(a)
      Identifier(b)
  ExprStmt:
    Binary('<')
      Identifier(a)
      Identifier(b)
  ExprStmt:
    Binary('<=')
      Identifier(a)
      Identifier(b)
  ExprStmt:
    Binary('>')
      Identifier(a)
      Identifier(b)
  ExprStmt:
    Binary('>=')
      Identifier(a)
      Identifier(b)
)");
}


TEST(ParserTest, UnaryMinusBinary) {
  expectAST("-a * -b;",
R"(Program:
  ExprStmt:
    Binary('*')
      Unary(-)
        Identifier(a)
      Unary(-)
        Identifier(b)
)");
}

TEST(ParserTest, EmptyBlock) {
  expectAST("{ }",
R"(Program:
  Block:
)");
}

TEST(ParserTest, NestedBlocks) {
  expectAST("{ { x = 1; } }",
R"(Program:
  Block:
    Block:
      ExprStmt:
        Assign(x)
          Literal(1)
)");
}

TEST(ParserTest, NestedIf) {
  expectAST("if (a) { if (b) { c = 1; } }",
R"(Program:
  If:
    Cond:
      Identifier(a)
    Then:
      Block:
        If:
          Cond:
            Identifier(b)
          Then:
            Block:
              ExprStmt:
                Assign(c)
                  Literal(1)
)");
}

TEST(ParserTest, NestedFor) {
  expectAST(
      "for (i = 0; i < 3; i = i + 1) { for (j = 0; j < 2; j = j + 1) { x = i; } }",
R"(Program:
  For:
    Init:
      Assign(i)
        Literal(0)
    Cond:
      Binary('<')
        Identifier(i)
        Literal(3)
    Post:
      Assign(i)
        Binary('+')
          Identifier(i)
          Literal(1)
    Body:
      Block:
        For:
          Init:
            Assign(j)
              Literal(0)
          Cond:
            Binary('<')
              Identifier(j)
              Literal(2)
          Post:
            Assign(j)
              Binary('+')
                Identifier(j)
                Literal(1)
          Body:
            Block:
              ExprStmt:
                Assign(x)
                  Identifier(i)
)");
}

TEST(ParserTest, MultipleStatements) {
  expectAST("a = 1; b = 2; c = 3;",
R"(Program:
  ExprStmt:
    Assign(a)
      Literal(1)
  ExprStmt:
    Assign(b)
      Literal(2)
  ExprStmt:
    Assign(c)
      Literal(3)
)");
}

TEST(ParserTest, CallNoArgs) {
  expectAST("foo();",
R"(Program:
  ExprStmt:
    Call:
      Callee:
        Identifier(foo)
      Args:
)");
}

TEST(ParserTest, CallChain) {
  expectAST("a()(b);",
R"(Program:
  ExprStmt:
    Call:
      Callee:
        Call:
          Callee:
            Identifier(a)
          Args:
      Args:
        Identifier(b)
)");
}

TEST(ParserTest, ErrorIfMissingCond) {
  EXPECT_THROW({
    parseAndDump("if () { }");
  }, ParseError);
}


TEST(ParserTest, ErrorUnclosedBlock) {
  EXPECT_THROW({
parseAndDump("{ x = 1;");
}, ParseError);
}

TEST(ParserTest, ErrorTrailingCommaCall) {
  EXPECT_THROW({
parseAndDump("f(1,);");
}, ParseError);
}

TEST(ParserTest, EmptyProgram) {
  expectAST("",
R"(Program:
)");
}

TEST(ParserTest, ErrorAssignInCondition) {
  EXPECT_THROW({
parseAndDump("if (a = b) { }");
}, ParseError);
}

TEST(ParserTest, ErrorMissingCommaArgs) {
  EXPECT_THROW({
parseAndDump("f(1 2);");
}, ParseError);
}

TEST(ParserEdgeTest, EmptyProgram) {
  expectAST("",
R"(Program:
)");
}

TEST(ParserEdgeTest, OnlySemicolons) {
  expectAST(";;;;",
R"(Program:
)");
}

TEST(ParserEdgeTest, DeepParentheses) {
  expectAST("((((x))));",
R"(Program:
  ExprStmt:
    Identifier(x)
)");
}
TEST(ParserEdgeTest, CallChainNoArgs) {
  expectAST("f()()();",
R"(Program:
  ExprStmt:
    Call:
      Callee:
        Call:
          Callee:
            Call:
              Callee:
                Identifier(f)
              Args:
          Args:
      Args:
)");
}


TEST(ParserEdgeTest, ForMinimal) {
  expectAST("for (;;){ }",
R"(Program:
  For:
    Init:
    Cond:
    Post:
    Body:
      Block:
)");
}

TEST(ParserEdgeTest, IfEmptyBlock) {
  expectAST("if (true) { }",
R"(Program:
  If:
    Cond:
      Literal(true)
    Then:
      Block:
)");

}

TEST(ParserEdgeTest, ReturnSimple) {
  expectAST("fun int f(){ return 0; }",
R"(Program:
  FuncDecl(int f())
    Block:
      Return:
        Literal(0)
)");
}

TEST(ParserNegativeTest, ErrorUnclosedParen) {
  EXPECT_THROW({
parseAndDump("(a + b;");
}, ParseError);
}

TEST(ParserNegativeTest, ErrorUnclosedBrace) {
  EXPECT_THROW({
parseAndDump("if (x) { y = 1;");
}, ParseError);
}

TEST(ParserNegativeTest, ErrorDoubleOperator) {
  EXPECT_THROW({
parseAndDump("a + * b;");
}, ParseError);
}

TEST(ParserNegativeTest, ErrorAssignMissingRHS) {
  EXPECT_THROW({
parseAndDump("x = ;");
}, ParseError);
}


TEST(ParserNegativeTest, ErrorForMissingSemicolon) {
  EXPECT_THROW({
parseAndDump("for (i=0 i<10; i=i+1){}");
}, ParseError);
}

TEST(ParserNegativeTest, ErrorTrailingCommaCall) {
  EXPECT_THROW({
parseAndDump("f(1,2,);");
}, ParseError);
}

TEST(ParserNegativeTest, ErrorBadParamType) {
  EXPECT_THROW({
parseAndDump("fun int f(a:){ }");
}, ParseError);
}

TEST(ParserExtraTest, DoubleUnaryMinus) {
  expectAST("--x;",
R"(Program:
  ExprStmt:
    Unary(-)
      Unary(-)
        Identifier(x)
)");
}

TEST(ParserExtraTest, UnaryWithBinary) {
  expectAST("-a * b;",
R"(Program:
  ExprStmt:
    Binary('*')
      Unary(-)
        Identifier(a)
      Identifier(b)
)");
}

TEST(ParserExtraTest, LongArithmeticChain) {
  expectAST("a + b - c + d;",
R"(Program:
  ExprStmt:
    Binary('+')
      Binary('-')
        Binary('+')
          Identifier(a)
          Identifier(b)
        Identifier(c)
      Identifier(d)
)");
}

TEST(ParserExtraTest, ModuloPrecedence) {
  expectAST("a % b * c;",
R"(Program:
  ExprStmt:
    Binary('*')
      Binary('%')
        Identifier(a)
        Identifier(b)
      Identifier(c)
)");
}

TEST(ParserExtraTest, ComparisonLogic) {
  expectAST("a < b && b < c;",
R"(Program:
  ExprStmt:
    Binary('&&')
      Binary('<')
        Identifier(a)
        Identifier(b)
      Binary('<')
        Identifier(b)
        Identifier(c)
)");
}

TEST(ParserExtraTest, AllComparisons) {
  expectAST("a <= b || c >= d;",
R"(Program:
  ExprStmt:
    Binary('||')
      Binary('<=')
        Identifier(a)
        Identifier(b)
      Binary('>=')
        Identifier(c)
        Identifier(d)
)");
}


TEST(ParserExtraTest, BlockInBlock) {
  expectAST("{ { a = 1; } }",
R"(Program:
  Block:
    Block:
      ExprStmt:
        Assign(a)
          Literal(1)
)");
}

TEST(ParserExtraTest, IfWithoutElse) {
  expectAST("if (a > 0) { b = 1; }",
R"(Program:
  If:
    Cond:
      Binary('>')
        Identifier(a)
        Literal(0)
    Then:
      Block:
        ExprStmt:
          Assign(b)
            Literal(1)
)");
}

TEST(ParserExtraTest, NestedIf) {
  expectAST("if (a) { if (b) { c = 1; } }",
R"(Program:
  If:
    Cond:
      Identifier(a)
    Then:
      Block:
        If:
          Cond:
            Identifier(b)
          Then:
            Block:
              ExprStmt:
                Assign(c)
                  Literal(1)
)");
}

TEST(ParserExtraTest, ForNoInitPost) {
  expectAST("for (; i < 10; ) { x = i; }",
R"(Program:
  For:
    Init:
    Cond:
      Binary('<')
        Identifier(i)
        Literal(10)
    Post:
    Body:
      Block:
        ExprStmt:
          Assign(x)
            Identifier(i)
)");
}

TEST(ParserExtraTest, ForEmptyBody) {
  expectAST("for (i=0; i<3; i=i+1) { }",
R"(Program:
  For:
    Init:
      Assign(i)
        Literal(0)
    Cond:
      Binary('<')
        Identifier(i)
        Literal(3)
    Post:
      Assign(i)
        Binary('+')
          Identifier(i)
          Literal(1)
    Body:
      Block:
)");
}

TEST(ParserExtraTest, CallInsideExpression) {
  expectAST("a = f(1) + g(2);",
R"(Program:
  ExprStmt:
    Assign(a)
      Binary('+')
        Call:
          Callee:
            Identifier(f)
          Args:
            Literal(1)
        Call:
          Callee:
            Identifier(g)
          Args:
            Literal(2)
)");
}

TEST(ParserExtraTest, NestedCallsExpression) {
  expectAST("x = f(g(1), h(2)) * k();",
R"(Program:
  ExprStmt:
    Assign(x)
      Binary('*')
        Call:
          Callee:
            Identifier(f)
          Args:
            Call:
              Callee:
                Identifier(g)
              Args:
                Literal(1)
            Call:
              Callee:
                Identifier(h)
              Args:
                Literal(2)
        Call:
          Callee:
            Identifier(k)
          Args:
)");
}

TEST(ParserExtraTest, EmptyFunctionBody) {
  expectAST("fun int f() { }",
R"(Program:
  FuncDecl(int f())
    Block:
)");
}

TEST(ParserExtraTest, FunctionMultipleStatements) {
  expectAST("fun int f(){ a=1; b=2; }",
R"(Program:
  FuncDecl(int f())
    Block:
      ExprStmt:
        Assign(a)
          Literal(1)
      ExprStmt:
        Assign(b)
          Literal(2)
)");
}

TEST(ParserExtraTest, FunctionParamsWithoutTypes) {
  expectAST("fun int f(a, b){ a=b; }",
R"(Program:
  FuncDecl(int f(a, b))
    Block:
      ExprStmt:
        Assign(a)
          Identifier(b)
)");
}


TEST(ParserExtraTest, ErrorDoubleCommaParams) {
EXPECT_THROW({
parseAndDump("fun int f(a,,b){}");
}, ParseError);
}

TEST(ParserExtraTest, ErrorUnclosedCallParen) {

EXPECT_THROW({
parseAndDump("f(1,2;");
}, ParseError);
}

TEST(ParserNegativeExtra, ErrorAssignToLiteral) {
  EXPECT_THROW({
    parseAndDump("1 = x;");
  }, ParseError);
}


TEST(ParserNegativeExtra, ErrorMissingLHS) {
  EXPECT_THROW({
    parseAndDump("= 3;");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorBinaryMissingRight) {
  EXPECT_THROW({
    parseAndDump("a + ;");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorBinaryMissingLeft) {
  EXPECT_THROW({
    parseAndDump("* a;");
  }, ParseError);
}


TEST(ParserNegativeExtra, ErrorCallMissingCallee) {
  EXPECT_THROW({
    parseAndDump("(1, 2);");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorCallExtraCommaStart) {
  EXPECT_THROW({
    parseAndDump("f(,1);");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorCallDoubleComma) {
  EXPECT_THROW({
    parseAndDump("f(1,,2);");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorCallMissingArgsParen) {
  EXPECT_THROW({
    parseAndDump("f(;");
  }, ParseError);
}


TEST(ParserNegativeExtra, ErrorIfMissingParenOpen) {
  EXPECT_THROW({
    parseAndDump("if x) { }");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorIfMissingParenClose) {
  EXPECT_THROW({
    parseAndDump("if (x { }");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorIfElseWithoutIf) {
  EXPECT_THROW({
    parseAndDump("else { x = 1; }");
  }, ParseError);
}


TEST(ParserNegativeExtra, ErrorForMissingParens) {
  EXPECT_THROW({
    parseAndDump("for i=0; i<10; i=i+1 { }");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorForDoubleInit) {
  EXPECT_THROW({
    parseAndDump("for (i=0, j=1; i<10; i=i+1) { }");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorForMissingBody) {
  EXPECT_THROW({
    parseAndDump("for (i=0; i<10; i=i+1)");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorForMissingCondSemicolon) {
  EXPECT_THROW({
    parseAndDump("for (i=0 i<10; i=i+1) { }");
  }, ParseError);
}


TEST(ParserNegativeExtra, ErrorFuncMissingName) {
  EXPECT_THROW({
    parseAndDump("fun int () { }");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorFuncParamMissingName) {
  EXPECT_THROW({
    parseAndDump("fun int f(:int) { }");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorFuncParamMissingComma) {
  EXPECT_THROW({
    parseAndDump("fun int f(a b) { }");
  }, ParseError);
}


TEST(ParserNegativeExtra, ErrorUnknownToken) {
  EXPECT_THROW({
    parseAndDump("@;");
  }, ParseError);
}

TEST(ParserNegativeExtra, ErrorColonOutsideParams) {
  EXPECT_THROW({
    parseAndDump("x : int;");
  }, ParseError);
}



