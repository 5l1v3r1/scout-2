//===- IndexTypeSourceInfo.cpp - Indexing types ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "IndexingContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;
using namespace index;

namespace {

class TypeIndexer : public RecursiveASTVisitor<TypeIndexer> {
  IndexingContext &IndexCtx;
  const NamedDecl *Parent;
  const DeclContext *ParentDC;
  bool IsBase;
  SmallVector<SymbolRelation, 3> Relations;

  typedef RecursiveASTVisitor<TypeIndexer> base;

public:
  TypeIndexer(IndexingContext &indexCtx, const NamedDecl *parent,
              const DeclContext *DC, bool isBase)
    : IndexCtx(indexCtx), Parent(parent), ParentDC(DC), IsBase(isBase) {
    if (IsBase) {
      assert(Parent);
      Relations.emplace_back((unsigned)SymbolRole::RelationBaseOf, Parent);
    }
  }
  
  bool shouldWalkTypesOfTypeLocs() const { return false; }

  bool VisitTypedefTypeLoc(TypedefTypeLoc TL) {
    return IndexCtx.handleReference(TL.getTypedefNameDecl(), TL.getNameLoc(),
                                    Parent, ParentDC, SymbolRoleSet(),
                                    Relations);
  }

#define TRY_TO(CALL_EXPR)                                                      \
  do {                                                                         \
    if (!CALL_EXPR)                                                            \
      return false;                                                            \
  } while (0)

  bool traverseParamVarHelper(ParmVarDecl *D) {
    TRY_TO(TraverseNestedNameSpecifierLoc(D->getQualifierLoc()));
    if (D->getTypeSourceInfo())
      TRY_TO(TraverseTypeLoc(D->getTypeSourceInfo()->getTypeLoc()));
    return true;
  }

  bool TraverseParmVarDecl(ParmVarDecl *D) {
    // Avoid visiting default arguments from the definition that were already
    // visited in the declaration.
    // FIXME: A free function definition can have default arguments.
    // Avoiding double visitaiton of default arguments should be handled by the
    // visitor probably with a bit in the AST to indicate if the attached
    // default argument was 'inherited' or written in source.
    if (auto FD = dyn_cast<FunctionDecl>(D->getDeclContext())) {
      if (FD->isThisDeclarationADefinition()) {
        return traverseParamVarHelper(D);
      }
    }

    return base::TraverseParmVarDecl(D);
  }

  bool TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS) {
    IndexCtx.indexNestedNameSpecifierLoc(NNS, Parent, ParentDC);
    return true;
  }

  bool VisitTagTypeLoc(TagTypeLoc TL) {
    TagDecl *D = TL.getDecl();
    if (D->getParentFunctionOrMethod())
      return true;

    if (TL.isDefinition()) {
      IndexCtx.indexTagDecl(D);
      return true;
    }

    return IndexCtx.handleReference(D, TL.getNameLoc(),
                                    Parent, ParentDC, SymbolRoleSet(),
                                    Relations);
  }

  bool VisitObjCInterfaceTypeLoc(ObjCInterfaceTypeLoc TL) {
    return IndexCtx.handleReference(TL.getIFaceDecl(), TL.getNameLoc(),
                                    Parent, ParentDC, SymbolRoleSet());
  }

  bool VisitObjCObjectTypeLoc(ObjCObjectTypeLoc TL) {
    for (unsigned i = 0, e = TL.getNumProtocols(); i != e; ++i) {
      IndexCtx.handleReference(TL.getProtocol(i), TL.getProtocolLoc(i),
                               Parent, ParentDC, SymbolRoleSet());
    }
    return true;
  }

  bool VisitTemplateSpecializationTypeLoc(TemplateSpecializationTypeLoc TL) {
    if (const TemplateSpecializationType *T = TL.getTypePtr()) {
      if (IndexCtx.shouldIndexImplicitTemplateInsts()) {
        if (CXXRecordDecl *RD = T->getAsCXXRecordDecl())
          IndexCtx.handleReference(RD, TL.getTemplateNameLoc(),
                                   Parent, ParentDC, SymbolRoleSet(), Relations);
      } else {
        if (const TemplateDecl *D = T->getTemplateName().getAsTemplateDecl())
          IndexCtx.handleReference(D, TL.getTemplateNameLoc(),
                                   Parent, ParentDC, SymbolRoleSet(), Relations);
      }
    }
    return true;
  }

  bool TraverseStmt(Stmt *S) {
    IndexCtx.indexBody(S, Parent, ParentDC);
    return true;
  }

