#include "chessboard.h"
bool operator< (const QPoint& a,const QPoint&b)
{
    return a.x()==b.x()?a.y()<b.y():a.x()<b.x();
}

ChessBoard::ChessBoard(QWidget *parent) : QWidget(parent),
    chess_pieces(nullptr)
{
    is_black = false;
    can_move = true;
    cur_select = QPoint(-1,-1);
    update();
}

void ChessBoard::paintEvent(QPaintEvent *event)
{
    int w=this->width()/8;
    int h=this->height()/8;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if(!is_black)
        painter.setWindow(0,this->height(),this->width(),-this->height());
    else
        painter.setWindow(this->width(),0,-this->width(),this->height());
    QPen pen;
    pen.setColor(QColor(0,0,0,0));
    painter.setPen(pen);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++)
        {
            int x,y;
            if(is_black){//translate coordination
                y=7-i;x=7-j;
            }
            else{
                y=i;x=j;
            }
            if(selected_pos.keys().contains(QPoint(x,y)))
                brush.setColor(selected_pos[QPoint(x,y)]);
            else if((x+y)%2!=0)
                brush.setColor(QColor(0xF0,0xD8,0xA8));
            else
                brush.setColor(QColor(0xC0,0x90,0x60));
            painter.setBrush(brush);
            painter.drawRect(x*w,y*h,w,h);
        }
    }
    if(chess_pieces == nullptr) return;
    foreach (auto it, *chess_pieces) {
        QImage image;
        switch (it.type) {
        case PAWN:
            if(it.is_black)
                image.load(":/images/black_pawn.png");
            else
                image.load(":/images/white_pawn.png");
            break;
        case KNIGHT:
            if(it.is_black)
                image.load(":/images/black_knight.png");
            else
                image.load(":/images/white_knight.png");
            break;
        case BISHOP:
            if(it.is_black)
                image.load(":/images/black_bishop.png");
            else
                image.load(":/images/white_bishop.png");
            break;
        case ROOK:
            if(it.is_black)
                image.load(":/images/black_rook.png");
            else
                image.load(":/images/white_rook.png");
            break;
        case QUEEN:
            if(it.is_black)
                image.load(":/images/black_queen.png");
            else
                image.load(":/images/white_queen.png");
            break;
        case KING:
            if(it.is_black)
                image.load(":/images/black_king.png");
            else
                image.load(":/images/white_king.png");
            break;
        }
        if(this->is_black) image=image.mirrored(true,false);//mirror
        else image=image.mirrored(false,true);
        painter.drawPixmap(it.x*w,it.y*h,w,h,QPixmap::fromImage(image));
    }
}

void ChessBoard::mousePressEvent(QMouseEvent *event)
{
    int w = this->width()/8;
    int h = this->height()/8;
    int x = event->x()/w;
    int y = event->y()/h;

    if(is_black)//translate coordination
        x=7-x;
    else
        y=7-y;
                //cur_select = QPoint(x,y);
    if(cur_select==QPoint(-1,-1) || !selected_pos.keys().contains(QPoint(x,y))){//not selected
        selected_pos.clear();
        if(chessAt(x,y)==nullptr)
            selected_pos[QPoint(x,y)]=QColor(Qt::yellow);
        else if(chessAt(x,y)->is_black!=this->is_black)
            selected_pos[QPoint(x,y)]=QColor(Qt::red);
        else{
            selected_pos.clear();
            selected_pos[QPoint(x,y)]=QColor(Qt::yellow).lighter();
            setPath(chessAt(x,y));
            }
        cur_select=QPoint(x,y);
    }else if(cur_select == QPoint(x,y)){ //cancel selection
        cur_select=QPoint(-1,-1);
        selected_pos.clear();
    }else if(can_move){//have selected and can move
        if(selected_pos.keys().contains(QPoint(x,y))){
            if(selected_pos[QPoint(x,y)]==QColor(Qt::red)){//eat
                chess_pieces->removeOne(*chessAt(x,y));
            }
            ChessPiece* tmp = chessAt(cur_select.x(),cur_select.y());
            tmp->x=x;tmp->y=y;//move
            if(tmp->type == KING && tmp->is_black==this->is_black) emit noCastling();
            else if(tmp->type == ROOK && tmp->is_black==this->is_black){
                if(tmp->x==0&&tmp->y==this->is_black*7) lCas=false;
                else if(tmp->x==7&&tmp->y==this->is_black*7) sCas=false;
                if(!lCas&&!sCas) emit noCastling();
            }
            selected_pos.clear();
            if(chessAt(x,y) != nullptr && chessAt(x,y)->type==PAWN && (
                     (!this->is_black && chessAt(x,y)->y==7) ||
                     (this->is_black && chessAt(x,y)->y==0))){
                QStringList list = {"Knight","Bishop","Rook","Queen"};
                QString promotion = QInputDialog::getItem(this,"Promotion","promotion to: ",list,3,false);
                if(promotion=="Queen")
                    chessAt(x,y)->type = QUEEN;
                else if(promotion=="Rook")
                    chessAt(x,y)->type = ROOK;
                else if(promotion=="Bishop")
                    chessAt(x,y)->type = BISHOP;
                else if(promotion=="Knight")
                    chessAt(x,y)->type = KNIGHT;
                sendCommand(cur_select,QPoint(x,y),promotion);
            }else{
                sendCommand(cur_select,QPoint(x,y));
            }
        }
    }
    update();
    checkWin();
    checkLose();
    return;
}

