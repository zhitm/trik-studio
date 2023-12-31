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

#include "trikKitInterpreterCommon/robotModel/twoD/parts/twoDDisplay.h"

#include <QtCore/QJsonArray>
#include <QsLog.h>

#include <utils/canvas/textObject.h>
#include <trikControl/utilities.h>

using namespace trik::robotModel::twoD::parts;
using namespace kitBase::robotModel;

const int realWidth = 240;
const int realHeight = 280;
const int topMenuHeight = 45;
const int textSize = 20;

Display::Display(const DeviceInfo &info
		, const PortInfo &port
		, twoDModel::engine::TwoDModelEngineInterface &engine)
	: robotModel::parts::TrikDisplay(info, port)
	, mEngine(engine)
{
	mEngine.display()->setPainter(this);

	connect(this, &Display::backgroundChanged, this, [=](const QColor &color) {
		emit propertyChanged("background", color);
	});
	connect(this, &Display::smileChanged, this, [=](bool smiles, bool happy) {
		emit propertyChanged("smiles", smiles && happy);
		emit propertyChanged("sadSmiles", smiles && !happy);
	});
	connect(this, &Display::shapesSetChanged, this, [=]() {
		// This is a bit hacky, but shapes set may be modified pretty often, can be pretty large
		// and its serialization to JSON may take notable time, so we don't want to do it without real need.
		if (isSignalConnected(QMetaMethod::fromSignal(&Display::propertyChanged))) {
			emit propertyChanged("objects", toJson());
		}
	});

	qRegisterMetaType<QVector<int32_t>>("QVector<int32_t>");
}

QString Display::background() const
{
	return mBackground.name();
}

bool Display::smiles() const
{
	return mSmiles;
}

bool Display::sadSmiles() const
{
	return mSadSmiles;
}

void Display::drawPixel(int x, int y)
{
	Canvas::drawPixel(x, y);
	emit shapesSetChanged();
}

void Display::drawLine(int x1, int y1, int x2, int y2)
{
	Canvas::drawLine(x1, y1, x2, y2);
	emit shapesSetChanged();
}

void Display::drawRect(int x, int y, int width, int height, bool filled)
{
	Canvas::drawRect(x, y, width, height, filled);
	emit shapesSetChanged();
}

void Display::drawEllipse(int x, int y, int width, int height, bool filled)
{
	Canvas::drawEllipse(x, y, width, height, filled);
	emit shapesSetChanged();
}

void Display::drawArc(int x, int y, int width, int height, int startAngle, int spanAngle)
{
	Canvas::drawArc(x, y, width, height, startAngle, spanAngle);
	emit shapesSetChanged();
}

void Display::drawSmile(bool sad)
{
	if (mSmiles == !sad && mSadSmiles == sad) {
		return;
	}
	mCurrentImage = QImage(sad ? ":/icons/sadSmile.png" : ":/icons/smile.png");
	mSmiles = !sad;
	mSadSmiles = sad;
	mEngine.display()->repaintDisplay();
	emit smileChanged(true, !sad);
}

void Display::setBackground(const QColor &color)
{
	mBackground = color;
	emit backgroundChanged(color);
}

void Display::printText(int x, int y, const QString &text, int fontSize)
{
	if (fontSize <= 0) {
		fontSize = textSize;
	}
	const QPair<int, int> coords(x, y);
	if (mLabelsMap.contains(coords)) {
		mLabelsMap[coords]->setText(text);
		mLabelsMap[coords]->setFontSize(fontSize);
	} else {
		utils::TextObject * const textObject = new utils::TextObject(x, y, text
			, mCurrentPenColor, mCurrentPenWidth, fontSize);
		mObjects << textObject;
		mLabelsMap[coords] = textObject;
		mLabels << textObject;
	}

	emit shapesSetChanged();
}

void Display::clearScreen()
{
	// Background color is not cleared
	mCurrentImage = QImage();
	mSmiles = false;
	mSadSmiles = false;
	mLabelsMap.clear();
	Canvas::reset();

	emit smileChanged(false, false);
	emit shapesSetChanged();
}

void Display::setPainterColor(const QColor &color)
{
	Canvas::setPainterColor(color);
}

void Display::setPainterWidth(int penWidth)
{
	Canvas::setPainterWidth(penWidth);
}

void Display::paint(QPainter *painter, const QRect &outputRect)
{
	Q_UNUSED(outputRect)

	const QRect displayRect(0, 0, mEngine.display()->displayWidth(), mEngine.display()->displayHeight());
	const int scaledTopMenuHeight = topMenuHeight * (displayRect.height()) / (realHeight * 1.0);

	painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
	painter->save();
	painter->setPen(mBackground);
	painter->setBrush(mBackground);
	painter->drawRect(displayRect);
	painter->drawImage(QRect(0, scaledTopMenuHeight
			, displayRect.width(), displayRect.height() - scaledTopMenuHeight), mCurrentImage);
	if (mBackground != Qt::transparent) {
		painter->setBrush(QBrush(Qt::darkRed, Qt::BDiagPattern));
		painter->drawRect(0, 0, mEngine.display()->displayWidth(), scaledTopMenuHeight);
	}
	painter->restore();

	painter->save();	
	QFont font;
	font.setPixelSize(textSize);
	painter->setFont(font);
	painter->setPen(Qt::black);
	painter->translate(0, scaledTopMenuHeight);
	const qreal xScale = displayRect.width() / (realWidth * 1.0);
	const qreal yScale = (displayRect.height() - scaledTopMenuHeight) / (realHeight * 1.0);
	painter->scale(xScale, yScale);
	Canvas::paint(painter, {0, 0, realWidth, realHeight});
	painter->restore();
}

void Display::reset()
{
	clearScreen();
	setBackground(Qt::transparent);
	redraw();
}

void Display::redraw()
{
	if (mEngine.display()) {
		mEngine.display()->repaintDisplay();
	}
}

void Display::show(const QVector<int32_t> &array, int width, int height, const QString &format)
{
	mCurrentImage = trikControl::Utilities::imageFromBytes(array, width, height, format);
	mEngine.display()->repaintDisplay();
}
