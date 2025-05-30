//===-- lib/runtime/main.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "flang/Runtime/main.h"
#include "flang-rt/runtime/environment.h"
#include "flang-rt/runtime/terminator.h"
#include <cfenv>
#include <cstdio>
#include <cstdlib>
#if defined(__x86_64__)
#include <immintrin.h>
#endif

extern "C" {
void RTNAME(feenableexcept)(uint32_t);
}

static void ConfigureFloatingPoint() {
#ifdef feclearexcept // a macro in some environments; omit std::
  feclearexcept(FE_ALL_EXCEPT);
#else
  std::feclearexcept(FE_ALL_EXCEPT);
#endif
#ifdef fesetround
  fesetround(FE_TONEAREST);
#else
  std::fesetround(FE_TONEAREST);
#endif

  if (Fortran::runtime::executionEnvironment.feEnableException) {
    RTNAME(feenableexcept)(
        Fortran::runtime::executionEnvironment.feEnableException);
  }
}

extern "C" {
void RTNAME(ProgramStart)(int argc, const char *argv[], const char *envp[],
    const EnvironmentDefaultList *envDefaults) {
  std::atexit(Fortran::runtime::NotifyOtherImagesOfNormalEnd);
  Fortran::runtime::executionEnvironment.Configure(
      argc, argv, envp, envDefaults);
  ConfigureFloatingPoint();
  // I/O is initialized on demand so that it works for non-Fortran main().
}

void RTNAME(ByteswapOption)() {
  if (Fortran::runtime::executionEnvironment.conversion ==
      Fortran::runtime::Convert::Unknown) {
    // The environment variable overrides the command-line option;
    // either of them take precedence over explicit OPEN(CONVERT=) specifiers.
    Fortran::runtime::executionEnvironment.conversion =
        Fortran::runtime::Convert::Swap;
  }
}
}
