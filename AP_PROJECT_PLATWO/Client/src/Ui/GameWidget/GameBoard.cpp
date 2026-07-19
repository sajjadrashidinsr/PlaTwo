/**
 * @file GameBoard.cpp
 * @brief Implementation of the GameBoard class.
 */

#include "GameBoard.h"
#include "GameController.h"
#include "BoardModel.h"
#include "Player.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QtMath>

/**
 * @brief Constructor for GameBoard.
 * @param parent Parent widget.
 */
GameBoard::GameBoard(QWidget *parent)
    : QWidget(parent)
    , m_controller(nullptr)
    , m_hasHover(false)
{
    setMinimumSize(400, 400);
    setMouseTracking(true);  // Enable mouse tracking for hover effects
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

/**
 * @brief Destructor for GameBoard.
 */
GameBoard::~GameBoard()
{
    // Qt handles deletion
}

/**
 * @brief Sets the game controller.
 * @param controller Pointer to the GameController.
 */
void GameBoard::setController(GameController* controller)
{
    m_controller = controller;
    m_hasHover = false;
    update();
}

/**
 * @brief Handles the paint event.
 * @param event The paint event.
 */
void GameBoard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    if (!m_controller) return;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // Calculate grid metrics
    int dotSpacing, dotRadius, lineWidth, padding;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth, padding);
    
    // Draw the board components in order (bottom to top)
    drawBoxes(painter, padding);
    drawLines(painter, lineWidth);
    drawDots(painter, dotRadius);
    
    // Draw hover effect on top
    if (m_hasHover && !m_controller->isGameOver()) {
        drawHoverEffect(painter, lineWidth);
    }
}

/**
 * @brief Handles mouse press events.
 * @param event The mouse event.
 */
void GameBoard::mousePressEvent(QMouseEvent* event)
{
    if (!m_controller || m_controller->isGameOver()) return;
    
    if (event->button() == Qt::LeftButton) {
        // Calculate grid metrics
        int dotSpacing, dotRadius, lineWidth, padding;
        calculateGridMetrics(dotSpacing, dotRadius, lineWidth, padding);
        
        // Find the nearest line to the click position
        QPair<QPoint, QPoint> line = findNearestLine(event->pos(),
                                                     dotSpacing, padding,
                                                     lineWidth);
        
        // If a valid line was found, try to draw it
        if (line.first != QPoint(-1, -1) && line.second != QPoint(-1, -1)) {
            m_controller->makeMove(line.first, line.second);
        }
    }
}

/**
 * @brief Handles mouse move events.
 * @param event The mouse event.
 */
void GameBoard::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_controller || m_controller->isGameOver()) {
        if (m_hasHover) {
            m_hasHover = false;
            update();
        }
        return;
    }
    
    // Calculate grid metrics
    int dotSpacing, dotRadius, lineWidth, padding;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth, padding);
    
    // Find the nearest line
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
    
    // Update hover state if changed
    if (m_hasHover != newHover || 
        (m_hasHover && (m_hoverLineStart != newStart || m_hoverLineEnd != newEnd))) {
        m_hasHover = newHover;
        m_hoverLineStart = newStart;
        m_hoverLineEnd = newEnd;
        update();
    }
}

/**
 * @brief Handles mouse leave events.
 * @param event The mouse event.
 */
void GameBoard::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    if (m_hasHover) {
        m_hasHover = false;
        update();
    }
}

/**
 * @brief Draws the dots on the board.
 * @param painter The QPainter object.
 * @param dotRadius The radius of each dot.
 */
void GameBoard::drawDots(QPainter& painter, int dotRadius)
{
    if (!m_controller) return;
    
    const BoardModel* model = m_controller->getModel();
    if (!model) return;
    
    int size = model->getBoardSize();
    int dotSpacing, dotRadius_ignored, lineWidth_ignored, padding;
    calculateGridMetrics(dotSpacing, dotRadius_ignored, lineWidth_ignored, padding);
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::black));
    
    // Draw dots at each grid position
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            QPoint pixelPos = gridToPixel(QPoint(row, col), dotSpacing, padding);
            painter.drawEllipse(pixelPos, dotRadius, dotRadius);
        }
    }
}

/**
 * @brief Draws the lines on the board.
 * @param painter The QPainter object.
 * @param lineWidth The width of each line.
 */
