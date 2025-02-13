// Copyright Boymue Authors. All rights reserved.
// Author boymue on 2021.01.26

#include "BoymueApplication.h"

#include "JSTestApi.h"
#include "JsLogApi.h"

namespace boymue {

BoymueApplication::BoymueApplication(BoymueAppInfo* info)
    : m_appInfo(info),
      m_uiThread("ui_thread_" + info->appName),
      m_ioThread("io_thread_" + info->appName),
      m_jsThread("js_thread_" + info->appName) {
  m_jsEngine = std::make_unique<JsEngine>();

  m_uiThread.start();
  m_ioThread.start();
  m_jsThread.start();

  getUITaskRunner().postTask(
      [self = this] { self->m_mainView = std::make_unique<BoymueView>(self); });

  getJSTaskRunner().postTask([self = this] {
    self->m_mainRuntime =
        std::unique_ptr<JsRuntime>(self->m_jsEngine->createRuntime());
    self->m_mainRuntime->registerApi(new boymue::JsLogApi(self));
    self->m_mainRuntime->registerApi(new boymue::JsTestApi(self));
    self->m_mainRuntime->setContext(self);
  });
}

void BoymueApplication::evaluateJs(const String& jsSource, const String& scriptId) {
  getJSTaskRunner().postTask(
      [=] { this->m_mainRuntime->evaluateJs(jsSource, scriptId); });
}

void BoymueApplication::doRuntimeAction(RuntimeClosure& action) {
    getJSTaskRunner().postTask(
        [=] { this->m_mainRuntime->doAction(action); });
}

// 结束当前应用的线程
BoymueApplication::~BoymueApplication() {
  m_uiThread.terminate();
  m_ioThread.terminate();
  m_jsThread.terminate();
}

TaskRunner& BoymueApplication::getUITaskRunner() const {
  return m_uiThread.getTaskRunner();
}

TaskRunner& BoymueApplication::getIOTaskRunner() const {
  return m_ioThread.getTaskRunner();
}

TaskRunner& BoymueApplication::getJSTaskRunner() const {
  return m_jsThread.getTaskRunner();
}
}  // namespace boymue
