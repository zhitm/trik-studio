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

#include "interpreterCore/managers/saveConvertionManager.h"
#include "QDomDocument"

using namespace interpreterCore;
using namespace qReal;

QString SaveConvertionManager::editor()
{
	return "RobotsMetamodel";
}

void SaveConvertionManager::reconnectEdges(const Id &newBlock, const Id &block
										   , details::ModelsAssistInterface &api)
{
	const bool isEdge = isEdgeType(block);
	if (isEdge) {
		// If out element is edge then connecting it to same elements as the old one was connected
		api.setFrom(newBlock, api.from(block));
		api.setTo(newBlock, api.to(block));
	} else {
		// Replacing old node in all incomming and outgoing edges of the old node with the new one.
		for (const Id &edge : api.mutableRepoApi().outgoingLinks(block)) {
			api.mutableRepoApi().setProperty(edge, "from", newBlock.toVariant());
		}

		for (const Id &edge : api.mutableRepoApi().incomingLinks(block)) {
			api.mutableRepoApi().setProperty(edge, "to", newBlock.toVariant());
		}
	}
}

QList<ProjectConverter> SaveConvertionManager::converters()
{
	return { before300Alpha1Converter()
		, from300Alpha4to300Alpha5Converter()
		, from300Beta2to300rc1Converter()
		, from300to301Converter()
		, from301to302Converter()
		, from302to310Converter()
		, from312to313Converter()
		, from320to330Converter()
		, from330to20204Converter()
		, from20204to20205Converter()
		, from20205to20222Converter()
	};
}

ProjectConverter SaveConvertionManager::before300Alpha1Converter()
{
	return ProjectConverter(editor(), Version(), Version::fromString("3.0.0-a1")
			, [=](GraphicalModelAssistInterface &, LogicalModelAssistInterface &)
	{
		return ProjectConverter::VersionTooOld;
	});
}

ProjectConverter SaveConvertionManager::from300Alpha4to300Alpha5Converter()
{
	return constructConverter("3.0.0-a4", "3.0.0-a5"
			, {
				replace({
						{"JA1", "A1"}
						, {"JA2", "A2"}
						, {"JA3", "A3"}
						, {"JA4", "A4"}
						, {"JA5", "A5"}
						, {"JA6", "A6"}

						, {"JD1", "D1"}
						, {"JD2", "D2"}

						, {"JF1", "F1"}

						, {"JM1", "M1"}
						, {"JM2", "M2"}
						, {"JM3", "M3"}
						, {"JM4", "M4"}

						, {"JB1", "B1"}
						, {"JB2", "B2"}
						, {"JB3", "B3"}
						, {"JB4", "B4"}

						, {"JE1", "E1"}
						, {"JE2", "E2"}
						, {"JE3", "E3"}
						, {"JE4", "E4"}

						, {"JC1", "C1"}
						, {"JC2", "C2"}
						, {"JC3", "C3"}

						, {"sensor1", "sensorA1"}
						, {"sensor2", "sensorA2"}
						, {"sensor3", "sensorA3"}
						, {"sensor4", "sensorA4"}
						, {"sensor5", "sensorA5"}
						, {"sensor6", "sensorA6"}

						, {"digitSensor1", "sensorD1"}
						, {"digitSensor2", "sensorD2"}
				})
			}
			, {}
			, [] (const Id &block) { return block.element().startsWith("Trik"); }
			);

}

ProjectConverter SaveConvertionManager::from300Beta2to300rc1Converter()
{
	return constructConverter("3.0.0-b2", "3.0.0-rc1"
			, { quote("TrikSay", "Text"), quote("PrintText", "PrintText") }
			);
}

