#ifndef PARSER_H
#define PARSER_H
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "chessboard.h"

struct Ending{
    bool isBlackFirst;
    QList<ChessPiece>* data;
};

class Parser{
private:
    Parser(){}
    ~Parser(){}

public:
    Parser(const Parser&) = delete;
    static Ending interpret(QString file_name){
        QTextStream in;
        QFile f(file_name);
        if(file_name.left(5)=="black"||file_name.left(5)=="white")
            in.setString(&file_name);
        else{
            if(!f.open(QIODevice::ReadOnly)){
                QMessageBox::information(nullptr,"Tip","Wrong input file");
                Ending no;
                no.data=nullptr;
                return no;
            }
            in.setDevice(&f);
        }
        Ending data;
        QList<ChessPiece> *ending = new QList<ChessPiece>;
        QStringList state = in.readLine().split(" ");
        while(!in.atEnd()){
            if(state.count()==1){
                bool is_black = state[0]=="black";
                data.isBlackFirst = is_black;
                state = in.readLine().split(" ");
                do{
                    for(int i=0;i<state[1].toInt();i++){
                        QPoint pos=ChessBoard::toPoint(state[i+2].toUpper());
                        Type type=ChessBoard::toType(state[0]);
                        ending->append(ChessPiece(type,pos.x(),pos.y(),is_black));
                    }
                    state = in.readLine().split(" ");
                }while(state.count()!=1&&state.count()!=0);
            }
        }
        data.isBlackFirst=!(data.isBlackFirst);//the loop will record who goes second
        data.data=ending;
        return data;
    }
    static QString toFile(const QList<ChessPiece> *src,bool isBlackFirst){
        if(src==nullptr) return "";
        QString file="";
        QMap<Type,QList<ChessPiece>> data;
        for(auto it : *src){
            data[it.type].append(it);
        }
        for(int i=0;i<2;i++){
            if(isBlackFirst)
                file.append("black\n");
            else file.append("white\n");
            for(auto it : data.keys()){
                int cnt = 0;
                QString poses="";
                for(auto piece : data[it]){
                    if(piece.is_black==isBlackFirst){
                        poses.append(" "+ChessBoard::toString(piece.x,piece.y).toLower());
                        cnt++;
                    }
                }
                file.append(ChessBoard::toString(it).toLower());
                file.append(" "+QString::number(cnt));
                file.append(poses+"\n");
            }
            isBlackFirst=!isBlackFirst;//change for the second execute
       }
       return file;
    }

};

#endif // PARSER_H
