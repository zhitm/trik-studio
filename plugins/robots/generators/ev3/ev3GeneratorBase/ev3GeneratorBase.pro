# Copyright 2007-2015 QReal Research Group
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

TARGET = robots-ev3-generator-base

TEMPLATE = lib

include(../../../../../global.pri)

QT += widgets

CONFIG += trikqscintilla2

includes(plugins/robots/generators/ev3/ev3GeneratorBase \
		plugins/robots/generators/generatorBase \
		plugins/robots/common/kitBase \
		plugins/robots/common/ev3Kit \
		plugins/robots/utils \
		qrtext \
)

links(qrkernel qrutils qrrepo robots-generator-base robots-ev3-kit robots-kit-base)

DEFINES += ROBOTS_EV3_GENERATOR_BASE_LIBRARY

TRANSLATIONS = $$PWD/../../../../../qrtranslations/ru/plugins/robots/ev3GeneratorBase_ru.ts

HEADERS += \
	$$PWD/include/ev3GeneratorBase/ev3GeneratorPluginBase.h \
	$$PWD/include/ev3GeneratorBase/ev3GeneratorBaseDeclSpec.h \
	$$PWD/include/ev3GeneratorBase/ev3MasterGeneratorBase.h \
	$$PWD/include/ev3GeneratorBase/ev3GeneratorFactory.h \
	$$PWD/include/ev3GeneratorBase/robotModel/ev3GeneratorRobotModel.h \
	$$PWD/include/ev3GeneratorBase/parts/ev3Mailboxes.h \
	$$PWD/src/ev3GeneratorCustomizer.h \
	$$PWD/src/simpleGenerators/drawLineGenerator.h \
	$$PWD/src/simpleGenerators/drawPixelGenerator.h \
	$$PWD/src/simpleGenerators/drawRectGenerator.h \
	$$PWD/src/simpleGenerators/drawCircleGenerator.h \
	$$PWD/src/simpleGenerators/enginesGenerator.h \
	$$PWD/src/simpleGenerators/enginesStopGenerator.h \
	$$PWD/src/simpleGenerators/ledGenerator.h \
	$$PWD/src/simpleGenerators/nullificationEncoderGenerator.h \
	$$PWD/src/converters/outputPortNameConverter.h \
	$$PWD/src/converters/ledColorConverter.h \
	$$PWD/src/simpleGenerators/sendMailGenerator.h \
	$$PWD/src/simpleGenerators/receiveMailGenerator.h \
	$$PWD/src/simpleGenerators/calibrateGyroscopeGenerator.h \
	$$PWD/src/simpleGenerators/startCompassCalibrationGenerator.h \
	$$PWD/src/simpleGenerators/stopCompassCalibrationGenerator.h \
	$$PWD/src/simpleGenerators/readRGBGenerator.h \
	$$PWD/src/simpleGenerators/lineLeader/calibrateBlackGenerator.h \
	$$PWD/src/simpleGenerators/lineLeader/calibratePIDGenerator.h \
	$$PWD/src/simpleGenerators/lineLeader/calibrateWhiteGenerator.h \
	$$PWD/src/simpleGenerators/lineLeader/readAverageGenerator.h \
	$$PWD/src/simpleGenerators/lineLeader/readSensorArrayGenerator.h \
	$$PWD/src/simpleGenerators/lineLeader/readSteeringGenerator.h \
	$$PWD/src/simpleGenerators/lineLeader/sleepGenerator.h \
	$$PWD/src/simpleGenerators/lineLeader/wakeUpGenerator.h \
	$$PWD/src/simpleGenerators/waitForColorBlockGenerator.h \
	$$PWD/src/converters/colorConverter.h \

SOURCES += \
	$$PWD/src/ev3GeneratorPluginBase.cpp \
	$$PWD/src/ev3MasterGeneratorBase.cpp \
	$$PWD/src/ev3GeneratorCustomizer.cpp \
	$$PWD/src/ev3GeneratorFactory.cpp \
	$$PWD/src/robotModel/ev3GeneratorRobotModel.cpp \
	$$PWD/src/parts/ev3Mailboxes.cpp \
	$$PWD/src/simpleGenerators/drawLineGenerator.cpp \
	$$PWD/src/simpleGenerators/drawPixelGenerator.cpp \
	$$PWD/src/simpleGenerators/drawRectGenerator.cpp \
	$$PWD/src/simpleGenerators/drawCircleGenerator.cpp \
	$$PWD/src/simpleGenerators/enginesGenerator.cpp \
	$$PWD/src/simpleGenerators/enginesStopGenerator.cpp \
	$$PWD/src/simpleGenerators/ledGenerator.cpp \
	$$PWD/src/simpleGenerators/nullificationEncoderGenerator.cpp \
	$$PWD/src/converters/outputPortNameConverter.cpp \
	$$PWD/src/converters/ledColorConverter.cpp \
	$$PWD/src/simpleGenerators/sendMailGenerator.cpp \
	$$PWD/src/simpleGenerators/receiveMailGenerator.cpp \
	$$PWD/src/simpleGenerators/calibrateGyroscopeGenerator.cpp \
	$$PWD/src/simpleGenerators/startCompassCalibrationGenerator.cpp \
	$$PWD/src/simpleGenerators/stopCompassCalibrationGenerator.cpp \
	$$PWD/src/simpleGenerators/readRGBGenerator.cpp \
	$$PWD/src/simpleGenerators/lineLeader/calibrateBlackGenerator.cpp \
	$$PWD/src/simpleGenerators/lineLeader/calibratePIDGenerator.cpp \
	$$PWD/src/simpleGenerators/lineLeader/calibrateWhiteGenerator.cpp \
	$$PWD/src/simpleGenerators/lineLeader/readAverageGenerator.cpp \
	$$PWD/src/simpleGenerators/lineLeader/readSensorArrayGenerator.cpp \
	$$PWD/src/simpleGenerators/lineLeader/readSteeringGenerator.cpp \
	$$PWD/src/simpleGenerators/lineLeader/sleepGenerator.cpp \
	$$PWD/src/simpleGenerators/lineLeader/wakeUpGenerator.cpp \
	$$PWD/src/simpleGenerators/waitForColorBlockGenerator.cpp \
	$$PWD/src/converters/colorConverter.cpp \

OTHER_FILES +=
