//===--- RISCV.cpp - RISCV ToolChain Implementations ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RISCV.h"
#include "CommonArgs.h"
#include "InputInfo.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

/// RISCV Toolchain
RISCVToolChain::RISCVToolChain(const Driver &D, const llvm::Triple &Triple,
                               const ArgList &Args)
  : Generic_ELF(D, Triple, Args) {
  GCCInstallation.init(Triple, Args);
  GCCVersion Version = GCCInstallation.getVersion();
  getFilePaths().push_back(D.SysRoot + "/usr/lib");
  getFilePaths().push_back(GCCInstallation.getInstallPath().str());
}

Tool *RISCVToolChain::buildLinker() const {
  return new tools::RISCV::Linker(*this);
}

void RISCV::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs,
                                 const ArgList &Args,
                                 const char *LinkingOutput) const {
  const ToolChain &ToolChain = getToolChain();
  const Driver &D = ToolChain.getDriver();
  bool IsIAMCU = getToolChain().getTriple().isOSIAMCU();
  ArgStringList CmdArgs;

  if (!D.SysRoot.empty())
    CmdArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));

  std::string Linker = getToolChain().GetProgramPath(getShortName());

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crt0.o")));
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crtbegin.o")));
  }

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  ToolChain.AddFilePathLibArgs(Args, CmdArgs);
  Args.AddAllArgs(CmdArgs,
                  {options::OPT_T_Group, options::OPT_e, options::OPT_s,
                   options::OPT_t, options::OPT_Z_Flag, options::OPT_r});

  if (D.isUsingLTO())
    AddGoldPlugin(ToolChain, Args, CmdArgs, D.getLTOMode() == LTOK_Thin, D);

  bool NeedsSanitizerDeps = addSanitizerRuntimes(ToolChain, Args, CmdArgs);
  AddLinkerInputs(ToolChain, Inputs, Args, CmdArgs, JA);

  if (Args.hasArg(options::OPT_Z_Xlinker__no_demangle))
    CmdArgs.push_back("--no-demangle");

  if (D.CCCIsCXX() &&
      !Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs)) {
    ToolChain.AddCXXStdlibLibArgs(Args, CmdArgs);
    CmdArgs.push_back("-lm");
  }

  if (!Args.hasArg(options::OPT_nostdlib)) {
    if (!Args.hasArg(options::OPT_nodefaultlibs)) {
        CmdArgs.push_back("--start-group");

      if (NeedsSanitizerDeps)
        linkSanitizerRuntimeDeps(ToolChain, CmdArgs);

      bool WantPthread = Args.hasArg(options::OPT_pthread) ||
                         Args.hasArg(options::OPT_pthreads);

      if (Args.hasFlag(options::OPT_fopenmp, options::OPT_fopenmp_EQ,
                       options::OPT_fno_openmp, false)) {
        // OpenMP runtimes implies pthreads when using the GNU toolchain.
        // FIXME: Does this really make sense for all GNU toolchains?
        WantPthread = true;

        // Also link the particular OpenMP runtimes.
        switch (ToolChain.getDriver().getOpenMPRuntime(Args)) {
        case Driver::OMPRT_OMP:
          CmdArgs.push_back("-lomp");
          break;
        case Driver::OMPRT_GOMP:
          CmdArgs.push_back("-lgomp");

          // FIXME: Exclude this for platforms with libgomp that don't require
          // librt. Most modern Linux platforms require it, but some may not.
          CmdArgs.push_back("-lrt");
          break;
        case Driver::OMPRT_IOMP5:
          CmdArgs.push_back("-liomp5");
          break;
        case Driver::OMPRT_Unknown:
          // Already diagnosed.
          break;
        }
        if (JA.isHostOffloading(Action::OFK_OpenMP))
          CmdArgs.push_back("-lomptarget");
      }

      if (WantPthread)
        CmdArgs.push_back("-lpthread");

      if (Args.hasArg(options::OPT_fsplit_stack))
        CmdArgs.push_back("--wrap=pthread_create");

      CmdArgs.push_back("-lc");

      CmdArgs.push_back("-lgloss");

      // Default static linking.
      // We may need to add AddRunTimeLibs when we support dynamic linking.
      //
      CmdArgs.push_back("--end-group");
      if (ToolChain.GetRuntimeLibType(Args) == ToolChain::RLT_Libgcc)
        CmdArgs.push_back("-lgcc");
      else
        CmdArgs.push_back(ToolChain.getCompilerRTArgString(Args, "builtins"));

      // Add IAMCU specific libs (outside the group), if needed.
      if (IsIAMCU) {
        CmdArgs.push_back("--as-needed");
        CmdArgs.push_back("-lsoftfp");
        CmdArgs.push_back("--no-as-needed");
      }
    }
  }

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles))
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crtend.o")));

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());
  C.addCommand(llvm::make_unique<Command>(JA, *this, Args.MakeArgString(Linker),
                                          CmdArgs, Inputs));
}

void RISCVToolChain::addLibStdCxxIncludePaths(const llvm::opt::ArgList &DriverArgs,
                                              llvm::opt::ArgStringList &CC1Args) const {
  // We need a detected GCC installation to provide libstdc++'s headers.
  if (!GCCInstallation.isValid())
    return;

  // By default, look for the C++ headers in an include directory adjacent to
  // the lib directory of the GCC installation. Note that this is expect to be
  // equivalent to '/usr/include/c++/X.Y' in almost all cases.
  StringRef LibDir = GCCInstallation.getParentLibPath();
  StringRef InstallDir = GCCInstallation.getInstallPath();
  StringRef TripleStr = GCCInstallation.getTriple().str();
  const Multilib &Multilib = GCCInstallation.getMultilib();
  const GCCVersion &Version = GCCInstallation.getVersion();

  // Otherwise, fall back on a bunch of options which don't use multiarch
  // layouts for simplicity.
  const std::string LibStdCXXIncludePathCandidates[] = {
      // Gentoo is weird and places its headers inside the GCC install,
      // so if the first attempt to find the headers fails, try these patterns.
      InstallDir.str() + "/include/g++-v" + Version.Text,
      InstallDir.str() + "/include/g++-v" + Version.MajorStr + "." +
          Version.MinorStr,
      InstallDir.str() + "/include/g++-v" + Version.MajorStr,
      // Android standalone toolchain has C++ headers in yet another place.
      LibDir.str() + "/../" + TripleStr.str() + "/include/c++/" + Version.Text,
      // Freescale SDK C++ headers are directly in <sysroot>/usr/include/c++,
      // without a subdirectory corresponding to the gcc version.
      LibDir.str() + "/../include/c++",
  };

  for (const auto &IncludePath : LibStdCXXIncludePathCandidates) {
    if (addLibStdCXXIncludePaths(IncludePath, /*Suffix*/ "", TripleStr,
                                 /*GCCMultiarchTriple*/ "",
                                 /*TargetMultiarchTriple*/ "",
                                 Multilib.includeSuffix(), DriverArgs, CC1Args))
      break;
  }
}
// RISCV tools end.
