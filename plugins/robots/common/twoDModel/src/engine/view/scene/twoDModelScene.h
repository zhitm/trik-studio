/* Copyright 2012-2018 CyberTech Labs Ltd.
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

#include <QtWidgets/QGraphicsScene>
#include <QtGui/QPainter>

#include <qrutils/graphicsUtils/abstractScene.h>
#include <qrutils/graphicsUtils/gridDrawer.h>

#include <kitBase/devicesConfigurationProvider.h>
#include <kitBase/readOnly.h>

#include "twoDModel/engine/model/image.h"

namespace qReal {
class ControllerInterface;
namespace commands {
class AbstractCommand;
}
}

namespace graphicsUtils {
class AbstractItem;
}

namespace twoDModel {

namespace items {
class WallItem;
class SkittleItem;
class BallItem;
class LineItem;
class CurveItem;
class StylusItem;
class RectangleItem;
class EllipseItem;
class CommentItem;
class ImageItem;
}

namespace model {
class Model;
class RobotModel;
}

namespace commands {
class ReshapeCommand;
}

namespace view {
class RobotItem;

/// Implementation of QGraphicsScene for 2D robot model
class TwoDModelScene: public graphicsUtils::AbstractScene, public kitBase::DevicesConfigurationProvider
{
	Q_OBJECT

public:
	TwoDModelScene(model::Model &model
			, graphicsUtils::AbstractView *view
			, QObject *parent = nullptr);

	~TwoDModelScene() override;

	/// Returns true if existing only one robot
	bool oneRobot() const;

	/// Passes into scene a reference to controller object that will execute commands.
	void setController(qReal::ControllerInterface &controller);

	/// Forbid all interaction with specified categories of objects on scene, for "challenge" mode where student
	/// shall provide program that makes robot do specific task in given unmodifyable world model.
	/// Categories of items include world model (walls, lines, etc.), sensors, robot position.
	void setInteractivityFlags(kitBase::ReadOnlyFlags flags);

public slots:
	/// Sets a flag that next user mouse actions should draw a wall on the scene.
	void addWall();

	/// Sets a flag that next user mouse actions should draw a skittle on the scene.
	void addSkittle();

	/// Sets a flag that next user mouse actions should draw a ball on the scene.
	void addBall();

	/// Sets a flag that next user mouse actions should draw a colored line on the scene.
	void addLine();

	/// Sets a flag that next user mouse actions should draw a colored bezier curve on the scene.
	void addBezier();

	/// Sets a flag that next user mouse actions should draw a stylus line on the scene.
	void addStylus();

	/// Sets a flag that next user mouse actions should draw a rectangle on the scene.
	void addRectangle();

	/// Sets a flag that next user mouse actions should draw an ellipse on the scene.
	void addEllipse();

	/// Sets a flag that next user mouse actions should draw an comment on the scene.
	void addComment();

	/// Shows dialog proposing user to add image on a scene.
	void addImage();

	/// Resets all drawing flags: next user mouse actions will behavior like usially.
	void setNoneStatus();

	/// Clears everything on the scene besides a robot and its sensors.
	/// @param removeRobot - if true robot graphics item will be fully recreated, its position will be nullified.
	/// @param reason - reason for scene clearing --- user action or internal needs. Depending on this we can decide
	///        whether to save changes into model.
	void clearScene(bool removeRobot, Reason reason);

	/// Aligns existing walls on the grid.
	/// @todo: Walls that do not fit on the grid must not be removed.
	void alignWalls();

	/// Returns a pointer to a robot graphics item.
	RobotItem *robot(model::RobotModel &robotModel);

	/// Focuses all graphics views on the robot if it is not visible.
	void centerOnRobot(RobotItem *selectedItem = nullptr);

	/// Reread sensor configuration on given port, delete old sensor item and create new.
	void reinitSensor(RobotItem *robotItem, const kitBase::robotModel::PortInfo &port);

signals:
	/// Emitted each time when user presses mouse button somewhere on the scene.
	void mousePressed();

	/// Emitted each time when user releases mouse button somewhere on the scene.
	void mouseReleased();

	/// Emitted each time when user presses mouse button on the robot item.
	void robotPressed();

	/// Emitted at any changes of robot list (adding or removing)
	void robotListChanged(RobotItem *robotItem);

private slots:
	/// Called after robot model was added and create new robot item
	/// @param robotModel Robot model which was added
	void onRobotAdd(model::RobotModel *robotModel);

	/// Called after robot model was removed and removes robot item associated with this robot model
	/// @param robotModel Robot model which was removed
	void onRobotRemove(model::RobotModel *robotModel);

	/// Called after new wall is added to a world model.
	void onWallAdded(QSharedPointer<items::WallItem> wall);

	/// Called after new skittle is added to a world model.
	void onSkittleAdded(QSharedPointer<items::SkittleItem> skittle);

	/// Called after new ball is added to a world model.
	void onBallAdded(const QSharedPointer<items::BallItem> &ball);

//	/// Called after new color field item is added to a world model.
//	void onColorItemAdded(const QSharedPointer<graphicsUtils::AbstractItem> &item);

//	/// Called after new image item is added to a world model.
//	void onImageItemAdded(const QSharedPointer<graphicsUtils::AbstractItem> &item);

	void onAbstractItemAdded(QSharedPointer<graphicsUtils::AbstractItem> item);

	/// Called after some item was kicked away from a world model.
	void onItemRemoved(const QSharedPointer<QGraphicsItem> &item);

	void drawAxes(QPainter *painter);

private:
	enum DrawingAction
	{
		none = 0
		, wall
		, skittle
		, ball
		, line
		, bezier
		, stylus
		, rectangle
		, ellipse
		, comment
		, image
	};

	void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

	void drawBackground(QPainter *painter, const QRectF &rect) override;
	void keyPressEvent(QKeyEvent *event) override;

	QPair<QStringList, QList<QPair<model::RobotModel *
			, kitBase::robotModel::PortInfo>>> parseItemsToID (const QList<QGraphicsItem *> &items);
	void deleteSelectedItems();
	void deleteWithCommand(const QStringList &worldItems
			, const QList<QPair<model::RobotModel *, kitBase::robotModel::PortInfo>> &sensors
			, const QList<qReal::commands::AbstractCommand *> &additionalCommands);
	void copySelectedItems();
	void pasteItemsFromClipboard();

	void reshapeWall(QGraphicsSceneMouseEvent *event);
	void reshapeLine(QGraphicsSceneMouseEvent *event);
	void reshapeCurve(QGraphicsSceneMouseEvent *event);
	void reshapeStylus(QGraphicsSceneMouseEvent *event);
	void reshapeRectangle(QGraphicsSceneMouseEvent *event);
	void reshapeEllipse(QGraphicsSceneMouseEvent *event);
	void reshapeComment(QGraphicsSceneMouseEvent *event);

	void registerInUndoStack(graphicsUtils::AbstractItem *item);
	void subscribeItem(graphicsUtils::AbstractItem *item);

	void handleMouseInteractionWithSelectedItems();

	bool hasIntersect(const graphicsUtils::AbstractItem *item2
					  , const graphicsUtils::AbstractItem *item1) const;
	bool isCorrectScene(const QList<QGraphicsItem *> &checkItems) const;

	qreal currentZoom() const;

	model::Model &mModel;
	qReal::ControllerInterface *mController = nullptr;

	graphicsUtils::GridDrawer mGridDrawer;
	qreal mWidthOfGrid { -1 };

	/// Current action (toggled button on left panel)
	DrawingAction mDrawingAction;

	QMap<model::RobotModel *, QSharedPointer<RobotItem>> mRobots;

	/// Temporary wall that's being created. When it's complete, it's added to world model
	QSharedPointer<items::WallItem> mCurrentWall;
	QSharedPointer<items::SkittleItem> mCurrentSkittle;
	QSharedPointer<items::BallItem> mCurrentBall;
	QSharedPointer<items::LineItem> mCurrentLine;
	QSharedPointer<items::CurveItem> mCurrentCurve;
	QSharedPointer<items::StylusItem> mCurrentStylus;
	QSharedPointer<items::RectangleItem> mCurrentRectangle;
	QSharedPointer<items::EllipseItem> mCurrentEllipse;
	QSharedPointer<items::CommentItem> mCurrentComment;

	commands::ReshapeCommand *mCurrentReshapeCommand = nullptr;

	bool mWorldReadOnly = false;
	bool mRobotReadOnly = false;
	bool mSensorsReadOnly = false;

	QList<QDomElement> mClipboard;
};

}
}