qReal::ProjectConverter SaveConvertionManager::from300to301Converter()
{
	return constructConverter("3.0.0", "3.0.1"
			, {
				replace({
						{"enterButton", "buttonEnter"}
						, {"escapeButton", "buttonEscape"}
						, {"leftButton", "buttonLeft"}
						, {"rightButton", "buttonRight"}
						, {"backButton", "buttonBack"}
						, {"downButton", "buttonDown"}
						, {"enterButton", "buttonEnter"}
						, {"upButton", "buttonUp"}
						, {"powerButton", "buttonEsc"}
				  })
				, [=] (const Id &block, LogicalModelAssistInterface &logicalApi) {
						if (block.element().startsWith("Trik")) {
							return replace({{"buttonEscape", "buttonEsc"}})(block, logicalApi);
						}

						return false;
					}
				, deleteBlocks({"Ev3WaitForUp"
						, "Ev3WaitForEnter"
						, "Ev3WaitForDown"
						, "Ev3WaitForRight"
						, "Ev3WaitForLeft"
						, "Ev3WaitForBack"

						, "NxtWaitForEnter"
						, "NxtWaitForEscape"
						, "NxtWaitForLeft"
						, "NxtWaitForRight"

						, "TrikWaitForEnter"
						, "TrikWaitForLeft"
						, "TrikWaitForRight"
						, "TrikWaitForDown"
						, "TrikWaitForUp"
						, "TrikWaitForPower"
						})
			}
			);
}

qReal::ProjectConverter SaveConvertionManager::from301to302Converter()
{
	return constructConverter("3.0.1", "3.0.2", { quote("TrikSystem", "Command") } );
}

ProjectConverter SaveConvertionManager::from302to310Converter()
{
	const QMap<QString, QString> replacementRules = {
			{ "interpreterBase", "kitBase"}
			, { "commonTwoDModel", "twoDModel" }
			, { "nxtKitInterpreter", "nxt" }
			, { "ev3KitInterpreter", "ev3" }
			, { "trikKitInterpreter", "trik" }
			, { "NxtRealRobotModel", "NxtUsbRealRobotModel" }
			, { "nxtKitRobot", "nxtKitUsbRobot" }
			, { "TrikRealRobotModelV6", "TrikRealRobotModel" }
			, { "lineSensorX", "lineSensor[0]"}
			, { "lineSensorSize", "lineSensor[1]" }
			, { "lineSensorCross", "lineSensor[2]" }
	};

	return constructConverter("3.0.2", "3.1.0"
			, {
				replace(replacementRules)
				, [=](const Id &block, LogicalModelAssistInterface &logicalApi) {
					if (block.element() == "RobotsDiagramNode") {
						QString worldModel = logicalApi.logicalRepoApi().stringProperty(block, "worldModel");
						for (const QString &toReplace : replacementRules.keys()) {
							if (worldModel.contains(toReplace)) {
								worldModel.replace(toReplace, replacementRules[toReplace]);
							}
						}

						logicalApi.mutableLogicalRepoApi().setMetaInformation("worldModel", worldModel);
						return true;
					}

					return false;
				}
			}
			, {
				// This one repairs labels positions. At some moment a number of labels on scene
				// reduced a lot, so old saves used wrong partial models. The easiest fix "by hand"
				// is to cut and paste element. This converter does the same thing.
				graphicalRecreate([](const Id &block, GraphicalModelAssistInterface &) { return block.type(); }
						, [](const Id &newBlock, const Id &oldBlock, GraphicalModelAssistInterface &model) {
							model.copyProperties(newBlock, oldBlock);
				})
			}
	);
}

ProjectConverter SaveConvertionManager::from312to313Converter()
{
	const QMap<QString, QString> replacementRules = {
			{ "gyroscopeX", "gyroscope[0]"}
			, { "gyroscopeY", "gyroscope[1]" }
			, { "gyroscopeZ", "gyroscope[2]" }
			, { "accelerometerX", "accelerometer[0]"}
			, { "accelerometerY", "accelerometer[1]" }
			, { "accelerometerZ", "accelerometer[2]" }
	};

	return constructConverter("3.1.2", "3.1.3", { replace(replacementRules) });
}

