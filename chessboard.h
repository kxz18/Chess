
#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QList>
#include <QPainter>
#include <QImage>
#include <QColor>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

enum Type{
    PAWN,KNIGHT,BISHOP,
    ROOK,QUEEN,KING,
};

struct ChessPiece
{
    Type type;
    int x;
    int y;
    bool is_black;
    ChessPiece(Type _type,int _x,int _y,bool b) :
        type(_type),x(_x),y(_y),is_black(b){}
    bool operator== (const ChessPiece &a){
        return this->type==a.type && this->x==a.x
                &&this->y==a.y&&this->is_black==a.is_black;
    }
};

class ChessBoard : public QWidget
{
    Q_OBJECT

private:
    bool can_move;
    bool is_black;
    bool sCas,lCas;
    QList<ChessPiece>* chess_pieces;
    QMap<QPoint,QColor> selected_pos;
    QPoint cur_select;

public:
    explicit ChessBoard(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    QList<ChessPiece>* getChessBoard();
    bool isBlack();
    bool isBlackTurn();
    void initialize(QList<ChessPiece>* in = nullptr,bool is_black = false);
    void setPath(const ChessPiece* cur);
    bool checkWin();
    bool checkLose();
    bool checkStalemate();
    bool check();
    ChessPiece* chessAt(int x,int y);
    static QPoint toPoint(const QString &point);
    static QString toString(int x,int y);
    static QString toString(Type type);
    static Type toType(QString type);
    void sendCommand(const QPoint &obj,const QPoint &tar,QString = "");
    void getCommand(const QStringList &command);
    void checkTimeOut();
    void setYourTurn(bool y);
    bool castling();

signals:
    void sendCommand(QStringList command);
    void gameStart();
    void gameOver();
    void noCastling();

public slots:
};

#endif // CHESSBOARD_H
