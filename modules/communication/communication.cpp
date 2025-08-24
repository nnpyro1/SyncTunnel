#include "communication.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QNetworkDatagram>
#include <QProcess>
#include <QHostInfo>
#include <QNetworkInterface>


Communication::Communication(){
    socket_stun = new QUdpSocket;
    socket_ipv6 = new QUdpSocket;
    socket = socket_stun;
    socket->setSocketOption(QUdpSocket::ReceiveBufferSizeSocketOption,INT_MAX);//设置大缓冲区
}
Communication::~Communication(){
    delete socket_stun;
    delete socket_ipv6;
}

Communication::ipport Communication::stun(){
//    qDebug() << "Checking firewall status...";
//    QProcess firewall;
//    firewall.start("netsh", QStringList() << "advfirewall" << "firewall" << "show" << "rule" << "name=all");
//    if (firewall.waitForFinished(3000)) {
//        QString output = firewall.readAllStandardOutput();
//        qDebug() << (output.contains("UDP") ? "Firewall rules exist" : "No UDP rules detected");
//    } else {
//        qWarning() << "Firewall check failed";
//    }
    
    
    
    socket->bind();
    disconnect(socket,&QUdpSocket::readyRead,this,&Communication::on_read);
//    qDebug()<<(stun_socket->state() == QUdpSocket::BoundState);
    
    //构造请求消息
    QByteArray request;
//    request.append((char)0x00);  // STUN 方法: Binding
//    request.append((char)0x01);
//    request.append((char)0x00);  // 消息长度
//    request.append((char)0x00);
//    request.append((char)0x21);  // Magic Cookie
//    request.append((char)0x12);
//    request.append((char)0xA4);
//    request.append((char)0x42);
    request.append('\x00').append('\x01'); // Binding Request
    request.append('\x00').append('\x00'); // Length (placeholder)
    request.append('\x21').append('\x12').append('\xA4').append('\x42'); // Magic Cookie
    for (int i = 0; i < 12; ++i) {// 添加随机事务ID (12字节)
        request.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
//    qDebug()<<"Communication::stun var:request :"<<request.toInt();
    
    
    //向服务器请求
//    stun_socket->writeDatagram(request,QHostAddress(stun_host.ip),stun_host.port);
    int sent = socket->writeDatagram(request,QHostAddress(QHostInfo::fromName(stun_host.ip).addresses()[0]),stun_host.port);
//    int sent = stun_socket->writeDatagram("HELLO",QHostAddress::LocalHost,12345);
    if(sent == -1){
        qWarning()<<"ERROR:"<<socket->errorString();
        
    }
    qDebug()<<"Communication::stun start stun";
//    qDebug()<<(stun_socket->state() == QUdpSocket::BoundState);
    socket->waitForReadyRead(2000);
//    qDebug()<<(stun_socket->state() == QUdpSocket::BoundState);
//    connect(stun_socket,&QUdpSocket::readyRead,this,[this]{    
//        QNetworkDatagram datagram = stun_socket->receiveDatagram();
//        QByteArray data = datagram.data();
//        qDebug()<<"Communication::stun var:data :"<<QString(data.toStdString().c_str());});
    
    
    //数据包解析
    QNetworkDatagram datagram = socket->receiveDatagram();
    connect(socket,&QUdpSocket::readyRead,this,&Communication::on_read);//链接
    QByteArray data = datagram.data();
    qDebug()<<"Communication::stun var:data :"<<data.isEmpty();
    if(data.size() < 20){
        qDebug()<<"TOO SHORT";
        return {"",0};
    }
    quint16 msgType = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
    if(msgType != 0x0101) {
        qWarning() << "Invalid STUN message type:" << QString::number(msgType, 16);
        return {"", 0};
    }
    
    int pos = 20; // 跳过STUN头部
     while(pos + 4 <= data.size()) {
         // 属性类型（2字节）
         quint16 attrType = (static_cast<quint8>(data[pos]) << 8) | static_cast<quint8>(data[pos+1]);
         pos += 2;
         
         // 属性长度（2字节）
         quint16 attrLength = (static_cast<quint8>(data[pos]) << 8) | static_cast<quint8>(data[pos+1]);
         pos += 2;
         
         // 计算填充后的长度（32位对齐）
         int paddedLength = (attrLength + 3) & ~0x3;
         
         // 检查边界
         if(pos + attrLength > data.size()) {
             qWarning() << "Attribute length exceeds packet size";
             break;
         }
         
         // 处理XOR-MAPPED-ADDRESS属性（0x0020）
         if(attrType == 0x0020) {
             // 检查属性长度（IPv4应为8字节）
             if(attrLength < 8) {
                 qWarning() << "Invalid XOR-MAPPED-ADDRESS length";
                 break;
             }
             
             // 获取地址族（1表示IPv4）
             quint8 family = static_cast<quint8>(data[pos+1]);
             if(family != 0x01) {
                 qWarning() << "Unsupported address family:" << family;
                 break;
             }
             
             // 解析端口（异或处理）
             quint16 xPort = (static_cast<quint8>(data[pos+2]) << 8) | static_cast<quint8>(data[pos+3]);
             quint16 port = xPort ^ 0x2112; // 异或Magic Cookie高16位
             
             // 解析IP地址（异或处理）
             quint32 xIP = (static_cast<quint8>(data[pos+4]) << 24) |
                          (static_cast<quint8>(data[pos+5]) << 16) |
                          (static_cast<quint8>(data[pos+6]) << 8) |
                          static_cast<quint8>(data[pos+7]);
             quint32 ip = xIP ^ 0x2112A442; // 异或Magic Cookie
             
             // 转换为QHostAddress
             QHostAddress address(ip);
             qDebug()<<"Communication::stun var:return"<<address.toString()<<port;
             return {address.toString(), port};
         }
         
         // 移动到下一个属性
         pos += paddedLength;
    ;
     }
     
     qWarning() << "XOR-MAPPED-ADDRESS attribute not found";
     return {"", 0};
}


Communication::ipport Communication::getIPv6(){
    //重复调用
    if(socket_ipv6->state() == QUdpSocket::BoundState){
        qWarning()<<"Communication::getIPv6 Warning:The function is repeatedly called";
        return {"",0};
    }
    
    foreach(const QNetworkInterface interface,QNetworkInterface::allInterfaces()){
        // 跳过无效接口
        if (!(interface.flags() & QNetworkInterface::IsUp) ||
            !(interface.flags() & QNetworkInterface::IsRunning) ||
            (interface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }
        
        foreach(auto entry,interface.addressEntries()){
            QHostAddress address = entry.ip();
            
            if(address.protocol() == QAbstractSocket::IPv6Protocol){//处理IPv6
                Q_IPV6ADDR ipv6 = address.toIPv6Address();
                if((ipv6[0]&0xE0) == 0x20){
                    socket = socket_ipv6;
                    if(!socket->bind()){
                        continue;
                    }
                    connect(socket,&QUdpSocket::readyRead,this,&Communication::on_read);
                    return {address.toString(),socket->localPort()};
                }
            }
        }
    }
    
    qWarning()<<"Communication::getIPv6 Warning:Control reaches end of the function";
    return {"",0};
}

//bool Communication::bind(int port){
//    connect(socket,&QUdpSocket::readyRead,this,[this](){emit readyRead();});
//    return socket->bind(port);
//}

qint64 Communication::send(ipport host, QByteArray msg){
    return socket->writeDatagram(msg,QHostAddress(host.ip),host.port);
}

QNetworkDatagram Communication::readDatagram(){
    return socket->receiveDatagram();
}