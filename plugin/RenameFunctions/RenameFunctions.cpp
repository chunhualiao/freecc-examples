#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace std;
using namespace clang;
using namespace llvm;

Rewriter rewriter;

class RenameVisitor : public RecursiveASTVisitor<RenameVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info
 
public:
    explicit RenameVisitor(CompilerInstance *CI)
        : astContext(&(CI->getASTContext())) // initialize private members
    {
        rewriter.setSourceMgr(astContext->getSourceManager(),
            astContext->getLangOpts());
    }
 
    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        string funcName = func->getNameInfo().getName().getAsString();
        if (funcName == "func1") {
            rewriter.ReplaceText(func->getLocation(), funcName.length(), "add");
        }
        if (funcName == "func2") {
            rewriter.ReplaceText(func->getLocation(), funcName.length(), "multiply");
        }

        return true;
    }     
     
    virtual bool VisitStmt(Stmt *st) {
        if (CallExpr *call = dyn_cast<CallExpr>(st)) {
            string callName = call->getDirectCallee()->getNameInfo().getName().getAsString();
            if(callName == "func1") {
                rewriter.ReplaceText(call->getBeginLoc(), callName.length(), "add");
            } else if(callName == "func2") {
                rewriter.ReplaceText(call->getBeginLoc(), callName.length(), "multiply");
            }
        }
        return true;
    }
};

class RenameASTConsumer : public ASTConsumer {
private:
    RenameVisitor *visitor; // doesn't have to be private
 
public:
    explicit RenameASTConsumer(CompilerInstance *CI)
        : visitor(new RenameVisitor(CI)) // initialize the visitor
        { }
 
    virtual void HandleTranslationUnit(ASTContext &Context) {
        errs() << "##########################################\n";
        errs() << "File before parsing\n";
        rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
        errs() << "##########################################\nAnalyzing the file\n";
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
        errs() << "##########################################\n";
        errs() << "File after parsing\n";
        errs() << "##########################################\n";
        rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
    }
};

class PluginRenameAction : public PluginASTAction {
protected:
    unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return make_unique<RenameASTConsumer>(&CI);
    }
 
    bool ParseArgs(const CompilerInstance &CI, const vector<string> &args) {
        return true;
    }
};

static FrontendPluginRegistry::Add<PluginRenameAction>
    X("-rename-plugin", "simple Plugin example");
