#include "GameBoard.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QtMath>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

GameBoard::GameBoard(QWidget *parent)
    : QWidget(parent)
    , m_boardSize(5)
    , m_hasHover(false)
{
    setMinimumSize(400, 400);
    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    clearBoard();
}

GameBoard::~GameBoard()
{
}

void GameBoard::setBoardSize(int size)
{
    if (size < 3) size = 3;
    if (size > 10) size = 10;
    m_boardSize = size;
    clearBoard();
    update();
}

void GameBoard::clearBoard()
{
    m_horizontalLines.resize(m_boardSize);
    for (int row = 0; row < m_boardSize; ++row) {
        m_horizontalLines[row].fill(INVALID_PLAYER, m_boardSize - 1);
    }

    m_verticalLines.resize(m_boardSize - 1);
    for (int row = 0; row < m_boardSize - 1; ++row) {
        m_verticalLines[row].fill(INVALID_PLAYER, m_boardSize);
    }

    m_boxes.resize(m_boardSize - 1);
    for (int row = 0; row < m_boardSize - 1; ++row) {
        m_boxes[row].fill(INVALID_PLAYER, m_boardSize - 1);
    }
}

void GameBoard::updateBoard(const QJsonObject& state)
{
    if (state.isEmpty()) return;

    QJsonArray horizontalLines = state["horizontalLines"].toArray();
    QJsonArray verticalLines = state["verticalLines"].toArray();
    QJsonArray boxes = state["boxes"].toArray();

    int size = state["boardSize"].toInt();
    if (size != m_boardSize) {
        m_boardSize = size;
        clearBoard();
    }

    for (int row = 0; row < horizontalLines.size() && row < m_boardSize; ++row) {
        QJsonArray rowData = horizontalLines[row].toArray();
        for (int col = 0; col < rowData.size() && col < m_boardSize - 1; ++col) {
            m_horizontalLines[row][col] = rowData[col].toInt();
        }
    }

    for (int row = 0; row < verticalLines.size() && row < m_boardSize - 1; ++row) {
        QJsonArray rowData = verticalLines[row].toArray();
        for (int col = 0; col < rowData.size() && col < m_boardSize; ++col) {
            m_verticalLines[row][col] = rowData[col].toInt();
        }
    }

    for (int row = 0; row < boxes.size() && row < m_boardSize - 1; ++row) {
        QJsonArray rowData = boxes[row].toArray();
        for (int col = 0; col < rowData.size() && col < m_boardSize - 1; ++col) {
            m_boxes[row][col] = rowData[col].toInt();
        }
    }

    update();
}

void GameBoard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int dotSpacing, dotRadius, lineWidth, padding;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth, padding);

    drawBoxes(painter, padding);
    drawLines(painter, lineWidth);
    drawDots(painter, dotRadius);

    if (m_hasHover) {
        drawHoverEffect(painter, lineWidth);
    }
}

void GameBoard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;

    int dotSpacing, dotRadius, lineWidth, padding;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth, padding);

    QPair<QPoint, QPoint> line = findNearestLine(event->pos(),
                                                 dotSpacing, padding,
                                                 lineWidth);

    if (line.first != QPoint(-1, -1) && line.second != QPoint(-1, -1)) {
        emit lineClicked(line.first, line.second);
    }
}

void GameBoard::mouseMoveEvent(QMouseEvent* event)
{
    int dotSpacing, dotRadius, lineWidth, padding;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth, padding);

    QPair<QPoint, QPoint> line = findNearestLine(event->pos(),
                                                 dotSpacing, padding,
                                                 lineWidth);

    bool newHover = false;
    QPoint newStart, newEnd;

    if (line.first != QPoint(-1, -1) && line.second != QPoint(-1, -1)) {
        newHover = true;
        newStart = line.first;
        newEnd = line.second;
    }

    if (m_hasHover != newHover ||
        (m_hasHover && (m_hoverLineStart != newStart || m_hoverLineEnd != newEnd))) {
        m_hasHover = newHover;
        m_hoverLineStart = newStart;
        m_hoverLineEnd = newEnd;
        update();
    }
}

