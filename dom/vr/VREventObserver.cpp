/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "VREventObserver.h"

#include "nsContentUtils.h"
#include "nsGlobalWindowInner.h"

namespace mozilla::dom {

using namespace gfx;

/**
 * This class is used by nsGlobalWindowInner to implement
 * window.onvrdisplayactivate, window.onvrdisplaydeactivate,
 * window.onvrdisplayconnected, window.onvrdisplaydisconnected, and
 * window.onvrdisplaypresentchange.
 */
VREventObserver::VREventObserver(nsGlobalWindowInner* aGlobalWindow)
    : mWindow(aGlobalWindow), mIs2DView(true), mStopActivity(false) {
  MOZ_ASSERT(aGlobalWindow);

  VRManagerChild* vmc = VRManagerChild::Get();
  if (vmc) {
    vmc->AddListener(this);
  }
}

VREventObserver::~VREventObserver() { DisconnectFromOwner(); }

void VREventObserver::DisconnectFromOwner() {
  // In the event that nsGlobalWindowInner is deallocated, VREventObserver may
  // still be AddRef'ed elsewhere.  Ensure that we don't UAF by
  // dereferencing mWindow.
  mWindow = nullptr;

  // Unregister from VRManagerChild
  if (VRManagerChild::IsCreated()) {
    VRManagerChild* vmc = VRManagerChild::Get();
    vmc->RemoveListener(this);
  }
  mStopActivity = true;
}

void VREventObserver::StartActivity() {
  mStopActivity = false;
  VRManagerChild* vmc = VRManagerChild::Get();
  vmc->StartActivity();
}

void VREventObserver::StopActivity() {
  mStopActivity = true;
  VRManagerChild* vmc = VRManagerChild::Get();
  vmc->StopActivity();
}

bool VREventObserver::GetStopActivityStatus() const { return mStopActivity; }

void VREventObserver::NotifyAfterLoad() {
  if (VRManagerChild::IsCreated()) {
    VRManagerChild* vmc = VRManagerChild::Get();
    vmc->FireDOMVRDisplayConnectEventsForLoad(this);
  }
}

void VREventObserver::NotifyVRDisplayMounted(uint32_t aDisplayID) {
  if (mWindow && mWindow->IsCurrentInnerWindow() && IsWebVR(aDisplayID)) {
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->DispatchVRDisplayActivate(aDisplayID,
                                       VRDisplayEventReason::Mounted);
  }
}

void VREventObserver::NotifyVRDisplayNavigation(uint32_t aDisplayID) {
  if (mWindow && mWindow->IsCurrentInnerWindow() && IsWebVR(aDisplayID)) {
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->DispatchVRDisplayActivate(aDisplayID,
                                       VRDisplayEventReason::Navigation);
  }
}

void VREventObserver::NotifyVRDisplayRequested(uint32_t aDisplayID) {
  if (mWindow && mWindow->IsCurrentInnerWindow() && IsWebVR(aDisplayID)) {
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->DispatchVRDisplayActivate(aDisplayID,
                                       VRDisplayEventReason::Requested);
  }
}

void VREventObserver::NotifyVRDisplayUnmounted(uint32_t aDisplayID) {
  if (mWindow && mWindow->IsCurrentInnerWindow() && IsWebVR(aDisplayID)) {
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->DispatchVRDisplayDeactivate(aDisplayID,
                                         VRDisplayEventReason::Unmounted);
  }
}

void VREventObserver::NotifyVRDisplayConnect(uint32_t aDisplayID) {
  /**
   * We do not call nsGlobalWindowInner::NotifyActiveVRDisplaysChanged here, as
   * we can assume that a newly enumerated display is not presenting WebVR
   * content.
   */
  if (mWindow && mWindow->IsCurrentInnerWindow() && IsWebVR(aDisplayID)) {
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->DispatchVRDisplayConnect(aDisplayID);
  }
}

void VREventObserver::NotifyVRDisplayDisconnect(uint32_t aDisplayID) {
  if (mWindow && mWindow->IsCurrentInnerWindow() && IsWebVR(aDisplayID)) {
    mWindow->NotifyActiveVRDisplaysChanged();
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->DispatchVRDisplayDisconnect(aDisplayID);
  }
}

void VREventObserver::NotifyVRDisplayPresentChange(uint32_t aDisplayID) {
  // When switching to HMD present mode, it is no longer
  // to be a 2D view.
  mIs2DView = false;

  if (mWindow && mWindow->IsCurrentInnerWindow() && IsWebVR(aDisplayID)) {
    mWindow->NotifyActiveVRDisplaysChanged();
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->DispatchVRDisplayPresentChange(aDisplayID);
  }
}

void VREventObserver::NotifyPresentationGenerationChanged(uint32_t aDisplayID) {
  if (mWindow && mWindow->IsCurrentInnerWindow() && IsWebVR(aDisplayID)) {
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->NotifyPresentationGenerationChanged(aDisplayID);
  }
}

void VREventObserver::NotifyEnumerationCompleted() {}

void VREventObserver::NotifyDetectRuntimesCompleted() {
  if (mWindow && mWindow->IsCurrentInnerWindow()) {
    MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());
    mWindow->NotifyDetectXRRuntimesCompleted();
  }
}

bool VREventObserver::IsWebVR(uint32_t aDisplayID) const {
  VRManagerChild* vmc = VRManagerChild::Get();
  if (vmc) {
    return vmc->GetVRAPIMode(aDisplayID) == gfx::VRAPIMode::WebVR;
  }
  return true;
}

}  // namespace mozilla::dom
