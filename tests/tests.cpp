#include <cmath>
#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "ast_dump.h"
#include "interpreter.h"

#define EXPECT_INT(v, x)  EXPECT_EQ(std::get<long long>(v), (x))
#define EXPECT_NUM(v, x)  EXPECT_NEAR(std::get<double>(v), (x), 1e-9)
#define EXPECT_BOOL(v,x)  EXPECT_EQ(std::get<bool>(v), (x))
#define EXPECT_STR(v, x)  EXPECT_EQ(std::get<std::string>(v), (x))


using namespace minilang;
using namespace minilang::ast;

static Value runProgram(const std::string &src) {
    Lexer lexer(makeStringSource(src));
    Parser parser(lexer);

    auto program = parser.parseProgram();

    Interpreter interp;
    program->accept(interp);

    return interp.getLastValue();
}

TEST(InterpreterLiteral, IntLiteral) {
    auto v = runProgram("42;");
    EXPECT_INT(v, 42);
}

TEST(InterpreterLiteral, BoolLiteral) {
    auto v = runProgram("true;");
    EXPECT_BOOL(v, true);
}

TEST(InterpreterLiteral, StringLiteral) {
    auto v = runProgram("\"hello\";");
    EXPECT_STR(v, "hello");
}

TEST(InterpreterArithmetic, BasicOps) {
    auto v = runProgram("1 + 2 * 3;");
    EXPECT_NUM(v, 7);
}

TEST(InterpreterArithmetic, Parentheses) {
    auto v = runProgram("(1 + 2) * 3;");
    EXPECT_NUM(v, 9);
}

TEST(InterpreterArithmetic, Modulo) {
    auto v = runProgram("10 % 3;");
    EXPECT_INT(v, 1);
}

TEST(InterpreterComparison, LessEqual) {
    auto v = runProgram("5 <= 3;");
    EXPECT_BOOL(v, false);
}

TEST(InterpreterComparison, Equality) {
    auto v = runProgram("3 == 3;");
    EXPECT_BOOL(v, true);
}

TEST(InterpreterComparison, NotEqual) {
    auto v = runProgram("3 != 4;");
    EXPECT_BOOL(v, true);
}

TEST(InterpreterLogic, AndOr) {
    auto v = runProgram("true && false || true;");
    EXPECT_BOOL(v, true);
}

TEST(InterpreterLogic, ShortCircuitLike) {
    auto v = runProgram("false && (1 / 0);");
    EXPECT_BOOL(v, false);
}

TEST(InterpreterVariables, Assignment) {
    auto v = runProgram("x = 5; x;");
    EXPECT_INT(v, 5);
}

TEST(InterpreterVariables, Reassign) {
    auto v = runProgram("x = 1; x = x + 2; x;");
    EXPECT_NUM(v, 3);
}

TEST(InterpreterIf, SimpleIf) {
    auto v = runProgram(R"(
        x = 0;
        if (true) { x = 1; }
        x;
    )");
    EXPECT_INT(v, 1);
}

TEST(InterpreterIf, IfElse) {
    auto v = runProgram(R"(
        x = 0;
        if (false) { x = 1; }
        else { x = 2; }
        x;
    )");
    EXPECT_INT(v, 2);
}

TEST(InterpreterFor, SimpleLoop) {
    auto v = runProgram(R"(
        sum = 0;
        for (i = 1; i <= 3; i = i + 1) {
            sum = sum + i;
        }
        sum;
    )");
    EXPECT_NUM(v, 6);
}

TEST(InterpreterFunction, SimpleFunction) {
    auto v = runProgram(R"(
        fun int add(a:int, b:int) {
            return a + b;
        }
        add(2,3);
    )");
    EXPECT_NUM(v, 5);
}

TEST(InterpreterFunction, Factorial) {
    auto v = runProgram(R"(
        fun int fact(n:int) {
            if (n <= 1) { return 1; }
            return n * fact(n - 1);
        }
        fact(5);
    )");
    EXPECT_NUM(v, 120);
}

