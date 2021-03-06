/*
 *
 * ###########################################################################
 *
 * Copyright (c) 2013, Los Alamos National Security, LLC.
 * All rights reserved.
 *
 *  Copyright 2013. Los Alamos National Security, LLC. This software was
 *  produced under U.S. Government contract DE-AC52-06NA25396 for Los
 *  Alamos National Laboratory (LANL), which is operated by Los Alamos
 *  National Security, LLC for the U.S. Department of Energy. The
 *  U.S. Government has rights to use, reproduce, and distribute this
 *  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
 *  LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
 *  FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
 *  derivative works, such modified software should be clearly marked,
 *  so as not to confuse it with the version available from LANL.
 *
 *  Additionally, redistribution and use in source and binary forms,
 *  with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Los Alamos National Security, LLC, Los
 *      Alamos National Laboratory, LANL, the U.S. Government, nor the
 *      names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 */

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
    OS << "vertices ";
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
    OS << "vertices ";
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

void StmtPrinter::VisitForallArrayStmt(ForallArrayStmt *Node) {
  size_t dims = Node->getDims();

  for (size_t i = 0; i < dims; i++) {
    OS << "for " << Node->getInductionVarInfo(i)->getName() << " in [";
    PrintExpr(Node->getStart(i));
    OS << ":";
    PrintExpr(Node->getEnd(i));
    OS << ":";
    PrintExpr(Node->getStride(i));
    OS << "]";
  }
  if (CompoundStmt *CS = dyn_cast<CompoundStmt>(Node->getBody())) {
    PrintRawCompoundStmt(CS);
    OS << "\n";
  } else {
    OS << "\n";
    PrintStmt(Node->getBody());
  }

}

void StmtPrinter::VisitScoutStmt(ScoutStmt *Node) {
  assert(false && "unimplemented");
}

void StmtPrinter::VisitQueryExpr(QueryExpr *Node) {
  assert(false && "unimplemented");
}

void StmtPrinter::VisitScoutExpr(ScoutExpr *Node) {
  switch(Node->kind()){
    case ScoutExpr::SpecObject:{
      SpecObjectExpr* o = static_cast<SpecObjectExpr*>(Node);

      OS << "{";
      
      auto m = o->memberMap();
      bool first = true;
      
      for(auto& itr : m){
        if(first){
          first = false;
        }
        else{
          OS << ",";
        }
        
        OS << itr.first << ":";

        PrintExpr(itr.second.second);
      }
      
      OS << "}";
      
      break;
    }
    case ScoutExpr::SpecValue:{
      SpecValueExpr* o = static_cast<SpecValueExpr*>(Node);
      PrintExpr(o->getExpression());
      break;
    }
    case ScoutExpr::SpecArray:{
      SpecArrayExpr* o = static_cast<SpecArrayExpr*>(Node);
      
      OS << "[";
      
      auto v = o->elements();
      size_t size = v.size();

      for(size_t i = 0; i < size; ++i){
        if(i > 0){
          OS << ",";
        }
        
        PrintExpr(v[i]);
      }
      
      OS << "]";
      
      break;
    }
    default:
      assert(false && "unimplemented");
  }
}

void StmtPrinter::VisitStencilShiftExpr(StencilShiftExpr *Node) {
  assert(false && "unimplemented");
}
