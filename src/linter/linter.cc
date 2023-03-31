#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static llvm::cl::OptionCategory MyToolCategory("MyTool options");
static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                                llvm::cl::desc("<input file>"),
                                                llvm::cl::init("-"),
                                                llvm::cl::cat(MyToolCategory));

// Matcher to find all function definitions that contain "no_sanitize" attribute
StatementMatcher NoSanitizeMatcher =
    compoundStmt(
        forEachDescendant(functionDecl(hasAttr(clang::attr::NoSanitize))))
        .bind("function");

class NoSanitizeFinder : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    const FunctionDecl *Function =
        Result.Nodes.getNodeAs<FunctionDecl>("function");
    if (Function) {
      llvm::outs() << "Function with no_sanitize attribute found: "
                   << Function->getNameAsString() << "\n";
    }
  }
};

int main(int argc, const char **argv) {
  auto option = CommonOptionsParser::create(argc, argv, MyToolCategory);

  ClangTool Tool(option->getCompilations(), option->getSourcePathList());

  NoSanitizeFinder Finder;
  MatchFinder MatchFinder;
  MatchFinder.addMatcher(NoSanitizeMatcher, &Finder);

  return Tool.run(newFrontendActionFactory(&MatchFinder).get());
}
