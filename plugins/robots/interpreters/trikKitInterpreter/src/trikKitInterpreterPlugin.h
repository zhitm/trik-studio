#pragma once

#include <QtCore/QScopedPointer>

#include <interpreterBase/kitPluginInterface.h>
#include <commonTwoDModel/engine/twoDModelControlInterface.h>

#include "trikAdditionalPreferences.h"
#include "blocks/trikBlocksFactory.h"
#include "robotModel/real/realRobotModelV4.h"
#include "robotModel/real/realRobotModelV5.h"
//#include "robotModel/twoD/twoDRobotModel.h"

namespace trikKitInterpreter {

class TrikKitInterpreterPlugin : public QObject, public interpreterBase::KitPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(interpreterBase::KitPluginInterface)
	Q_PLUGIN_METADATA(IID "nxtKitInterpreter.TrikKitInterpreterPlugin")

public:
	TrikKitInterpreterPlugin();

	void init(interpreterBase::EventsForKitPluginInterface const &eventsForKitPlugin
			, qReal::SystemEventsInterface const &systemEvents
			, interpreterBase::InterpreterControlInterface &interpreterControl) override;

	QString kitId() const override;

	QString friendlyKitName() const override;

	QList<interpreterBase::robotModel::RobotModelInterface *> robotModels() override;

	interpreterBase::blocksBase::BlocksFactoryInterface *blocksFactoryFor(
			interpreterBase::robotModel::RobotModelInterface const *model) override;

	interpreterBase::robotModel::RobotModelInterface *defaultRobotModel() override;

	// Transfers ownership.
	interpreterBase::AdditionalPreferences *settingsWidget() override;

	QList<qReal::ActionInfo> customActions() override;

	interpreterBase::DevicesConfigurationProvider * devicesConfigurationProvider() override;

private slots:
	/// Shows or hides 2d model action depending on whether current tab is robots diagram.
	void onActiveTabChanged(qReal::Id const &rootElementId);

private:
	robotModel::real::RealRobotModelV4 mRealRobotModelV4;
	robotModel::real::RealRobotModelV5 mRealRobotModelV5;
//	robotModel::twoD::TwoDRobotModel mTwoDRobotModel;
	blocks::TrikBlocksFactory *mBlocksFactory;  // Transfers ownership
	TrikAdditionalPreferences *mAdditionalPreferences;  // Transfers ownership
//	QScopedPointer<twoDModel::TwoDModelControlInterface> mTwoDModel;
	interpreterBase::InterpreterControlInterface *mInterpreterControl;  // Does not have ownership.
};

}
