// Copyright 2014 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos-dbus-bindings/adaptor_generator.h"

#include <string>
#include <vector>

#include <base/file_util.h>
#include <base/files/file_path.h>
#include <base/files/scoped_temp_dir.h>
#include <gtest/gtest.h>

#include "chromeos-dbus-bindings/interface.h"

using std::string;
using std::vector;
using testing::Test;

namespace chromeos_dbus_bindings {

namespace {

const char kMethod0Name[] = "Kaneda";
const char kMethod0Return[] = "s";
const char kMethod0Argument0[] = "s";
const char kMethod0ArgumentName0[] = "iwata";
const char kMethod0Argument1[] = "ao";
const char kMethod0ArgumentName1[] = "clarke";
const char kMethod1Name[] = "Tetsuo";
const char kMethod1Argument1[] = "i";
const char kMethod1Return[] = "x";
const char kMethod2Name[] = "Kei";
const char kInterfaceName[] = "org.chromium.TestInterface";
const char kExpectedContent[] = R"literal_string(
#include <string>
#include <vector>

#include <base/macros.h>
#include <dbus/object_path.h>
#include <chromeos/dbus/dbus_object.h>
#include <chromeos/dbus/exported_object_manager.h>
#include <chromeos/variant_dictionary.h>

namespace org {
namespace chromium {

class TestInterfaceAdaptor {
 public:
  class TestInterfaceAdaptorMethodInterface {
   public:
    virtual std::string Kaneda(
        chromeos::ErrorPtr* /* error */,
        const std::string& /* iwata */,
        const std::vector<dbus::ObjectPath>& /* clarke */) = 0;
    virtual int64_t Tetsuo(
        chromeos::ErrorPtr* /* error */,
        int32_t) = 0;
    virtual void Kei(
        chromeos::ErrorPtr* /* error */) = 0;
  };
  TestInterfaceAdaptor(
      chromeos::dbus_utils::ExportedObjectManager* object_manager,
      const std::string& object_path,
      TestInterfaceAdaptorMethodInterface* interface)  // Owned by caller.
      : interface_(interface),
        dbus_object_(
            object_manager,
            object_manager->GetBus(),
            dbus::ObjectPath(object_path)) {
    auto* itf =
        dbus_object_.AddOrGetInterface("org.chromium.TestInterface");
    itf->AddMethodHandler(
        "Kaneda",
        base::Unretained(interface_),
        &TestInterfaceAdaptorMethodInterface::Kaneda);
    itf->AddMethodHandler(
        "Tetsuo",
        base::Unretained(interface_),
        &TestInterfaceAdaptorMethodInterface::Tetsuo);
    itf->AddMethodHandler(
        "Kei",
        base::Unretained(interface_),
        &TestInterfaceAdaptorMethodInterface::Kei);
    dbus_object_.RegisterAsync(base::Bind(
        &TestInterfaceAdaptor::OnRegisterComplete, base::Unretained(this)));
  }
  virtual ~TestInterfaceAdaptor() = default;
  virtual void OnRegisterComplete(bool success) {}
 private:
  TestInterfaceAdaptorMethodInterface* interface_;  // Owned by caller.
  chromeos::dbus_utils::DBusObject dbus_object_;
  DISALLOW_COPY_AND_ASSIGN(TestInterfaceAdaptor);
};

}  // namespace chromium
}  // namespace org
)literal_string";

}  // namespace
class AdaptorGeneratorTest : public Test {
 public:
  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
  }

 protected:
  base::FilePath CreateInputFile(const string& contents) {
    base::FilePath path;
    EXPECT_TRUE(base::CreateTemporaryFileInDir(temp_dir_.path(), &path));
    EXPECT_EQ(contents.size(),
              base::WriteFile(path, contents.c_str(), contents.size()));
    return path;
  }

  base::ScopedTempDir temp_dir_;
  AdaptorGenerator generator_;
};

TEST_F(AdaptorGeneratorTest, GenerateAdaptors) {
  Interface interface;
  interface.name = kInterfaceName;
  interface.methods.emplace_back(
      kMethod0Name,
      vector<Interface::Argument>{
          Interface::Argument(kMethod0ArgumentName0, kMethod0Argument0),
          Interface::Argument(kMethod0ArgumentName1, kMethod0Argument1) },
      vector<Interface::Argument>{ Interface::Argument("", kMethod0Return) });
  interface.methods.emplace_back(
      kMethod1Name,
      vector<Interface::Argument>{ Interface::Argument("", kMethod1Argument1) },
      vector<Interface::Argument>{ Interface::Argument("", kMethod1Return) });
  interface.methods.emplace_back(kMethod2Name);
  base::FilePath output_path = temp_dir_.path().Append("output.h");
  EXPECT_TRUE(generator_.GenerateAdaptor(interface, output_path));
  string contents;
  EXPECT_TRUE(base::ReadFileToString(output_path, &contents));
  // The header guards contain the (temporary) filename, so we search for
  // the content we need within the string.
  EXPECT_NE(string::npos, contents.find(kExpectedContent))
      << "Expected to find the following content...\n"
      << kExpectedContent << "...within content...\n" << contents;
}

}  // namespace chromeos_dbus_bindings