void GameBoard::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    if (m_hasHover) {
        m_hasHover = false;
        update();
    }
}

void GameBoard::drawDots(QPainter& painter, int dotRadius)
{
    int dotSpacing, dotRadius_ignored, lineWidth_ignored, padding;
    calculateGridMetrics(dotSpacing, dotRadius_ignored, lineWidth_ignored, padding);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::black));

    for (int row = 0; row < m_boardSize; ++row) {
        for (int col = 0; col < m_boardSize; ++col) {
            QPoint pixelPos = gridToPixel(QPoint(row, col), dotSpacing, padding);
            painter.drawEllipse(pixelPos, dotRadius, dotRadius);
        }
    }
}

void GameBoard::drawLines(QPainter& painter, int lineWidth)
{
    int dotSpacing, dotRadius_ignored, lineWidth_ignored, padding;
    calculateGridMetrics(dotSpacing, dotRadius_ignored, lineWidth_ignored, padding);

    QPen pen;
    pen.setWidth(lineWidth);
    pen.setCapStyle(Qt::RoundCap);

    QColor player1Color(33, 150, 243);
    QColor player2Color(244, 67, 54);

    for (int row = 0; row < m_boardSize; ++row) {
        for (int col = 0; col < m_boardSize - 1; ++col) {
            int playerId = m_horizontalLines[row][col];
            if (playerId != INVALID_PLAYER) {
                pen.setColor(playerId == 0 ? player1Color : player2Color);
                painter.setPen(pen);

                QPoint p1 = gridToPixel(QPoint(row, col), dotSpacing, padding);
                QPoint p2 = gridToPixel(QPoint(row, col + 1), dotSpacing, padding);
                painter.drawLine(p1, p2);
            }
        }
    }

    for (int row = 0; row < m_boardSize - 1; ++row) {
        for (int col = 0; col < m_boardSize; ++col) {
            int playerId = m_verticalLines[row][col];
            if (playerId != INVALID_PLAYER) {
                pen.setColor(playerId == 0 ? player1Color : player2Color);
                painter.setPen(pen);

                QPoint p1 = gridToPixel(QPoint(row, col), dotSpacing, padding);
                QPoint p2 = gridToPixel(QPoint(row + 1, col), dotSpacing, padding);
                painter.drawLine(p1, p2);
            }
        }
    }
}

void GameBoard::drawBoxes(QPainter& painter, int padding)
{
    int dotSpacing, dotRadius, lineWidth, padding_ignored;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth, padding_ignored);

    QColor player1Color(33, 150, 243);
    QColor player2Color(244, 67, 54);

    for (int row = 0; row < m_boardSize - 1; ++row) {
        for (int col = 0; col < m_boardSize - 1; ++col) {
            int playerId = m_boxes[row][col];
            if (playerId != INVALID_PLAYER) {
                QPoint topLeft = gridToPixel(QPoint(row, col), dotSpacing, padding);
                QPoint bottomRight = gridToPixel(QPoint(row + 1, col + 1), dotSpacing, padding);

                QRect boxRect(topLeft.x() + padding / 2,
                              topLeft.y() + padding / 2,
                              bottomRight.x() - topLeft.x() - padding,
                              bottomRight.y() - topLeft.y() - padding);

                QColor color = (playerId == 0) ? player1Color : player2Color;
                color.setAlpha(100);

                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(color));
                painter.drawRect(boxRect);
            }
        }
    }
}

void GameBoard::drawHoverEffect(QPainter& painter, int lineWidth)
{
    if (!m_hasHover) return;

    int dotSpacing, dotRadius, lineWidth_ignored, padding;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth_ignored, padding);

    QPoint p1 = gridToPixel(m_hoverLineStart, dotSpacing, padding);
    QPoint p2 = gridToPixel(m_hoverLineEnd, dotSpacing, padding);

    QColor hoverColor(100, 100, 100);
    hoverColor.setAlpha(HOVER_ALPHA);

    QPen pen;
    pen.setColor(hoverColor);
    pen.setWidth(lineWidth * 2);
    pen.setCapStyle(Qt::RoundCap);

    painter.setPen(pen);
    painter.drawLine(p1, p2);
}