QList<ChessPiece>* ChessBoard::getChessBoard()
{
    return this->chess_pieces;
}

bool ChessBoard::isBlack()
{
    return is_black;
}

bool ChessBoard::isBlackTurn()
{
    return (is_black&&can_move) ||
            (!is_black&&!can_move);
}

void ChessBoard::initialize(QList<ChessPiece> *in,bool b)
{
    this->is_black = b;
    can_move = !is_black;
    lCas=true;sCas=true;
    if(in!=chess_pieces||in==nullptr){
        if(chess_pieces != nullptr){
            delete chess_pieces;
            chess_pieces = nullptr;
        }
        if(in == nullptr){
            chess_pieces = new QList<ChessPiece>;
            for(int i=0;i<8;i++){
                chess_pieces->append(ChessPiece(PAWN,i,1,false));
                chess_pieces->append(ChessPiece(PAWN,i,6,true));
            }
            for(int i=0;i<2;i++){
                chess_pieces->append(ChessPiece(ROOK,i*7,0,false));
                chess_pieces->append(ChessPiece(ROOK,i*7,7,true));
                chess_pieces->append(ChessPiece(KNIGHT,1+i*5,0,false));
                chess_pieces->append(ChessPiece(KNIGHT,1+i*5,7,true));
                chess_pieces->append(ChessPiece(BISHOP,2+i*3,0,false));
                chess_pieces->append(ChessPiece(BISHOP,2+i*3,7,true));
            }
            chess_pieces->append(ChessPiece(QUEEN,3,0,false));
            chess_pieces->append(ChessPiece(QUEEN,3,7,true));
            chess_pieces->append(ChessPiece(KING,4,0,false));
            chess_pieces->append(ChessPiece(KING,4,7,true));
        }else{
            chess_pieces = in;
        }
    }

    emit gameStart();
    update();
}

