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

#include "sensorItem.h"

#include <QtGui/QCursor>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <kitBase/robotModel/robotParts/touchSensor.h>
#include <kitBase/robotModel/robotParts/rangeSensor.h>
#include <kitBase/robotModel/robotParts/lightSensor.h>
#include <kitBase/robotModel/robotParts/lidarSensor.h>

#include "twoDModel/robotModel/parts/colorSensorFull.h"
#include "twoDModel/robotModel/parts/colorSensorPassive.h"
#include "twoDModel/robotModel/parts/colorSensorRed.h"
#include "twoDModel/robotModel/parts/colorSensorGreen.h"
#include "twoDModel/robotModel/parts/colorSensorBlue.h"
#include "twoDModel/robotModel/parts/colorSensorAmbient.h"
#include "twoDModel/robotModel/parts/colorSensorReflected.h"
#include "twoDModel/robotModel/parts/colorSensorRaw.h"

using namespace twoDModel::view;
using namespace graphicsUtils;
using namespace kitBase::robotModel;

const int selectionDrift = 7;

SensorItem::SensorItem(model::SensorsConfiguration &configuration
		, const PortInfo &port, const QString &pathToImage, const QRect &imageRect)
	: mConfiguration(configuration)
	, mPort(port)
	, mImageRect(imageRect.isEmpty() ? this->calculateImageRect() : imageRect)
	, mBoundingRect(mImageRect.adjusted(-selectionDrift, -selectionDrift
			, selectionDrift, selectionDrift))
	, mImage(pathToImage.isEmpty() ? this->pathToImage() : pathToImage, true)
	, mPortItem(new PortItem(port))
{
	setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);

	setAcceptDrops(true);
	mPortItem->setParentItem(this);
	mPortItem->moveBy(-mPortItem->boundingRect().width() - 5, -mPortItem->boundingRect().height() - 5);
	mPortItem->setFlag(ItemIgnoresTransformations);
	mPortItem->hide();

	RotateItem::init();
}

void SensorItem::drawItem(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{
	Q_UNUSED(style)
	Q_UNUSED(widget)

	painter->save();
	painter->setRenderHints(painter->renderHints()
			| QPainter::SmoothPixmapTransform
			| QPainter::HighQualityAntialiasing);
	mImage.draw(*painter, mImageRect.toRect());
	painter->restore();
}

void SensorItem::drawExtractionForItem(QPainter *painter)
{
	if (!isSelected()) {
		return;
	}

	painter->save();
	const QPen pen = QPen(Qt::black);
	painter->setPen(pen);
	painter->setOpacity(0.7);
	painter->setRenderHints(painter->renderHints() | QPainter::Antialiasing);
	painter->drawRoundedRect(mBoundingRect, 4, 4);
	painter->restore();
}

QRectF SensorItem::boundingRect() const
{
	return mBoundingRect;
}

void SensorItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	AbstractItem::mousePressEvent(event);
	mPortItem->hide();
}

void SensorItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	AbstractItem::hoverEnterEvent(event);
	mPortItem->show();
}

void SensorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	AbstractItem::hoverLeaveEvent(event);
	mPortItem->hide();
}

QDomElement SensorItem::serialize(QDomElement &parent) const
{
	QDomElement result = RotateItem::serialize(parent);
	result.setTagName("sensor");
	result.setAttribute("position", QString::number(x()) + ":" + QString::number(y()));
	result.setAttribute("direction", QString::number(rotation()));
	return result;
}

void SensorItem::deserialize(const QDomElement &element)
{
	const QString positionStr = element.attribute("position", "0:0");
	const QStringList splittedStr = positionStr.split(":");
	const qreal x = static_cast<qreal>(splittedStr[0].toDouble());
	const qreal y = static_cast<qreal>(splittedStr[1].toDouble());
	setPos(x, y);

	setRotation(element.attribute("direction", "0").toDouble());
}