TEST(InterpreterBuiltin, PrintReturnsVoid) {
    auto v = runProgram("print(1);");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(InterpreterError, UndefinedVariable) {
    EXPECT_THROW(
        runProgram("x;"),
        RuntimeError
    );
}

TEST(InterpreterError, InvalidCondition) {
    EXPECT_THROW(
        runProgram("if (abc) { }"),
        RuntimeError
    );
}

TEST(InterpreterArithmetic, DivisionByZeroGivesInf) {
    auto v = runProgram("1 / 0;");
    EXPECT_TRUE(std::holds_alternative<double>(v));
    EXPECT_TRUE(std::isinf(std::get<double>(v)));
}


TEST(InterpreterError, CallNonFunction) {
    EXPECT_THROW(
        runProgram("x = 5; x();"),
        RuntimeError
    );
}

TEST(InterpreterError, WrongArity) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f(a:int) { return a; }
            f(1,2);
        )"),
        RuntimeError
    );
}

TEST(InterpreterError, ReturnOutsideFunction) {
    EXPECT_THROW(
        runProgram("return 5;"),
        ReturnSignal
    );
}

TEST(InterpreterEdge, EmptyProgram) {
    auto v = runProgram("");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(InterpreterEdge, NestedCalls) {
    auto v = runProgram(R"(
        fun fun f() {
            return g;
        }
        fun int g(x:int) {
            return x + 1;
        }
        f()(4);
    )");
    EXPECT_NUM(v, 5);
}


TEST(InterpreterExtraArithmetic, MixedIntFloat) {
    auto v = runProgram("1 + 2.5;");
    EXPECT_NUM(v, 3.5);
}

TEST(InterpreterExtraArithmetic, DivisionByZeroInf) {
    auto v = runProgram("1 / 0;");
    EXPECT_TRUE(std::isinf(std::get<double>(v)));
}

TEST(InterpreterExtraArithmetic, DoubleChain) {
    auto v = runProgram("1.5 + 2.5 + 3.0;");
    EXPECT_NUM(v, 7.0);
}

TEST(InterpreterExtraLogic, TruthyNumber) {
    auto v = runProgram("if (5) { true; } else { false; }");
    EXPECT_BOOL(v, true);
}

TEST(InterpreterExtraLogic, FalseyZero) {
    auto v = runProgram("if (0) { true; } else { false; }");
    EXPECT_BOOL(v, false);
}

TEST(InterpreterExtraLogic, ComparisonChain) {
    auto v = runProgram("1 < 2 && 2 < 3;");
    EXPECT_BOOL(v, true);
}


TEST(InterpreterExtraScope, VariableOverwrite) {
    auto v = runProgram("x = 1; x = 2; x;");
    EXPECT_INT(v, 2);
}

TEST(InterpreterExtraScope, BlockShadowingAllowed) {
    auto v = runProgram(R"(
        x = 1;
        {
            x = 5;
        }
        x;
    )");
    EXPECT_INT(v, 5);
}

TEST(InterpreterExtraScope, BlockDoesNotLeakNewVar) {
    EXPECT_THROW(
        runProgram(R"(
            {
                y = 10;
            }
            y;
        )"),
        RuntimeError
    );
}


TEST(InterpreterExtraFunction, ReturnWithoutValue) {
    auto v = runProgram(R"(
        fun int f() {
            return;
        }
        f();
    )");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(InterpreterExtraFunction, RecursiveCount) {
    auto v = runProgram(R"(
        fun int f(n:int) {
            if (n <= 0){
                return 0;
           }
            return 1 + f(n - 1);
        }
        f(4);
    )");
    EXPECT_NUM(v, 4);
}

TEST(InterpreterExtraFunction, FunctionAsValue) {
    auto v = runProgram(R"(
        fun fun get() {
            return inc;
        }
        fun int inc(x:int) {
            return x + 1;
        }
        get()(10);
    )");
    EXPECT_NUM(v, 11);
}

TEST(InterpreterExtraBuiltin, PrintReturnsVoid) {
    auto v = runProgram("print(123);");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(InterpreterExtraBuiltin, PrintInAssignment) {
    auto v = runProgram("x = print(1); x;");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(InterpreterExtraFor, ForLoopCounter) {
    auto v = runProgram(R"(
        for (i = 0; i < 3; i = i + 1) { }
        i;
    )");
    EXPECT_NUM(v, 3);
}

TEST(InterpreterExtraFor, ForEarlyReturn) {
    auto v = runProgram(R"(
        fun int f() {
            for (i = 0; i < 10; i = i + 1) {
                return i;
            }
        }
        f();
    )");
    EXPECT_INT(v, 0);
}


TEST(InterpreterExtraError, UndefinedVarInFunction) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f() { return x; }
            f();
        )"),
        RuntimeError
    );
}


