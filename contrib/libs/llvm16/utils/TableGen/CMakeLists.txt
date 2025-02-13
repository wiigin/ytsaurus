
# This file was generated by the build system used internally in the Yandex monorepo.
# Only simple modifications are allowed (adding source-files to targets, adding simple properties
# like target_include_directories). These modifications will be ported to original
# ya.make files by maintainers. Any complex modifications which can't be ported back to the
# original buildsystem will not be accepted.


add_subdirectory(GlobalISel)

add_executable(llvm-tblgen)
target_compile_options(llvm-tblgen PRIVATE
  $<IF:$<CXX_COMPILER_ID:MSVC>,,-Wno-everything>
)
target_include_directories(llvm-tblgen PRIVATE
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen
)
target_link_libraries(llvm-tblgen PUBLIC
  contrib-libs-linux-headers
  contrib-libs-cxxsupp
  contrib-libs-llvm16
  llvm16-lib-Demangle
  llvm16-lib-Support
  llvm16-lib-TableGen
  utils-TableGen-GlobalISel
)
target_link_options(llvm-tblgen PRIVATE
  -ldl
  -lrt
  -Wl,--no-as-needed
  -fPIC
  -fPIC
  -lpthread
  -lrt
  -ldl
)
target_sources(llvm-tblgen PRIVATE
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/AsmMatcherEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/AsmWriterEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/AsmWriterInst.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/Attributes.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CTagsEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CallingConvEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CodeEmitterGen.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CodeGenDAGPatterns.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CodeGenHwModes.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CodeGenInstruction.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CodeGenMapTable.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CodeGenRegisters.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CodeGenSchedule.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CodeGenTarget.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/CompressInstEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DAGISelEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DAGISelMatcher.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DAGISelMatcherEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DAGISelMatcherGen.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DAGISelMatcherOpt.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DFAEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DFAPacketizerEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DXILEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DecoderEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DirectiveEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/DisassemblerEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/ExegesisEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/FastISelEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/GICombinerEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/GlobalISelEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/InfoByHwMode.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/InstrDocsEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/InstrInfoEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/IntrinsicEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/OptEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/OptParserEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/OptRSTEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/PredicateExpander.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/PseudoLoweringEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/RISCVTargetDefEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/RegisterBankEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/RegisterInfoEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/SDNodeProperties.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/SearchableTableEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/SubtargetEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/SubtargetFeatureInfo.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/TableGen.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/Types.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/VarLenCodeEmitterGen.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/WebAssemblyDisassemblerEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/X86DisassemblerTables.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/X86EVEX2VEXTablesEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/X86FoldTablesEmitter.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/X86MnemonicTables.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/X86ModRMFilters.cpp
  ${CMAKE_SOURCE_DIR}/contrib/libs/llvm16/utils/TableGen/X86RecognizableInstr.cpp
)
target_allocator(llvm-tblgen
  cpp-malloc-tcmalloc
  libs-tcmalloc-no_percpu_cache
)
vcs_info(llvm-tblgen)
