#include "wallItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include <math.h>
#include <qrkernel/settingsManager.h>

#include "d2ModelScene.h"

using namespace qReal::interpreters::robots;
using namespace details::d2Model;
using namespace graphicsUtils;

WallItem::WallItem(QPointF const &begin, QPointF const &end)
	: LineItem(begin, end)
	, mDragged(false)
	, mImage(":/icons/2d_wall.png")
	, mIsCircle(false)
	, mOldX1(0)
	, mOldY1(0)
{
	setPrivateData();
	setAcceptDrops(true);
}

void WallItem::setWallPath()
{
	QPainterPath wallPath;
	if (mX1 == mX2 && mY1 == mY2) {
		wallPath.addEllipse(mX1, mY1, 10, 10);
		mCenter = QPointF(mX1, mY1);
		mIsCircle = true;
	} else {
		wallPath.moveTo(mPoints[0]);
		wallPath.lineTo(mPoints[1]);
		wallPath.lineTo(mPoints[2]);
		wallPath.lineTo(mPoints[3]);
		wallPath.lineTo(mPoints[0]);
		mPath = wallPath;
		mIsCircle = false;
	}
}

void WallItem::setPrivateData()
{
	setZValue(1);
	mPen.setWidth(10);
	mPen.setStyle(Qt::NoPen);
	mBrush.setStyle(Qt::SolidPattern);
	mBrush.setTextureImage(mImage);
	mSerializeName = "wall";

}

QPointF WallItem::begin()
{
	return QPointF(mX1, mY1) + scenePos();
}

QPointF WallItem::end()
{
	return QPointF(mX2, mY2) + scenePos();
}

void WallItem::drawItem(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->drawPath(shape());
	recalculateBorders();
	setWallPath();
}

void WallItem::drawExtractionForItem(QPainter *painter)
{
	if (!isSelected()) {
		return;
	}

	painter->setPen(QPen(Qt::green));
	mLineImpl.drawExtractionForItem(painter, mX1, mY1, mX2, mY2, drift);
	mLineImpl.drawFieldForResizeItem(painter, resizeDrift, mX1, mY1, mX2, mY2);
}

void WallItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	AbstractItem::mousePressEvent(event);
	mDragged = (flags() & ItemIsMovable) || mOverlappedWithRobot;
	mOldX1 = qAbs(mX1 - event->scenePos().x());
	mOldY1 = qAbs(mY1 - event->scenePos().y());
}

void WallItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	QPointF const oldPos = pos();
	if (SettingsManager::value("2dShowGrid").toBool() && mDragged && ((flags() & ItemIsMovable) || mOverlappedWithRobot)){
		QPointF const pos = event->scenePos();
		int const indexGrid = SettingsManager::value("2dGridCellSize").toInt();
		qreal const deltaX = (mX1 - mX2);
		qreal const deltaY = (mY1 - mY2);
		mX1 = pos.x() - mOldX1;
		mY1 = pos.y() - mOldY1;
		reshapeBeginWithGrid(indexGrid);
		setDraggedEndWithGrid(deltaX, deltaY);
		mCellNumbX1 = mX1 / indexGrid;
		mCellNumbY1 = mY1 / indexGrid;
		mCellNumbX2 = mX2 / indexGrid;
		mCellNumbY2 = mY2 / indexGrid;
	} else if (mDragged) {
		QGraphicsItem::mouseMoveEvent(event);
	}

	// Items under cursor cannot be dragged when adding new item,
	// but it mustn`t confuse the case when item is unmovable
	// because overapped with robot
	if (mDragged && ((flags() & ItemIsMovable) || mOverlappedWithRobot)) {
		emit wallDragged(this, realShape(), oldPos);
	}
	event->accept();
}

bool WallItem::isDragged() const
{
	return mDragged;
}

bool WallItem::isCircle() const
{
	return mIsCircle;
}

qreal WallItem::width() const
{
	return mPen.width();
}

void WallItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mouseReleaseEvent(event);
	mDragged = false;
}

QDomElement WallItem::serialize(QDomDocument &document, QPoint const &topLeftPicture)
{
	QDomElement wallNode = document.createElement(mSerializeName);
	wallNode.setAttribute("begin", QString::number(mX1 + scenePos().x() - topLeftPicture.x())
			+ ":" + QString::number(mY1 + scenePos().y() - topLeftPicture.y()));
	wallNode.setAttribute("end", QString::number(mX2 + scenePos().x() - topLeftPicture.x())
			+ ":" + QString::number(mY2 + scenePos().y() - topLeftPicture.y()));
	return wallNode;
}

void WallItem::deserializePenBrush(QDomElement const &element)
{
	Q_UNUSED(element)
	setPrivateData();
}

void WallItem::onOverlappedWithRobot(bool overlapped)
{
	mOverlappedWithRobot = overlapped;
}

QPointF WallItem::center() const
{
	return mCenter;
}

QLineF WallItem::line(int i) const
{
	return mLines[i];
}

QPointF WallItem::point(int i) const
{
	return mPoints[i];
}

QPainterPath WallItem::path() const
{
	return mPath;
}

void WallItem::recalculateBorders()
{
	mLines.clear();

	qreal const x1 = begin().rx();
	qreal const x2 = end().rx();
	qreal const y1 = begin().ry();
	qreal const y2 = end().ry();

	qreal dx = (x2 - x1);
	qreal dy = y2 - y1;
	qreal const len = sqrt(dx * dx + dy * dy);
	dx /= len;
	dy /= len;
	dx *= 5;
	dy *= 5;

	QVector2D norm(y1 - y2, x2 - x1);
	norm.normalize();
	norm *= mPen.widthF() / 2;

	mPoints[0] = QPointF(x1 - dx + norm.x(), y1 - dy + norm.y());
	mPoints[1] = QPointF(x1 - dx - norm.x(), y1 - dy - norm.y());
	mPoints[2] = QPointF(x2 + dx - norm.x(), y2 + dy - norm.y());
	mPoints[3] = QPointF(x2 + dx + norm.x(), y2 + dy + norm.y());

	mLines << QLineF(mPoints[0],mPoints[1])
			<< QLineF(mPoints[1],mPoints[2])
			<< QLineF(mPoints[2],mPoints[3])
			<< QLineF(mPoints[3],mPoints[0]);
}
