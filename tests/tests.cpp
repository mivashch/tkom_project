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

    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "hello");
}

TEST(LexerTests, KeywordDetection) {
    Lexer lex(makeStringSource("fun"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Keyword);
    EXPECT_EQ(t.lexeme, "fun");
}

TEST(LexerTests, KeywordInsideIdentifier) {
    Lexer lex(makeStringSource("funcs"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "funcs");
}

TEST(LexerTests, BoolTrue) {
    Lexer lex(makeStringSource("true"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Bool);
    EXPECT_EQ(std::get<bool>(t.getValue()), true);
}

TEST(LexerTests, WrongLessKeywordDetection) {
    Lexer lex(makeStringSource("fu"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "fu");
}

TEST(LexerTests, WrongExtraKeywordDetection) {
    Lexer lex(makeStringSource("trues"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "trues");
}

TEST(LexerTests, BoolFalse) {
    Lexer lex(makeStringSource("false"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Bool);
    EXPECT_EQ(std::get<bool>(t.getValue()), false);
}

TEST(LexerTests, KeywordsRecognizedInt) {
    Lexer lex(makeStringSource("int"));

    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Keyword);
}

TEST(LexerTests, KeywordsRecognizedFloat) {
    Lexer lex(makeStringSource("float "));
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Keyword);
}

TEST(LexerTests, KeywordsRecognizedIf) {
    Lexer lex(makeStringSource("if"));
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Keyword);
}

TEST(LexerTests, KeywordsRecognizedElse) {
    Lexer lex(makeStringSource("else"));
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Keyword);
}

TEST(LexerTests, KeywordsRecognizedReturn) {
    Lexer lex(makeStringSource("return "));
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Keyword);
}

TEST(LexerTests, KeywordsRecognizedTrue) {
    Lexer lex(makeStringSource("true"));
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Bool);
}

TEST(LexerTests, KeywordsRecognizedFalse) {
    Lexer lex(makeStringSource("false"));
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Bool);
}

