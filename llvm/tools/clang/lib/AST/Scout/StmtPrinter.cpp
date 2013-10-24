// Note - this file is included by the StmtPrinter source file
// one directory up (StmtPrinter is all contained in a single
// file there...).
//
void StmtPrinter::VisitForallMeshStmt(ForallMeshStmt *Node) {
  Indent() << "forall ";

  if (Node->isOverCells())
    OS << "cells ";
  else if (Node->isOverEdges())
    OS << "edges ";
  else if (Node->isOverVertices())
    OS << "verticies ";
  else if (Node->isOverFaces())
    OS << "faces ";
  else
    OS << "<unknown mesh element>";

  OS << Node->getRefVarInfo()->getName() << " in ";
  OS << Node->getMeshInfo()->getName() << " ";

  if (Node->hasPredicate()) {
    OS << "(";
    PrintExpr(Node->getPredicate());
    OS << ")";
  }

  if (CompoundStmt *CS = dyn_cast<CompoundStmt>(Node->getBody())) {
    PrintRawCompoundStmt(CS);
    OS << "\n";
  } else {
    OS << "\n";
    PrintStmt(Node->getBody());
  }
}



void StmtPrinter::VisitRenderallMeshStmt(RenderallMeshStmt *Node) {
  Indent() << "renderall ";

  if (Node->isOverCells())
    OS << "cells ";
  else if (Node->isOverEdges())
    OS << "edges ";
  else if (Node->isOverVertices())
    OS << "verticies ";
  else if (Node->isOverFaces())
    OS << "faces ";
  else
    OS << "<unknown mesh element>";

  OS << Node->getRefVarInfo()->getName() << " in ";
  OS << Node->getMeshInfo()->getName() << " ";

  if (Node->hasPredicate()) {
    OS << "(";
    PrintExpr(Node->getPredicate());
    OS << ")";
  }

  if (CompoundStmt *CS = dyn_cast<CompoundStmt>(Node->getBody())) {
    PrintRawCompoundStmt(CS);
    OS << "\n";
  } else {
    OS << "\n";
    PrintStmt(Node->getBody());
  }
}