TEST(InterpreterExtraProgram, LastExpressionWins) {
    auto v = runProgram("1; 2; 3;");
    EXPECT_INT(v, 3);
}

TEST(InterpreterExtraProgram, EmptyProgram) {
    auto v = runProgram("");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(InterpreterArithmeticExtra, MixedIntFloat) {
    auto v = runProgram("1 + 2.5;");
    EXPECT_NUM(v, 3.5);
}

TEST(InterpreterArithmeticExtra, UnaryMinusChain) {
    auto v = runProgram("---5;");
    EXPECT_INT(v, -5);
}

TEST(InterpreterArithmeticExtra, DivisionFloat) {
    auto v = runProgram("5 / 2;");
    EXPECT_NUM(v, 2.5);
}

TEST(InterpreterArithmeticExtra, InfDivision) {
    auto v = runProgram("1 / 0;");
    EXPECT_TRUE(std::isinf(std::get<double>(v)));
}

TEST(InterpreterLogicExtra, ComparisonChain) {
    auto v = runProgram("1 < 2 && 2 < 3;");
    EXPECT_BOOL(v, true);
}

TEST(InterpreterLogicExtra, EqualityFalse) {
    auto v = runProgram("1 == 2;");
    EXPECT_BOOL(v, false);
}

TEST(InterpreterLogicExtra, BoolFromInt) {
    auto v = runProgram("if (1) { true; } else { false; }");
    EXPECT_BOOL(v, true);
}

TEST(InterpreterLogicExtra, BoolFromZero) {
    auto v = runProgram("if (0) { true; } else { false; }");
    EXPECT_BOOL(v, false);
}
TEST(InterpreterScope, Shadowing) {
    auto v = runProgram(R"(
        x = 1;
        {
            x = 2;
        }
        x;
    )");
    EXPECT_INT(v, 2);
}

TEST(InterpreterScope, NestedBlockVar) {
    auto v = runProgram(R"(
        x = 1;
        {
            y = 2;
            x = y;
        }
        x;
    )");
    EXPECT_INT(v, 2);
}

TEST(InterpreterIfExtra, NestedIfElse) {
    auto v = runProgram(R"(
        if (true) {
            if (false) { 1; }
            else { 2; }
        }
    )");
    EXPECT_INT(v, 2);
}
TEST(InterpreterReturnExtra, ReturnWithoutValue) {
    auto v = runProgram(R"(
        fun int f() { return; }
        f();
    )");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(InterpreterReturnExtra, EarlyReturn) {
    auto v = runProgram(R"(
        fun int f(x:int) {
            if (x > 0){
             return 1;
            }
            return 2;
        }
        f(5);
    )");
    EXPECT_INT(v, 1);
}

TEST(InterpreterFunctionExtra, FunctionAsValue) {
    auto v = runProgram(R"(
        fun fun g() { return f; }
        fun int f(x:int) { return x + 1; }
        g()(4);
    )");
    EXPECT_NUM(v, 5);
}

TEST(InterpreterBuiltinExtra, PrintExpression) {
    auto v = runProgram("print(1 + 2);");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}
TEST(InterpreterErrorExtra, UseBeforeAssign) {
    EXPECT_THROW(
        runProgram("x + 1;"),
        RuntimeError
    );
}

TEST(InterpreterErrorExtra, CallResultNotCallable) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f(){ return 1; }
            f()();
        )"),
        RuntimeError
    );
}

