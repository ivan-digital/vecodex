// src/DouumentMethadata.h
#include "DocumentMetadata.h"

DocumentMetadata::DocumentMetadata(const std::string& id, const std::unordered_map<std::string, std::string>& attributes)
    : id(id), attributes(attributes) {}

const std::string& DocumentMetadata::getId() const {
    return id;
}

const std::unordered_map<std::string, std::string>& DocumentMetadata::getAttributes() const {
    return attributes;
}
