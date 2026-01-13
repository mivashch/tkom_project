#include "ast_dump.h"
#include <iostream>

namespace minilang::ast {

    struct OutputLiteral {
        std::ostream& os;

        void operator()(const std::string& v) const {
            os << "\"" << v << "\"";
        }

        void operator()(bool v) const {
            os << std::boolalpha << v;
        }

        void operator()(long long v) const {
            os << v;
        }

        void operator()(double v) const {
            os << v;
        }

        void operator()(std::monostate) const {
            os << "<null>";
        }
    };


    void AstPrinter::visit(LiteralExpr& e) {
        pad();
        os << "Literal(";
        std::visit(OutputLiteral{os}, e.value);
        os << ")\n";
    }



void AstPrinter::visit(IdentifierExpr& e) {
    pad();
    os << "Identifier(" << e.name << ")\n";
}

void AstPrinter::visit(UnaryExpr& e) {
    pad();
    os << "Unary(" << e.op << ")\n";
    indent++;
    e.rhs->accept(*this);
    indent--;
}

void AstPrinter::visit(BinaryExpr& e) {
    pad();
    os << "Binary('" << e.op << "')\n";
    indent++;
    e.lhs->accept(*this);
    e.rhs->accept(*this);
    indent--;
}

void AstPrinter::visit(CallExpr& e) {
    pad();
    os << "Call:\n";

    indent++;
    pad();
    os << "Callee:\n";
    indent++;
    e.callee->accept(*this);
    indent--;

    pad();
    os << "Args:\n";
    indent++;
    for (auto& arg : e.args)
        arg->accept(*this);
    indent -= 2;
}


void AstPrinter::visit(ExprStmt& s) {
    if (s.expr) {
        pad();
        os << "ExprStmt:\n";
        indent++;
        s.expr->accept(*this);
        indent--;
    }
}

void AstPrinter::visit(VarDeclStmt& s) {
    pad();
    os << "VarDecl(";
    if (s.isConst) os << "const ";
    if (s.typeName) os << *s.typeName << " ";
    os << s.name << ")\n";

    if (s.init) {
        indent++;
        s.init->accept(*this);
        indent--;
    }
}

void AstPrinter::visit(AssignExpr& s) {
    pad();
    os << "Assign(" << s.target << ")\n";
    indent++;
    s.value->accept(*this);
    indent--;
}

void AstPrinter::visit(ReturnStmt& s) {
    pad();
    os << "Return:\n";
    indent++;
    if (s.value)
        s.value->accept(*this);
    indent--;
}

void AstPrinter::visit(BlockStmt& s) {
    pad();
    os << "Block:\n";

    indent++;
    for (auto& st : s.stmts)
        st->accept(*this);
    indent--;
}

void AstPrinter::visit(IfStmt& s) {
    pad();
    os << "If:\n";

    indent++;
    pad(); os << "Cond:\n";
    indent++;
    s.cond->accept(*this);
    indent--;

    pad(); os << "Then:\n";
    indent++;
    s.thenBlock->accept(*this);
    indent--;

    if (s.elseBlock) {
        pad(); os << "Else:\n";
        indent++;
        s.elseBlock->accept(*this);
        indent--;
    }

    indent--;
}

void AstPrinter::visit(ForStmt& s) {
    pad();
    os << "For:\n";

    indent++;
    pad(); os << "Init:\n";
    indent++;
    if (s.initDecl) s.initDecl->accept(*this);
    if (s.initExpr) s.initExpr->accept(*this);
    indent--;

    pad(); os << "Cond:\n";
    indent++;
    if (s.cond) s.cond->accept(*this);
    indent--;

    pad(); os << "Post:\n";
    indent++;
    if (s.post) s.post->accept(*this);
    indent--;

    pad(); os << "Body:\n";
    indent++;
    s.body->accept(*this);
    indent--;

    indent--;
}

void AstPrinter::visit(FuncDeclStmt& s) {
    pad();
    os << "FuncDecl(";

    if (s.returnType) os << *s.returnType << " ";
    os << s.name << "(";

    for (size_t i = 0; i < s.params.size(); i++) {
        auto& [name, type] = s.params[i];
        if (i) os << ", ";
        os << name;
        if (type) os << ":" << *type;
    }
    os << "))\n";

    indent++;
    s.body->accept(*this);
    indent--;
}

void AstPrinter::visit(Program& p) {
    pad();
    os << "Program:\n";

    indent++;
    for (auto& st : p.stmts)
        st->accept(*this);
    indent--;
}

void AstPrinter::visit(TupleExpr& e) {
    pad();
    os << "Tuple:\n";

    indent++;
    for (auto& elem : e.elements) {
        elem->accept(*this);
    }
    indent--;
}


}