void ChessBoard::setPath(const ChessPiece *cur)
{
    if(cur == nullptr) return;
    int x=cur->x;
    int y=cur->y;
    switch (cur->type) {
    case PAWN:
        if(cur->is_black){
            if(chessAt(x,y-1)==nullptr){
                selected_pos[QPoint(x,y-1)]=QColor(Qt::yellow);
                if(chessAt(x,y-2)==nullptr&&y==6)
                    selected_pos[QPoint(x,y-2)]=QColor(Qt::yellow);
            }/*else if(chessAt(x,y-1)->is_black==false){
                bool ok = false;
                if(x>0 && chessAt(x-1,y-1)==nullptr){
                    ok = true;
                    selected_pos[QPoint(x-1,y-1)]=QColor(Qt::yellow);
                }
                if(x<7 && chessAt(x+1,y-1)==nullptr){
                    ok = true;
                    selected_pos[QPoint(x+1,y-1)]=QColor(Qt::yellow);
                }
                if(ok)
                    selected_pos[QPoint(x,y-1)]=QColor(Qt::red);
            }*/
            if(x>0 && chessAt(x-1,y-1)!=nullptr && !chessAt(x-1,y-1)->is_black){
                selected_pos[QPoint(x-1,y-1)]=QColor(Qt::red);
            }
            if(x<7 && chessAt(x+1,y-1)!=nullptr && !chessAt(x+1,y-1)->is_black)
                selected_pos[QPoint(x+1,y-1)]=QColor(Qt::red);
        }else{
            if(chessAt(x,y+1)==nullptr){
                selected_pos[QPoint(x,y+1)]=QColor(Qt::yellow);
                if(chessAt(x,y+2)==nullptr&&y==1)
                    selected_pos[QPoint(x,y+2)]=QColor(Qt::yellow);
            }/*else if(chessAt(x,y+1)->is_black==true){
                bool ok = false;
                if(x>0 && chessAt(x-1,y+1)==nullptr){
                    ok = true;
                    selected_pos[QPoint(x-1,y+1)]=QColor(Qt::yellow);
                }
                if(x<7 && chessAt(x+1,y+1)==nullptr){
                    ok = true;
                    selected_pos[QPoint(x+1,y+1)]=QColor(Qt::yellow);
                }
                if(ok)
                    selected_pos[QPoint(x,y+1)]=QColor(Qt::red);
            }*/
            if(x>0 && chessAt(x-1,y+1)!=nullptr && chessAt(x-1,y+1)->is_black){
                selected_pos[QPoint(x-1,y+1)]=QColor(Qt::red);
            }
            if(x<7 && chessAt(x+1,y+1)!=nullptr && chessAt(x+1,y+1)->is_black){
                selected_pos[QPoint(x+1,y+1)]=QColor(Qt::red);
            }
        }
        break;
    case KNIGHT:
       {
        QList<int> addX = {1,1,-1,-1,2,2,-2,-2};
        QList<int> addY = {2,-2,2,-2,1,-1,1,-1};
        for(int i=0;i<8;i++){
            int tmpx=x+addX[i],tmpy=y+addY[i];
            if(chessAt(tmpx,tmpy)==nullptr)
                selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::yellow);
            else if(chessAt(tmpx,tmpy)->is_black!=this->is_black)
                selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::red);
        }
       }
        break;
    case ROOK:
        for(int i=0;i<4;i++){
            for(int j=1;j<8;j++){
                int tmpx=0,tmpy=0;
                switch (i) {
                case 0:tmpx=x+j;tmpy=y;break;
                case 1:tmpx=x;tmpy=y+j;break;
                case 2:tmpx=x-j;tmpy=y;break;
                case 3:tmpx=x;tmpy=y-j;break;
                }
                if(chessAt(tmpx,tmpy)==nullptr)
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::yellow);
                else if(chessAt(tmpx,tmpy)->is_black!=this->is_black){
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::red);
                    break;
                }else break;
            }
        }
        break;
    case BISHOP:
        for(int i=0;i<4;i++){
            for(int j=1;j<8;j++){
                int tmpx=0,tmpy=0;
                switch (i) {
                case 0:tmpx=x+j;tmpy=y+j;break;
                case 1:tmpx=x-j;tmpy=y+j;break;
                case 2:tmpx=x-j;tmpy=y-j;break;
                case 3:tmpx=x+j;tmpy=y-j;break;
                }
                if(chessAt(tmpx,tmpy)==nullptr)
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::yellow);
                else if(chessAt(tmpx,tmpy)->is_black!=this->is_black){
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::red);
                    break;
                }else break;
            }
        }
        break;
    case QUEEN://ROOK + BISHOP
        for(int i=0;i<4;i++){
            for(int j=1;j<8;j++){
                int tmpx=0,tmpy=0;
                switch (i) {
                case 0:tmpx=x+j;tmpy=y;break;
                case 1:tmpx=x;tmpy=y+j;break;
                case 2:tmpx=x-j;tmpy=y;break;
                case 3:tmpx=x;tmpy=y-j;break;
                }
                if(chessAt(tmpx,tmpy)==nullptr)
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::yellow);
                else if(chessAt(tmpx,tmpy)->is_black!=this->is_black){
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::red);
                    break;
                }else break;
            }
        }
        for(int i=0;i<4;i++){
            for(int j=1;j<8;j++){
                int tmpx=0,tmpy=0;
                switch (i) {
                case 0:tmpx=x+j;tmpy=y+j;break;
                case 1:tmpx=x-j;tmpy=y+j;break;
                case 2:tmpx=x-j;tmpy=y-j;break;
                case 3:tmpx=x+j;tmpy=y-j;break;
                }
                if(chessAt(tmpx,tmpy)==nullptr)
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::yellow);
                else if(chessAt(tmpx,tmpy)->is_black!=this->is_black){
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::red);
                    break;
                }else break;
            }
        }
        break;
    case KING:
        for(int i=-1;i<2;i++){
            for(int j=-1;j<2;j++){
                if(i==0&&j==0) continue;
                int tmpx=x+i,tmpy=y+j;
                if(chessAt(tmpx,tmpy)==nullptr)
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::yellow);
                else if(chessAt(tmpx,tmpy)->is_black!=this->is_black)
                    selected_pos[QPoint(tmpx,tmpy)]=QColor(Qt::red);
            }
        }
        break;
    }
}