void GameBoard::drawLines(QPainter& painter, int lineWidth)
{
    if (!m_controller) return;
    
    const BoardModel* model = m_controller->getModel();
    if (!model) return;
    
    int size = model->getBoardSize();
    int dotSpacing, dotRadius_ignored, lineWidth_ignored, padding;
    calculateGridMetrics(dotSpacing, dotRadius_ignored, lineWidth_ignored, padding);
    
    QPen pen;
    pen.setWidth(lineWidth);
    pen.setCapStyle(Qt::RoundCap);
    
    // Draw horizontal lines
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size - 1; ++col) {
            int playerId = model->getLine(row, col, BoardModel::LineType::Horizontal);
            if (playerId != -1) {
                const Player* player = m_controller->getPlayer(playerId);
                if (player) {
                    pen.setColor(player->getColor());
                    painter.setPen(pen);
                    
                    QPoint p1 = gridToPixel(QPoint(row, col), dotSpacing, padding);
                    QPoint p2 = gridToPixel(QPoint(row, col + 1), dotSpacing, padding);
                    painter.drawLine(p1, p2);
                }
            }
        }
    }
    
    // Draw vertical lines
    for (int row = 0; row < size - 1; ++row) {
        for (int col = 0; col < size; ++col) {
            int playerId = model->getLine(row, col, BoardModel::LineType::Vertical);
            if (playerId != -1) {
                const Player* player = m_controller->getPlayer(playerId);
                if (player) {
                    pen.setColor(player->getColor());
                    painter.setPen(pen);
                    
                    QPoint p1 = gridToPixel(QPoint(row, col), dotSpacing, padding);
                    QPoint p2 = gridToPixel(QPoint(row + 1, col), dotSpacing, padding);
                    painter.drawLine(p1, p2);
                }
            }
        }
    }
}

/**
 * @brief Draws the boxes on the board.
 * @param painter The QPainter object.
 * @param padding The padding around each box.
 */
void GameBoard::drawBoxes(QPainter& painter, int padding)
{
    if (!m_controller) return;
    
    const BoardModel* model = m_controller->getModel();
    if (!model) return;
    
    int size = model->getBoardSize();
    int dotSpacing, dotRadius, lineWidth, padding_ignored;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth, padding_ignored);
    
    // Draw boxes
    for (int row = 0; row < size - 1; ++row) {
        for (int col = 0; col < size - 1; ++col) {
            int playerId = model->getBox(row, col);
            if (playerId != -1) {
                const Player* player = m_controller->getPlayer(playerId);
                if (player) {
                    QPoint topLeft = gridToPixel(QPoint(row, col), dotSpacing, padding);
                    QPoint bottomRight = gridToPixel(QPoint(row + 1, col + 1), dotSpacing, padding);
                    
                    QRect boxRect(topLeft.x() + padding / 2,
                                 topLeft.y() + padding / 2,
                                 bottomRight.x() - topLeft.x() - padding,
                                 bottomRight.y() - topLeft.y() - padding);
                    
                    QColor color = player->getColor();
                    color.setAlpha(100);  // Semi-transparent
                    
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QBrush(color));
                    painter.drawRect(boxRect);
                }
            }
        }
    }
}

/**
 * @brief Draws hover effect on a potential line.
 * @param painter The QPainter object.
 * @param lineWidth The width of the line.
 */
void GameBoard::drawHoverEffect(QPainter& painter, int lineWidth)
{
    if (!m_controller || !m_hasHover) return;
    
    int dotSpacing, dotRadius, lineWidth_ignored, padding;
    calculateGridMetrics(dotSpacing, dotRadius, lineWidth_ignored, padding);
    
    QPoint p1 = gridToPixel(m_hoverLineStart, dotSpacing, padding);
    QPoint p2 = gridToPixel(m_hoverLineEnd, dotSpacing, padding);
    
    const Player* currentPlayer = m_controller->getCurrentPlayer();
    if (!currentPlayer) return;
    
    QColor hoverColor = currentPlayer->getColor();
    hoverColor.setAlpha(HOVER_ALPHA);
    
    QPen pen;
    pen.setColor(hoverColor);
    pen.setWidth(lineWidth * 2);
    pen.setCapStyle(Qt::RoundCap);
    
    painter.setPen(pen);
    painter.drawLine(p1, p2);
}

/**
 * @brief Calculates the grid metrics.
 * @param dotSpacing Reference to store the spacing between dots.
 * @param dotRadius Reference to store the dot radius.
 * @param lineWidth Reference to store the line width.
 * @param padding Reference to store the padding.
 */
void GameBoard::calculateGridMetrics(int& dotSpacing, int& dotRadius,
                                    int& lineWidth, int& padding) const
{
    if (!m_controller) {
        dotSpacing = 50;
        dotRadius = 5;
        lineWidth = 4;
        padding = 20;
        return;
    }
    
    const BoardModel* model = m_controller->getModel();
    if (!model) {
        dotSpacing = 50;
        dotRadius = 5;
        lineWidth = 4;
        padding = 20;
        return;
    }
    
    int size = model->getBoardSize();
    
    // Calculate available space
    int widgetWidth = width();
    int widgetHeight = height();
    
    // Use the smaller dimension to maintain square aspect ratio
    int availableSize = qMin(widgetWidth, widgetHeight);
    
    // Calculate padding (20% of available size, but at least 20px)
    padding = qMax(20, static_cast<int>(availableSize * 0.1));
    
    // Calculate dot spacing
    int totalSpacing = availableSize - 2 * padding;
    dotSpacing = totalSpacing / (size - 1);
    
    // Calculate dot radius (proportional to spacing)
    dotRadius = qMax(4, dotSpacing / 12);
    
    // Calculate line width (proportional to spacing)
    lineWidth = qMax(3, dotSpacing / 8);
}

