#ifndef DOCUMENT_METADATA_H
#define DOCUMENT_METADATA_H

#include <string>
#include <unordered_map>

class DocumentMetadata {
public:
    DocumentMetadata(const std::string& id, const std::unordered_map<std::string, std::string>& attributes);

    const std::string& getId() const;
    const std::unordered_map<std::string, std::string>& getAttributes() const;

private:
    std::string id;
    std::unordered_map<std::string, std::string> attributes;
};

#endif // DOCUMENT_METADATA_H
