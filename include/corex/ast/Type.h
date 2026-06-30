#pragma once
#include <string>
#include <vector>
#include <memory>

enum class TypeKind {
    Named,
    Pointer,
    Array,
    Slice,
    Generic
};

struct Type {
    TypeKind kind;
    int line;
    int column;

    Type(TypeKind kind, int line, int column)
        : kind(kind), line(line), column(column) {}

    virtual ~Type() = default;
};

struct NamedType : Type {
    std::string name;

    NamedType(std::string name, int line, int column)
        : Type(TypeKind::Named, line, column), name(std::move(name)) {}
};

struct PointerType : Type {
    std::unique_ptr<Type> pointee;
    bool isMutable;
    bool isVolatile;

    PointerType(std::unique_ptr<Type> pointee, bool isMutable, bool isVolatile, int line, int column)
        : Type(TypeKind::Pointer, line, column), pointee(std::move(pointee)), isMutable(isMutable), isVolatile(isVolatile) {}
};

struct ArrayType : Type {
    std::unique_ptr<Type> elementType;
    std::string size;

    ArrayType(std::unique_ptr<Type> elementType, std::string size, int line, int column)
        : Type(TypeKind::Array, line, column), elementType(std::move(elementType)), size(std::move(size)) {}
};

struct SliceType : Type {
    std::unique_ptr<Type> elementType;

    SliceType(std::unique_ptr<Type> elementType, int line, int column)
        : Type(TypeKind::Slice, line, column), elementType(std::move(elementType)) {}
};

struct GenericType : Type {
    std::string name;
    std::vector<std::unique_ptr<Type>> arguments;

    GenericType(std::string name, std::vector<std::unique_ptr<Type>> arguments, int line, int column)
        : Type(TypeKind::Generic, line, column), name(std::move(name)), arguments(std::move(arguments)) {}
};