void GameBoard::calculateGridMetrics(int& dotSpacing, int& dotRadius,
                                     int& lineWidth, int& padding) const
{
    int widgetWidth = width();
    int widgetHeight = height();

    int availableSize = qMin(widgetWidth, widgetHeight);

    padding = qMax(20, static_cast<int>(availableSize * 0.1));

    int totalSpacing = availableSize - 2 * padding;
    dotSpacing = totalSpacing / (m_boardSize - 1);

    dotRadius = qMax(4, dotSpacing / 12);
    lineWidth = qMax(3, dotSpacing / 8);
}

QPoint GameBoard::gridToPixel(const QPoint& gridPos, int dotSpacing, int padding) const
{
    return QPoint(padding + gridPos.y() * dotSpacing,
                  padding + gridPos.x() * dotSpacing);
}

QPoint GameBoard::pixelToGrid(const QPoint& pixelPos, int dotSpacing, int padding) const
{
    int row = qRound(static_cast<double>(pixelPos.y() - padding) / dotSpacing);
    int col = qRound(static_cast<double>(pixelPos.x() - padding) / dotSpacing);

    if (row >= 0 && row < m_boardSize && col >= 0 && col < m_boardSize) {
        return QPoint(row, col);
    }

    return QPoint(-1, -1);
}

QPair<QPoint, QPoint> GameBoard::findNearestLine(const QPoint& pixelPos,
                                                 int dotSpacing, int padding,
                                                 int lineWidth) const
{
    int threshold = lineWidth * 4;

    for (int row = 0; row < m_boardSize; ++row) {
        for (int col = 0; col < m_boardSize - 1; ++col) {
            QPoint p1 = gridToPixel(QPoint(row, col), dotSpacing, padding);
            QPoint p2 = gridToPixel(QPoint(row, col + 1), dotSpacing, padding);

            if (isNearLine(pixelPos, p1, p2, threshold)) {
                if (m_horizontalLines[row][col] == INVALID_PLAYER) {
                    return QPair<QPoint, QPoint>(QPoint(row, col), QPoint(row, col + 1));
                }
            }
        }
    }

    for (int row = 0; row < m_boardSize - 1; ++row) {
        for (int col = 0; col < m_boardSize; ++col) {
            QPoint p1 = gridToPixel(QPoint(row, col), dotSpacing, padding);
            QPoint p2 = gridToPixel(QPoint(row + 1, col), dotSpacing, padding);

            if (isNearLine(pixelPos, p1, p2, threshold)) {
                if (m_verticalLines[row][col] == INVALID_PLAYER) {
                    return QPair<QPoint, QPoint>(QPoint(row, col), QPoint(row + 1, col));
                }
            }
        }
    }

    return QPair<QPoint, QPoint>(QPoint(-1, -1), QPoint(-1, -1));
}

bool GameBoard::isNearLine(const QPoint& point, const QPoint& p1,
                           const QPoint& p2, int threshold) const
{
    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();
    int lenSq = dx * dx + dy * dy;

    if (lenSq == 0) {
        return (point - p1).manhattanLength() < threshold;
    }

    double t = static_cast<double>((point.x() - p1.x()) * dx + (point.y() - p1.y()) * dy) / lenSq;

    if (t < 0) {
        return (point - p1).manhattanLength() < threshold;
    } else if (t > 1) {
        return (point - p2).manhattanLength() < threshold;
    } else {
        int projX = p1.x() + static_cast<int>(t * dx);
        int projY = p1.y() + static_cast<int>(t * dy);
        return (point - QPoint(projX, projY)).manhattanLength() < threshold;
    }
}