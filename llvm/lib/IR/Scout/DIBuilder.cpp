/*
 * ###########################################################################
 * Copyright (c) 2010, Los Alamos National Security, LLC.
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

#include "llvm/IR/DIBuilder.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Dwarf.h"

using namespace llvm;
using namespace llvm::dwarf;

/// createMeshMemberType - Create debugging information entry for a scout
/// mesh member field member.

// Note: the layout of the metadata must be kept in sync. with the
// layout of DIDerivedType as there may be new fields which are added
// to the DIDerivedType, and we need to maintain the ability that
// DIScoutDerivedType is a proper subclass of DIDerivedType

/// getNonCompileUnitScope - If N is compile unit return NULL otherwise return
/// N.

// -------------- The following functions were copied from LLVM
// DIBuilder.cpp and must be kept in sync in the event of changes.

static MDScope *getNonCompileUnitScope(MDNode *N) {
  if (!N || isa<MDCompileUnit>(N))
    return nullptr;
  return cast<MDScope>(N);
}

namespace {
  class HeaderBuilder {
    /// \brief Whether there are any fields yet.
    ///
    /// Note that this is not equivalent to \c Chars.empty(), since \a concat()
    /// may have been called already with an empty string.
    bool IsEmpty;
    SmallVector<char, 256> Chars;
    
  public:
    HeaderBuilder() : IsEmpty(true) {}
    HeaderBuilder(const HeaderBuilder &X) : IsEmpty(X.IsEmpty), Chars(X.Chars) {}
    HeaderBuilder(HeaderBuilder &&X)
    : IsEmpty(X.IsEmpty), Chars(std::move(X.Chars)) {}
    
    template <class Twineable> HeaderBuilder &concat(Twineable &&X) {
      if (IsEmpty)
        IsEmpty = false;
      else
        Chars.push_back(0);
      Twine(X).toVector(Chars);
      return *this;
    }
    
    MDString *get(LLVMContext &Context) const {
      return MDString::get(Context, StringRef(Chars.begin(), Chars.size()));
    }
    
    static HeaderBuilder get(unsigned Tag) {
      return HeaderBuilder().concat("0x" + Twine::utohexstr(Tag));
    }
  };
}

// ----------------------------------------------------

DIScoutCompositeType DIBuilder::createUniformMeshType(DIDescriptor Context,
    StringRef Name, DIFile File,
    unsigned LineNumber,
    uint64_t SizeInBits,
    uint64_t AlignInBits,
    unsigned Flags, DIType DerivedFrom,
    DIArray Elements,
    unsigned DimX,
    unsigned DimY,
    unsigned DimZ,
    unsigned RunTimeLang,
    DIType VTableHolder,
    StringRef UniqueIdentifier
) {
  DIScoutCompositeType R = MDScoutCompositeType::get(
    VMContext, dwarf::DW_TAG_SCOUT_uniform_mesh_type, Name, File, LineNumber,
    MDScopeRef::get(DIScope(getNonCompileUnitScope(Context))), MDTypeRef::get(DerivedFrom),
    SizeInBits, AlignInBits, 0, Flags, Elements, RunTimeLang,
    MDTypeRef::get(VTableHolder), nullptr, UniqueIdentifier, DimX, DimY, DimZ);
  if (!UniqueIdentifier.empty())
    retainType(R);
  trackIfUnresolved(R);
  return R;
}

DIScoutCompositeType DIBuilder::createALEMeshType(DIDescriptor Context,
                                                      StringRef Name, DIFile File,
                                                      unsigned LineNumber,
                                                      uint64_t SizeInBits,
                                                      uint64_t AlignInBits,
                                                      unsigned Flags, DIType DerivedFrom,
                                                      DIArray Elements,
                                                      unsigned DimX,
                                                      unsigned DimY,
                                                      unsigned DimZ,
                                                      unsigned RunTimeLang,
                                                      DIType VTableHolder,
                                                      StringRef UniqueIdentifier
                                                      ) {
  assert(false && "unimplemented");
}


DIScoutCompositeType DIBuilder::createStructuredMeshType(DIDescriptor Context,
    StringRef Name, DIFile File,
    unsigned LineNumber,
    uint64_t SizeInBits,
    uint64_t AlignInBits,
    unsigned Flags, DIType DerivedFrom,
    DIArray Elements,
    unsigned dimX,
    unsigned dimY,
    unsigned dimZ,
    unsigned RunTimeLang,
    DIType VTableHolder,
    StringRef UniqueIdentifier) {
  assert(false && "unimplemented");
}

DIScoutCompositeType DIBuilder::createRectilinearMeshType(DIDescriptor Context,
    StringRef Name, DIFile File,
    unsigned LineNumber,
    uint64_t SizeInBits,
    uint64_t AlignInBits,
    unsigned Flags, DIType DerivedFrom,
    DIArray Elements,
    unsigned dimX,
    unsigned dimY,
    unsigned dimZ,
    unsigned RunTimeLang,
    DIType VTableHolder,
    StringRef UniqueIdentifier) {
  assert(false && "unimplemented");
}

DIScoutCompositeType DIBuilder::createUnstructuredMeshType(DIDescriptor Context,
    StringRef Name, DIFile File,
    unsigned LineNumber,
    uint64_t SizeInBits,
    uint64_t AlignInBits,
    unsigned Flags, DIType DerivedFrom,
    DIArray Elements,
    unsigned dimX,
    unsigned dimY,
    unsigned dimZ,
    unsigned RunTimeLang,
    DIType VTableHolder,
    StringRef UniqueIdentifier) {
  assert(false && "unimplemented");
}

DIScoutDerivedType 
DIBuilder::createMeshMemberType(DIDescriptor Scope, StringRef Name,
                                DIFile File, unsigned LineNumber,
                                uint64_t SizeInBits,
                                uint64_t AlignInBits,
                                uint64_t OffsetInBits,
                                unsigned Flags,
                                unsigned ScoutFlags,
                                DIType Ty) {
  return MDScoutDerivedType::get(VMContext, dwarf::DW_TAG_member, Name, File, LineNumber,
                                 MDScopeRef::get(DIScope(getNonCompileUnitScope(Scope))),
                                 MDTypeRef::get(Ty), SizeInBits,
                                 AlignInBits, OffsetInBits, Flags, ScoutFlags);
}

DIScoutCompositeType
DIBuilder::createScoutForwardDecl(unsigned Tag, StringRef Name, DIDescriptor Scope,
                                  DIFile F, unsigned Line, unsigned RuntimeLang,
                                  uint64_t SizeInBits, uint64_t AlignInBits,
                                  StringRef UniqueIdentifier) {
  // FIXME: Define in terms of createReplaceableForwardDecl() by calling
  // replaceWithUniqued().
  DIScoutCompositeType RetTy =
  MDScoutCompositeType::get(
                            VMContext, Tag, Name, F, Line,
                            MDScopeRef::get(DIScope(getNonCompileUnitScope(Scope))), nullptr,
                            SizeInBits, AlignInBits, 0, DIDescriptor::FlagFwdDecl, nullptr, RuntimeLang, nullptr,
                            nullptr, UniqueIdentifier, 0, 0, 0);
  if (!UniqueIdentifier.empty())
    retainType(RetTy);
  trackIfUnresolved(RetTy);
  return RetTy;
}

DIScoutCompositeType DIBuilder::createReplaceableScoutCompositeType(
                                                                    unsigned Tag, StringRef Name, DIDescriptor Scope, DIFile F, unsigned Line,
                                                                    unsigned RuntimeLang, uint64_t SizeInBits, uint64_t AlignInBits,
                                                                    unsigned Flags, StringRef UniqueIdentifier) {
  DIScoutCompositeType RetTy =
  MDScoutCompositeType::getTemporary(
                                     VMContext, Tag, Name, F, Line,
                                     MDScopeRef::get(DIScope(getNonCompileUnitScope(Scope))),
                                     nullptr, SizeInBits, AlignInBits, 0, Flags,
                                     nullptr, RuntimeLang, nullptr, nullptr,
                                     UniqueIdentifier, 0, 0, 0).release();
  if (!UniqueIdentifier.empty())
    retainType(RetTy);
  trackIfUnresolved(RetTy);
  return RetTy;
}

void DIBuilder::replaceArrays(DIScoutCompositeType &T, DIArray Elements,
                              DIArray TParams) {
  {
    TypedTrackingMDRef<MDScoutCompositeType> N(T);
    if (Elements)
      N->replaceElements(Elements);
    if (TParams)
      N->replaceTemplateParams(MDTemplateParameterArray(TParams));
    T = N.get();
  }
  
  // If T isn't resolved, there's no problem.
  if (!T->isResolved())
    return;
  
  // If "T" is resolved, it may be due to a self-reference cycle.  Track the
  // arrays explicitly if they're unresolved, or else the cycles will be
  // orphaned.
  if (Elements)
    trackIfUnresolved(Elements.get());
  if (TParams)
    trackIfUnresolved(TParams.get());
}
