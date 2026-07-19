/**
 * @file GameBoard.h
 */

#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QWidget>
#include <QPoint>      // ← این خط را اضافه کنید
#include <QVector>
#include <QPair>

class GameController;

class GameBoard : public QWidget
{
    Q_OBJECT

public:
    explicit GameBoard(QWidget *parent = nullptr);
    ~GameBoard() override;

    void setController(GameController* controller);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void drawDots(QPainter& painter, int dotRadius);
    void drawLines(QPainter& painter, int lineWidth);
    void drawBoxes(QPainter& painter, int padding);
    void drawHoverEffect(QPainter& painter, int lineWidth);
    void calculateGridMetrics(int& dotSpacing, int& dotRadius,
                              int& lineWidth, int& padding) const;
    QPoint gridToPixel(const QPoint& gridPos, int dotSpacing, int padding) const;
    QPoint pixelToGrid(const QPoint& pixelPos, int dotSpacing, int padding) const;
    QPair<QPoint, QPoint> findNearestLine(const QPoint& pixelPos,
                                          int dotSpacing, int padding,
                                          int lineWidth) const;
    bool isNearLine(const QPoint& point, const QPoint& p1,
                    const QPoint& p2, int threshold) const;

    GameController* m_controller;
    QPoint m_hoverLineStart;
    QPoint m_hoverLineEnd;
    bool m_hasHover;

    static constexpr int HOVER_ALPHA = 80;
};

#endif // GAMEBOARD_H