ProjectConverter SaveConvertionManager::from320to330Converter()
{
	return ProjectConverter(editor(), Version::fromString("3.2.0"), Version::fromString("3.3.0")
			, [=](GraphicalModelAssistInterface &, LogicalModelAssistInterface &logicalApi)
	{
		QString worldModel = logicalApi.logicalRepoApi().metaInformation("worldModel").toString();
		if (!worldModel.contains("trikV62KitRobot")) {
			return ProjectConverter::NoModificationsMade;
		}
		worldModel.replace("trikV62KitRobot", "trikKitRobot");
		logicalApi.mutableLogicalRepoApi().setMetaInformation("worldModel", worldModel);
		return ProjectConverter::Success;
	});
}

ProjectConverter SaveConvertionManager::from330to20204Converter()
{
	return ProjectConverter(editor(), Version::fromString("3.3.0"), Version::fromString("2020.4")
			, [=](GraphicalModelAssistInterface &, LogicalModelAssistInterface &logicalApi)
	{
		QString worldModel = logicalApi.logicalRepoApi().metaInformation("worldModel").toString();
		if (!worldModel.contains("trik::robotModel::parts::TrikLineSensor")
				&& !worldModel.contains("value=\"trik::robotModel::twoD::parts::TwoDInfraredSensor")) {
			return ProjectConverter::NoModificationsMade;
		}

		worldModel.replace("value=\"trik::robotModel::twoD::parts::TwoDInfraredSensor"
				, "value=\"twoDModel::robotModel::parts::RangeSensor");

		worldModel.replace("TrikLineSensorPort###lineSensor", "###");
		worldModel.replace("LineSensorPort", "Video2Port");

		worldModel.replace("TrikLineSensor", "TrikVideoCamera");

		logicalApi.mutableLogicalRepoApi().setMetaInformation("worldModel", worldModel);
		return ProjectConverter::Success;
	});
}

ProjectConverter SaveConvertionManager::from20204to20205Converter()
{
	return ProjectConverter(editor(), Version::fromString("2020.4"), Version::fromString("2020.4.1")
			, [=](GraphicalModelAssistInterface &graphicalApi, LogicalModelAssistInterface &logicalApi)
	{
		if (!logicalApi.logicalRepoApi()
				.metaInformation("lastKitId").toString().contains("trik", Qt::CaseInsensitive)) {
			return ProjectConverter::NoModificationsMade;
		}

		bool modificationsMade = false;

		for (const Id &block : elementsOfRobotsDiagrams(logicalApi)) {
			const Id logicalBlock = graphicalApi.isGraphicalId(block)
					? graphicalApi.logicalId(block)
					: block;

			Id graphicalBlock;
			if (graphicalApi.isGraphicalId(block)) {
				graphicalBlock = block;
			} else {
				const IdList graphicalIds = graphicalApi.graphicalIdsByLogicalId(logicalBlock);
				if (graphicalIds.isEmpty()) {
					continue;
				}

				graphicalBlock = graphicalIds.first();
			}

			if (graphicalBlock.element() == "PrintText") {
				const auto newType = Id(graphicalBlock.editor(), graphicalBlock.diagram(), "TrikPrintText");
				const auto newGraphicalBlock = graphicalApi.createElement(graphicalApi.parent(graphicalBlock)
						, newType.sameTypeId()
						, false
						, graphicalApi.name(graphicalBlock)
						, graphicalApi.position(graphicalBlock)
						, logicalApi.createElement(logicalApi.parent(logicalBlock), newType));
				graphicalApi.copyProperties(newGraphicalBlock, graphicalBlock);
				const auto newLogicalBlock = graphicalApi.logicalId(newGraphicalBlock);

				auto iterator = logicalApi.logicalRepoApi().propertiesIterator(logicalBlock);
				while (iterator.hasNext()) {
					iterator.next();
					logicalApi.mutableLogicalRepoApi().setProperty(newLogicalBlock, iterator.key(), iterator.value());
				}

				logicalApi.setPropertyByRoleName(newLogicalBlock, 20, "FontSize");
				reconnectEdges(newGraphicalBlock, graphicalBlock, graphicalApi);
				reconnectEdges(newLogicalBlock, logicalBlock, logicalApi);
				graphicalApi.removeElement(graphicalBlock);
				logicalApi.removeElement(logicalBlock);
				modificationsMade = true;
			}
		}

		return modificationsMade ? ProjectConverter::Success : ProjectConverter::NoModificationsMade;
	});
}