bool ChessBoard::checkWin()
{
    if(chess_pieces == nullptr) return false;
    for(auto it : *chess_pieces)
    {
        if(it.type==KING && it.is_black!=this->is_black)
            return false;
    }
    QMessageBox::information(this,"Congratulation","You win!");
    can_move = false;
    emit gameOver();
    return true;
}

bool ChessBoard::checkLose()
{
    if(chess_pieces == nullptr) return false;
    for(auto it : *chess_pieces)
    {
        if(it.type==KING && it.is_black==this->is_black)
            return false;
    }
    QMessageBox::information(this,"Over","You Lose!");
    can_move = false;
    emit gameOver();
    return true;
}

ChessPiece* ChessBoard::chessAt(int x, int y)
{
    if(chess_pieces == nullptr) return nullptr;
    for(ChessPiece& it : *chess_pieces){
        if(it.x==x && it.y==y)
            return &it;
    }
    return nullptr;
}

QPoint ChessBoard::toPoint(const QString &point)
{
    std::string s=point.toStdString();
    std::string base = "A";
    int x = s[0]-base[0];
    int y = point[1].digitValue()-1;
    return QPoint(x,y);
}

QString ChessBoard::toString(int x, int y)
{
    std::string base = "A";
    char _x = static_cast<char>(x+base[0]);
    return QString::asprintf("%c%d",_x,y+1);
}

QString ChessBoard::toString(Type type)
{
    switch (type) {
    case PAWN:return "Pawn";
    case KNIGHT:return "Knight";
    case BISHOP:return "Bishop";
    case ROOK:return "Rook";
    case QUEEN:return "Queen";
    case KING:return "King";
    }
}

Type ChessBoard::toType(QString type)
{
    type=type.toLower();
    if(type=="king") return KING;
    else if(type=="queen") return QUEEN;
    else if(type=="rook") return ROOK;
    else if(type=="bishop") return BISHOP;
    else if(type=="knight") return KNIGHT;
    else return PAWN;
}

void ChessBoard::sendCommand(const QPoint &obj, const QPoint &tar, QString pro)
{
    QStringList commands;
    if(this->is_black) commands.append("Black");
    else commands.append("White");
    ChessPiece* _obj = chessAt(tar.x(),tar.y()); //have changed position to tar
    if(_obj->type == PAWN)
        commands.append("Pawn");
    else if(_obj->type == KNIGHT)
        commands.append("Knight");
    else if(_obj->type == BISHOP)
        commands.append("Bishop");
    else if(_obj->type == ROOK)
        commands.append("Rook");
    else if(_obj->type == QUEEN)
        commands.append("Queen");
    else if(_obj->type == KING)
        commands.append("King");
    commands.append(toString(obj.x(),obj.y()));
    commands.append(toString(tar.x(),tar.y()));
    if(pro != ""){
        commands[1] = "Pawn";
        commands.append(pro);
    }
    emit sendCommand(commands);
    can_move = false;
}

