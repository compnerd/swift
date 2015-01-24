
cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0022 NEW)
project(swift)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "LLVM: ${LLVM_PACKAGE_VERSION}")

include(CheckIncludeFiles)
include(CheckLibraryExists)
check_include_files(histedit.h HAVE_HISTEDIT_H)
if (HAVE_HISTEDIT_H)
  check_library_exists(edit el_init "" HAVE_LIBEDIT)
endif ()

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++1z -fno-exceptions -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")

include_directories(include)

add_library(support
            STATIC
              lib/support/error-handling.cc
              lib/support/u32string_map.cc
              lib/support/ucs4-support.cc)

add_library(compiler
            STATIC
              lib/compiler/target_info.cc)

add_library(diagnostics
            STATIC
              lib/diagnostics/consumer.cc
              lib/diagnostics/diagnostic_info.cc
              lib/diagnostics/diagnostics.cc
              lib/diagnostics/engine.cc)

add_library(irgen
            STATIC
              lib/irgen/generator.cc
              lib/irgen/module.cc)

add_library(lexer
            STATIC
              lib/lexer/identifier-table.cc
              lib/lexer/lexer.cc
              lib/lexer/location.cc
              lib/lexer/token.cc)

add_library(parser
            STATIC
              lib/parser/parser.cc)

add_library(semantics
            STATIC
              lib/semantics/access.cc
              lib/semantics/analyzer.cc)

add_library(syntax
            STATIC
              lib/syntax/break-statement.cc
              lib/syntax/build-configuration-statement.cc
              lib/syntax/context.cc
              lib/syntax/continue-statement.cc
              lib/syntax/declaration-context.cc
              lib/syntax/declaration.cc
              lib/syntax/expression.cc
              lib/syntax/fallthrough-statement.cc
              lib/syntax/for-in-statement.cc
              lib/syntax/for-statement.cc
              lib/syntax/if-statement.cc
              lib/syntax/labelled-statement.cc
              lib/syntax/repeat-while-statement.cc
              lib/syntax/return-statement.cc
              lib/syntax/source-file.cc
              lib/syntax/statement.cc
              lib/syntax/statements.cc
              lib/syntax/switch-statement.cc
              lib/syntax/defer-statement.cc
              lib/syntax/line-control-statement.cc

              lib/syntax/pattern.cc

              lib/syntax/type.cc
              lib/syntax/type-array.cc
              lib/syntax/type-composite.cc
              lib/syntax/type-dictionary.cc
              lib/syntax/type-function.cc
              lib/syntax/type-identifier.cc
              lib/syntax/type-inout.cc
              lib/syntax/type-metatype.cc
              lib/syntax/type-tuple.cc
              lib/syntax/while-statement.cc

              lib/syntax/printer.cc)

llvm_map_components_to_libnames(_llvm_libs support)
target_link_libraries(parser
                        INTERFACE
                          ${_llvm_libs}
                          compiler
                          diagnostics
                          semantics
                          support
                          syntax)

# add_executable(swiftc tools/swiftc/swiftc.cc)
# target_link_libraries(swiftc lexer)

add_executable(swifti
                 tools/swifti/swifti.cc
                 tools/swifti/stream.cc
                 tools/swifti/line-editor.cc
                 tools/swifti/interpreter.cc)
target_link_libraries(swifti parser lexer)
if (HAVE_LIBEDIT)
  set_target_properties(swifti PROPERTIES COMPILE_DEFINITIONS HAVE_LIBEDIT)
  target_link_libraries(swifti edit)
endif ()

add_executable(ParserTest
                 unit/parser/parser.cc)
target_link_libraries(ParserTest parser lexer)

add_executable(LexerTest
                 unit/lexer/lexer.cc)
target_link_libraries(LexerTest lexer diagnostics support ${_llvm_libs})