ProjectConverter SaveConvertionManager::from20205to20222Converter()
{
	return ProjectConverter(editor(), Version::fromString("2020.5"), Version::fromString("2022.2")
			, [=](GraphicalModelAssistInterface &, LogicalModelAssistInterface &logicalApi)
	{
		QString xml = logicalApi.logicalRepoApi().metaInformation("worldModel").toString();

		if (xml.isEmpty()) return ProjectConverter::NoModificationsMade;
		QDomDocument worldModel;
		if (!worldModel.setContent(xml)) return ProjectConverter::SaveInvalid;
		QDomElement root = worldModel.firstChildElement("root");

		QDomElement world = root.firstChildElement("world");
		if (world.isNull()) {
			world = root.ownerDocument().createElement("world");
			root.appendChild(world);
		}

		QDomElement oldRobot = root.firstChildElement("robots").firstChildElement("robot");
		QDomElement robot = world.ownerDocument().createElement("robot");
		robot.setAttribute("position", oldRobot.attribute("position", "0:0"));
		robot.setAttribute("direction", oldRobot.attribute("direction", "0"));
		robot.appendChild(oldRobot.firstChildElement("startPosition"));
		world.appendChild(robot);

		oldRobot.removeAttribute("position");
		oldRobot.removeAttribute("direction");
		// start position reparented automatically

		logicalApi.mutableLogicalRepoApi().setMetaInformation("worldModel", worldModel.toString(4));
		return ProjectConverter::Success;
	});
}

bool SaveConvertionManager::isRobotsDiagram(const Id &element)
{
	const QStringList robotsDiagrams = { "RobotsDiagram", "SubprogramDiagram" };
	return element.editor() == editor() && robotsDiagrams.contains(element.diagram());
}

bool SaveConvertionManager::isDiagramType(const Id &element)
{
	const QStringList robotsDiagrams = { "RobotsDiagramNode", "SubprogramDiagram" };
	return isRobotsDiagram(element) && robotsDiagrams.contains(element.element());
}

bool SaveConvertionManager::isEdgeType(const Id &element)
{
	return element.element() == "ControlFlow";
}

IdList SaveConvertionManager::elementsOfRobotsDiagrams(const LogicalModelAssistInterface &logicalApi)
{
	IdList nodes;
	IdList edges;
	for (const Id &blockId : logicalApi.children(Id::rootId())) {
		if (isRobotsDiagram(blockId)) {
			if (isEdgeType(blockId)) {
				edges += blockId;
			} else {
				nodes += blockId;
			}
		}
	}

	// When we traverse elements it is important to traverse first nodes and then edges.
	return nodes + edges;
}

