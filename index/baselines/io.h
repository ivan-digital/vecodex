#pragma once
#include <iostream>
#include <type_traits>
template <typename T>
void writeBinary(FILE* fd, T value) {
	if constexpr (std::is_same_v<T, std::string>) {
		size_t sz = value.size();
		std::fwrite(&sz, sizeof(sz), 1, fd);
		std::fwrite(value.data(), sizeof(value[0]), sz, fd);
		return;
	}
	std::fwrite(&value, sizeof(value), 1, fd);
}
template <typename T>
void readBinary(FILE* fd, T& value) {
	if constexpr (std::is_same_v<T, std::string>) {
		size_t sz = 0;
		std::fread(&sz, sizeof(sz), 1, fd);
		value.resize(sz);
		std::fread(value.data(), sizeof(value[0]), sz, fd);
		return;
	}
	std::fread(&value, sizeof(value), 1, fd);
}