include_directories(
  ../
  ${CMAKE_BINARY_DIR}/config/include
)

set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_library(scoutSema
  ASTVisitors.cpp
  Sema.cpp
  SemaChecking.cpp
  SemaDecl.cpp
  SemaExpr.cpp
  SemaExprMember.cpp
  SemaOverload.cpp
  SemaStmt.cpp

  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangEdit
  clangLex
  )
