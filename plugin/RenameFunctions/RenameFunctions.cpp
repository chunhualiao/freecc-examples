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

    // Function to get the base name of the file provided by path
    string basename(std::string path) {
        return std::string( std::find_if(path.rbegin(), path.rend(), MatchPathSeparator()).base(), path.end());
    }

    // Used by std::find_if
    struct MatchPathSeparator
    {
        bool operator()(char ch) const {
            return ch == '/';
        }
    };
 
public:
    explicit RenameASTConsumer(CompilerInstance *CI)
        : visitor(new RenameVisitor(CI)) // initialize the visitor
        { }
 
    virtual void HandleTranslationUnit(ASTContext &Context) {
        visitor->TraverseDecl(Context.getTranslationUnitDecl());

        // Create an output file to write the updated code
        FileID id = rewriter.getSourceMgr().getMainFileID();
        string filename = "/tmp/" + basename(rewriter.getSourceMgr().getFilename(rewriter.getSourceMgr().getLocForStartOfFile(id)).str());
        std::error_code OutErrorInfo;
        std::error_code ok;
        llvm::raw_fd_ostream outFile(llvm::StringRef(filename),
            OutErrorInfo, llvm::sys::fs::F_None);
        if (OutErrorInfo == ok) {
            const RewriteBuffer *RewriteBuf = rewriter.getRewriteBufferFor(id);
            outFile << std::string(RewriteBuf->begin(), RewriteBuf->end());
            errs() << "Output file created - " << filename << "\n";
        } else {
            llvm::errs() << "Could not create file\n";
        }
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