  // +==== Scout =============================================================+
  // SC_TODO -- implement these.
  bool TraverseMeshTypeLoc(MeshTypeLoc)
  { return true; }
  bool TraverseUniformMeshTypeLoc(UniformMeshTypeLoc)
  { return true; }
  bool TraverseALEMeshTypeLoc(ALEMeshTypeLoc)
  { return true; }
  bool TraverseStructuredMeshTypeLoc(StructuredMeshTypeLoc)
  { return true; }
  bool TraverseRectilinearMeshTypeLoc(RectilinearMeshTypeLoc)
  { return true; }
  bool TraverseUnstructuredMeshTypeLoc(UnstructuredMeshTypeLoc)
  { return true; }

  bool TraverseMeshDecl(MeshDecl*)
  { return true; }
  bool TraverseUniformMeshDecl(UniformMeshDecl*)
  { return true; }
  bool TraverseALEMeshDecl(ALEMeshDecl*)
  { return true; }
  bool TraverseStructuredMeshDecl(StructuredMeshDecl*)
  { return true; }
  bool TraverseRectilinearMeshDecl(RectilinearMeshDecl*)
  { return true; }
  bool TraverseUnstructuredMeshDecl(UnstructuredMeshDecl*)
  { return true; }

  bool TraverseMeshType(MeshType*)
  { return true; }
  bool TraverseUniformMeshType(UniformMeshType*)
  { return true; }
  bool TraverseALEMeshType(ALEMeshType*)
  { return true; }
  bool TraverseStructuredMeshType(StructuredMeshType*)
  { return true; }
  bool TraverseRectilinearMeshType(RectilinearMeshType*)
  { return true; }
  bool TraverseUnstructuredMeshType(UnstructuredMeshType*)
  { return true; }
  // ====================================================================================
};

} // anonymous namespace

void IndexingContext::indexTypeSourceInfo(TypeSourceInfo *TInfo,
                                          const NamedDecl *Parent,
                                          const DeclContext *DC,
                                          bool isBase) {
  if (!TInfo || TInfo->getTypeLoc().isNull())
    return;
  
  indexTypeLoc(TInfo->getTypeLoc(), Parent, DC, isBase);
}

void IndexingContext::indexTypeLoc(TypeLoc TL,
                                   const NamedDecl *Parent,
                                   const DeclContext *DC,
                                   bool isBase) {
  if (TL.isNull())
    return;

  if (!DC)
    DC = Parent->getLexicalDeclContext();
  TypeIndexer(*this, Parent, DC, isBase).TraverseTypeLoc(TL);
}

void IndexingContext::indexNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS,
                                                  const NamedDecl *Parent,
                                                  const DeclContext *DC) {
  if (!NNS)
    return;

  if (NestedNameSpecifierLoc Prefix = NNS.getPrefix())
    indexNestedNameSpecifierLoc(Prefix, Parent, DC);

  if (!DC)
    DC = Parent->getLexicalDeclContext();
  SourceLocation Loc = NNS.getSourceRange().getBegin();

  switch (NNS.getNestedNameSpecifier()->getKind()) {
  case NestedNameSpecifier::Identifier:
  case NestedNameSpecifier::Global:
  case NestedNameSpecifier::Super:
    break;

  case NestedNameSpecifier::Namespace:
    handleReference(NNS.getNestedNameSpecifier()->getAsNamespace(),
                    Loc, Parent, DC, SymbolRoleSet());
    break;
  case NestedNameSpecifier::NamespaceAlias:
    handleReference(NNS.getNestedNameSpecifier()->getAsNamespaceAlias(),
                    Loc, Parent, DC, SymbolRoleSet());
    break;

  case NestedNameSpecifier::TypeSpec:
  case NestedNameSpecifier::TypeSpecWithTemplate:
    indexTypeLoc(NNS.getTypeLoc(), Parent, DC);
    break;
  }
}

void IndexingContext::indexTagDecl(const TagDecl *D) {
  if (!shouldIndexFunctionLocalSymbols() && isFunctionLocalDecl(D))
    return;

  if (handleDecl(D)) {
    if (D->isThisDeclarationADefinition()) {
      indexNestedNameSpecifierLoc(D->getQualifierLoc(), D);
      if (auto CXXRD = dyn_cast<CXXRecordDecl>(D)) {
        for (const auto &I : CXXRD->bases()) {
          indexTypeSourceInfo(I.getTypeSourceInfo(), CXXRD, CXXRD, /*isBase=*/true);
        }
      }
      indexDeclContext(D);
    }
  }
}