QPolygonF SensorItem::collidingPolygon() const
{
	/// @todo: returning different polygons based on sensor type
	return QPolygonF(QRectF(-7, -7, 15, 15));
}

qreal SensorItem::friction() const
{
	return 0.0;
}

bool SensorItem::isCircle() const
{
	return true;
}

twoDModel::items::SolidItem::BodyType SensorItem::bodyType() const
{
	return BodyType::DYNAMIC;
}

qreal SensorItem::mass() const
{
	return 0.01; /// @todo
}

QString SensorItem::name() const
{
	const DeviceInfo sensor = mConfiguration.type(mPort);
	if (sensor.isA<robotParts::TouchSensor>()) {
		return "touch";
	} else if (sensor.isA<robotParts::ColorSensorFull>()
			|| sensor.isA<robotParts::ColorSensorPassive>()
			|| sensor.isA<robotParts::ColorSensorRaw>()) {
		return "color_empty";
	} else if (sensor.isA<robotParts::ColorSensorRed>()) {
		return "color_red";
	} else if (sensor.isA<robotParts::ColorSensorGreen>()) {
		return "color_green";
	} else if (sensor.isA<robotParts::ColorSensorBlue>()) {
		return "color_blue";
	} else if (sensor.isA<robotParts::RangeSensor>()) {
		return "range";
	} else if (sensor.isA<robotParts::LidarSensor>()) {
		return "range";
	} else if (sensor.isA<robotParts::LightSensor>()
		|| sensor.isA<robotParts::ColorSensorAmbient>()
		|| sensor.isA<robotParts::ColorSensorReflected>()) {
		return "light";
	} else {
		Q_ASSERT(!"Unknown sensor type");
		return "";
	}
}

QRectF SensorItem::calculateImageRect() const
{
	const DeviceInfo sensor = mConfiguration.type(mPort);
	if (sensor.isA<robotParts::TouchSensor>()) {
		return QRectF(-12, -5, 25, 10);
	} else if (sensor.isA<robotParts::ColorSensor>()
			|| sensor.isA<robotParts::ColorSensorRaw>()
			|| sensor.isA<robotParts::LightSensor>()) {
		return QRectF(-6, -6, 12, 12);
	}
	if (sensor.isA<robotParts::RangeSensor>()
			|| sensor.isA<robotParts::LidarSensor>()) {
		return QRectF(-20, -10, 40, 20);
	} else {
		Q_ASSERT(!"Unknown sensor type");
		return QRectF();
	}
}

QString SensorItem::pathToImage() const
{
	return QString(":/icons/sensors/2d_%1.png").arg(name());
}

void SensorItem::changeDragState(qreal x, qreal y)
{
	Q_UNUSED(x);
	Q_UNUSED(y);
}

void SensorItem::resizeItem(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
}

QVariant SensorItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionHasChanged) {
		mConfiguration.setPosition(mPort, value.toPointF());
	}

	if (change == ItemRotationHasChanged) {
		mConfiguration.setDirection(mPort, value.toReal());
	}

	return RotateItem::itemChange(change, value);
}

void SensorItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	QGraphicsItem::contextMenuEvent(event);
}

SensorItem::PortItem::PortItem(const PortInfo &port)
	: mPort(port)
	, mFont("Times", 10, QFont::Normal, true)
	, mBoundingRect(QFontMetrics(mFont).boundingRect(port.userFriendlyName()))
{
}

void SensorItem::PortItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->save();

	QPen pen;
	pen.setWidth(2);
	pen.setColor(Qt::yellow);

	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	brush.setColor(Qt::yellow);

	painter->setPen(pen);
	painter->setBrush(brush);

	painter->drawRoundedRect(boundingRect(), 2, 2);

	pen.setColor(Qt::black);
	painter->setPen(pen);
	painter->setFont(mFont);
	painter->drawText(boundingRect(), mPort.userFriendlyName(), QTextOption(Qt::AlignCenter));

	painter->restore();
}

QRectF SensorItem::PortItem::boundingRect() const
{
	return mBoundingRect;
}
