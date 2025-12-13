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
        Assign(y)
          Literal(1)
    Else:
      Block:
        Assign(y)
          Literal(2)
)");
}

TEST(ParserTest, ForLoop) {
    expectAST("for (i = 0; i < 10; i = i + 1) { x = x + i; }",
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
        Assign(x)
          Binary('+')
            Identifier(x)
            Identifier(i)
)");
}

TEST(ParserTest, BlockMultiple) {
    expectAST("{ a = 1; b = 2; }",
R"(Program:
  Block:
    Assign(a)
      Literal(1)
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
    std::string out = parseAndDump("x = 1");
    EXPECT_TRUE(out.rfind("ERROR:", 0) == 0);
}

TEST(ParserTest, ErrorBadExpr) {
    std::string out = parseAndDump("x = * 10;");
    EXPECT_TRUE(out.rfind("ERROR:", 0) == 0);
}

TEST(ParserTest, ErrorBadFunction) {
    std::string out = parseAndDump("fun int f( { }");
    EXPECT_TRUE(out.rfind("ERROR:", 0) == 0);
}