TEST(InterpreterErrorExtra, TooFewArgs) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f(a:int, b:int) { return a + b; }
            f(1);
        )"),
        RuntimeError
    );
}
TEST(InterpreterEdgeExtra, OnlyWhitespace) {
    auto v = runProgram("   \n\t ");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(InterpreterEdgeExtra, ExpressionOnly) {
    auto v = runProgram("(((1)));");
    EXPECT_INT(v, 1);
}


TEST(InterpreterScope, ShadowingVariable) {
    auto v = runProgram(R"(
        x = 1;
        {
            x = 2;
        }
        x;
    )");
    EXPECT_INT(v, 2);
}

TEST(InterpreterScope, InnerScopeVariableNotVisible) {
    EXPECT_THROW(
        runProgram(R"(
            {
                y = 10;
            }
            y;
        )"),
        RuntimeError
    );
}

TEST(InterpreterScope, FunctionLocalScope) {
    auto v = runProgram(R"(
        fun int f() {
            x = 10;
            return x;
        }
        f();
    )");
    EXPECT_INT(v, 10);
}

TEST(InterpreterScope, NoLeakFromFunction) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f() {
                x = 10;
                return x;
            }
            f();
            x;
        )"),
        RuntimeError
    );
}


TEST(InterpreterFunctionValue, AssignFunctionToVar) {
    auto v = runProgram(R"(
        fun int inc(x:int) { return x + 1; }
        f = inc;
        f(5);
    )");
    EXPECT_NUM(v, 6);
}

TEST(InterpreterFunctionValue, ReturnFunctionAndCallLater) {
    auto v = runProgram(R"(
        fun fun make() {
            return inc;
        }
        fun int inc(x:int) { return x + 1; }
        g = make();
        g(4);
    )");
    EXPECT_NUM(v, 5);
}

TEST(InterpreterIf, ConditionFalseSkipsThen) {
    auto v = runProgram(R"(
        x = 0;
        if (false) { x = 1; }
        x;
    )");
    EXPECT_INT(v, 0);
}

TEST(InterpreterIf, NestedIfElse) {
    auto v = runProgram(R"(
        x = 0;
        if (true) {
            if (false){ x = 1;}
            else x = 2;
        }
        x;
    )");
    EXPECT_INT(v, 2);
}

TEST(InterpreterFor, ZeroIterations) {
    auto v = runProgram(R"(
        x = 0;
        for (i = 0; i < 0; i = i + 1) {
            x = 1;
        }
        x;
    )");
    EXPECT_INT(v, 0);
}

TEST(InterpreterReturn, EarlyReturnStopsExecution) {
    auto v = runProgram(R"(
        fun int f() {
            return 1;
            return 2;
        }
        f();
    )");
    EXPECT_INT(v, 1);
}

TEST(InterpreterReturn, ReturnWithoutValue) {
    auto v = runProgram(R"(
        fun int f() {
            return;
        }
        f();
    )");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}


TEST(InterpreterExpr, UnaryMinusNested) {
    auto v = runProgram("--5;");
    EXPECT_INT(v, 5);
}

TEST(InterpreterExpr, ComplexExpression) {
    auto v = runProgram("1 + 2 * (3 + 4) - 5;");
    EXPECT_NUM(v, 10);
}

TEST(InterpreterExpr, ComparisonChain) {
    auto v = runProgram("1 < 2 && 2 < 3;");
    EXPECT_BOOL(v, true);
}


TEST(InterpreterBuiltin, PrintReturnsVoidEvenInExpression) {
    auto v = runProgram("x = print(1); x;");
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}


TEST(InterpreterError, CallUndefinedFunction) {
    EXPECT_THROW(
        runProgram("foo(1);"),
        RuntimeError
    );
}

TEST(InterpreterError, UseFunctionAsValueInArithmetic) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f() { return 1; }
            f + 1;
        )"),
        RuntimeError
    );
}