void ChessBoard::getCommand(const QStringList &command)
{
    if(command.count()==1&&chess_pieces==nullptr){                     //initialize
        this->is_black = command[0]=="White";
        initialize(nullptr,is_black);
        return;
    }else if(command.count()==1&&chess_pieces!=nullptr){
        this->is_black = command[0]=="White";
        update();
        return;
    }

    if((command[0]=="Black"&&this->is_black) ||
            (command[0]=="White"&&!this->is_black))//if valid
        return;

    if(command[0]=="Castling"){
        QPoint kingPos = toPoint(command[1]);
        QPoint rookPos = toPoint(command[2]);
        ChessPiece *king = chessAt(kingPos.x(),kingPos.y());
        ChessPiece *rook = chessAt(rookPos.x(),rookPos.y());
        if(rookPos.x()>kingPos.x()){
            king->x+=2;
            rook->x-=2;
        }else{
            king->x-=2;
            rook->x+=3;
        }
    }else {

        QPoint obj = toPoint(command[2]);
        QPoint tar = toPoint(command[3]);
        ChessPiece *_obj = chessAt(obj.x(),obj.y());
        ChessPiece *_tar = chessAt(tar.x(),tar.y());

        if(_obj != nullptr){                    //move
            if(_tar != nullptr){
                chess_pieces->removeOne(*_tar);
            }
            _obj->x = tar.x();
            _obj->y = tar.y();
            if(command.count()==5){                 //promotion
                if(command[4]=="Queen")
                    _obj->type = QUEEN;
                else if(command[4]=="Rook"||command[4]=="Castle")
                    _obj->type = ROOK;
                else if(command[4]=="Bishop")
                    _obj->type = BISHOP;
                else if(command[4]=="Knight")
                    _obj->type = KNIGHT;
            }
        }
    }
    update();
    checkWin();
    checkLose();
    can_move = true;
}

void ChessBoard::checkTimeOut()
{
    if(can_move)
        QMessageBox::information(this,"Tip","Time out! You Lose!");
    else
        QMessageBox::information(this,"Tip","Time out! You win!");
    can_move = false;
    emit gameOver();
}

void ChessBoard::setYourTurn(bool y)
{
    can_move = y;
}

bool ChessBoard::castling()
{
    if((!lCas&&!sCas) || !can_move) return false;
    else{//king and rook haven't moved
        QList<QPoint> attacked;
        ChessPiece *king = nullptr;
        selected_pos.clear();
        for(auto &it : *chess_pieces){
            if(it.is_black!=this->is_black){
                is_black=!is_black;//temporarily change my color(check checking state)
                setPath(&it);
                is_black=!is_black;
                attacked+=selected_pos.keys();
                selected_pos.clear();
            }else if(it.is_black==this->is_black&&it.type==KING)
                king = &it;
        }//set attack path and king position
        bool shortOk=true,longOk=true;
        for(int i=0;i<3;i++){
            if(attacked.contains(QPoint(king->x+i,king->y)))//attack
                shortOk=false;
            if(attacked.contains(QPoint(king->x-i,king->y))||
                    chessAt(king->x-i-1,king->y)!=nullptr)
                longOk=false;
        }//judge path
        if(chessAt(king->x+1,king->y)!=nullptr||chessAt(king->x+2,king->y)!=nullptr)
            shortOk=false;
        bool ok = true;
        QStringList choice;
        if(lCas&&longOk) choice.append("Long castling");
        if(sCas&&shortOk) choice.append("Short castling");
        if(choice.count()==0){
            QMessageBox::information(this,"Tip","King is being checked or both paths are under attack or uncleared");
            return false;
        }
        QString dir = QInputDialog::getItem(this,"Castling","Please choose",choice,0,false,&ok);
        if(ok){
            QStringList command = {"Castling",toString(king->x,king->y)};
            if(dir=="Short castling"){
                ChessPiece* rook=chessAt(king->x+3,king->y);
                command.append(toString(rook->x,rook->y));
                king->x+=2;
                rook->x-=2;
            }else{
                ChessPiece* rook=chessAt(king->x-4,king->y);
                command.append(toString(rook->x,rook->y));
                king->x-=2;
                rook->x+=3;
            }
            update();
            lCas=false;sCas=false;
            emit noCastling();
            emit sendCommand(command);
            return true;
        }else {
            return false;
        }
    }
}
