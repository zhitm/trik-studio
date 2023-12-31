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

#include "ev3RbfGeneratorPlugin.h"

#include <QtWidgets/QApplication>
#include <QtCore/QDirIterator>
#include <QtCore/QProcess>

#include <qrutils/widgets/qRealMessageBox.h>
#include <qrkernel/logging.h>
#include <ev3Kit/communication/ev3RobotCommunicationThread.h>
#include <ev3GeneratorBase/robotModel/ev3GeneratorRobotModel.h>
#include <qrkernel/settingsManager.h>
#include "ev3RbfMasterGenerator.h"
#include <QsLog.h>

using namespace ev3::rbf;
using namespace qReal;

Ev3RbfGeneratorPlugin::Ev3RbfGeneratorPlugin()
	: Ev3GeneratorPluginBase("Ev3RbfUsbGeneratorRobotModel", tr("Autonomous mode (USB)"), 9
			, "Ev3RbfBluetoothGeneratorRobotModel", tr("Autonomous mode (Bluetooth)"), 8)
	, mGenerateCodeAction(new QAction(this))
	, mUploadProgramAction(new QAction(this))
	, mRunProgramAction(new QAction(this))
	, mStopRobotAction(new QAction(this))
{
	mGenerateCodeAction->setText(tr("Generate to Ev3 Robot Byte Code File"));
	mGenerateCodeAction->setIcon(QIcon(":/ev3/rbf/images/generateRbfCode.svg"));
	connect(mGenerateCodeAction, &QAction::triggered, this, &Ev3RbfGeneratorPlugin::generateCode);

	mUploadProgramAction->setText(tr("Upload program"));
	mUploadProgramAction->setIcon(QIcon(":/ev3/rbf/images/uploadProgram.svg"));
	connect(mUploadProgramAction
			, &QAction::triggered
			, this
			, [this](){
		auto rp = static_cast<RunPolicy>(SettingsManager::value("ev3RunPolicy").toInt());
		uploadAndRunProgram(rp);
	});

	mRunProgramAction->setObjectName("runEv3RbfProgram");
	mRunProgramAction->setText(tr("Run program"));
	mRunProgramAction->setIcon(QIcon(":/ev3/rbf/images/run.png"));
	connect(mRunProgramAction, &QAction::triggered
			, this
			, [this]{ uploadAndRunProgram(RunPolicy::AlwaysRun); }
	);

	mStopRobotAction->setObjectName("stopEv3RbfRobot");
	mStopRobotAction->setText(tr("Stop robot"));
	mStopRobotAction->setIcon(QIcon(":/ev3/rbf/images/stop.png"));
	connect(mStopRobotAction, &QAction::triggered, this, &Ev3RbfGeneratorPlugin::stopRobot, Qt::UniqueConnection);

	text::Languages::registerLanguage(text::LanguageInfo{ "lms"
			, tr("EV3 Source Code language")
			, true
			, 4
			, 2
			, "//"
			, QString()
			, "/*"
			, "*/"
			, nullptr
			, {}
	});

	mJavaDetected = javaInstalled();
}

QList<ActionInfo> Ev3RbfGeneratorPlugin::customActions()
{
	const ActionInfo generateCodeActionInfo(mGenerateCodeAction, "generators", "tools");
	const ActionInfo uploadProgramActionInfo(mUploadProgramAction, "generators", "tools");
	const ActionInfo runProgramActionInfo(mRunProgramAction, "interpreters", "tools");
	const ActionInfo stopRobotActionInfo(mStopRobotAction, "interpreters", "tools");
	return {generateCodeActionInfo, uploadProgramActionInfo, runProgramActionInfo, stopRobotActionInfo};
}