TEST(InterpreterError, InvalidForConditionType) {
    EXPECT_THROW(
        runProgram(R"(
            for (; "abc"; ) { }
        )"),
        RuntimeError
    );
}


TEST(InterpreterEdge, LastExpressionIsResult) {
    auto v = runProgram(R"(
        x = 1;
        y = 2;
        x + y;
    )");
    EXPECT_NUM(v, 3);
}

TEST(InterpreterEdge, MultipleFunctionsIndependent) {
    auto v = runProgram(R"(
        fun int a() { return 1; }
        fun int b() { return 2; }
        a() + b();
    )");
    EXPECT_NUM(v, 3);
}

TEST(InterpreterBind, BindOneArgument) {
    auto v = runProgram(R"(
        fun int add(a:int, b:int) {
            return a + b;
        }
        add10 = (10) =>> add;
        add10(5);
    )");
    EXPECT_NUM(v, 15);
}

TEST(InterpreterBind, BindTwoArguments) {
    auto v = runProgram(R"(
        fun int add3(a:int, b:int, c:int) {
            return a + b + c;
        }
        f = (1, 2) =>> add3;
        f(3);
    )");
    EXPECT_NUM(v, 6);
}

TEST(InterpreterBind, ChainedBind) {
    auto v = runProgram(R"(
        fun int add3(a:int, b:int, c:int) {
            return a + b + c;
        }
        f1 = (1) =>> add3;
        f2 = (2) =>> f1;
        f2(3);
    )");
    EXPECT_NUM(v, 6);
}

TEST(InterpreterBind, BindInsideExpression) {
    auto v = runProgram(R"(
        fun int mul(a:int, b:int) {
            return a * b;
        }
        ((2) =>> mul)(5);
    )");
    EXPECT_NUM(v, 10);
}

TEST(InterpreterBindError, RightSideNotFunction) {
    EXPECT_THROW(
        runProgram("(1) =>> 42;"),
        RuntimeError
    );
}

TEST(InterpreterBindError, TooManyBoundArguments) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f(a:int, b:int) {
                return a + b;
            }
            g = (1,2,3) =>> f;
            g();
        )"),
        RuntimeError
    );
}

TEST(InterpreterBindError, WrongArityAfterBind) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f(a:int, b:int, c:int) {
                return a + b + c;
            }
            g = (1) =>> f;
            g(2);
        )"),
        RuntimeError
    );
}

TEST(InterpreterBindEdge, TupleWithExpressions) {
    auto v = runProgram(R"(
        fun int add3(a:int,b:int,c:int) {
            return a + b + c;
        }
        g = (1+1, 2*2) =>> add3;
        g(3);
    )");
    EXPECT_NUM(v, 9);
}

TEST(InterpreterBindEdge, TupleWithCall) {
    auto v = runProgram(R"(
        fun int inc(x:int) { return x+1; }
        fun int add(a:int,b:int) { return a+b; }
        g = (inc(4)) =>> add;
        g(5);
    )");
    EXPECT_NUM(v, 10);
}

TEST(InterpreterBindBehavior, BindReturnsFunction) {
    auto v = runProgram(R"(
        fun int add(a:int,b:int){ return a+b; }
        f = (2) =>> add;
        g = f;
        g(3);
    )");
    EXPECT_NUM(v, 5);
}

TEST(InterpreterBindBehavior, BindWithRecursiveFunction) {
    auto v = runProgram(R"(
        fun int fact(n:int) {
            if (n <= 1){
             return 1;
            }
            return n * fact(n - 1);
        }
        f = (5) =>> fact;
        f();
    )");
    EXPECT_NUM(v, 120);
}

TEST(InterpreterDecorator, BasicDecorator) {
    auto v = runProgram(R"(
        fun int ident(x:int) {
            return x;
        }

        fun int add1(f:fun, x:int) {
            return f(x + 1);
        }

        decorated = ident &*& add1;
        decorated(7);
    )");
    EXPECT_NUM(v, 8);
}

