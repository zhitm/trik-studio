/* Copyright 2017 QReal Research Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#pragma once

#include <QtCore/QString>

namespace pioneer {

/// Names for robot models supported by Pioneer plugins.
namespace modelNames {
const auto realCopter = "PioneerRealCopter";
const auto simulator = "PioneerSimulator";
}

/// Names for used global settings.
namespace settings {

/// String parameter. Path to a Lua compiler that is used to produce binaries for real copter.
const auto pioneerRealCopterLuaPath = "pioneerRealCopterLuaPath";

/// String parameter. Path to a Lua compiler that is used to produce binaries for a simulator.
const auto pioneerSimulatorLuaPath = "pioneerSimulatorLuaPath";

/// String parameter. IP address of a Pioneer base station.
const auto pioneerBaseStationIP = "pioneerBaseStationIP";

/// String parameter. Port of a Pioneer base station.
const auto pioneerBaseStationPort = "pioneerBaseStationPort";

/// String parameter. Mode of a Pioneer base station.
const auto pioneerBaseStationMode = "pioneerBaseStationMode";

/// String parameter. Path to a Python interpreter. Default is empty string, which is treated as "use python from PATH".
const auto pioneerPythonPath = "pioneerPythonPath";

/// Boolean parameter. True if "controller" shall be used to upload or run programs instead of POST requests.
const auto pioneerUseController = "pioneerUseController";

/// Boolean parameter. If set, COM port and "controller" are used to upload and run program instead of IP address.
const auto pioneerUseComPort = "pioneerUseComPort";

/// String parameter. Name of a COM port to use to upload or run programs.
const auto pioneerComPort = "pioneerComPort";

}

}
