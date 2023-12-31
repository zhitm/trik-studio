/* Copyright 2014-2016 CyberTech Labs Ltd.
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

#include "ev3KitInterpreterPlugin.h"

#include <QtWidgets/QApplication>

#include <twoDModel/engine/twoDModelEngineFacade.h>
#include <utils/widgets/comPortPicker.h>

using namespace ev3;
using namespace qReal;

const Id robotDiagramType = Id("RobotsMetamodel", "RobotsDiagram", "RobotsDiagramNode");
const Id subprogramDiagramType = Id("RobotsMetamodel", "RobotsDiagram", "SubprogramDiagram");

Ev3KitInterpreterPlugin::Ev3KitInterpreterPlugin()
	: mUsbRealRobotModel(kitId(), "ev3KitUsbRobot") // todo: somewhere generate robotId for each robot
	, mBluetoothRealRobotModel(kitId(), "ev3KitBluetoothRobot")
	, mTwoDRobotModel(mUsbRealRobotModel)
	, mBlocksFactory(new blocks::Ev3BlocksFactory({"ev3KitUsbRobot", "ev3KitBluetoothRobot"}))
{
	mAdditionalPreferences = new Ev3AdditionalPreferences;

	auto modelEngine = new twoDModel::engine::TwoDModelEngineFacade(mTwoDRobotModel);

	mTwoDRobotModel.setEngine(modelEngine->engine());
	mTwoDModel.reset(modelEngine);

	connect(mAdditionalPreferences, &Ev3AdditionalPreferences::settingsChanged
			, &mUsbRealRobotModel, &robotModel::real::RealRobotModel::rereadSettings);
	connect(mAdditionalPreferences, &Ev3AdditionalPreferences::settingsChanged
			, &mBluetoothRealRobotModel, &robotModel::real::RealRobotModel::rereadSettings);
	connect(mAdditionalPreferences, &Ev3AdditionalPreferences::settingsChanged
			, &mTwoDRobotModel, &robotModel::twoD::TwoDRobotModel::rereadSettings);
}

Ev3KitInterpreterPlugin::~Ev3KitInterpreterPlugin()
{
	release();
}

void Ev3KitInterpreterPlugin::init(const kitBase::KitPluginConfigurator &configurator)
{
	connect(&configurator.eventsForKitPlugin(), &kitBase::EventsForKitPluginInterface::robotModelChanged
			, this, [this](const QString &modelName)
	{
		mCurrentlySelectedModelName = modelName;
	});

	qReal::gui::MainWindowInterpretersInterface &interpretersInterface
			= configurator.qRealConfigurator().mainWindowInterpretersInterface();

	connect(&mUsbRealRobotModel, &robotModel::real::RealRobotModel::errorOccured
			, this, [&interpretersInterface](const QString &message) {
				interpretersInterface.errorReporter()->addError(message);
	});
	connect(&mUsbRealRobotModel, &robotModel::real::RealRobotModel::messageArrived
			, this, [&interpretersInterface](const QString &message) {
				interpretersInterface.errorReporter()->addInformation(message);
	});
	connect(&mBluetoothRealRobotModel, &robotModel::real::RealRobotModel::errorOccured
			, this, [&interpretersInterface](const QString &message) {
				interpretersInterface.errorReporter()->addError(message);
	});
	connect(&mBluetoothRealRobotModel, &robotModel::real::RealRobotModel::messageArrived
			, this, [&interpretersInterface](const QString &message) {
				interpretersInterface.errorReporter()->addInformation(message);
	});

	mTwoDModel->init(configurator.eventsForKitPlugin()
			, configurator.qRealConfigurator().systemEvents()
			, configurator.qRealConfigurator().logicalModelApi()
			, configurator.qRealConfigurator().controller()
			, interpretersInterface
			, configurator.qRealConfigurator().mainWindowDockInterface()
			, configurator.qRealConfigurator().projectManager()
			, configurator.interpreterControl());
}

void Ev3KitInterpreterPlugin::release()
{
	if (mOwnsAdditionalPreferences) {
		delete mAdditionalPreferences;
		mAdditionalPreferences = nullptr;
	}
	mTwoDModel.reset();
}

QString Ev3KitInterpreterPlugin::kitId() const
{
	return "ev3Kit";
}

QString Ev3KitInterpreterPlugin::friendlyKitName() const
{
	return tr("Lego EV3");
}

QList<kitBase::robotModel::RobotModelInterface *> Ev3KitInterpreterPlugin::robotModels()
{
	return {&mUsbRealRobotModel, &mBluetoothRealRobotModel, &mTwoDRobotModel};
}

QSharedPointer<kitBase::blocksBase::BlocksFactoryInterface> Ev3KitInterpreterPlugin::blocksFactoryFor(
		const kitBase::robotModel::RobotModelInterface *model)
{
	if (robotModels().contains(const_cast<kitBase::robotModel::RobotModelInterface *>(model))) {
		return mBlocksFactory;
	} else {
		return nullptr;
	}
}

kitBase::robotModel::RobotModelInterface *Ev3KitInterpreterPlugin::defaultRobotModel()
{
	return &mTwoDRobotModel;
}

QList<kitBase::AdditionalPreferences *> Ev3KitInterpreterPlugin::settingsWidgets()
{
	mOwnsAdditionalPreferences = false;
	return { mAdditionalPreferences };
}

QWidget *Ev3KitInterpreterPlugin::quickPreferencesFor(const kitBase::robotModel::RobotModelInterface &model)
{
	return model.name().toLower().contains("bluetooth")
			? produceBluetoothPortConfigurer()
			: nullptr;
}

QList<qReal::ActionInfo> Ev3KitInterpreterPlugin::customActions()
{
	return {};
}

QList<qReal::HotKeyActionInfo> Ev3KitInterpreterPlugin::hotKeyActions()
{
	return {};
}

QIcon Ev3KitInterpreterPlugin::iconForFastSelector(const kitBase::robotModel::RobotModelInterface &robotModel) const
{
	return &robotModel == &mUsbRealRobotModel
			? QIcon(":/ev3/interpreter/images/switch-real-ev3-usb.svg")
			: &robotModel == &mBluetoothRealRobotModel
					? QIcon(":/ev3/interpreter/images/switch-real-ev3-bluetooth.svg")
					: QIcon(":/ev3/interpreter/images/switch-2d.svg");
}

kitBase::DevicesConfigurationProvider *Ev3KitInterpreterPlugin::devicesConfigurationProvider()
{
	return &mTwoDModel->devicesConfigurationProvider();
}

QString Ev3KitInterpreterPlugin::defaultSettingsFile() const
{
	return ":/ev3/interpreter/ev3DefaultSettings.ini";
}

QWidget *Ev3KitInterpreterPlugin::produceBluetoothPortConfigurer()
{
	QWidget * const result = new ui::ComPortPicker("Ev3BluetoothPortName", this);
	return result;
}
