#include "ASTNormalizer.h"

#include <clang-c/Index.h>
#include <fstream>
#include <stdexcept>
#include <unordered_set>














static const char* kindLabel(CXCursorKind k)
{
    switch (k)
    {
        
        case CXCursor_FunctionDecl:         return "FunctionDecl";
        case CXCursor_CXXMethod:            return "MethodDecl";
        case CXCursor_Constructor:          return "ConstructorDecl";
        case CXCursor_Destructor:           return "DestructorDecl";
        case CXCursor_VarDecl:              return "VarDecl";
        case CXCursor_ParmDecl:             return "ParamDecl";
        case CXCursor_FieldDecl:            return "FieldDecl";
        case CXCursor_StructDecl:           return "StructDecl";
        case CXCursor_ClassDecl:            return "ClassDecl";
        case CXCursor_EnumDecl:             return "EnumDecl";
        case CXCursor_EnumConstantDecl:     return "EnumConst";
        case CXCursor_TypedefDecl:          return "TypedefDecl";
        case CXCursor_Namespace:        return "Namespace";
        case CXCursor_TemplateTypeParameter: return "TemplateTypeParam";

        
        case CXCursor_CompoundStmt:         return "Block";
        case CXCursor_IfStmt:               return "IfStmt";
        case CXCursor_ForStmt:              return "ForStmt";
        case CXCursor_WhileStmt:            return "WhileStmt";
        case CXCursor_DoStmt:               return "DoStmt";
        case CXCursor_SwitchStmt:           return "SwitchStmt";
        case CXCursor_CaseStmt:             return "CaseStmt";
        case CXCursor_DefaultStmt:          return "DefaultStmt";
        case CXCursor_BreakStmt:            return "BreakStmt";
        case CXCursor_ContinueStmt:         return "ContinueStmt";
        case CXCursor_ReturnStmt:           return "ReturnStmt";
        case CXCursor_GotoStmt:             return "GotoStmt";
        case CXCursor_NullStmt:             return "NullStmt";
        case CXCursor_DeclStmt:             return "DeclStmt";

        
        case CXCursor_BinaryOperator:       return "BinOp";
        case CXCursor_CompoundAssignOperator: return "CompoundAssign";
        case CXCursor_UnaryOperator:        return "UnaryOp";
        case CXCursor_CallExpr:             return "CallExpr";
        case CXCursor_ArraySubscriptExpr:   return "ArrayAccess";
        case CXCursor_MemberRefExpr:        return "MemberAccess";
        case CXCursor_ConditionalOperator:  return "TernaryOp";
        case CXCursor_CStyleCastExpr:       return "CastExpr";
        case CXCursor_InitListExpr:         return "InitList";
        case CXCursor_CXXNewExpr:           return "NewExpr";
        case CXCursor_CXXDeleteExpr:        return "DeleteExpr";
        case CXCursor_CXXThrowExpr:         return "ThrowExpr";
        case CXCursor_CXXTryStmt:           return "TryStmt";
        case CXCursor_CXXCatchStmt:         return "CatchStmt";

        
        case CXCursor_IntegerLiteral:
        case CXCursor_FloatingLiteral:
        case CXCursor_CharacterLiteral:
        case CXCursor_StringLiteral:
        case CXCursor_CXXBoolLiteralExpr:
        case CXCursor_CXXNullPtrLiteralExpr: return "LIT";

        
        case CXCursor_DeclRefExpr:          return nullptr;
        case CXCursor_TypeRef:              return nullptr;

        
        case CXCursor_UnexposedExpr:        return nullptr;
        case CXCursor_UnexposedStmt:        return nullptr;
        case CXCursor_UnexposedDecl:        return nullptr;
        case CXCursor_UnexposedAttr:        return nullptr;

        
        case CXCursor_MacroDefinition:      return nullptr;
        case CXCursor_MacroExpansion:       return nullptr;
        case CXCursor_InclusionDirective:   return nullptr;

        default:                            return nullptr;
    }
}



struct VisitorData
{
    TreeNode* currentNode;
    CXTranslationUnit tu;
};

static CXChildVisitResult buildVisitor(CXCursor cursor,
                                  CXCursor ,
                                  CXClientData data)
{
    CXSourceLocation loc = clang_getCursorLocation(cursor);
    if (clang_Location_isInSystemHeader(loc))
        return CXChildVisit_Continue;

    CXCursorKind kind = clang_getCursorKind(cursor);

    const char* label = kindLabel(kind);
    if (!label)
    {
        clang_visitChildren(cursor, buildVisitor, data);
        return CXChildVisit_Continue;
    }

    auto* parentNode = reinterpret_cast<TreeNode*>(data);
    parentNode->children.push_back(TreeNode{label, {}});
    TreeNode* myNode = &parentNode->children.back();

    clang_visitChildren(cursor, buildVisitor, myNode);
    return CXChildVisit_Continue;
}

TreeNode
ASTNormalizer::normalize(const std::string& filepath)
{
    CXIndex idx = clang_createIndex(
        0,
        0);

    if (!idx)
        throw std::runtime_error("clang_createIndex failed");

    const bool isCpp = filepath.ends_with(".cpp") ||
                       filepath.ends_with(".cc")  ||
                       filepath.ends_with(".cxx") ||
                       filepath.ends_with(".hpp");

    std::vector<const char*> args = {
        "-std=c++20",
        "-w",
        "-fno-delayed-template-parsing",
    };
    if (!isCpp)
        args = {"-std=c11", "-w"};

    CXTranslationUnit tu = clang_parseTranslationUnit(
        idx,
        filepath.c_str(),
        args.data(), (int)args.size(),
        nullptr,
        0,
        CXTranslationUnit_SingleFileParse    );

    if (!tu)
    {
        clang_disposeIndex(idx);
        throw std::runtime_error("AST parse failed: " + filepath);
    }

    TreeNode root{"TranslationUnit", {}};
    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(rootCursor, buildVisitor, &root);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(idx);

    return root;
}