QList<HotKeyActionInfo> Ev3RbfGeneratorPlugin::hotKeyActions()
{
	mGenerateCodeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
	mUploadProgramAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
	mRunProgramAction->setShortcut(QKeySequence(Qt::Key_F5));
	mStopRobotAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F5));

	HotKeyActionInfo generateActionInfo("Generator.GenerateEv3Rbf"
			, tr("Generate Ev3 Robot Byte Code File"), mGenerateCodeAction);
	HotKeyActionInfo uploadProgramInfo("Generator.UploadEv3", tr("Upload EV3 Program"), mUploadProgramAction);
	HotKeyActionInfo runProgramInfo("Generator.RunEv3", tr("Run EV3 Program"), mRunProgramAction);
	HotKeyActionInfo stopRobotInfo("Generator.StopEv3", tr("Stop EV3 Program"), mStopRobotAction);

	return { generateActionInfo, uploadProgramInfo, runProgramInfo, stopRobotInfo };
}

QIcon Ev3RbfGeneratorPlugin::iconForFastSelector(const kitBase::robotModel::RobotModelInterface &robotModel) const
{
	Q_UNUSED(robotModel)
	return QIcon(":/ev3/rbf/images/switch-to-ev3-rbf.svg");
}

int Ev3RbfGeneratorPlugin::priority() const
{
	return 9;
}

QString Ev3RbfGeneratorPlugin::defaultFilePath(QString const &projectName) const
{
	return QString("ev3-rbf/%1/%1.lms").arg(projectName);
}

text::LanguageInfo Ev3RbfGeneratorPlugin::language() const
{
	return text::Languages::pickByExtension("lms");
}

QString Ev3RbfGeneratorPlugin::generatorName() const
{
	return "ev3/rbf";
}

QString Ev3RbfGeneratorPlugin::uploadProgram()
{
	QFileInfo const fileInfo = generateCodeForProcessing();
	if (!fileInfo.exists()) {
		return QString();
	}

	if (!copySystemFiles(fileInfo.absolutePath())) {
		mMainWindowInterface->errorReporter()->addError(tr("Can't write source code files to disk!"));
		return QString();
	}

	if (!compile(fileInfo)) {
		QLOG_ERROR() << "EV3 bytecode compillation process failed!";
		mMainWindowInterface->errorReporter()->addError(tr("Compilation error occured."));
		return QString();
	}

	return upload(fileInfo);
}

void Ev3RbfGeneratorPlugin::uploadAndRunProgram(RunPolicy runPolicy)
{
	const QString fileOnRobot = uploadProgram();
	const auto &communicator = currentCommunicator();

	if (fileOnRobot.isEmpty() || !communicator) {
		return;
	}

	switch (runPolicy) {
		case RunPolicy::Ask:
			if (utils::QRealMessageBox::question(mMainWindowInterface->windowWidget()
												, tr("The program has been uploaded")
												, tr("Do you want to run it?")
			) != QMessageBox::Yes) {
				return;
			}
			Q_FALLTHROUGH();
		case RunPolicy::AlwaysRun:
			QMetaObject::invokeMethod(communicator.get(), [=](){ communicator->runProgram(fileOnRobot); });
			Q_FALLTHROUGH();
		case RunPolicy::NeverRun:
			return;
	}
}

void Ev3RbfGeneratorPlugin::stopRobot()
{
	if (const auto &communicator = currentCommunicator()) {
		QMetaObject::invokeMethod(communicator.get(), &communication::Ev3RobotCommunicationThread::stopProgram);
	}
}

bool Ev3RbfGeneratorPlugin::javaInstalled()
{
	QProcess java;
	java.setEnvironment(QProcess::systemEnvironment());

	java.start("java", {"-version"});
	auto started = java.waitForStarted(5000);
	// do not check the result, because it is false when process already finished
	java.waitForFinished();
	auto version = java.readAllStandardError();
	QLOG_INFO() << "Java test says:" << version;
	return started && !version.isEmpty();
}

bool Ev3RbfGeneratorPlugin::copySystemFiles(const QString &destination)
{
	QDirIterator iterator(":/ev3/rbf/thirdparty");
	while (iterator.hasNext()) {
		const QFileInfo fileInfo(iterator.next());
		const QString destFile = destination + "/" + fileInfo.fileName();
		if (!QFile::exists(destFile) && !QFile::copy(fileInfo.absoluteFilePath(), destFile)) {
			return false;
		}
	}

	return true;
}

