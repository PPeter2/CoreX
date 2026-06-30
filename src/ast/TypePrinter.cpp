#include "corex/ast/TypePrinter.h"

std::string typeToString(const Type* type) {
    if (type == nullptr) return "null";

    switch (type->kind) {
        case TypeKind::Named:
            return static_cast<const NamedType*>(type)->name;

        case TypeKind::Pointer: {
            const PointerType* pointer = static_cast<const PointerType*>(type);
            std::string result = "(ptr";
            if (pointer->isVolatile) result += " volatile";
            result += pointer->isMutable ? " mut" : " const";
            result += " " + typeToString(pointer->pointee.get()) + ")";
            return result;
        }

        case TypeKind::Array: {
            const ArrayType* array = static_cast<const ArrayType*>(type);
            return "(array " + typeToString(array->elementType.get()) + " " + array->size + ")";
        }

        case TypeKind::Slice: {
            const SliceType* slice = static_cast<const SliceType*>(type);
            return "(slice " + typeToString(slice->elementType.get()) + ")";
        }

        case TypeKind::Generic: {
            const GenericType* generic = static_cast<const GenericType*>(type);
            std::string result = "(generic " + generic->name;
            for (const auto& argument : generic->arguments) {
                result += " " + typeToString(argument.get());
            }
            result += ")";
            return result;
        }
    }

    return "unknown";
}