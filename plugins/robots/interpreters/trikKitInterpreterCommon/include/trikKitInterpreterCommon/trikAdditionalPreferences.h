/* Copyright 2007-2015 QReal Research Group
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

#include <kitBase/additionalPreferences.h>

#include "declSpec.h"

namespace Ui {
class TrikAdditionalPreferences;
}

namespace trik {

class ROBOTS_TRIK_KIT_INTERPRETER_COMMON_EXPORT TrikAdditionalPreferences : public kitBase::AdditionalPreferences
{
	Q_OBJECT

public:
	explicit TrikAdditionalPreferences(const QStringList &realRobotNames, QWidget *parent = nullptr);
	~TrikAdditionalPreferences() override;

	void save() override;
	void restoreSettings() override;
	void onRobotModelChanged(kitBase::robotModel::RobotModelInterface * const robotModel) override;

signals:
	void settingsChanged();
	void packImagesToProjectClicked();

private:
	Ui::TrikAdditionalPreferences *mUi;
	const QStringList mRealRobotNames;

	bool mailboxSavedState = false;
};

}