bool Ev3RbfGeneratorPlugin::compile(const QFileInfo &lmsFile)
{
	if (!mJavaDetected) {
		mMainWindowInterface->errorReporter()->addError(
					tr("<a href=\"https://java.com/ru/download/\">Java</a> is "\
				"not installed properly, but is required to upload programs to EV3."));
		return false;
	}

	QFile rbfFile(lmsFile.absolutePath() + "/" + lmsFile.baseName() + ".rbf");
	if (rbfFile.exists()) {
		rbfFile.remove();
	}

	QProcess java;
	QEventLoop loop;
	java.setEnvironment(QProcess::systemEnvironment());
	java.setWorkingDirectory(lmsFile.absolutePath());
	java.setProgram("java");
	java.setArguments({"-jar", "assembler.jar", lmsFile.baseName()});

	connect(&java, &QProcess::readyRead, &loop, [&java]() { QLOG_INFO() << java.readAll(); });
	connect(&java, &QProcess::errorOccurred, &loop, [&java, &loop](QProcess::ProcessError e) {
		QLOG_ERROR() << "Failed to start process (status" << e << "):"
					 << java.program() << java.arguments();
		loop.quit();
	});
	connect(&java, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished)
		, &loop, [&loop, &java](int e, QProcess::ExitStatus s) {
		if (e || s != QProcess::ExitStatus::NormalExit) {
			QLOG_ERROR() << "Failed to execute process (errCode:" << e << ", exitStatus:" << s << "):"
						 << java.program() << java.arguments();
		}
		loop.quit();
	});

	java.start();
	loop.exec();
	return true;
}

QString Ev3RbfGeneratorPlugin::upload(const QFileInfo &lmsFile)
{
	const QString folderName = SettingsManager::value("Ev3CommonFolderChecboxChecked", false).toBool()
			? SettingsManager::value("Ev3CommonFolderName", "ts").toString() : lmsFile.baseName();
	const QString targetPath = "../prjs/" + folderName;
	const QString rbfPath = lmsFile.absolutePath() + "/" + lmsFile.baseName() + ".rbf";
	bool connected = false;
	const auto &communicator = currentCommunicator();
	if (!communicator) {
		return QString();
	}
	auto errorReporter = connect(
			communicator.get(), &utils::robotCommunication::RobotCommunicationThreadInterface::errorOccured,
			this, [this](const QString &message){
				mMainWindowInterface->errorReporter()->addError(message);
			});
	auto blockingConnectionType = communicator->thread() == QThread::currentThread()
			? Qt::DirectConnection : Qt::BlockingQueuedConnection;
	QMetaObject::invokeMethod(communicator.get(), &communication::Ev3RobotCommunicationThread::connect
			, blockingConnectionType
			, &connected);
	if (!connected) {
		const bool isUsb = mRobotModelManager->model().name().contains("usb", Qt::CaseInsensitive);
		mMainWindowInterface->errorReporter()->addError(tr("Could not upload file to robot. "\
				"Connect to a robot via %1.").arg(isUsb ? tr("USB") : tr("Bluetooth")));
		return QString();
	}

	QString res;
	QMetaObject::invokeMethod(communicator.get(), [=](){ return communicator->uploadFile(rbfPath, targetPath); }
			, blockingConnectionType
			, &res);
	disconnect(errorReporter);
	return res;
}

generatorBase::MasterGeneratorBase *Ev3RbfGeneratorPlugin::masterGenerator()
{
	return new Ev3RbfMasterGenerator(*mRepo
			, *mMainWindowInterface->errorReporter()
			, *mParserErrorReporter
			, *mRobotModelManager
			, *mTextLanguage
			, mMainWindowInterface->activeDiagram()
			, generatorName());
}
