/*
 * ###########################################################################
 * Copyright (c) 2013, Los Alamos National Security, LLC.
 * All rights reserved.
 * 
 *  Copyright 2010. Los Alamos National Security, LLC. This software was
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
 * ########################################################################### 
 * 
 * Notes
 *
 * ##### 
 */ 
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Scout/BuiltinsScout.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"
#include <stdio.h>
using namespace clang;
using namespace CodeGen;
using namespace llvm;

static char IRNameStr[160];


// deal w/ scout builtins
bool CodeGenFunction::EmitScoutBuiltinExpr(const FunctionDecl *FD,
                                           unsigned BuiltinID,
                                           const CallExpr *E ,
                                           RValue *RV) {

  switch (BuiltinID) {

  //vector position
  case Builtin::BIposition: {
    static const char *IndexNames[] = { "x", "y", "z", "w"};
    Value *Result =
       llvm::UndefValue::get(llvm::VectorType::get(Int32Ty, 4));

     for (unsigned i = 0; i <= 3; ++i) {
       if (i == 3) sprintf(IRNameStr, "forall.linearidx");
       else sprintf(IRNameStr, "forall.induct.%s", IndexNames[i]);
       LoadInst *LI = Builder.CreateLoad(LookupInductionVar(i), IRNameStr);

       sprintf(IRNameStr, "position.%s", IndexNames[i]);
       Result = Builder.CreateInsertElement(Result, LI,
            Builder.getInt32(i), IRNameStr);
     }
     *RV = RValue::get(Result);
     return true;
  }

  case Builtin::BIpositionx: {
    *RV = RValue::get(Builder.CreateLoad(LookupInductionVar(0), "forall.induct.x"));
    return true;
  }

  case Builtin::BIpositiony: {
    *RV = RValue::get(Builder.CreateLoad(LookupInductionVar(1), "forall.induct.y"));
    return true;
  }

  case Builtin::BIpositionz: {
    *RV = RValue::get(Builder.CreateLoad(LookupInductionVar(2), "forall.induct.z"));
    return true;
  }

  case Builtin::BIpositionw: {
    *RV = RValue::get(Builder.CreateLoad(LookupInductionVar(3), "forall.linearidx"));
    return true;
  }

  case Builtin::BIwidth: {
    // number of args is already known to be 0 or 1 as it was checked in sema
    if(E->getNumArgs() == 0) { //inside forall/renderall/stencil
      // if we can lookup the LoopBound Decl then we must be in a stencil function
      if (MeshDims[0] || LocalDeclMap.lookup(ScoutABIMeshDimDecl[0])) {
        *RV = RValue::get(Builder.CreateLoad(LookupMeshDim(0), "width"));
      } else {
        CGM.getDiags().Report(E->getExprLoc(), diag::warn_mesh_intrinsic_outside_scope);
        *RV = RValue::get(llvm::ConstantInt::get(Int32Ty, 0));
      }
    } else  { //outside forall/renderall/stencil
      *RV = EmitMeshParameterExpr(E->getArg(0), MeshParameterOffset::WidthOffset);
    }
    return true;
  }

  case Builtin::BIheight: {
    // number of args is already known to be 0 or 1 as it was checked in sema
    if(E->getNumArgs() == 0) { //inside forall/renderall/stencil
      // if we can lookup the MeshDims Decl then we must be in a stencil function
      if (MeshDims[1] || LocalDeclMap.lookup(ScoutABIMeshDimDecl[1])) {
        *RV = RValue::get(Builder.CreateLoad(LookupMeshDim(1), "height"));
      } else {
        CGM.getDiags().Report(E->getExprLoc(), diag::warn_mesh_intrinsic_outside_scope);
        *RV = RValue::get(llvm::ConstantInt::get(Int32Ty, 0));
      }
    } else { //outside forall/renderall/stencil
      *RV = EmitMeshParameterExpr(E->getArg(0), MeshParameterOffset::HeightOffset);
    }
    return true;
  }

  case Builtin::BIdepth: {
    // number of args is already known to be 0 or 1 as it was checked in sema
    if(E->getNumArgs() == 0) { //inside forall/renderall/stencil
      // if we can lookup the MeshDims Decl then we must be in a stencil function
      if (MeshDims[2] || LocalDeclMap.lookup(ScoutABIMeshDimDecl[2])) {
        *RV = RValue::get(Builder.CreateLoad(LookupMeshDim(2), "depth"));
      } else {
        CGM.getDiags().Report(E->getExprLoc(), diag::warn_mesh_intrinsic_outside_scope);
        *RV = RValue::get(llvm::ConstantInt::get(Int32Ty, 0));
      }
    } else { //outside forall/renderall/stencil
      *RV = EmitMeshParameterExpr(E->getArg(0), MeshParameterOffset::DepthOffset);
    }
    return true;
	  }

  case Builtin::BIrank: {
    // number of args is already known to be 0 or 1 as it was checked in sema
    if(E->getNumArgs() == 0) { //inside forall/renderall
      if (MeshRank)
        *RV = RValue::get(Builder.CreateLoad(MeshRank, "rank"));
      else {
        CGM.getDiags().Report(E->getExprLoc(), diag::warn_mesh_intrinsic_outside_scope);
        *RV = RValue::get(llvm::ConstantInt::get(Int32Ty, 0));
      }
    } else { //outside forall/renderall
         *RV = EmitMeshParameterExpr(E->getArg(0), MeshParameterOffset::RankOffset);
    }
      return true;
  }
    
  case Builtin::BIcshift:
  case Builtin::BIcshifti:
  case Builtin::BIcshiftf:
  case Builtin::BIcshiftd:
  {
    *RV = EmitCShiftExpr(E->arg_begin(), E->arg_end());
    return true;
  }

  case Builtin::BIeoshift:
  case Builtin::BIeoshifti:
  case Builtin::BIeoshiftf:
  case Builtin::BIeoshiftd:
  {
    *RV = EmitEOShiftExpr(E->arg_begin(), E->arg_end());
    return true;
  }

  default: return false;
  }
}
