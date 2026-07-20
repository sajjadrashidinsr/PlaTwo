#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QWidget>
#include <QPoint>
#include <QVector>
#include <QPair>
#include <QJsonObject>

class GameBoard : public QWidget
{
    Q_OBJECT

public:
    explicit GameBoard(QWidget *parent = nullptr);
    ~GameBoard() override;

    void setBoardSize(int size);
    void updateBoard(const QJsonObject& state);
    void clearBoard();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

signals:
    void lineClicked(const QPoint& p1, const QPoint& p2);

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

    int m_boardSize;
    QVector<QVector<int>> m_horizontalLines;
    QVector<QVector<int>> m_verticalLines;
    QVector<QVector<int>> m_boxes;

    QPoint m_hoverLineStart;
    QPoint m_hoverLineEnd;
    bool m_hasHover;

    static constexpr int HOVER_ALPHA = 80;
    static constexpr int INVALID_PLAYER = -1;
};

#endif // GAMEBOARD_H