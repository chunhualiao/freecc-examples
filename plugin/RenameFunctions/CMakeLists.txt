add_llvm_library(RenameFunctions MODULE RenameFunctions.cpp PLUGIN_TOOL clang)

if(LLVM_ENABLE_PLUGINS AND (WIN32 OR CYGWIN))
  target_link_libraries(RenameFunctions PRIVATE
    clangAST
    clangBasic
    clangFrontend
    LLVMSupport
    )
endif()
