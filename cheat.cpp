#include <iostream>

#include "lang.h"
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

int main() {
  auto code = scanProgram(std::cin);
  auto cheat = Cheat().transformProgram(code);
  std::cout << cheat->toString();
  return 0;
}
