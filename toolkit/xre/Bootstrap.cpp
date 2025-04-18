/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/Bootstrap.h"
#include "nsXPCOM.h"

#include "AutoSQLiteLifetime.h"

#if defined(XP_WIN) && defined(_M_X64) && defined(MOZ_DIAGNOSTIC_ASSERT_ENABLED)
#  include <windows.h>
#endif  // XP_WIN && _M_X64 && MOZ_DIAGNOSTIC_ASSERT_ENABLED

#ifdef MOZ_WIDGET_ANDROID
#  include "mozilla/jni/Utils.h"
#  ifdef MOZ_PROFILE_GENERATE
extern "C" int __llvm_profile_dump(void);
#  endif
#endif

namespace mozilla {

class BootstrapImpl final : public Bootstrap {
 protected:
  AutoSQLiteLifetime mSQLLT;

  virtual void Dispose() override { delete this; }

 public:
  BootstrapImpl() = default;

  ~BootstrapImpl() = default;

  virtual void NS_LogInit() override { ::NS_LogInit(); }

  virtual void NS_LogTerm() override { ::NS_LogTerm(); }

  virtual void XRE_StartupTimelineRecord(int aEvent,
                                         mozilla::TimeStamp aWhen) override {
    ::XRE_StartupTimelineRecord(aEvent, aWhen);
  }

  virtual int XRE_main(int argc, char* argv[],
                       const BootstrapConfig& aConfig) override {
    return ::XRE_main(argc, argv, aConfig);
  }

  virtual void XRE_StopLateWriteChecks() override {
    ::XRE_StopLateWriteChecks();
  }

  virtual int XRE_XPCShellMain(int argc, char** argv, char** envp,
                               const XREShellData* aShellData) override {
    return ::XRE_XPCShellMain(argc, argv, envp, aShellData);
  }

  virtual nsresult XRE_InitChildProcess(
      int argc, char* argv[], const XREChildData* aChildData) override {
    return ::XRE_InitChildProcess(argc, argv, aChildData);
  }

  virtual void XRE_EnableSameExecutableForContentProc() override {
    ::XRE_EnableSameExecutableForContentProc();
  }

#ifdef MOZ_WIDGET_ANDROID
  virtual void XRE_SetGeckoThreadEnv(JNIEnv* aEnv) override {
    mozilla::jni::SetGeckoThreadEnv(aEnv);
  }

  virtual void XRE_SetAndroidChildFds(JNIEnv* aEnv, jintArray aFds) override {
    ::XRE_SetAndroidChildFds(aEnv, aFds);
  }

#  ifdef MOZ_PROFILE_GENERATE
  virtual void XRE_WriteLLVMProfData() override {
    __android_log_print(ANDROID_LOG_INFO, "GeckoLibLoad",
                        "Calling __llvm_profile_dump()");
    __llvm_profile_dump();
  }
#  endif
#endif

#ifdef LIBFUZZER
  virtual void XRE_LibFuzzerSetDriver(LibFuzzerDriver aDriver) override {
    ::XRE_LibFuzzerSetDriver(aDriver);
  }
#endif

#ifdef MOZ_ENABLE_FORKSERVER
  virtual int XRE_ForkServer(int* argc, char*** argv) override {
    return ::XRE_ForkServer(argc, argv);
  }
#endif
};

#if defined(XP_WIN) && defined(_M_X64) && defined(MOZ_DIAGNOSTIC_ASSERT_ENABLED)
extern "C" uint32_t _tls_index;

extern "C" NS_EXPORT bool XRE_CheckBlockScopeStaticVarInit(
    uint32_t* aTlsIndex) {
  // Copy the value of xul's _tls_index for diagnostics.
  if (aTlsIndex) {
    *aTlsIndex = _tls_index;
  }

  // Check that block-scope static variable initialization works. We use
  // volatile here to keep the compiler honest - we want it to generate the code
  // that will ensure that only a single thread goes through the lambda.
  static bool sItWorks = []() -> bool {
    bool const volatile value = true;
    return value;
  }();
  return sItWorks;
}
#endif  // XP_WIN && _M_X64 && MOZ_DIAGNOSTIC_ASSERT_ENABLED

extern "C" NS_EXPORT void NS_FROZENCALL
XRE_GetBootstrap(Bootstrap::UniquePtr& b) {
  static bool sBootstrapInitialized = false;
  MOZ_RELEASE_ASSERT(!sBootstrapInitialized);

  sBootstrapInitialized = true;
  b.reset(new BootstrapImpl());
}

}  // namespace mozilla