qReal::ProjectConverter SaveConvertionManager::constructConverter(const QString &oldVersion
		, const QString &newVersion
		, const QList<LogicalFilter> &logicalFilters
			, const QList<GraphicalFilter> &graphicalFilters
		, const std::function<bool(const qReal::Id &)> &condition
		)
{
	return ProjectConverter(editor(), Version::fromString(oldVersion), Version::fromString(newVersion)
			, [=](GraphicalModelAssistInterface &graphicalApi, LogicalModelAssistInterface &logicalApi)
	{
		bool modificationsMade = false;

		for (const Id &block : elementsOfRobotsDiagrams(logicalApi)) {
			const Id logicalBlock = graphicalApi.isGraphicalId(block)
					? graphicalApi.logicalId(block)
					: block;

			if (!condition(logicalBlock)) {
				continue;
			}

			for (const auto &filter : logicalFilters) {
				modificationsMade |= filter(logicalBlock, logicalApi);
			}

			if (graphicalFilters.isEmpty()) {
				// A small optimization not to count graphical id.
				continue;
			}

			Id graphicalBlock;
			if (graphicalApi.isGraphicalId(block)) {
				graphicalBlock = block;
			} else {
				const IdList graphicalIds = graphicalApi.graphicalIdsByLogicalId(logicalBlock);
				if (graphicalIds.isEmpty()) {
					continue;
				}

				graphicalBlock = graphicalIds.first();
			}

			for (const auto &filter : graphicalFilters) {
				modificationsMade |= filter(graphicalBlock, graphicalApi);
			}
		}

		return modificationsMade ? ProjectConverter::Success : ProjectConverter::NoModificationsMade;
	});
}

SaveConvertionManager::LogicalFilter SaveConvertionManager::replace(const QMap<QString, QString> &replacementRules)
{
	return [=] (const Id &block, LogicalModelAssistInterface &logicalApi) {
		bool modificationsMade = false;
		auto iterator = logicalApi.logicalRepoApi().propertiesIterator(block);
		while (iterator.hasNext()) {
			iterator.next();
			const auto name = iterator.key();
			auto value = iterator.value().toString();
			bool replacementOccured = false;
			for (const auto &toReplace : replacementRules.keys()) {
				if (value.contains(toReplace)) {
					replacementOccured = true;
					modificationsMade = true;

					value.replace(toReplace, replacementRules[toReplace]);
				}
			}

			if (replacementOccured) {
				logicalApi.setPropertyByRoleName(block, value, name);
			}
		}

		 return modificationsMade;
	};
}

SaveConvertionManager::LogicalFilter SaveConvertionManager::deleteBlocks(const QStringList &blocks)
{
	return [=] (const Id &block, LogicalModelAssistInterface &logicalApi) {
		if (blocks.contains(block.element())) {
			logicalApi.removeElement(block);
			return true;
		}

		return false;
	};
}

SaveConvertionManager::LogicalFilter SaveConvertionManager::quote(const QString &blockType, const QString &property)
{
	return [blockType, property] (const qReal::Id &block, qReal::LogicalModelAssistInterface &logicalApi) {
		if (block.element() == blockType) {
			const QString oldValue = logicalApi.logicalRepoApi().property(block, property).toString();
			if (!oldValue.startsWith("\"")) {
				logicalApi.mutableLogicalRepoApi().setProperty(block, property, "\"" + oldValue + "\"");
				return true;
			}
		}

		return false;
	};
}

SaveConvertionManager::GraphicalFilter SaveConvertionManager::graphicalRecreate(
		const SaveConvertionManager::GraphicalReplacer &replacer
		, const SaveConvertionManager::GraphicalConstructor &constructor)
{
	return [replacer, constructor](const Id &block, GraphicalModelAssistInterface &graphicalApi) {
		// Just iterating throught the elements on some diagram, ignoring the diagram itself.
		if (isDiagramType(block)) {
			return false;
		}

		// For each element trying to find out what to replace it with.
		const Id newType = replacer(block, graphicalApi);
		if (newType.isNull()) {
			// Not every element be replaced, concrete implementation must decide it.
			return false;
		}

		// Then creating new element of some type...
		const Id newBlock = Id::createElementId(newType.editor(), newType.diagram(), newType.element());
		graphicalApi.createElement(graphicalApi.parent(block)
				, newBlock
				, false
				, graphicalApi.name(block)
				, graphicalApi.position(block)
				, graphicalApi.logicalId(block));
		// And initializing it...
		constructor(newBlock, block, graphicalApi);

		reconnectEdges(newBlock, block, graphicalApi);
		// And finally disposing of outdated entity.
		graphicalApi.removeElement(block);
		return true;
	};
}