/**
 * @brief Converts grid position to pixel position.
 * @param gridPos The grid position.
 * @param dotSpacing The spacing between dots.
 * @param padding The padding.
 * @return The pixel position.
 */
QPoint GameBoard::gridToPixel(const QPoint& gridPos, int dotSpacing, int padding) const
{
    return QPoint(padding + gridPos.y() * dotSpacing,
                  padding + gridPos.x() * dotSpacing);
}

/**
 * @brief Converts pixel position to grid position.
 * @param pixelPos The pixel position.
 * @param dotSpacing The spacing between dots.
 * @param padding The padding.
 * @return The grid position, or (-1,-1) if invalid.
 */
QPoint GameBoard::pixelToGrid(const QPoint& pixelPos, int dotSpacing, int padding) const
{
    if (!m_controller) return QPoint(-1, -1);
    
    const BoardModel* model = m_controller->getModel();
    if (!model) return QPoint(-1, -1);
    
    int size = model->getBoardSize();
    
    int row = qRound(static_cast<double>(pixelPos.y() - padding) / dotSpacing);
    int col = qRound(static_cast<double>(pixelPos.x() - padding) / dotSpacing);
    
    if (row >= 0 && row < size && col >= 0 && col < size) {
        return QPoint(row, col);
    }
    
    return QPoint(-1, -1);
}

/**
 * @brief Finds the nearest line to a pixel position.
 * @param pixelPos The pixel position.
 * @param dotSpacing The spacing between dots.
 * @param padding The padding.
 * @param lineWidth The width of lines.
 * @return The line endpoints, or invalid pair if none.
 */
QPair<QPoint, QPoint> GameBoard::findNearestLine(const QPoint& pixelPos,
                                                int dotSpacing, int padding,
                                                int lineWidth) const
{
    if (!m_controller) return QPair<QPoint, QPoint>(QPoint(-1, -1), QPoint(-1, -1));
    
    const BoardModel* model = m_controller->getModel();
    if (!model) return QPair<QPoint, QPoint>(QPoint(-1, -1), QPoint(-1, -1));
    
    int size = model->getBoardSize();
    int threshold = lineWidth * 4;  // Hit detection threshold
    
    // Check horizontal lines
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size - 1; ++col) {
            QPoint p1 = gridToPixel(QPoint(row, col), dotSpacing, padding);
            QPoint p2 = gridToPixel(QPoint(row, col + 1), dotSpacing, padding);
            
            if (isNearLine(pixelPos, p1, p2, threshold)) {
                // Check if line is already drawn
                if (model->getLine(row, col, BoardModel::LineType::Horizontal) == -1) {
                    return QPair<QPoint, QPoint>(QPoint(row, col), QPoint(row, col + 1));
                }
            }
        }
    }
    
    // Check vertical lines
    for (int row = 0; row < size - 1; ++row) {
        for (int col = 0; col < size; ++col) {
            QPoint p1 = gridToPixel(QPoint(row, col), dotSpacing, padding);
            QPoint p2 = gridToPixel(QPoint(row + 1, col), dotSpacing, padding);
            
            if (isNearLine(pixelPos, p1, p2, threshold)) {
                if (model->getLine(row, col, BoardModel::LineType::Vertical) == -1) {
                    return QPair<QPoint, QPoint>(QPoint(row, col), QPoint(row + 1, col));
                }
            }
        }
    }
    
    return QPair<QPoint, QPoint>(QPoint(-1, -1), QPoint(-1, -1));
}

/**
 * @brief Checks if a point is near a line segment.
 * @param point The point to check.
 * @param p1 The first endpoint of the line.
 * @param p2 The second endpoint of the line.
 * @param threshold The maximum distance.
 * @return True if the point is near the line.
 */
bool GameBoard::isNearLine(const QPoint& point, const QPoint& p1,
                          const QPoint& p2, int threshold) const
{
    // Calculate the distance from point to line segment
    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();
    int lenSq = dx * dx + dy * dy;
    
    if (lenSq == 0) {
        // This is a point, not a line
        return (point - p1).manhattanLength() < threshold;
    }
    
    // Project point onto line
    double t = static_cast<double>((point.x() - p1.x()) * dx + (point.y() - p1.y()) * dy) / lenSq;
    
    if (t < 0) {
        // Closest point is p1
        return (point - p1).manhattanLength() < threshold;
    } else if (t > 1) {
        // Closest point is p2
        return (point - p2).manhattanLength() < threshold;
    } else {
        // Closest point is on the segment
        int projX = p1.x() + static_cast<int>(t * dx);
        int projY = p1.y() + static_cast<int>(t * dy);
        return (point - QPoint(projX, projY)).manhattanLength() < threshold;
    }
}