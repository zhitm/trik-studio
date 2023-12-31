/* Copyright 2013-2016 CyberTech Labs Ltd.
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

#include "nxtKitInterpreterPlugin.h"

#include <QtWidgets/QApplication>

#include <twoDModel/engine/twoDModelEngineFacade.h>
#include <utils/widgets/comPortPicker.h>

using namespace nxt;
using namespace qReal;

const Id robotDiagramType = Id("RobotsMetamodel", "RobotsDiagram", "RobotsDiagramNode");
const Id subprogramDiagramType = Id("RobotsMetamodel", "RobotsDiagram", "SubprogramDiagram");

NxtKitInterpreterPlugin::NxtKitInterpreterPlugin()
	: mUsbRealRobotModel(kitId(), "nxtKitUsbRobot") // todo: somewhere generate robotId for each robot
	, mBluetoothRealRobotModel(kitId(), "nxtKitBluetoothRobot")
	, mTwoDRobotModel(mUsbRealRobotModel)
	, mBlocksFactory(new blocks::NxtBlocksFactory)
{
	mAdditionalPreferences = new NxtAdditionalPreferences(mBluetoothRealRobotModel.name());

	auto modelEngine = new twoDModel::engine::TwoDModelEngineFacade(mTwoDRobotModel);

	mTwoDRobotModel.setEngine(modelEngine->engine());
	mTwoDModel.reset(modelEngine);

	connect(mAdditionalPreferences, &NxtAdditionalPreferences::settingsChanged
			, &mUsbRealRobotModel, &robotModel::real::RealRobotModel::rereadSettings);
	connect(mAdditionalPreferences, &NxtAdditionalPreferences::settingsChanged
			, &mBluetoothRealRobotModel, &robotModel::real::RealRobotModel::rereadSettings);
	connect(mAdditionalPreferences, &NxtAdditionalPreferences::settingsChanged
			, &mTwoDRobotModel, &robotModel::twoD::TwoDRobotModel::rereadSettings);
}

NxtKitInterpreterPlugin::~NxtKitInterpreterPlugin()
{
	release();
}

void NxtKitInterpreterPlugin::init(const kitBase::KitPluginConfigurator &configurator)
{
	connect(&configurator.eventsForKitPlugin(), &kitBase::EventsForKitPluginInterface::robotModelChanged
			, this, [this](const QString &modelName)
	{
		mCurrentlySelectedModelName = modelName;
	});

	qReal::gui::MainWindowInterpretersInterface &interpretersInterface
			= configurator.qRealConfigurator().mainWindowInterpretersInterface();
	connect(&mUsbRealRobotModel, &robotModel::real::RealRobotModel::errorOccured
			, [&interpretersInterface](const QString &message) {
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

void NxtKitInterpreterPlugin::release()
{
	if (mOwnsAdditionalPreferences) {
		delete mAdditionalPreferences;
		mAdditionalPreferences = nullptr;
	}
	mTwoDModel.reset();
}

QString NxtKitInterpreterPlugin::kitId() const
{
	return "nxtKit";
}

QString NxtKitInterpreterPlugin::friendlyKitName() const
{
	return tr("Lego NXT");
}

QList<kitBase::robotModel::RobotModelInterface *> NxtKitInterpreterPlugin::robotModels()
{
	return {&mUsbRealRobotModel, &mBluetoothRealRobotModel, &mTwoDRobotModel};
}

QSharedPointer<kitBase::blocksBase::BlocksFactoryInterface> NxtKitInterpreterPlugin::blocksFactoryFor(
		const kitBase::robotModel::RobotModelInterface *model)
{
	Q_UNUSED(model)
	return mBlocksFactory;
}

kitBase::robotModel::RobotModelInterface *NxtKitInterpreterPlugin::defaultRobotModel()
{
	return &mTwoDRobotModel;
}

QList<kitBase::AdditionalPreferences *> NxtKitInterpreterPlugin::settingsWidgets()
{
	mOwnsAdditionalPreferences = false;
	return {mAdditionalPreferences};
}

QWidget *NxtKitInterpreterPlugin::quickPreferencesFor(const kitBase::robotModel::RobotModelInterface &model)
{
	return model.name().toLower().contains("bluetooth")
			? produceBluetoothPortConfigurer()
			: nullptr;
}

QList<qReal::ActionInfo> NxtKitInterpreterPlugin::customActions()
{
	return {};
}

QList<HotKeyActionInfo> NxtKitInterpreterPlugin::hotKeyActions()
{
	return {};
}

QString NxtKitInterpreterPlugin::defaultSettingsFile() const
{
	return ":/nxtDefaultSettings.ini";
}

QIcon NxtKitInterpreterPlugin::iconForFastSelector(
		const kitBase::robotModel::RobotModelInterface &robotModel) const
{
	return &robotModel == &mUsbRealRobotModel
			? QIcon(":/icons/switch-real-nxt-usb.svg")
			: &robotModel == &mBluetoothRealRobotModel
					? QIcon(":/icons/switch-real-nxt-bluetooth.svg")
					: QIcon(":/icons/switch-2d.svg");
}

kitBase::DevicesConfigurationProvider *NxtKitInterpreterPlugin::devicesConfigurationProvider()
{
	return &mTwoDModel->devicesConfigurationProvider();
}

QWidget *NxtKitInterpreterPlugin::produceBluetoothPortConfigurer()
{
	QWidget * const result = new ui::ComPortPicker("NxtBluetoothPortName", this);
	return result;
}
