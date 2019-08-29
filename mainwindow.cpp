#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    server = nullptr;
    client = nullptr;
    timer = new QTimer(this);
    timer->setInterval(1000);
    ui->lcd_time->display(TURNTIME);
    QPalette pal = ui->lcd_time->palette();
    pal.setColor(QPalette::Normal,QPalette::WindowText,QColor(Qt::white));
    ui->lcd_time->setPalette(pal);
    ui->lcd_time->setSegmentStyle(QLCDNumber::Flat);
    connect(ui->chess_board,SIGNAL(sendCommand(QStringList)),this,SLOT(sendCommand(QStringList)));
    connect(ui->chess_board,&ChessBoard::gameStart,[=](){
        ui->btn_castling->setEnabled(true);
        this->timer->start();
    });
    connect(ui->chess_board,SIGNAL(gameOver()),this,SLOT(slt_gameOver()));
    connect(ui->chess_board,&ChessBoard::noCastling,[=](){
        ui->btn_castling->setEnabled(false);
    });
    connect(timer,SIGNAL(timeout()),this,SLOT(timeCount()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionServer_triggered()
{
    QString hostName = QHostInfo::localHostName();
    QHostInfo hostInfo = QHostInfo::fromName(hostName);
    QString ip = QInputDialog::getText(this,"Create a server","Host IP: ",QLineEdit::Normal,hostInfo.addresses().at(0).toString());
    QStringList all;
    for(auto it : QNetworkInterface::allAddresses())
        all.append(it.toString());
    if(!(all.contains(ip)||ip.left(3)=="127")){
        QMessageBox::information(this,"Error","Wrong IP");
        return;
    }
    server = new QTcpServer;
    server->listen(QHostAddress::Any,2333);
    connect(this->server,SIGNAL(newConnection()),this,SLOT(acceptConnection()));
    ui->txt_connection->clear();
    ui->txt_connection->setText("Server created\nHost IP:\n"+ip+"\nWaiting to be connected ...");
}

void MainWindow::on_actionClient_triggered()
{
    if(client != nullptr) delete client;
    client = new QTcpSocket;
    connect(client,&QAbstractSocket::stateChanged,[=](QAbstractSocket::SocketState s){qDebug()<<s;});
    QString ip = QInputDialog::getText(this,"Join a server","Server IP: ");
    QStringList judge = ip.split(".");
    bool ok = true;
    if(judge.count()==4){
        for(auto it : judge){
            bool isInt = true;
            it.toInt(&isInt);
            if(isInt&&it.toInt()>=0&&it.toInt()<=255) continue;
            else ok = false;
        }
    }else ok = false;
    if(!ok){
        QMessageBox::information(this,"Error","Wrong IP");
        return;
    }
    QHostAddress address(ip);
    client->connectToHost(address,2333);
    connect(client,SIGNAL(connected()),this,SLOT(getConnected()));
    connect(client,SIGNAL(readyRead()),this,SLOT(getCommand()));
}

void MainWindow::on_btn_quit_clicked()
{
    this->close();
}

void MainWindow::acceptConnection()
{
    if(server->hasPendingConnections())
    {
        if(client != nullptr){
            delete client;
            client = nullptr;
        }
        client = server->nextPendingConnection();
        connect(client,SIGNAL(readyRead()),this,SLOT(getCommand()));
        connect(client,SIGNAL(disconnected()),this,SLOT(clientDisconnect()));
        QString ip = client->peerAddress().toString();
        ui->txt_connection->append("\nConnection created\nClient IP:\n"+ip);
        QString ifNew = QInputDialog::getItem(this,"Choose","Start a new game or load an ending",{"New game","Load ending"},0,false);
        if(ifNew == "Load ending")
            on_actionLoad_ending_triggered();
        else{
            QString color = QInputDialog::getItem(this,"Choose color","Choose your color",{"Black","White"},0,false);
            ui->chess_board->initialize(nullptr,color == "Black");
            sendCommand({color});
        }
    }
}

void MainWindow::getConnected()
{
    if(client->state()==QAbstractSocket::ConnectedState){
        QHostAddress ip = client->peerAddress();
        ui->txt_connection->append("Connection established\nServer IP: "+ip.toString());
    }
}

void MainWindow::getCommand()
{
    ui->lcd_time->display(TURNTIME);//refresh timer
    QString info = client->readAll();
    if(info == "Black"){
        ui->txt_status->append("You are white");
        ui->chess_board->setYourTurn(!(ui->chess_board->isBlack()));
        ui->txt_status->append(!(ui->chess_board->isBlack())?"Your turn":"Opponent's turn");
        timer->start();
    }else if(info == "White"){
        timer->start();
        ui->txt_status->append("You are black");
        ui->chess_board->setYourTurn(ui->chess_board->isBlack());
        ui->txt_status->append(ui->chess_board->isBlack()?"Your turn":"Opponent's turn");
    }else if(info == "Forfeit"){
        ui->txt_status->append("The game was forfeited, you win");
        QMessageBox::information(this,"game over","The game was forfeited. You win!");
        slt_gameOver();
        return;
    }else if(info.left(6)=="Ending"){
        Ending in = Parser::interpret(info.mid(7));
        ui->chess_board->initialize(in.data,in.isBlackFirst);
        return;
    }else
        ui->txt_status->append(info+"\nYour turn");
    ui->chess_board->getCommand(info.split(" "));
}

void MainWindow::sendCommand(QStringList command)
{
    ui->lcd_time->display(TURNTIME);//refresh timer
    if(command.count()==1&&command[0]!="Forfeit"&&this->server==nullptr) return;//client don't send color choice
    QString info = command.join(" ");
    if(command.count() !=1 ){
        ui->txt_status->append(info+"\nOpponent's turn");
    }else if(command[0]!="Forfeit"){
        ui->txt_status->clear();//restart a game
        ui->txt_status->append("You are "+info);
        if((info=="White"&&!ui->chess_board->isBlackTurn()) ||
                (info=="Black"&&ui->chess_board->isBlackTurn())) ui->txt_status->append("Your turn");
        else ui->txt_status->append("Opponent's turn");
        timer->start();
    }else {
        ui->txt_status->append("The game was forfeited");
    }
    if(client == nullptr)
        return;
    QByteArray ba;
    ba.clear();
    qDebug()<<info;
    ba.append(info);
    client->write(ba.data());
    if(info == "Forfeit"){
         QMessageBox::information(this,"game over","You have forfeited the game");
         slt_gameOver();
    }
}

void MainWindow::timeCount()
{
    int count = static_cast<int>(ui->lcd_time->value());
    ui->lcd_time->display(--count);

    QPalette pal = ui->lcd_time->palette();
    if(count<=10)
        pal.setColor(QPalette::Normal,QPalette::WindowText,QColor(Qt::red));
    else
        pal.setColor(QPalette::Normal,QPalette::WindowText,QColor(Qt::white));
    ui->lcd_time->setPalette(pal);

    if(count == 0){
        timer->stop();
        ui->chess_board->checkTimeOut();
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::clientDisconnect()
{
        ui->txt_connection->append("\nClient disconnected");
        client = nullptr;
}

void MainWindow::on_actionDisconnect_triggered()
{
    if(server != nullptr){
        server->close();
        delete server;
        server = nullptr;
        ui->txt_connection->append("\nServer shut down");
    }else if(client != nullptr){
        client->disconnectFromHost();
        delete client;
        client = nullptr;
        ui->txt_connection->append("Disconnected from server");
    }
}

void MainWindow::on_btn_reset_clicked()
{
    if(server != nullptr || client != nullptr){
        QStringList choice = {"Black","White"};
        QString color = QInputDialog::getItem(this,"Choose color","Choose your color",choice,0,false);
        ui->chess_board->initialize(nullptr,color == "Black");

    }
}

void MainWindow::slt_gameOver()
{
    timer->stop();
    delete client;
    client = nullptr;
}

void MainWindow::on_btn_forfeit_clicked()
{
    if(server != nullptr || client != nullptr){
        QString confirm = QInputDialog::getItem(this,"Confirm","You sure?",{"No","Yes"},0,false);
        if(confirm == "Yes")
            sendCommand({"Forfeit"});
    }
}

void MainWindow::on_btn_castling_clicked()
{
    ui->chess_board->castling();
}

void MainWindow::on_actionSave_ending_triggered()
{
    QString status = Parser::toFile(ui->chess_board->getChessBoard(),
                                    ui->chess_board->isBlackTurn());
    if(status=="") return;
    QString fileName = QFileDialog::getSaveFileName(this,"Save ending","./");
    if(fileName=="") return;
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly)) return;
    QTextStream in(&f);
    in<<status;
}

void MainWindow::on_actionLoad_ending_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Load ending","./");
    Ending data = Parser::interpret(fileName);
    ui->chess_board->initialize(data.data,data.isBlackFirst);
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    QString co= "Ending "+in.readAll();
    sendCommand(co.split(" "));
    QString color = QInputDialog::getItem(this,"Choose color","Choose your color",{"Black","White"},0,false);
    ui->chess_board->initialize(data.data,color == "Black");
    ui->chess_board->setYourTurn(data.isBlackFirst&&color=="Black");
    sendCommand({color});
}
