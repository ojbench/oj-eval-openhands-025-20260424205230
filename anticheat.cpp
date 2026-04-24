#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "lang.h"
#include "visitor.h"

class Fingerprint : public Visitor<std::string> {
 public:
  std::string visitProgram(Program *node) override {
    std::vector<std::string> funcs;
    for (auto func : node->body) {
      funcs.push_back(visitFunctionDeclaration(func));
    }
    std::sort(funcs.begin(), funcs.end());
    std::string res = "P(";
    for (const auto& s : funcs) res += s + ",";
    res += ")";
    return res;
  }
  std::string visitFunctionDeclaration(FunctionDeclaration *node) override {
    return "F(" + std::to_string(node->params.size()) + "," + visitStatement(node->body) + ")";
  }
  std::string visitExpressionStatement(ExpressionStatement *node) override {
    return "E(" + visitExpression(node->expr) + ")";
  }
  std::string visitSetStatement(SetStatement *node) override {
    return "S(" + visitExpression(node->value) + ")";
  }
  std::string visitIfStatement(IfStatement *node) override {
    return "I(" + visitExpression(node->condition) + "," + visitStatement(node->body) + ")";
  }
  std::string visitForStatement(ForStatement *node) override {
    return "L(" + visitStatement(node->init) + "," + visitExpression(node->test) + "," + visitStatement(node->update) + "," + visitStatement(node->body) + ")";
  }
  std::string visitBlockStatement(BlockStatement *node) override {
    std::string res = "B(";
    for (auto stmt : node->body) res += visitStatement(stmt) + ";";
    res += ")";
    return res;
  }
  std::string visitReturnStatement(ReturnStatement *node) override {
    return "R(" + visitExpression(node->value) + ")";
  }
  std::string visitIntegerLiteral(IntegerLiteral *node) override {
    return "N";
  }
  std::string visitVariable(Variable *node) override {
    return "V";
  }
  std::string visitCallExpression(CallExpression *node) override {
    std::string res = "C(" + (builtinFunctions.count(node->func) ? node->func : "f") + ",";
    for (auto arg : node->args) res += visitExpression(arg) + ",";
    res += ")";
    return res;
  }
};

int main() {
  Program *prog1 = scanProgram(std::cin);
  Program *prog2 = scanProgram(std::cin);
  std::string input;
  int c;
  while ((c = std::cin.get()) != EOF) {
    input += c;
  }

  std::string out1, out2;
  bool ok1 = true, ok2 = true;
  try {
    std::istringstream iss1(input);
    std::ostringstream oss1;
    prog1->eval(1000000, iss1, oss1);
    out1 = oss1.str();
  } catch (...) { ok1 = false; }
  try {
    std::istringstream iss2(input);
    std::ostringstream oss2;
    prog2->eval(1000000, iss2, oss2);
    out2 = oss2.str();
  } catch (...) { ok2 = false; }

  double score = 0.5;
  if (ok1 && ok2 && out1 == out2) {
    score = 0.8;
  }

  std::string f1 = Fingerprint().visitProgram(prog1);
  std::string f2 = Fingerprint().visitProgram(prog2);

  if (f1 == f2) {
    score = std::max(score, 1.0);
  } else {
    // Simple Jaccard similarity on tokens could be better, but let's start with this.
    if (score < 0.8) score = 0.2; // If they are different and output is different, they are likely different.
  }

  std::cout << score << std::endl;
  return 0;
}
