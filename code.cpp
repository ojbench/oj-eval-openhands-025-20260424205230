

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <vector>

#include "lang.h"
#include "visitor.h"
#include "transform.h"

class Cheat : public Transform {
  std::unordered_map<std::string, std::string> varMap;
  std::unordered_map<std::string, std::string> funcMap;
  int varCount = 0;
  int funcCount = 0;

  std::string getNewVarName(const std::string& name) {
    if (varMap.find(name) == varMap.end()) {
      varMap[name] = "v" + std::to_string(++varCount);
    }
    return varMap[name];
  }

  std::string getNewFuncName(const std::string& name) {
    if (name == "main") return "main";
    if (builtinFunctions.count(name)) return name;
    if (funcMap.find(name) == funcMap.end()) {
      funcMap[name] = "f" + std::to_string(++funcCount);
    }
    return funcMap[name];
  }

 public:
  Variable *transformVariable(Variable *node) override {
    return new Variable(getNewVarName(node->name));
  }

  CallExpression *transformCallExpression(CallExpression *node) override {
    std::vector<Expression *> args;
    for (auto arg : node->args) {
      args.push_back(transformExpression(arg));
    }
    return new CallExpression(getNewFuncName(node->func), args);
  }

  FunctionDeclaration *transformFunctionDeclaration(FunctionDeclaration *node) override {
    varMap.clear();
    std::vector<Variable *> params;
    for (auto param : node->params) {
      params.push_back(transformVariable(param));
    }
    return new FunctionDeclaration(getNewFuncName(node->name), params, transformStatement(node->body));
  }
};

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
  removeWhitespaces(std::cin);
  if (std::cin.peek() == EOF) {
    // Cheat
    auto cheat = Cheat().transformProgram(prog1);
    std::cout << cheat->toString();
  } else {
    // Anticheat
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
      if (score < 0.8) score = 0.2;
    }
    std::cout << score << std::endl;
  }
  return 0;
}

