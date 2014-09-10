// Copyright 2014 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include <base/command_line.h>
#include <base/files/file_path.h>
#include <base/logging.h>

#include "chromeos-dbus-bindings/adaptor_generator.h"
#include "chromeos-dbus-bindings/method_name_generator.h"
#include "chromeos-dbus-bindings/xml_interface_parser.h"

namespace switches {

static const char kHelp[] = "help";
static const char kInput[] = "input";
static const char kMethodNames[] = "method-names";
static const char kAdaptor[] = "adaptor";
static const char kHelpMessage[] = "\n"
    "Available Switches: \n"
    "  --input=<interface>\n"
    "    The input XML interface file (mandatory).\n"
    "  --method-names=<method name header filename>\n"
    "    The output header file with string constants for each method name.\n"
    "  --adaptor=<adaptor header filename>\n"
    "    The output header file with DBus adaptor class.\n";

}  // namespace switches

int main(int argc, char** argv) {
  CommandLine::Init(argc, argv);
  CommandLine* cl = CommandLine::ForCurrentProcess();

  if (cl->HasSwitch(switches::kHelp)) {
    LOG(INFO) << switches::kHelpMessage;
    return 0;
  }

  if (!cl->HasSwitch(switches::kInput)) {
    LOG(ERROR) << switches::kInput << " switch is mandatory.";
    LOG(ERROR) << switches::kHelpMessage;
    return 1;
  }

  std::string input = cl->GetSwitchValueASCII(switches::kInput);

  chromeos_dbus_bindings::XmlInterfaceParser parser;
  if (!parser.ParseXmlInterfaceFile(base::FilePath(input))) {
    LOG(ERROR) << "Failed to parse interface file.";
    return 1;
  }

  if (cl->HasSwitch(switches::kMethodNames)) {
    std::string method_name_file =
        cl->GetSwitchValueASCII(switches::kMethodNames);
    LOG(INFO) << "Outputting method names to " << method_name_file;
    chromeos_dbus_bindings::MethodNameGenerator method_name_generator;
    if (!method_name_generator.GenerateMethodNames(
            parser.interface(),
            base::FilePath(method_name_file))) {
      LOG(ERROR) << "Failed to output method names.";
      return 1;
     }
  }

  if (cl->HasSwitch(switches::kAdaptor)) {
    std::string adaptor_file = cl->GetSwitchValueASCII(switches::kAdaptor);
    LOG(INFO) << "Outputting adaptor to " << adaptor_file;
    chromeos_dbus_bindings::AdaptorGenerator adaptor_generator;
    if (!adaptor_generator.GenerateAdaptor(
            parser.interface(),
            base::FilePath(adaptor_file))) {
      LOG(ERROR) << "Failed to output adaptor.";
      return 1;
     }
  }

  return 0;
}
