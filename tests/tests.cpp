#include <gtest/gtest.h>
#include "lexer.h"
#include "source.h"

using namespace minilang;

static Token next(std::unique_ptr<Source> src) {
    Lexer lex(std::move(src));
    return lex.nextToken();
}


//    IDENTIFIERS & KEYWORDS

TEST(LexerTests, IdentifierSimple) {
    Lexer lex(makeStringSource("hello"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "hello");
}

TEST(LexerTests, KeywordDetection) {
    Lexer lex(makeStringSource("fun"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Keyword);
    EXPECT_EQ(t.lexeme, "fun");
}

TEST(LexerTests, BoolTrue) {
    Lexer lex(makeStringSource("true"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Bool);
    EXPECT_EQ(std::get<bool>(t.value), true);
}

TEST(LexerTests, WrongLessKeywordDetection) {
    Lexer lex(makeStringSource("fu"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "fu");
}

TEST(LexerTests, WrongExtraKeywordDetection) {
    Lexer lex(makeStringSource("trues"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "trues");
}

TEST(LexerTests, BoolFalse) {
    Lexer lex(makeStringSource("false"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Bool);
    EXPECT_EQ(std::get<bool>(t.value), false);
}

//    NUMBERS

TEST(LexerTests, IntegerLiteral) {
    Lexer lex(makeStringSource("123"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.value), 123);
}

TEST(LexerTests, IntegerLiteralMalfomedByLetter) {
    Lexer lex(makeStringSource("123b"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, IntegerLiteralMalfomedByLetterInside) {
    Lexer lex(makeStringSource("12b3"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, FloatLiteral) {
    Lexer lex(makeStringSource("12.5"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::NumberFloat);
    EXPECT_DOUBLE_EQ(std::get<double>(t.value), 12.5);
}

TEST(LexerTests, MalformedFloatTwoDots) {
    Lexer lex(makeStringSource("12.3.4"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, MalformedFloatbyLetter) {
    Lexer lex(makeStringSource("12.2t"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, MalformedFloatbyLetterJustAfterDots) {
    Lexer lex(makeStringSource("12.t"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

//    STRINGS


TEST(LexerTests, SimpleStringLiteral) {
    Lexer lex(makeStringSource("\"Hello\""));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::String);
    EXPECT_EQ(std::get<std::string>(t.value), "Hello");
}

TEST(LexerTests, StringWithEscapes) {
    Lexer lex(makeStringSource("\"A\\nB\\tC\""));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::String);
    EXPECT_EQ(std::get<std::string>(t.value), "A\nB\tC");
}

TEST(LexerTests, UnterminatedStringThrows) {
    Lexer lex(makeStringSource("\"Hello"));

    EXPECT_THROW(lex.nextToken(), std::runtime_error);
}

//    OPERATORS & PUNCTUATORS

TEST(LexerTests, OperatorPlus) {
    Lexer lex(makeStringSource("+"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Operator);
    EXPECT_EQ(t.lexeme, "+");
}

TEST(LexerTests, OperatorEquality) {
    Lexer lex(makeStringSource("=="));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Operator);
    EXPECT_EQ(t.lexeme, "==");
}

TEST(LexerTests, OperatorEqualityWrong) {
    Lexer lex(makeStringSource("=*"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, OperatorEqualityWrongLetter) {
    Lexer lex(makeStringSource("*="));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, MultiCharAmpersandOperator) {
    Lexer lex(makeStringSource("&*&"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Operator);
    EXPECT_EQ(t.lexeme, "&*&");
}

TEST(LexerTests, MultiCharAmpersandWrongOperator) {
    Lexer lex(makeStringSource("&*="));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, MultiCharAmpersandWrongOperatorBefore) {
    Lexer lex(makeStringSource("=&*&"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, MultiCharAmpersandWrongOperatorEnd) {
    Lexer lex(makeStringSource("&*"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

TEST(LexerTests, MultiCharArrowOperator) {
    Lexer lex(makeStringSource("=>>"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Operator);
    EXPECT_EQ(t.lexeme, "=>>");
}

TEST(LexerTests, PunctuatorLeftParen) {
    Lexer lex(makeStringSource("("));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Punctuator);
    EXPECT_EQ(t.lexeme, "(");
}

//    COMMENTS


TEST(LexerTests, SkipLineComment) {
    Lexer lex(makeStringSource("// comment here\n123"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.value), 123);
}

TEST(LexerTests, SkipBlockComment) {
    Lexer lex(makeStringSource("/* abc */  12"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.value), 12);
}

TEST(LexerTests, UnterminatedBlockCommentThrowsBefore) {
    Lexer lex(makeStringSource("/* abc"));

    EXPECT_THROW(lex.nextToken(), std::runtime_error);
}

TEST(LexerTests, UnterminatedBlockCommentThrowsAfter) {
    Lexer lex(makeStringSource("*/"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
}

 //  UNKNOWN TOKENS

TEST(LexerTests, UnknownSymbol) {
    Lexer lex(makeStringSource("@"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.kind, TokenKind::Unknown);
    EXPECT_EQ(t.lexeme, "@");
}

//   TOKEN STREAM BEHAVIOR

TEST(LexerTests, SequenceOfTokens) {
    Lexer lex(makeStringSource("a = 10"));

    Token t1 = lex.nextToken();
    Token t2 = lex.nextToken();
    Token t3 = lex.nextToken();

    EXPECT_EQ(t1.kind, TokenKind::Identifier);
    EXPECT_EQ(t2.kind, TokenKind::Operator);
    EXPECT_EQ(t3.kind, TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t3.value), 10);
}

TEST(LexerTests, PeekDoesNotConsume) {
    Lexer lex(makeStringSource("abc"));

    Token t1 = lex.peekToken();
    Token t3 = lex.nextToken();

    EXPECT_EQ(t1.lexeme, "abc");
    EXPECT_EQ(t3.lexeme, "abc");
}

TEST(LexerTests, EOFToken) {
    Lexer lex(makeStringSource(""));

    Token t = lex.nextToken();
    EXPECT_EQ(t.kind, TokenKind::EndOfFile);
}

//    UNCLASIFIED

TEST(LexerTests, ErrorMissingOperandAfterOperator) {
    Lexer lex(makeStringSource("10 + "));

    Token t1 = lex.nextToken();
    Token t2 = lex.nextToken();
    Token t3 = lex.nextToken();

    EXPECT_EQ(t1.kind, TokenKind::NumberInt);
    EXPECT_EQ(t2.kind, TokenKind::Operator);
    EXPECT_EQ(t3.kind, TokenKind::EndOfFile);
}