TEST(InterpreterDecorator, MultiplyBeforeCall) {
    auto v = runProgram(R"(
        fun int square(x:int) {
            return x * x;
        }

        fun int deco(f:fun, x:int) {
            return f(x * 2);
        }

        g = square &*& deco;
        g(3);
    )");
    EXPECT_NUM(v, 36);
}

TEST(InterpreterDecorator, ChainedDecorators) {
    auto v = runProgram(R"(
        fun int ident(x:int) { return x; }

        fun int inc(f:fun, x:int) {
            return f(x + 1);
        }

        fun int dbl(f:fun, x:int) {
            return f(x * 2);
        }

        f = ident &*& inc;
        g = f &*& dbl;

        g(3);
    )");
    EXPECT_NUM(v, 7);
}

TEST(InterpreterDecorator, DecoratorChangesResult) {
    auto v = runProgram(R"(
        fun int f(x:int) { return x; }

        fun bool deco(f:fun, x:int) {
            return f(x) > 5;
        }

        g = f &*& deco;
        g(10);
    )");
    EXPECT_BOOL(v, true);
}

TEST(InterpreterDecorator, NestedDecoratorCall) {
    auto v = runProgram(R"(
        fun int add1(x:int) { return x + 1; }

        fun int deco(f:fun, x:int) {
            return f(f(x));
        }

        g = add1 &*& deco;
        g(3);
    )");
    EXPECT_NUM(v, 5);
}

TEST(InterpreterDecoratorError, LeftNotFunction) {
    EXPECT_THROW(
        runProgram(R"(
            fun int deco(f:fun, x:int) { return f(x); }
            42 &*& deco;
        )"),
        RuntimeError
    );
}

TEST(InterpreterDecoratorError, RightNotFunction) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f(x:int) { return x; }
            f &*& 123;
        )"),
        RuntimeError
    );
}

TEST(InterpreterDecoratorError, DecoratorWrongArity) {
    EXPECT_THROW(
        runProgram(R"(
            fun int f(x:int) { return x; }

            fun int deco(f:fun) { return 0; }

            g = f &*& deco;
            g(1);
        )"),
        RuntimeError
    );
}

TEST(InterpreterDecoratorEdge, DecoratorAfterBind) {
    auto v = runProgram(R"(
        fun int add(a:int,b:int) { return a+b; }

        fun int deco(f:fun, x:int) {
            return f(x + 1);
        }

        f = (10) =>> add;
        g = f &*& deco;

        g(5);
    )");
    EXPECT_NUM(v, 16);
}

TEST(InterpreterDecoratorEdge, BindAfterDecorator) {
    EXPECT_THROW(
        runProgram(R"(
        fun int add(a:int,b:int) { return a+b; }

        fun int deco(f:fun, x:int) {
            return f(x * 2);
        }

        g = add &*& deco;
        h = (3) =>> g;
        h(4);
        )"),
        RuntimeError
    );
}

TEST(InterpreterDecoratorEdge, DecoratorAsValue) {
    auto v = runProgram(R"(
        fun int ident(x:int){ return x; }

        fun int deco(f:fun, x:int){
            return f(x + 1);
        }

        fun int apply(f:fun, x:int){
            return f(x);
        }

        g = ident &*& deco;
        apply(g, 4);
    )");
    EXPECT_NUM(v, 5);
}

TEST(InterpreterDecoratorBehavior, DecoratorReturnsFunction) {
    auto v = runProgram(R"(
        fun int ident(x:int){ return x; }

        fun fun deco(f:fun, x:int){
            return f;
        }

        g = ident &*& deco;
        h = g(10);
        h(3);
    )");
    EXPECT_INT(v, 3);
}

TEST(InterpreterDecoratorBehavior, RecursiveWithDecorator) {
    auto v = runProgram(R"(
        fun int fact(n:int){
            if (n <= 1) {return 1;}
            return n * fact(n - 1);
        }

        fun int deco(f:fun, x:int){
            return f(x);
        }

        g = fact &*& deco;
        g(5);
    )");
    EXPECT_NUM(v, 120);
}
