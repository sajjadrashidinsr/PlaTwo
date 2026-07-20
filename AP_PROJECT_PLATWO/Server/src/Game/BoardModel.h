#ifndef BOARDMODEL_H
#define BOARDMODEL_H

#include <QObject>
#include <QVector>
#include <QPoint>

class BoardModel : public QObject
{
    Q_OBJECT

public:
    enum class LineType {
        Horizontal,
        Vertical
    };

    explicit BoardModel(int boardSize, QObject* parent = nullptr);
    ~BoardModel() override;

    int getBoardSize() const { return m_boardSize; }

    int getLine(int row, int col, LineType type) const;
    bool setLine(int row, int col, LineType type, int playerId);
    int getBox(int row, int col) const;
    bool isLineDrawn(int row, int col, LineType type) const;
    bool isValidMove(const QPoint& p1, const QPoint& p2) const;
    QVector<QPoint> makeMove(const QPoint& p1, const QPoint& p2, int playerId);
    bool isGameOver() const;
    int getTotalBoxes() const;
    int getBoxesForPlayer(int playerId) const;
    void reset();

signals:
    void boxCompleted(int row, int col, int playerId);

private:
    bool isBoxComplete(int row, int col) const;
    bool areBoxLinesDrawn(int row, int col) const;
    bool getLineIndices(const QPoint& p1, const QPoint& p2,
                        int& row, int& col, LineType& type) const;

    int m_boardSize;
    QVector<QVector<int>> m_horizontalLines;
    QVector<QVector<int>> m_verticalLines;
    QVector<QVector<int>> m_boxes;

    static constexpr int INVALID_PLAYER = -1;
};

#endif // BOARDMODEL_H