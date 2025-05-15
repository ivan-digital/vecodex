# Copilot Instructions

This project is a C++ codebase with a focus on modern best practices and maintainability.

## Coding Standards
- Use C++17 features where possible.
- Use snake_case for function and variable names.
- Use PascalCase for class and struct names.
- Prefer smart pointers (std::unique_ptr, std::shared_ptr) over raw pointers.
- Always write unit tests for new code using GoogleTest.
- Use RAII for resource management.
- Prefer `const` correctness and mark member functions as `const` when possible.
- Use `override` for overridden virtual methods.
- Use `nullptr` instead of `NULL` or 0 for pointers.
- Prefer range-based for loops and STL algorithms.
- Use `#pragma once` in header files.
- Document public APIs with Doxygen-style comments.

## Project Context
- The codebase includes components for coordination, searching, writing, and storage.
- External dependencies include Abseil, AWS SDK, Etcd, and gRPC.
- Tests are located in the `tests/` directories and use GoogleTest.

## Commit and Review Guidelines
- Write clear, descriptive commit messages.
- Ensure all new code is covered by tests.
- Run all tests before submitting a pull request.
- **Before executing any change, create and review a plan outlining the intended modifications.**

---

Feel free to expand or modify these instructions to better fit your team's workflow.
