/* Copyright 2021 CyberTech Labs Ltd.
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

#include <kitBase/robotModel/robotParts/lidarSensor.h>

#include "twoDModel/twoDModelDeclSpec.h"

namespace twoDModel {

namespace engine {
class TwoDModelEngineInterface;
}

namespace robotModel {
namespace parts {

/// 2D-model simulation of lidar.
/// Configuration is perfomed immediately, the answer is ready immediately too.
class TWO_D_MODEL_EXPORT Lidar : public kitBase::robotModel::robotParts::LidarSensor
{
	Q_OBJECT

public:
	Lidar(const kitBase::robotModel::DeviceInfo &info
			, const kitBase::robotModel::PortInfo &port
			, engine::TwoDModelEngineInterface &engine
			, QPair<qreal, int> angleAndRange);

	void read() override;

protected:
	engine::TwoDModelEngineInterface &mEngine;
	qreal mAngle;
	int mRange;
};

}
}
}