TEST(LexerTests, IdentifierStartsWithUnderscore) {
    Lexer lex(makeStringSource("_abc"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "_abc");
}

TEST(LexerTests, IdentifierContainsUnderscore) {
    Lexer lex(makeStringSource("var_123"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "var_123");
}

TEST(LexerTests, IdentifierDoubleUnderscore) {
    Lexer lex(makeStringSource("__hidden"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "__hidden");
}


TEST(LexerTests, IdentifierWithUnderscore2) {
    Lexer lex(makeStringSource("var_name123"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "var_name123");
}

TEST(LexerTests, IdentifierStartingWithUnderscore) {
    Lexer lex(makeStringSource("_abc"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t.lexeme, "_abc");
}

//    NUMBERS

TEST(LexerTests, IntegerLiteral) {
    Lexer lex(makeStringSource("123"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.getValue()), 123);
}

TEST(LexerTests, IntegerLiteralMalfomedByLetter) {
    Lexer lex(makeStringSource("123b"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, IntegerLiteralMalfomedByLetterInside) {
    Lexer lex(makeStringSource("12b3"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, FloatLiteral) {
    Lexer lex(makeStringSource("12.5"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::NumberFloat);
    EXPECT_DOUBLE_EQ(std::get<double>(t.getValue()), 12.5);
}

TEST(LexerTests, MalformedFloatTwoDots) {
    Lexer lex(makeStringSource("12.3.4"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, MalformedFloatbyLetter) {
    Lexer lex(makeStringSource("12.2t"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, MalformedFloatbyLetterJustAfterDots) {
    Lexer lex(makeStringSource("12.t"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, InvalidDoubleDot) {
    Lexer lex(makeStringSource("1..2"));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}


TEST(LexerTests, NumberZero) {
    Lexer lex(makeStringSource("0"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.getValue()), 0);
}

TEST(LexerTests, NumberSimpleInt) {
    Lexer lex(makeStringSource("123"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.getValue()), 123);
}

TEST(LexerTests, NumberWithLeadingZeros) {
    Lexer lex(makeStringSource("0007"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.getValue()), 7);
}

TEST(LexerTests, NumberFloatSimple) {
    Lexer lex(makeStringSource("42.0"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberFloat);
    EXPECT_DOUBLE_EQ(std::get<double>(t.getValue()), 42.0);
}

TEST(LexerTests, NumberFloatLeadingZero) {
    Lexer lex(makeStringSource("0.001"));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberFloat);
    EXPECT_DOUBLE_EQ(std::get<double>(t.getValue()), 0.001);
}

TEST(LexerTests, NumberFloatMissingFraction) {
    Lexer lex(makeStringSource("10."));

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
    EXPECT_EQ(t.lexeme, "10.");
}


TEST(LexerTests, ZeroAsInteger) {
    Lexer lex(makeStringSource("0"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.getValue()), 0);
}

TEST(LexerTests, FloatStartingWithDotInvalid) {
    Lexer lex(makeStringSource(".5"));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, FloatEndingWithDotInvalid) {
    Lexer lex(makeStringSource("5."));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, VeryLargeInteger) {
    Lexer lex(makeStringSource("99999999999999"));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
}

TEST(LexerTests, IntOverflow) {
    Lexer lex(makeStringSource(
        "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"));
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Unknown);
}

TEST(LexerTests, FloatOverflow) {
    Lexer lex(makeStringSource(
        "9.99999999999999999999999999999999999999999999999999999999999999999999999999999999999999"));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberFloat);
    EXPECT_DOUBLE_EQ(std::get<double>(t.getValue()), 10.0);
}


//    STRINGS


TEST(LexerTests, SimpleStringLiteral) {
    Lexer lex(makeStringSource("\"Hello\""));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::String);
    EXPECT_EQ(std::get<std::string>(t.getValue()), "Hello");
}

TEST(LexerTests, StringWithEscapes) {
    Lexer lex(makeStringSource("\"A\\nB\\tC\""));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::String);
    EXPECT_EQ(std::get<std::string>(t.getValue()), "A\nB\tC");
}

TEST(LexerTests, UnterminatedStringThrows) {
    Lexer lex(makeStringSource("\"Hello"));

    EXPECT_THROW(lex.nextToken(), std::runtime_error);
}

TEST(LexerTests, StringEscapes) {
    Lexer lex(makeStringSource("\"Hello\\nWorld\\t!\""));
    Token t = lex.nextToken();

    ASSERT_EQ(t.getKind(), TokenKind::String);
    EXPECT_EQ(std::get<std::string>(t.getValue()), "Hello\nWorld\t!");
}

TEST(LexerTests, StrayQuoteError) {
    Lexer lex(makeStringSource("\"test\" \"oops"));
    lex.nextToken();
    EXPECT_THROW(lex.nextToken(), std::runtime_error);
}

TEST(LexerTests, EmptyStringOK) {
    Lexer lex(makeStringSource("\"\""));
    Token t = lex.nextToken();
    ASSERT_EQ(t.getKind(), TokenKind::String);
    EXPECT_EQ(std::get<std::string>(t.getValue()), "");
}

TEST(LexerTests, WhitespaceOnly) {
    Lexer lex(makeStringSource("   \t \n   "));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::EndOfFile);
}

TEST(LexerTests, MixedWhitespaceBetweenTokens) {
    Lexer lex(makeStringSource("  a\t \n  =   5  "));
    Token t1 = lex.nextToken();
    Token t2 = lex.nextToken();
    Token t3 = lex.nextToken();

    EXPECT_EQ(t1.lexeme, "a");
    EXPECT_EQ(t2.lexeme, "=");
    EXPECT_EQ(std::get<long long>(t3.getValue()), 5);
}

TEST(LexerTests, EmptyStringLiteral) {
    ;
    Lexer lex(makeStringSource("\"\""));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::String);
    EXPECT_EQ(std::get<std::string>(t.getValue()), "");
}

//    OPERATORS & PUNCTUATORS

TEST(LexerTests, OperatorPlus) {
    Lexer lex(makeStringSource("+"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::OpPlus);
    EXPECT_EQ(t.lexeme, "+");
}

TEST(LexerTests, OperatorEquality) {
    Lexer lex(makeStringSource("=="));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::OpEq);
    EXPECT_EQ(t.lexeme, "==");
}

TEST(LexerTests, OperatorEqualityWrong) {
    Lexer lex(makeStringSource("=*"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, OperatorEqualityWrongLetter) {
    Lexer lex(makeStringSource("*="));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::OpMul);
}

TEST(LexerTests, MultiCharAmpersandOperator) {
    Lexer lex(makeStringSource("&*&"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::OpRefStarRef);
    EXPECT_EQ(t.lexeme, "&*&");
}

TEST(LexerTests, MultiCharAmpersandWrongOperator) {
    Lexer lex(makeStringSource("&*="));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, MultiCharAmpersandWrongOperatorBefore) {
    Lexer lex(makeStringSource("=&*&"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, MultiCharAmpersandWrongOperatorEnd) {
    Lexer lex(makeStringSource("&*"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, MultiCharArrowOperator) {
    Lexer lex(makeStringSource("=>>"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::OpDoubleArrow);
    EXPECT_EQ(t.lexeme, "=>>");
}

TEST(LexerTests, PunctuatorLeftParen) {
    Lexer lex(makeStringSource("("));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Punctuator);
    EXPECT_EQ(t.lexeme, "(");
}

TEST(LexerTests, PunctuatorParenLeft) {
    Lexer lex(makeStringSource("("));
    EXPECT_EQ(lex.nextToken().lexeme, "(");
}

TEST(LexerTests, PunctuatorParenRight) {
    Lexer lex(makeStringSource(")"));
    EXPECT_EQ(lex.nextToken().lexeme, ")");
}

TEST(LexerTests, PunctuatorBraceLeft) {
    Lexer lex(makeStringSource("{"));
    EXPECT_EQ(lex.nextToken().lexeme, "{");
}

TEST(LexerTests, PunctuatorBraceRight) {
    Lexer lex(makeStringSource("}"));
    EXPECT_EQ(lex.nextToken().lexeme, "}");
}

TEST(LexerTests, PunctuatorComma) {
    Lexer lex(makeStringSource(","));
    EXPECT_EQ(lex.nextToken().lexeme, ",");
}

TEST(LexerTests, PunctuatorSemicolon) {
    Lexer lex(makeStringSource(";"));
    EXPECT_EQ(lex.nextToken().lexeme, ";");
}

TEST(LexerTests, OperatorTripleArrow) {
    Lexer lex(makeStringSource("=>>"));
    EXPECT_EQ(lex.nextToken().lexeme, "=>>");
}

TEST(LexerTests, OperatorAmpersandStarAmpersand) {
    Lexer lex(makeStringSource("&*&"));
    EXPECT_EQ(lex.nextToken().lexeme, "&*&");
}

TEST(LexerTests, OperatorDoubleEquals) {
    Lexer lex(makeStringSource("=="));
    EXPECT_EQ(lex.nextToken().lexeme, "==");
}

TEST(LexerTests, OperatorNotEquals) {
    Lexer lex(makeStringSource("!="));
    EXPECT_EQ(lex.nextToken().lexeme, "!=");
}

TEST(LexerTests, LoneAmpersandInvalid) {
    Lexer lex(makeStringSource("&"));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, LongOperatorChain) {
    Lexer lex(makeStringSource("a==b!=c<=d>=e&&f||g")); //should chek all sequence

    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::Identifier);
    EXPECT_EQ(lex.nextToken().lexeme, "==");
    EXPECT_EQ(lex.nextToken().lexeme, "b");
    EXPECT_EQ(lex.nextToken().lexeme, "!=");
    EXPECT_EQ(lex.nextToken().lexeme, "c");
    EXPECT_EQ(lex.nextToken().lexeme, "<=");
    EXPECT_EQ(lex.nextToken().lexeme, "d");
    EXPECT_EQ(lex.nextToken().lexeme, ">=");
    EXPECT_EQ(lex.nextToken().lexeme, "e");
    EXPECT_EQ(lex.nextToken().lexeme, "&&");
    EXPECT_EQ(lex.nextToken().lexeme, "f");
    EXPECT_EQ(lex.nextToken().lexeme, "||");
    EXPECT_EQ(lex.nextToken().lexeme, "g");
}


//    COMMENTS


TEST(LexerTests, SkipLineComment) {
    Lexer lex(makeStringSource("// comment here\n123"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.getValue()), 123);
}

TEST(LexerTests, SkipBlockComment) {
    Lexer lex(makeStringSource("/* abc */  12"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t.getValue()), 12);
}

TEST(LexerTests, UnterminatedBlockCommentThrowsBefore) {
    Lexer lex(makeStringSource("/* abc"));

    EXPECT_THROW(lex.nextToken(), std::runtime_error);
}

TEST(LexerTests, UnterminatedBlockCommentThrowsAfter) {
    Lexer lex(makeStringSource("*/"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

TEST(LexerTests, LineCommentSkipped) {
    Lexer lex(makeStringSource("123 // hello world\n456"));
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::NumberInt);
    EXPECT_EQ(lex.nextToken().getKind(), TokenKind::NumberInt);
}

TEST(LexerTests, CommentAtEOF) {
    Lexer lex(makeStringSource("123 // nothing after"));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);

    Token eof = lex.nextToken();
    EXPECT_EQ(eof.getKind(), TokenKind::EndOfFile);
}


TEST(LexerTests, BlockCommentWithStarsInside) {
    Lexer lex(makeStringSource("/* ** * **** */ 12"));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::NumberInt);
}

TEST(LexerTests, NestedCommentNotSupported) {
    Lexer lex(makeStringSource("/* ab /* cd */ ef */"));
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Identifier);
    t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}

//  UNKNOWN TOKENS

TEST(LexerTests, UnknownSymbol) {
    Lexer lex(makeStringSource("@"));
    Token t = lex.nextToken();

    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
    EXPECT_EQ(t.lexeme, "@");
}

//   TOKEN STREAM BEHAVIOR

TEST(LexerTests, SequenceOfTokens) {
    Lexer lex(makeStringSource("a = 10"));

    Token t1 = lex.nextToken();
    Token t2 = lex.nextToken();
    Token t3 = lex.nextToken();

    EXPECT_EQ(t1.getKind(), TokenKind::Identifier);
    EXPECT_EQ(t2.getKind(), TokenKind::OpAssign);
    EXPECT_EQ(t3.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(std::get<long long>(t3.getValue()), 10);
}

TEST(LexerTests, PeekTokenMultipleTimes) {
    Lexer lex(makeStringSource("xyz"));

    Token t1 = lex.peekToken();
    Token t2 = lex.peekToken();
    Token t3 = lex.nextToken();

    EXPECT_EQ(t1.lexeme, "xyz");
    EXPECT_EQ(t2.lexeme, "xyz");
    EXPECT_EQ(t3.lexeme, "xyz");
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
    EXPECT_EQ(t.getKind(), TokenKind::EndOfFile);
}

TEST(LexerTests, ManyTokensSequence) {
    Lexer lex(makeStringSource("a+b*c/3-(d+4)"));

    std::vector<std::string> expected = {
        "a", "+", "b", "*", "c", "/", "3", "-", "(", "d", "+", "4", ")"
    };

    for (auto &e: expected) {
        Token t = lex.nextToken();
        EXPECT_EQ(t.lexeme, e);
    }
}

//    UNCLASIFIED (JUST SOME LEXER HUMILIATION)

TEST(LexerTests, ErrorMissingOperandAfterOperator) {
    Lexer lex(makeStringSource("10 + "));

    Token t1 = lex.nextToken();
    Token t2 = lex.nextToken();
    Token t3 = lex.nextToken();

    EXPECT_EQ(t1.getKind(), TokenKind::NumberInt);
    EXPECT_EQ(t2.getKind(), TokenKind::OpPlus);
    EXPECT_EQ(t3.getKind(), TokenKind::EndOfFile);
}


TEST(LexerTests, RandomInvalidSymbolInsideExpression) {
    Lexer lex(makeStringSource("10 @ 20"));
    lex.nextToken();
    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
    EXPECT_EQ(t.lexeme, "@");
}

TEST(LexerTests, MixedGarbageInput) {
    Lexer lex(makeStringSource("abc #$% def"));
    lex.nextToken();

    Token t = lex.nextToken();
    EXPECT_EQ(t.getKind(), TokenKind::Unknown);
}
