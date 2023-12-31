/* Copyright 2007-2016 QReal Research Group
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

#include "insertIntoEdgeCommand.h"

#include <qrgui/models/models.h>

#include "nodeElement.h"
#include "edgeElement.h"

using namespace qReal;
using namespace qReal::commands;
using namespace qReal::gui::editor::commands;

InsertIntoEdgeCommand::InsertIntoEdgeCommand(EditorViewScene &scene
		, const models::Models &models
		, const Id &firstElem
		, const Id &lastElem
		, const Id &parent
		, const QPointF &scenePos
		, const QPointF &shift
		, bool isFromLogicalModel
		, CreateElementsCommand *createCommand)
	: AbstractCommand()
	, mScene(scene)
	, mModels(models)
	, mLogicalAssistApi(models.logicalModelAssistApi())
	, mGraphicalAssistApi(models.graphicalModelAssistApi())
	, mExploser(models.exploser())
	, mFirstId(firstElem)
	, mLastId(lastElem)
	, mParentId(parent)
	, mOldEdge(Id())
	, mPos(scenePos)
	, mShift(shift)
	, mIsLogical(isFromLogicalModel)
	, mCreateFirst(nullptr)
	, mCreateSecond(nullptr)
	, mRemoveOldEdge(nullptr)
	, mCreateCommand(createCommand)
{
	if (mCreateCommand) {
		mCreateCommand->setParent(this);
	}
}

InsertIntoEdgeCommand::~InsertIntoEdgeCommand()
{
	delete mCreateFirst;
	delete mCreateSecond;
	delete mRemoveOldEdge;
}

bool InsertIntoEdgeCommand::execute()
{
	if (mCreateCommand) {
		mCreateCommand->redo();
		mFirstId = mCreateCommand->results().first().id();
		mLastId = mCreateCommand->results().last().id();
	}

	NodeElement *firstNode = mScene.getNodeById(mFirstId);
	NodeElement *lastNode = mScene.getNodeById(mLastId);

	auto insertPos = mFirstId == mLastId && firstNode ?
			firstNode->mapToScene(firstNode->contentsRect().center()) : mPos;

	EdgeElement *edge;
	edge = mRemoveOldEdge ? mScene.getEdgeById(mOldEdge) : mScene.edgeForInsertion(insertPos);
	if (!edge && mPos != insertPos)
	{
		insertPos = mPos;
		edge = mScene.edgeForInsertion(insertPos);
	}
	if (!edge) {
		return true;
	}

	NodeElement *oldSrc = edge->src();
	NodeElement *oldDst = edge->dst();

	if (oldSrc && oldDst && oldSrc->id() != mFirstId && oldDst->id() != mLastId) {
		mParentId = (mParentId == Id::rootId()) ? mScene.rootItemId() : mParentId;
		Id type = edge->id().type();

		auto insertSegment = edge->defineSegment(insertPos);
		QPolygonF firstLine = edge->line().mid(1, insertSegment);
		firstLine.translate(edge->pos());

		auto lineSize = edge->line().size();
		QPolygonF lastLine = edge->line().mid(insertSegment + 1, lineSize - insertSegment - 2);
		lastLine.translate(edge->pos());

		initCommand(mCreateFirst, type, mGraphicalAssistApi.properties(edge->logicalId()));
		initCommand(mCreateSecond, type, {});

		makeLink(mCreateFirst, oldSrc, firstNode, firstLine);
		makeLink(mCreateSecond, lastNode, oldDst, lastLine);

		mConfiguration = mGraphicalAssistApi.configuration(edge->id());
		if (!mRemoveOldEdge) {
			mRemoveOldEdge = new RemoveElementsCommand(mModels);
			if (mIsLogical) {
				mRemoveOldEdge->withLogicalItemToDelete(edge->id());
			} else {
				mRemoveOldEdge->withItemsToDelete({edge->id()});
			}
		}

		mRemoveOldEdge->redo();

		mElementShifting.clear();
		mScene.resolveOverlaps(firstNode, mPos, mShift, mElementShifting);
		mScene.resolveOverlaps(lastNode, mPos, mShift, mElementShifting);
	}

	return true;
}

bool InsertIntoEdgeCommand::restoreState()
{
	if (mRemoveOldEdge && mCreateSecond && mCreateFirst) {
		mScene.returnElementsToOldPositions(mElementShifting);

		mRemoveOldEdge->undo();

		mOldEdge = mRemoveOldEdge->results().first().id();
		EdgeElement *edge = mScene.getEdgeById(mOldEdge);
		edge->setSrc(mScene.getEdgeById(mCreateFirst->results().first().id())->src());
		edge->setDst(mScene.getEdgeById(mCreateSecond->results().first().id())->dst());
		mScene.reConnectLink(edge);
		mGraphicalAssistApi.setConfiguration(edge->id(), mConfiguration);

		mCreateSecond->undo();
		mCreateFirst->undo();
	}

	if (mCreateCommand) {
		mCreateCommand->undo();
	}

	return true;
}

void InsertIntoEdgeCommand::initCommand(CreateElementsCommand *&command, const Id &type
		, const QMap<QString, QVariant> &additionalProperties)
{
	if (!command) {
		const QString name = mLogicalAssistApi.editorManagerInterface().friendlyName(type);
		const Id newId = type.sameTypeId();
		QMap<QString, QVariant> logicalProperties = additionalProperties;
		logicalProperties["name"] = name;
		const ElementInfo element(newId, mIsLogical ? newId : Id(), mScene.rootItemId(), mParentId
				, logicalProperties, {{"position", mPos}}, Id(), false);
		command = new CreateElementsCommand(mModels, {element});
	}
}

void InsertIntoEdgeCommand::makeLink(CreateElementsCommand *command, NodeElement *src, NodeElement *dst, QPolygonF line)
{
	command->redo();
	Id newLink = command->results().first().id();
	if (src) {
		mGraphicalAssistApi.setFrom(newLink, src->id());
		line.prepend(src->mapToScene(src->contentsRect().center()));
	}

	if (dst) {
		mGraphicalAssistApi.setTo(newLink, dst->id());
		line.append(dst->mapToScene(dst->contentsRect().center()));
	}

	EdgeElement * const edge = mScene.getEdgeById(newLink);
	line.translate(-edge->pos());
	if (!line.isEmpty()) {
		edge->setLine(line);
	}
	mScene.reConnectLink(edge, src, dst);
}
