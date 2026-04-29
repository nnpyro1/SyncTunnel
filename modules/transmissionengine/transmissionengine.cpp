#include "transmissionengine.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <qaesencryption.h>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>
#include <QThread>
#include <QDir>
#include <QtEndian>
#include <QElapsedTimer>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <QJsonArray>
#include <QQueue>


TransmissionEngine::TransmissionEngine(Communication *m_communication, QString username, QString pwd, device public_ip, QObject *parent): QObject(parent),m_communication(m_communication),user_name(username),pwd(pwd),public_ip(public_ip){
    //信号槽绑定
    connect(m_communication,&Communication::readyRead,this,&TransmissionEngine::on_readyRead);
//    connect(&timer_fileResend,&QTimer::timeout,this,&TransmissionEngine::on_request_resend); 
    connect(&timer_is_uploading,&QTimer::timeout,this,[this]{chunks.clear();is_uploading=false;send_current_delay=SEND_MAX_DELAY-10;/*send_stable_count=0;send_ack_count.clear();*/send_req_ack_loop=5;send_lost_loop_count=0;send_lost_count.clear();/*foreach(auto i,schedule_list)i->setEnabled(true);*/});      //发送方清除状态
//    connect(&timer_clear_currentFileMap,&QTimer::timeout,this,[this]{currentFileMap.clear();currentFileTotal = -1;/*;receive_lost_count=0;*/receive_last_pack_index=-1;receive_last_ack_total=-1;timer_fileResend.stop();/*receive_last_ack_index=-1*//*;foreach(auto i,schedule_list)i->setEnabled(true);*/});//接收方清除状态
    connect(&timer_keepAlive,&QTimer::timeout,this,[this]{if(chunks.empty()&&currentFileMap.empty())send("KEEP_ALIVE");});  
    connect(this,&TransmissionEngine::signal_reliableMessage_received,this,&TransmissionEngine::on_reliableMessage_received,Qt::QueuedConnection);
    connect(this,&TransmissionEngine::signal_resend_finished,this,[this]{
        if(!queue_fileSendingTask.isEmpty()){
            auto task_ptr = queue_fileSendingTask.front();
            queue_fileSendingTask.pop_front();
            QMetaObject::invokeMethod(this,[this,task_ptr=std::move(task_ptr)]{SPTP_sendTo(clients.indexOf(task_ptr->dst),task_ptr->msg);},Qt::QueuedConnection);
        }
        else{emit SPTP_sendFinished();}
    },Qt::QueuedConnection);
    
//    timer_keepAlive.start(????);///废弃
}

TransmissionEngine::~TransmissionEngine(){
    
}

void TransmissionEngine::send(QByteArray msg, bool e, int d){
    //自动补全信息
    QJsonDocument jd = QJsonDocument::fromJson(msg);
    if(jd.isObject()){
        QJsonObject json = jd.object();
        if(!json.contains("user")){
            json.insert("user",user_name);
        }
        if(!json.contains("ip")){
            json.insert("ip",public_ip.ip);
        }
        if(!json.contains("port")){
            json.insert("port",public_ip.port);
        }
        msg = QJsonDocument(json).toJson();
    }
    
    //加密并发送
//    ndb<<"发送消息"<<msg;
    int ret;
    auto cli = clients;
    auto encodedMsg= e?encode(msg):msg;
    if(d==-1)cli.removeAll(public_ip);//不给自己发送
    if(d==-1||d==-2)foreach(auto i,cli)ret=m_communication->send(i,encodedMsg);
    else ret=m_communication->send(clients[d],encodedMsg);
    if(ret!=encodedMsg.size()){
        ncritical<<"发送消息失败：返回"<<ret<<"实际大小"<<encodedMsg.size();
    }
}


//转而使用Utils::encode/decode(msg)
QByteArray TransmissionEngine::encode(const QByteArray &msg){
//#ifndef DEBUG_NO_ENCRYPTION
//    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
//    QByteArray key = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
//    QByteArray iv = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Md5);
//    QByteArray encode = encription.encode(msg,key,iv)/*.toBase64()*/;
//    return /*qCompress(*/encode/*,9)*/;
//#else   
//    return msg;
//#endif
    return Utils::encode(msg,pwd);
}


QByteArray TransmissionEngine::decode(const QByteArray &msg){
//#ifndef DEBUG_NO_ENCRYPTION
//    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
//    QByteArray key = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
//    QByteArray iv = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Md5);
//    QByteArray decoded = encription.removePadding(encription.decode(/*QByteArray::fromBase64(msg)*//*qUncompress(*/msg/*)*/,key,iv));
//    return decoded;
//#else
//    return msg;
//#endif
    return Utils::decode(msg,pwd);
}


//SPTP协议相关
void TransmissionEngine::SPTP_sendTo(int n, QByteArray data){
    if(!chunks.isEmpty() || !currentFileMap.isEmpty()){
        nwarning<<"非空闲，已有的同步任务取消";
        return;
    }
    //发送前准备
//    foreach(auto s , schedule_list){//禁用日程
//        s->setEnabled(false);
//    }
#define process_events_without_useript QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents)
    currentSendDst = n;
    //进行客户端连通性测试
    test_if_connected_set.clear();
    emit messageChanged(tr("正在测试连通性"));
    send("{\n    \"opt\":\"test_if_connected\"\n}",1,currentSendDst);
    QEventLoop loop_test;
    connect(this,&TransmissionEngine::signal_test_if_connected_finished,&loop_test,&QEventLoop::quit);
    QTimer::singleShot(5000,&loop_test,&QEventLoop::quit);
    bool is_test_success = false;
    connect(this,&TransmissionEngine::signal_test_if_connected_finished,this,[&,this]{is_test_success=true;(void)this;});
    loop_test.exec();
    ninfo<<"var:is_test_success="<<is_test_success;
    if(!is_test_success){
        QSet<ipport> missing = QSet<ipport>(clients.begin(),clients.end()) - test_if_connected_set;
        QStringList missingStr;
        foreach(auto i , missing){missingStr<<i;}
        int button = QMessageBox::No;
        QString msgTxt=tr("以下客户端没有响应连通性测试:\n\n%1\n\n这可能是由于客户端掉线造成的，也可能是网络波动。\n如果您想继续发送，请点击“是”。如果您想取消发送，请点击“否”。如果您想重试，请点击“重试”。");
        QString titleTxt=tr("连通性警告");
        QMetaObject::invokeMethod(qApp,[&button,missingStr,msgTxt,titleTxt]{button = QMessageBox::warning(qApp->activeWindow(),titleTxt,QString(msgTxt).arg(missingStr.join("\n")),QMessageBox::Yes|QMessageBox::No|QMessageBox::Retry);},QThread::currentThread()!=qApp->thread()?Qt::BlockingQueuedConnection:Qt::DirectConnection);
        switch (button) {
        case QMessageBox::Yes:
            break;
        case QMessageBox::Retry:
            SPTP_sendTo(n,data);//自动到下面的return
        case QMessageBox::No:
            return;
        }
    }
    
    
    //生成文件表
    is_uploading = true;
    emit messageChanged(tr("正在加载文件……"));
    process_events_without_useript;
    QByteArray &fileList = data;
    ninfo<<"fileList.size = "<<fileList.size();
    chunks.clear();
    
    //文件分片
//    const int SPC = /*3 * 1024;//3kb*/ /*1400;//基于MTU1500的值*/ /*1024;//小值*/ 8 * 1024 -1;//7kb
//    const int SPC = 1382;//基于MTU1500的值
    const int SPC = 1420;
    emit messageChanged(tr("正在分片文件……"));
    process_events_without_useript;
//    QTextStream stm(&fileList);
//    for(;!stm.atEnd();){
//        chunks.append(stm.read(SPC));
//    }
    for(int i=0;i<fileList.size();i+=SPC){
        chunks.append(fileList.mid(i,SPC));
    }
    
    
//    QJsonObject json;
    QList<QByteArray> send_buf;//发送缓冲区
//    json.insert("user",this->user_name);
//    json.insert("total",chunks.size()-1);
//    json.insert("filebody","");
//    json.insert("no",-1);
    emit messageChanged(tr("正在加密文件……"));
    process_events_without_useript;
    header_filebody_p header;
    header.check_type = qToBigEndian((qint32)mt_filebody);
    header.total = qToBigEndian(chunks.size()-1);
    for(int i=0;i<chunks.size();i++){//生成发送内容
//        json["filebody"] = chunks[i];
//        json["no"] = i;
//        send_buf.push_back(encode(QJsonDocument(json).toJson()));
//        if(i%3000==0){
//            label_status->setText("正在加密文件…… 加密时间较长，请耐心等待");
//        }
        header.no = qToBigEndian(i);
        QByteArray msg;msg.reserve(sizeof(header) + chunks[i].size());
        msg.append(QByteArray(reinterpret_cast<const char *>(&header),sizeof(header)));msg.append(chunks[i]);
        send_buf.push_back(encode(msg));
        if(i%8==0)emit messageChanged(QString(tr("正在加密文件…… %1/%2")).arg(i).arg(chunks.size()));
        if(i%8==0)process_events_without_useript;
    }
    
    
    //分块发送
//    int loop_count = 0;//循环次数
    /*const int DELAY_LOOP = 1;
    for(int i=0;i<chunks.size();i++){
        json["filebody"] = chunks[i];
        json["no"] = i;
        QEventLoop loop;
        if(i % DELAY_LOOP == 0)QTimer::singleShot(1,Qt::PreciseTimer,&loop,&QEventLoop::quit);
        send(QJsonDocument(json).toJson());
        if(i % DELAY_LOOP == 0)loop.exec(QEventLoop::ExcludeUserInputEvents);//循环两次停止一次
        else for(int j=0;j<100;j++);//挨时间
    }//*/
    
    //发送文件
    QElapsedTimer clock;
    clock.start();
#ifdef Q_OS_WIN
    timeBeginPeriod(1);
#endif
    QElapsedTimer clock_total;
    clock_total.start();
    QString str_debug;//调试字符串
    
    const int DELAY_LOOP = 7;
    const int PROCESS_LOOP = 1;
    const int SEND_MAX_REQACKLOOP = 120;
    const int SEND_MIN_REQACKLOOP = 5;
    int display_lost = 0;//显示的丢包率
//    double value_plus_lost = 0;//对低延迟状态的丢包阈值增加
    bool fast_plus = false;//快速恢复
    double average_loss = 0.1;//平均丢包率
    double average_good_loss = 0.01;//平均良好丢包率
    double average_bad_loss = 0.3;//平均拥塞丢包率
    sendState state = ss_normal_better;//发送当前状态
    bool is_exploring = false;
    double alpha = 0.02;
    int stable_delay = SEND_MIN_DELAY;
    int start_reqAck = 0;
    int min_rtt = INT_MAX;
    int current_rtt = 0;
    QSet<int> skip;
    bool is_first_explore = true;
    int maxdelaycnt = 0;//连续触发最大延迟的次数，用于重启
//    QPair<int,int>loadedRange = {0,-1};
    
    /*line_delay->clear();
    line_speed->clear();
    line_ackloop->clear();
    axis_y->setRange(0,SEND_MAX_DELAY);
    axis_y_r->setRange(0,1000);
    ui->tabWidget->setCurrentIndex(1);//切换到发送速度页
    ui->statusBar->setMinimumHeight(55);*/
    str_debug = QString("%1%2%3%4%5%6%7%8%9\n").arg("Time",13).arg("State",10).arg("AvgLoss",13).arg("AvgGLoss",13).arg("AvgBLoss",13).arg("Delay",10).arg("AckLoop",10).arg("LostRate",10).arg("Cong",10);
    
    for(int i=0;i<send_buf.size();i++){
//        QEventLoop loop;
//        QTimer::singleShot(1,Qt::PreciseTimer,&loop,&QEventLoop::quit);
        if(!skip.contains(i))send(send_buf[i],0,currentSendDst);//不加密是因为已经加密过了
//        loop.exec(QEventLoop::ExcludeUserInputEvents);
        
        if(i%7==0 || send_current_delay>50){
            int pc=7;
            if(send_current_delay>50)pc=1;
            double speed = (pc * SPC/(clock.elapsed()/1000.0));
            double dsp;//显示速度
            QString dw="B";
            if(speed>1024){
                dw="KB";dsp=speed/1024;
            }
            if(speed>(1024*1024)){
                dw="MB";dsp=speed/(1024*1024);
            }
            if(send_current_delay>20||i%4==0){
                emit messageChanged(QString(tr("发送文件中:包%1/%2 显示%3毫秒 速度%4%5/s \n本轮丢包%6% 延迟%7ms 验证轮%8个 累计%9 稳定%10\nGAB%12:%11:%13 状态%14 探索%15")).arg(i).arg(send_buf.size()-1).arg(clock.elapsed()).arg(dsp,0,'f',4).arg(dw).arg(display_lost).arg(send_current_delay).arg(send_req_ack_loop).arg(send_lost_loop_count).arg(stable_delay).arg(average_loss,0,'f',3).arg(average_good_loss,0,'f',3).arg(average_bad_loss,0,'f',3).arg(state).arg(is_exploring));
//                line_delay->append(i,send_current_delay);
//                line_ackloop->append(i,send_req_ack_loop);
//                line_speed->append(i,speed/1024.);//KB/s
//                axis_x->setRange(0,i+5);
//                if(speed > axis_y_r->max()) axis_y_r->setRange(0,speed+15);
//                chart_send->update();
//                ui->widget_info_chart1->update();
//                ui->label_info_time->setText(QString("共记录%1s").arg(clock_total.elapsed()/1000.));
            }
            clock.restart();
        }
        
        if(i%PROCESS_LOOP==0){
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents,1);
        }
//        if(elapsed_lase_ack.elapsed() > send_current_delay + 80){
//            send_current_delay += 30;//强力降速
//            if(send_current_delay > SEND_MAX_DELAY){
//                send_current_delay = SEND_MAX_DELAY;
//            }
//            ndb<<"强力降速"<<send_current_delay;
//        }
        
        
//        if(i%2==0)send_current_delay+=1;
        if(!skip.contains(i)){
            if((i&DELAY_LOOP)==0)/*QThread::msleep(send_current_delay);*/multiDelay(send_current_delay);
            else if(i%send_req_ack_loop == 0)/*QThread::msleep(2);*/multiDelay(2);//为了防止拥塞
        }

        
        //进行ACK请求
        if(i%send_req_ack_loop == 0){
            send_lost_count.clear();
            ninfo<<"于包"<<i;
            send_req_ack_uuid = QUuid::createUuid();
            bool flag = false;
            QEventLoop loop;
            QTimer timer;
            connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
            connect(this,&TransmissionEngine::signal_reqAck_finished,&loop,&QEventLoop::quit);
            connect(this,&TransmissionEngine::signal_reqAck_finished,this,[&,this]{flag=true;(void)this;});
            timer.start(650);
            send_current_reqAck = i;
            ninfo<<"请求接收统计：从"<<start_reqAck<<"到"<<i;
            QElapsedTimer clock1;clock1.start();
            for(int ii=0;ii<3;ii++)send(QString("{\n    \"req_ack\":%1,\"start_req_ack\":%2,\n\"uuid\":\"%3\"\n}").arg(i).arg(start_reqAck).arg(send_req_ack_uuid.toString()).toUtf8(),1,currentSendDst);
//            send(QString("{\n    \"req_ack\":%1,\"start_req_ack\":%2,\n\"uuid\":\"%3\"\n}").arg(i).arg(start_reqAck).arg(send_req_ack_uuid.toString()).toUtf8());//防止丢包
            int overdrop_cnt = 0;//透支包计数
            QElapsedTimer clock_overdrop;clock_overdrop.start();
//            for(int j=i+1;j<qMin(i+send_req_ack_loop+5,send_buf.size());j++){//利用网络延迟等待的时间预加载缓存数据提高性能
//                send_buf[j].load();
//            }
//            loadedRange.second= qMin(i + send_req_ack_loop + 5, send_buf.size())-1;
//            for(int j=loadedRange.first;j<=qMin(i,loadedRange.second);j++){//利用网络延迟等待的时间卸载无用数据减少内存占用
//                send_buf[j].unload();
//            }
//            loadedRange.first=qMin(i,loadedRange.second)+1;
            if(send_lost_loop_count > 2 && send_current_delay < 20){//网络条件好的时候 开始透支
                for(;;){
                    if(flag==true)break;
                    if(clock_overdrop.elapsed()>min_rtt)break;
                    if(overdrop_cnt >= qMin(send_req_ack_loop/2,5)) break;
                    overdrop_cnt++;
                    send(send_buf[i+overdrop_cnt],0,currentSendDst);
                    skip.insert(i+overdrop_cnt);
                    multiDelay(send_current_delay);
                }
                ninfo<<"透支了从"<<i+1<<"开始的"<<overdrop_cnt<<"个包";
            }
            if(!flag)loop.exec(QEventLoop::ExcludeUserInputEvents);
            if(clock.elapsed()>10)ndb<<"请求所用时间>10ms:"<<clock.elapsed()<<"ms";
            current_rtt = clock.elapsed();
            if(current_rtt < min_rtt)min_rtt = current_rtt;
            loop.deleteLater();
            //开始计算条件
            bool retry_flag = true;
            bool congestion = false;
            int last_lost_pack_index = -1;
            int fat_cnt = 0;;
            for(auto it=send_current_fastresend_map.rbegin();it!=send_current_fastresend_map.rend();it++,fat_cnt++){//检测是否因为缓冲区膨胀导致的丢包
                if(last_lost_pack_index==-1)last_lost_pack_index=*it;
                else if(abs(last_lost_pack_index-*it)>2){ 
                    if(fat_cnt<3)retry_flag=false;
                    break;
                }
            }
            if(send_current_fastresend_map.size()<3)retry_flag=false;
            QSet<int> old_fastresend = send_current_fastresend_map;
            if(retry_flag && !send_current_fastresend_map.empty()){//重试
                multiDelay(2);
                send_lost_count.clear();
                send_current_fastresend_map.clear();
                ninfo<<"于包"<<i<<"开始重新请求统计信息";
                send_req_ack_uuid = QUuid::createUuid();
                bool flag = false;
                QEventLoop loop;
                QTimer timer;
                connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
                connect(this,&TransmissionEngine::signal_reqAck_finished,&loop,&QEventLoop::quit);
                connect(this,&TransmissionEngine::signal_reqAck_finished,this,[&,this]{flag=true;(void)this;});
                timer.start(600);
                send_current_reqAck = i;
                send(QString("{\n    \"req_ack\":%1,\"start_req_ack\":%2,\n\"uuid\":\"%3\"\n}").arg(i).arg(start_reqAck).arg(send_req_ack_uuid.toString()).toUtf8(),1,currentSendDst);
                loop.exec(QEventLoop::ExcludeUserInputEvents);
                loop.deleteLater();
                if(old_fastresend.size() - send_current_fastresend_map.size() < 2)congestion=true;//如果丢包集中在尾部且重新请求ACK无用就认为拥塞
            }
            float lost = 0;//平均丢包率
            for(int i : send_lost_count){
                lost += i;
            }
            ninfo<<"丢包数量"<<lost;
            lost /= send_lost_count.size();
            lost = lost / send_req_ack_loop;
            ninfo<<"丢包率"<<lost;
            ninfo<<"拥塞状态"<<congestion;
            display_lost = lost * 100;//百分数
            if(flag && lost <= 1.001){
                if(1)if(lost!=0)ndb<<"此轮有丢包";
                //快重传
                if(1)/*调试禁用重传*/if(!send_current_fastresend_map.empty()){
                    ninfo<<"快速重传"<<send_current_fastresend_map;
                    foreach(int i , send_current_fastresend_map){
                        send(send_buf[i],0,currentSendDst);
                        multiDelay(send_current_delay);
                        emit messageChanged(QString("正在快速重传%1包").arg(i));
                        if(i%2==0)process_events_without_useript;
                    }
                    multiDelay(1);
                    send_current_fastresend_map.clear();
                }
                
                //拥塞控制算法
                //状态更新
                if(lost <= average_loss){
                    if(lost <= average_good_loss) state=ss_excellent;
                    else{
                        if(abs(lost-average_good_loss) <= abs(lost-average_loss)) state=ss_good;
                        else state=ss_normal_better;
                    }
                }
                else{
                    if(lost > average_bad_loss*1.05) state=ss_worst;
                    else{
                        if(abs(lost-average_bad_loss) < abs(lost-average_loss)) state=ss_bad;
                        else state=ss_normal_worse;
                    }
                }
                //计算平均
                average_loss = alpha * lost + (1-alpha) * average_loss;
                switch (state) {
                case ss_excellent:
                case ss_good:
                    average_good_loss = alpha * lost + (1-alpha) * average_good_loss;
                    break;
                case ss_worst:
                case ss_bad:
                    average_bad_loss = alpha * lost + (1-alpha) * average_bad_loss;
                    break;
                default:
                    break;
                }
                //RTT控制
//                if(current_rtt > min_rtt + 35){
////                    congestion = true;
//                    if(stable_delay < send_current_delay - 5){
//                        stable_delay = send_current_delay - 4;
//                    }
//                    min_rtt ++;
//                }
                //根据状态控制加减速
                switch (state) {
                case ss_excellent://良好状态进行加速
                    if(send_lost_loop_count<0) send_lost_loop_count=0;
                    else send_lost_loop_count++;
                    //进行加减速
                    if(is_exploring){//探索中 线性加速
                        send_current_delay -= 5;
                        send_req_ack_loop -= 0;
                    }
                    else{
                        if(!congestion){
                            send_current_delay *= 0.8;
                            send_req_ack_loop *= 1.2;
                        }
                        else{
//                            multiDelay(1);
                            average_loss -= 0.005;//既然拥塞了都能进ss_excellent就说明average_loss被污染
                            average_good_loss -= 0.005;
                        }
                    }
                    //探索控制
                    if(send_current_delay < stable_delay && send_current_delay>SEND_MIN_DELAY){//需要探索
                        if(send_lost_loop_count > 2){//连续稳定允许探索
                            is_exploring = true;
                            if(send_req_ack_loop > 50){
                                send_req_ack_loop = 50;//不允许过大
                            }
                        }
                        else{
                            send_current_delay = stable_delay;
                        }
                    }
                    if(is_exploring && send_lost_loop_count > 5){
                        stable_delay = send_current_delay + 1;
                        is_exploring = false;
                        send_lost_loop_count = 0;
                    }
                    break;
                case ss_good://较好
                    if(send_lost_loop_count<0) send_lost_loop_count=0;
                    else send_lost_loop_count++;
                    //进行加减速
                    if(is_exploring){//探索中 线性加速
                        send_current_delay -= 1;
                    }
                    else{
                        if(!congestion){
                            send_current_delay -= 5;
                            send_req_ack_loop += 5;
                        }
                        else{
                            
                        }
                    }
                    //探索控制
                    if(send_current_delay < stable_delay && send_current_delay>SEND_MIN_DELAY){//需要探索
                        if(send_lost_loop_count>4){
                            is_exploring = true;
                            if(send_req_ack_loop > 50){
                                send_req_ack_loop = 50;//不允许过大
                            }
                        }
                        else{
                            send_current_delay = stable_delay;
                        }
                    }
                    if(is_exploring && send_lost_loop_count > 4){
                        stable_delay = send_current_delay + 5;
                        is_exploring = false;
                        send_lost_loop_count = 0;
                    }
                    break;
                case ss_normal_better:
//                    if(send_lost_loop_count<0) send_lost_loop_count=0;
//                    else send_lost_loop_count++;
                    //进行加减速
                    if(is_exploring){//停止探索
                        is_exploring = false;
                        stable_delay = send_current_delay + 1;
                        send_lost_loop_count = 0;
                    }
                    else{
//                        if(send_lost_loop_count<0)send_current_delay -= 1;
                        if(congestion){
                            multiDelay(1);
                        }
                    }
                    //探索控制
                    if(send_current_delay < stable_delay){//不允许探索
                    }
                    break;
                case ss_normal_worse:
                    if(send_lost_loop_count>0) send_lost_loop_count=0;
                    else send_lost_loop_count--;
                    //进行加减速
                    if(is_exploring){//停止探索，单轮延迟
                        is_exploring = false;
                        if(send_current_delay+5<stable_delay)stable_delay=send_current_delay+5;
                        multiDelay(5);
                    }
                    else{
                        send_current_delay += 1;
//                        if(send_lost_loop_count < -5){
//                            if(stable_delay > send_current_delay){//更新stable_delay
//                                stable_delay = send_current_delay - 5;
//                                send_lost_loop_count = 0;
//                            }
//                        }
                        if(congestion){
                            multiDelay(3);
                        }
                    }
                    if(is_first_explore){
                        is_first_explore=false;
                        stable_delay=send_current_delay;
                    }
                    break;
                case ss_bad:
                    if(send_lost_loop_count>0) send_lost_loop_count=0;
                    else send_lost_loop_count--;
                    //进行加减速
                    if(is_exploring){//停止探索，单轮延迟，回归初始延迟
                        is_exploring = false;
                        multiDelay(5);
                        send_current_delay = stable_delay;
                    }
                    else{
                        send_current_delay += 3;
                        send_req_ack_loop -= 5;
                        if(send_lost_loop_count < -2){
                            if(stable_delay > send_current_delay){//更新stable_delay
                                stable_delay = send_current_delay - 5;
                            }
                        }
                        if(congestion){
                            send_current_delay+=1;
                            multiDelay(5);
                        }
                    }
                    if(is_first_explore){
                        is_first_explore=false;
                        stable_delay=send_current_delay+3;
                    }
                    break;
                case ss_worst:
                    if(send_lost_loop_count>0) send_lost_loop_count=0;
                    else send_lost_loop_count--;
                    //进行加减速
                    if(is_exploring){//停止探索，单轮延迟，回归初始延迟-8
                        is_exploring = false;
                        multiDelay(8);
                        send_current_delay = stable_delay-8;
                    }
                    else{
//                        int old_send_current_delay=send_current_delay;
//                        send_current_delay *= 1.2;
//                        if(abs(old_send_current_delay-send_current_delay)<10)send_current_delay+=10;
                        send_current_delay += 10;
                        send_req_ack_loop -= 10;
                        if(send_lost_loop_count < -2){
                            if(stable_delay > send_current_delay){//更新stable_delay
                                stable_delay = send_current_delay - 10;
                            }   
                            send_current_delay += 15;
                        }
                        if(congestion)multiDelay(5);
                    }
                    if(is_first_explore){
                        is_first_explore=false;
                        stable_delay=send_current_delay+8;
                    }
                    break;
                }
                //限制范围
                send_current_delay = qBound(SEND_MIN_DELAY,send_current_delay,SEND_MAX_DELAY);
                send_req_ack_loop = qBound(SEND_MIN_REQACKLOOP,send_req_ack_loop,SEND_MAX_REQACKLOOP);
                //输出信息
                ninfo<<"控制统计信息："<<QString("延迟=%1,验证轮=%2,状态机状态=%3,平均丢包=%4,正常网络状态下的平均丢包=%5,拥塞状态下的平均丢包=%6,连续状态计数=%7,探索状态=%8,拥塞=%9").arg(send_current_delay).arg(send_req_ack_loop).arg(state).arg(average_loss).arg(average_good_loss).arg(average_bad_loss).arg(send_lost_loop_count).arg(is_exploring?"true":"false").arg(congestion?"true":"false");
            }
            else{
                send_current_delay += 3;//如果连控制包都丢了，就减速
                if(send_lost_loop_count>0)send_lost_loop_count=0;
                else send_lost_loop_count --;
                average_loss += 0.03;
                multiDelay(10);
                send_req_ack_loop -= 5;
                if(send_req_ack_loop < 3)send_req_ack_loop=3;//不能太小
                if(send_current_delay > SEND_MAX_DELAY) send_current_delay=SEND_MAX_DELAY;
                fast_plus = true;//控制包丢失启用快速恢复
                int current_max_reqackloop = (send_current_delay>30?2000.:3500.)/send_current_delay;//要求至少2.5s更新一次
                if(send_req_ack_loop > current_max_reqackloop){
                    send_req_ack_loop=current_max_reqackloop;
                    ninfo<<"传输速率控制:限制reqackloop"<<send_req_ack_loop;
                }
                ninfo<<"控制包丢失";
            }
            ninfo<<"var:send_current_delay ="<<send_current_delay;
//            str_debug.append(QString("%4:\t%1 \t\t%2 \t%3\n").arg(lost).arg(send_current_delay).arg(send_req_ack_loop).arg(i));
//            str_debug.append(QString("%1%2%3%4%5%6%7%8%9\n").arg(QTime::currentTime().toString("hhmmsszzz"),13).arg(state,10).arg(average_loss,13,'f',4).arg(average_good_loss,13,'f',4).arg(average_bad_loss,13,'f',4).arg(send_current_delay,10).arg(send_req_ack_loop,10).arg(lost,10).arg(congestion?"True":"False",10));
            if(flag)start_reqAck = i+1;//在这里更新因为为了预防控制包丢失导致无法快速重传和漏轮的情况
//            i += overdrop_cnt - 1;//已经透支过的包不再发送
            //重启判断
            if(send_current_delay==SEND_MAX_DELAY){
                maxdelaycnt++;
                if(maxdelaycnt==3){//重启
                    emit messageChanged("发送遇到错误，正在重启，请留意对方SyncTunnel左下角文本");
                    for(int i=0;i<10;i++){
                        multiDelay(200);
                        process_events_without_useript;
                    }
                    //再次打洞
                    for(int i=0;i<50;i++){
                        send("{\n   \"hole\":2\n}",1,currentSendDst);
                        multiDelay(1);
                    }
                    for(int i=0;i<10;i++){
                        multiDelay(150);
                        process_events_without_useript;
                    }
                    //重置变量
                    send_current_delay = SEND_MAX_DELAY-50;
                    send_req_ack_loop = 5;
                    is_first_explore = true;
                    average_loss = 0.1;//平均丢包率
                    average_good_loss = 0.01;//平均良好丢包率
                    average_bad_loss = 0.3;//平均拥塞丢包率
                    state = ss_normal_better;//发送当前状态
                    is_exploring = false;
                }
            }
            else{
                maxdelaycnt=0;
            }
        }
    }
    
    emit messageChanged(tr("发送文件完毕"));
//    if(1)ui->textEdit_debug1->setText(str_debug);
//    ui->statusBar->setMinimumHeight(0);
    timer_is_uploading.start(10000);
    int spd = fileList.size() / (clock_total.elapsed()/1000.);
//    ui->label_info_time->setText(QString("已记录%1s(%5),%2(%3,%4)").arg(clock_total.elapsed()/1000.,0,'f',2).arg(spd).arg(spd/1024.).arg(spd/1024./1024).arg(fileList.size()));
    ninfo<<QString("文件发送操作完毕。概要信息:%1s(%5),%2(%3,%4)").arg(clock_total.elapsed()/1000.,0,'f',2).arg(spd).arg(spd/1024.).arg(spd/1024./1024).arg(fileList.size());
//    ui->textBrowser_debug1->append(str_debug);
#ifdef Q_OS_WIN
    timeEndPeriod(1);
#endif
    
    //要求重传
    ndb<<"流程A";
    multiDelay(500);
    ndb<<"流程B";
    sendReliableMessage(currentSendDst,"PLEASE_REQ_RESEND");
    ndb<<"流程C";
}

void TransmissionEngine::SPTP_send(QByteArray msg, QList<device> dst){
    if(!chunks.isEmpty() || !currentFileMap.isEmpty()){
        nwarning<<"非空闲，已有的同步任务取消";
        return;
    }
    if(dst.empty()){
        QMetaObject::invokeMethod(qApp,[]{QMessageBox::warning(qApp->activeWindow(),tr("同步文件"),tr("请选择目标！"));});
        return;
    }
    dst.removeAll(public_ip);//文件不发给自己
    //开始规划
    auto plan = planAutoSend(dst);
    auto senders = dst;
    senders.append(public_ip);
    //提取每个人任务
    QList<QString> tasks;//索引和senders一一对应
    for(int i=0;i<senders.size();i++){
        QStringList task;
        foreach(auto j , plan){
            foreach(auto k , j){
                if(k.first==senders[i]){
                    task.append(k.second);
                }
            }
        }
        tasks.append(task.join(';'));
    }
    //通知任务
    QStringList self_tasks;
    for(int i=0;i<senders.size();i++){
        if(!(senders[i]==public_ip)){
            if(!tasks[i].isEmpty()){
                sendReliableMessage(clients.indexOf(senders[i]),"SEND_TASK"+tasks[i]);
            }
        }
        else self_tasks = tasks[i].split(';');
    }
    
    //发送
    for(auto t : self_tasks){
        //查找
        int index = -1;
        for(int i=0;i<clients.size();i++){
            if(clients[i].operator QString const() == t){
                index=i;
                break;
            }
        }
//        QMetaObject::invokeMethod(this,[=]{sendFileTo(index);},Qt::QueuedConnection);//QueuedConnection在事件循环运行并且顺序按照invoke的顺序运行。sendFileTo不能在除了事件循环以外的其他地方运行
        /*SPTP_sendTo(index,msg);
        emit messageChanged("正在等待状态重置……");
        QEventLoop loop;
        QTimer::singleShot(21000,&loop,&QEventLoop::quit);
        connect(this,&TransmissionEngine::signal_resend_finished,&loop,&QEventLoop::quit);
        loop.exec();*/
//        ui->textEdit_debug1->append(QString("发送文件到%1").arg(index));
        
        file_sending_task task = {clients[index],msg};
        auto task_ptr = std::make_shared<const file_sending_task>(clients[index], msg);
        queue_fileSendingTask.append((task_ptr));
    }
    
    emit signal_resend_finished();
}

bool TransmissionEngine::sendReliableMessage(int dst, QString msg){
    if(dst<0||dst>=clients.size()){
        ncritical<<"Invaild dst:"<<dst;
        return false;
    }
    /*  if(!currentReliableUuid.isEmpty()){//等待可靠消息接受完成
        QEventLoop loop;
        QTimer tm;
        tm.setSingleShot(false);
        connect(&tm,&QTimer::timeout,this,[this,&loop]{if(currentReliableUuid.isEmpty()){loop.quit();}});
        loop.exec();
    }*/
    //生成唯一ID
    QUuid uuid = QUuid::createUuid();
    //发送消息-1
    /*for(int i=0;i<3;i++){
        QJsonObject json1;
        json1.insert("reliable_msg","DATA");
        json1.insert("uuid",uuid.toString(QUuid::Id128));
        json1.insert("value",msg);
        send(QJsonDocument(json1).toJson(),1,dst);
        //等待回复
        QEventLoop loop;
//        bool flag1=false;
        bool flag_is_succeed = false;
        QTimer::singleShot(3000,&loop,&QEventLoop::quit);
        auto conn = connect(this,
                &TransmissionEngine::communicationReadyRead,this,[uuid,&loop,&flag_is_succeed](QByteArray lastMessage){
            if(lastMessage.contains("R_ACK_DATA")&&
                    lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())){
                loop.quit();
                flag_is_succeed=true;}
        });
//        connect(m_communication,&Communication::readyRead,this,[&]{flag1=true;});
        ndb<<"可靠消息:发送DATA";
        loop.exec();
        disconnect(conn);
//        if(flag1&&lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())&&lastMessage.contains("R_ACK_DATA"))break;
        if(flag_is_succeed){
            ndb<<"可靠消息：成功接收R_ACK_DATA";
            break;
        }
        if(i==2){
            ndb<<"可靠消息：等待R_ACK_DATA失败";
            return false;
        }
    }
    //发送允许释放
    multiDelay(10);
    for(int i=0;i<3;i++){
        QJsonObject json1;
        json1.insert("reliable_msg","ALO_RLS");
        json1.insert("uuid",uuid.toString(QUuid::Id128));
        send(QJsonDocument(json1).toJson(),1,dst);
        //等待回复
        QEventLoop loop;
        bool flag1=false;
        bool flag_is_succeed = false;
        QTimer::singleShot(3000,&loop,&QEventLoop::quit);
//        connect(this,&TransmissionEngine::communicationReadyRead,this,[uuid,&loop,&flag_is_succeed,this](QByteArray lastMessage){if(lastMessage.contains("R_ACK_RLS")&&lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())){loop.quit();flag_is_succeed=true;}});//SIGSEGV
        auto conn = connect(this,
                &TransmissionEngine::communicationReadyRead,this,[uuid,&loop,&flag_is_succeed](QByteArray lastMessage){
            if(lastMessage.contains("R_ACK_RLS")&&
                    lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())){
                loop.quit();
                flag_is_succeed=true;}
        });
        connect(m_communication,&Communication::readyRead,this,[&]{flag1=true;});
        ndb<<"可靠消息:发送ALO_RLS";
        loop.exec();
        disconnect(conn);
//        if(flag1&&lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())&&lastMessage.contains("R_ACK_RLS"))break;
        if(flag_is_succeed){
            ndb<<"可靠消息：成功接收R_ACK_RLS";
            break;
        }
        if(i==2){
            ndb<<"可靠消息：等待R_ACK_RLS失败";
            return false;
        }
    }
    return true;*/
    
    //流程：
    /* 发送方           接收方
     * DATA ->
     *               <-ACK
     * DONE ->
     *               <-COMP               
     */
    
    //1.发送DATA，等待ACK
    for(int i=0;i<6;i++){
        QJsonObject json1;
        json1.insert("reliable_msg","DATA");
        json1.insert("uuid",uuid.toString(QUuid::Id128));
        json1.insert("value",msg);
        send(QJsonDocument(json1).toJson(),1,dst);
        //等待回复
        QEventLoop loop;
    //            bool flag1=false;
        bool flag_is_succeed = false;
        QTimer::singleShot(1000,&loop,&QEventLoop::quit);
        auto conn = connect(this,
                            &TransmissionEngine::communicationReadyRead,this,[uuid,&loop,&flag_is_succeed](QByteArray lastMessage){
            if(lastMessage.contains("R_ACK_DATA")&&
                    lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())){
                loop.quit();
                flag_is_succeed=true;}
        });
        ndb<<"可靠消息:发送DATA";
        loop.exec();
        disconnect(conn);
        if(flag_is_succeed){
            ndb<<"可靠消息：成功接收R_ACK_DATA";
            break;
        }
        if(i==5){
            ndb<<"可靠消息：等待R_ACK_DATA失败";
            return false;
        }
    } 
    //2.发送DONE，等待COMP
    for(int i=0;i<6;i++){
        QJsonObject json1;
        json1.insert("reliable_msg","DONE");
        json1.insert("uuid",uuid.toString(QUuid::Id128));
        send(QJsonDocument(json1).toJson(),1,dst);
        //等待回复
        QEventLoop loop;
    //            bool flag1=false;
        bool flag_is_succeed = false;
        QTimer::singleShot(1000,&loop,&QEventLoop::quit);
        auto conn = connect(this,
                            &TransmissionEngine::communicationReadyRead,this,[uuid,&loop,&flag_is_succeed](QByteArray lastMessage){
            if(lastMessage.contains("R_COMP")&&
                    lastMessage.contains(uuid.toString(QUuid::Id128).toUtf8())){
                loop.quit();
                flag_is_succeed=true;}
        });
        ndb<<"可靠消息:发送DONE";
        loop.exec();
        disconnect(conn);
        if(flag_is_succeed){
            ndb<<"可靠消息：成功接收R_COMP";
            break;
        }
        if(i==5){
            ndb<<"可靠消息：等待R_COMP失败";
            return false;
        }
    } 
    return true;
}

QByteArray TransmissionEngine::SPTP_sendCommon(QByteArray msg, int d){
    header_common_p hCommon;
    hCommon.check_type = mt_bh;
    QHostAddress ip(public_ip.ip);
    if(ip.protocol() == QAbstractSocket::IPv4Protocol){
        hCommon.ip_type=4;
        hCommon.src_public_ip_1=qToBigEndian(ip.toIPv4Address());
        hCommon.src_public_ip_2=0;
    }
    else if(ip.protocol() == QAbstractSocket::IPv6Protocol){
        hCommon.ip_type=6;
        Q_IPV6ADDR addr = ip.toIPv6Address();
        memcpy(&hCommon.src_public_ip_1, &addr[0], 8);
        memcpy(&hCommon.src_public_ip_2, &addr[8], 8);
    }
    else{
        nwarning<<"Unsupported protocol:"<<QMetaEnum::fromType<QAbstractSocket::NetworkLayerProtocol>().valueToKey(ip.protocol());
        return "";
    }
    hCommon.src_port=public_ip.port;
    //端序转换
//    hCommon.ip_type = qToBigEndian(hCommon.ip_type);
    hCommon.src_port = qToBigEndian(hCommon.src_port);
    hCommon.check_type = qToBigEndian(hCommon.check_type);
    hCommon.src_public_ip_1 = qToBigEndian(hCommon.src_public_ip_1);
    hCommon.src_public_ip_2 = qToBigEndian(hCommon.src_public_ip_2);
    //发送
    QByteArray fullmsg;
    fullmsg.resize(sizeof(hCommon));
    memcpy(fullmsg.data(),&hCommon,sizeof(hCommon));
    fullmsg.append(msg);
    if(d!=-3)send(fullmsg,1,d);
    return fullmsg;
}

QByteArray TransmissionEngine::SPTP_sendCtrl(QByteArray ctrl, QVariant v, int d){
    msg_ctrl_p msg;
    memset(&msg,0,sizeof(msg));
    if(ctrl.size()>=20){
        nwarning<<"Ctrl尺寸过大";
        return "";
    }
    QString value = v.toString();
    if(value.size()>=100){
        nwarning<<"value尺寸过大";
        return "";
    }
    msg.check_type=qToBigEndian((quint32)mt_ctrl);
    strcpy(msg.ctrl,ctrl.toStdString().c_str());
    strcpy(msg.value,value.toStdString().c_str());
    QByteArray data;
    data.resize(sizeof(msg));
    memcpy(data.data(),&msg,sizeof(msg));
    return SPTP_sendCommon(data,d);
}

void TransmissionEngine::setClients(QList<device> clients){
    ninfo<<"TransmissionEngine的clients"<<clients;
    this->clients=clients;
}

QList<device> TransmissionEngine::Clients(){
    return clients;
}



void TransmissionEngine::multiDelay(float ms)
{
#ifdef Q_OS_WIN
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);  // 获取计数器频率
    QueryPerformanceCounter(&start);   // 获取开始时间
    
    // 计算目标计数值
    LONGLONG targetCount = start.QuadPart + 
        static_cast<LONGLONG>(((ms-0.5) / 1000.0) * freq.QuadPart);
    
    // 智能忙等待循环
    while (true) {
        QueryPerformanceCounter(&end);
        if (end.QuadPart >= targetCount) break;  // 达到目标时间
        
        // CPU优化策略
        LONGLONG remainingCount = targetCount - end.QuadPart;
        float remainingMs = (remainingCount * 1000.0f) / freq.QuadPart;
        
        if(remainingMs>20.){
            Sleep(0);
        }
    }
#else
    // 其他平台的实现
    QThread::usleep(static_cast<useconds_t>(ms * 1000));
#endif
}



void TransmissionEngine::on_readyRead(){
//    ndb<<"进入消息处理on_readyRead";
    while(m_communication->hasPendingDatagrams() /*|| ((!currentReliableMsg.isEmpty())&&reliableMsg_available)*/){
    bool isHandled = false;
    bool penDingDatagramFlag = m_communication->hasPendingDatagrams();
    QNetworkDatagram datagram = m_communication->readDatagram();
//    ipport sender = {datagram.senderAddress().toString(),(quint16)datagram.senderPort()}; //###
//    ndb<<"data"<<datagram.data();
    //解密数据
    QByteArray msg;
//    if(penDingDatagramFlag){
        msg = decode(datagram.data());
        lastMessage=msg;
//        ndb<<"var:msg"<<msg;
        QJsonObject json_temp = QJsonDocument::fromJson(msg).object();json_temp.remove("filebody");
        ninfo<<"var:msg(no filebody)"<<QJsonDocument(json_temp).toJson();
//    }
//    else if(!currentReliableMsg.isEmpty()&&reliableMsg_available){
//        msg=currentReliableMsg.toUtf8();
//        currentReliableMsg.clear();
//        reliableMsg_available = false;
//        ninfo<<"var:msg(relieableMsg)"<<msg;
//    }
    
    
    QJsonDocument jd = QJsonDocument::fromJson(msg);
    QJsonObject json;
    
    if(!jd.isObject()){
        if(!msg.startsWith("FB")&&!msg.startsWith("BF"))ninfo<<"var:msg = "<<msg;
        //外泄处理控制包
        if(msg.startsWith("BH")||msg.startsWith("HB")){
            QMetaObject::invokeMethod(this,[=]{on_bh_received(msg);},Qt::QueuedConnection);
        }
        //自己的
        if(msg == "FILE_RELEASE_SUCCESSFULLY" && !chunks.empty()){
            isHandled=true;
            emit signal_resend_finished();
            emit messageChanged(tr("文件发送可能成功"));
//            QSound::play("C:/Windows/Media/Alarm02.wav");
        }
        else if(msg == "KEEP_ALIVE"){
            
        }
        else if(msg.startsWith("FB")||msg.startsWith("BF")){
            isHandled=true;
            if(msg.size()<(int)sizeof(header_filebody_p)){
                nwarning<<"Warning:长度过小";
            }
            else{
//                if(!schedule_list.isEmpty() && schedule_list[0]->isEnabled()){
//                    foreach(auto s , schedule_list){//禁用日程
//                        s->setEnabled(false);
//                    }
//                }
                header_filebody_p header;
                memcpy(&header,msg.constData(),sizeof(header));
                header.no=qFromBigEndian(header.no);
                header.check_type=qFromBigEndian(header.check_type);
                header.total=qFromBigEndian(header.total);
                QByteArray fileContent = msg.mid(sizeof(header));
                
                ninfo<<"收到包"<<header.no<<"/"<<header.total;
                if(currentFileTotal!=header.total)currentFileMap.clear();
#ifdef Q_OS_WIN
                if(currentFileMap.empty())timeBeginPeriod(1);
#endif
                currentFileTotal = header.total;
                receive_last_pack_index = header.no;
                currentFileMap[header.no] = fileContent;
                timer_fileResend.stop();
                timer_fileResend.start(3000);
                if(this->chunks.empty()){
                    emit messageChanged(QString(tr("正在接收：包%1/%2")).arg(header.no).arg(header.total));
                }
                if(currentFileMap.size() == currentFileTotal){
                    emit messageChanged(tr("文件操作成功"));
                }
            }
        }
        else if(msg.startsWith("SEND_TASK")){
            isHandled=true;
            QString task = msg.mid(9);
            sendTask = task.split(';');
            emit messageChanged(QString("接受传输任务分配成功 任务数:%1").arg(sendTask.size()));
            ninfo<<"传输任务："<<sendTask;
//            ui->textBrowser_debug1->append("传输任务：");
//            ui->textBrowser_debug1->append(sendTask.join(";"));
        }
        else if(msg=="PLEASE_REQ_RESEND"){
            isHandled = true;
            ndb<<"接收到PLEASE_REQ_RESEND";
            multiDelay(100);
            ndb<<"开始进行";
            on_request_resend();
        }
        else{
//            ncritical<<"ERROR:msg isn`t an object!";
//            ninfo<<QString(msg);
//            return;
            ndb<<"外泄消息";
            emit communicationReadyRead(msg);
        }
        continue;//跳过下面的json处理逻辑
//        return;///////////////留意此处，修改循环请注意。。。。。。。。
    }
    else{
        json = jd.object();
    }
    
    //基本变量创建
    ipport sender = {
        json["ip"].toString(),
        static_cast<quint16>(json["port"].toInt())
    };
    int sender_index = clients.indexOf(sender);
    
    
    //消息解析
    /*if(json.contains("filebody")){
        if(currentFileTotal!=json["total"].toInt())currentFileMap.clear();
#ifdef Q_OS_WIN
        if(currentFileMap.empty())timeBeginPeriod(1);
#endif
        currentFileTotal = json["total"].toInt();
        receive_last_pack_index = json["no"].toInt();
        currentFileMap[json["no"].toInt()] = json["filebody"].toString();
        timer_fileResend.stop();
        timer_fileResend.start(3000);
        if(this->chunks.empty()){
            label_status->setText(QString("正在接收：包%1/%2").arg(json["no"].toInt()).arg(json["total"].toInt()));
        }
        if(currentFileMap.size() == currentFileTotal){
            label_status->setText("文件操作成功");
        }
        
//        //检测map连续
//        auto it = currentFileMap.constBegin();
//        int pre = -1;
//        QList<int> resendList;
//        for(;it != currentFileMap.constEnd();it++){
//            if(it.key() != pre+1){
//                for(int i=pre+1;i<it.key();i++){
//                    resendList.append(i);
//                }
//            }
//            pre=it.key();
//        }
//        if(currentFileMap.lastKey() != json["no"].toInt()){
//            for(int i=currentFileMap.lastKey()+1;i<=currentFileTotal;i++){
//                resendList.append(i);
//            }
//        }
        
//        if(resendList.size()>receive_lost_count){
//            send(QString("{\n    \"lost\":\"%1\"\n}").arg(resendList.size()).toUtf8(),1,sender_index);
//            receive_lost_count = resendList.size();
//        }
//        else{
//            send(QString("{\n    \"ack\":\" \"\n}").toUtf8(),1,sender_index);
//        }
        
    }*/
    if(json.contains("request_resend")/*&&is_uploading*/ && !chunks.empty()){
        isHandled=true;
//        QJsonObject rpjson;
//    //    json.insert("user",this->user_name);
//        rpjson.insert("total",chunks.size()-1);
//        rpjson.insert("filebody","");
//        rpjson.insert("no",-1);
//            rpjson["filebody"] = chunks[json["request_resend"].toInt()];
//            rpjson["no"] = json["request_resend"].toInt();
//            send(QJsonDocument(rpjson).toJson());
//            timer_is_uploading.stop();
//            timer_is_uploading.start(10000);
//            label_status->setText("正在回复重传请求……");
        header_filebody_p header;
        header.check_type = qToBigEndian((qint32)mt_filebody);
        header.total=qToBigEndian(chunks.size()-1);
        header.no = qToBigEndian(json["request_resend"].toInt());
        QByteArray msg_to_send;
        msg_to_send.append(reinterpret_cast<const char *>(&header),sizeof(header));
        msg_to_send.append(chunks[qFromBigEndian(header.no)]);
//        send(reinterpret_cast<const char *>(&header)+chunks[header.no]);
        send(msg_to_send,1,sender_index);
        timer_is_uploading.stop();
        timer_is_uploading.start(10000);
        emit messageChanged(tr("正在回复重传请求……"));
    }
    if(json.contains("opt")){
        QString opt = json["opt"].toString();
        
        if(opt == "test_if_connected"){//连通性测试
            ninfo << "  解析到的 sender=" << sender;
            ninfo << "  sender_index=" << sender_index << ", clients.size()=" << clients.size();
            ninfo << "  clients 列表内容=" << clients;
            isHandled=true;
            receive_sender=sender;
            if(sender_index == -1){
                emit messageChanged("<font color=\"red\">错误：收到未知来源的连通性测试包("+sender+")</font>");
            }
            else{
//                QThread::msleep(20);
                ndb<<"发送了acktestifconnected啊啊啊"<<sender_index;
                send("{\n    \"opt\":\"ack_test_if_connected\"\n}",1,sender_index);
            }
        }
        if(opt == "ack_test_if_connected"){
            isHandled=true;
            test_if_connected_set.insert(sender);
//            if(!test_if_connected_set.contains(public_ip)) test_if_connected_set.insert(public_ip);
//            if(test_if_connected_set == QSet<ipport>(clients.begin(),clients.end())){
            if(sender_index==currentSendDst){
                emit signal_test_if_connected_finished({});
            }
        }
    }
    if(json.contains("lost") && !chunks.empty()){
        isHandled=true;
        ndb<<"收到lost包";
//        send_current_delay += json["lost"].toInt() * 2;
//        send_stable_count = 0;
//        if(send_current_delay > SEND_MAX_DELAY) send_current_delay = SEND_MAX_DELAY;
//        ndb<<"发送速度调低到"<<send_current_delay;
        if(/*json["check1"].toInt()==send_current_reqAck*/json["uuid"].toString() == send_req_ack_uuid.toString()){//必需是本轮发送的包而不是上轮延迟到达的
            send_lost_count[sender] += QString(json["lost"].toString()).toInt();
            QJsonArray ar = json["lost_list"].toArray();
            foreach(auto i , ar) send_current_fastresend_map.insert(i.toInt());
            if(send_lost_count.size() >= /*clients.size()-1*/1){
                emit signal_reqAck_finished({});
            }
        }
    }
    if(json.contains("req_ack")){
        isHandled=true;
        if(receive_last_uuid!=json["uuid"].toString()){
            //检查丢包
            int lost = 0;
            int ack_pack = json["req_ack"].toInt();//应该收到的包的index
            QJsonArray ar;
            for(int i = json["start_req_ack"].toInt(); i <= ack_pack; i++){
                if(!currentFileMap.contains(i)){
                    lost++;
                    ninfo<<"lost"<<i;
                    ar.append(i);
                }
            }
    //        receive_last_ack_index = ack_pack;
            ninfo<<"丢失"<<lost;
//            QThread::usleep(2000);
            QJsonObject rpJson;
            rpJson.insert("lost",QString::number(lost));
            rpJson.insert("uuid",json["uuid"].toString());
            rpJson.insert("lost_list",ar);
            QByteArray sendData = QJsonDocument(rpJson).toJson();
            send(sendData,1,sender_index);//回复丢失信息
    //        QThread::usleep(500);
            send(sendData,1,sender_index);//回复丢失信息
    //        QThread::usleep(30000);
            send(sendData,1,sender_index);//三次回复丢失信息防止丢包
    //        send(QString("{\n    \"lost\":\"%1\"\n}").arg(lost).toUtf8(),1,sender_index);//四次回复丢失信息防止丢包
            receive_last_uuid = json["uuid"].toString();
        }
    }
    if(json.contains("reliable_msg")){//可靠消息处理
        isHandled=true;
        QString control_msg = json["reliable_msg"].toString();
        auto currentUuid = json["uuid"].toString();
        if(control_msg=="DATA"){
//            if(currentReliableUuid.isEmpty()){
//                reliableMsg_available = 0;
//                currentReliableMsg=json["value"].toString();
//                currentReliableUuid=json["uuid"].toString();
//                send((QString("R_ACK_DATA")+currentReliableUuid).toUtf8(),1,sender_index);
//            }
            //1.检验去重
            if(!processedReliableUuids.contains(currentUuid)){
                //直接处理消息
//                currentReliableMsg=;
                reliableMsg_available = false;
                reliableMessages.insert(currentUuid,json["value"].toString());
                //添加去重列表
                processedReliableUuids.insert(currentUuid);
            }
            //不管是否重发都回ACK
            send((QString("R_ACK_DATA")+currentUuid).toUtf8(),1,sender_index);
        }
        if(control_msg=="DONE"){
            //删除uuid
            if(processedReliableUuids.remove(currentUuid)){//检查消息是否存在，存在的话remove会返回true,顺便移除
                reliableMsg_available = true;
                auto crmsg = reliableMessages[currentUuid];
                reliableMessages.remove(currentUuid);
                emit signal_reliableMessage_received(crmsg.toUtf8());
            }
            send((QString("R_COMP")+currentUuid).toUtf8(),1,sender_index);
            ndb<<"发送了R_COMP";
//            reliableMsg_available = true;
//            currentReliableMsg = reliableMessages[currentUuid];
//            QMetaObject::invokeMethod(this,&TransmissionEngine::on_readyRead);
        }
        /*if(control_msg=="ALO_RLS"){
            emit messageChanged("收到ALO_RLS");
            if(currentReliableUuid==json["uuid"].toString()){
                currentReliableUuid.clear();
                reliableMsg_available = 1;
                QMetaObject::invokeMethod(this,&TransmissionEngine::on_readyRead,Qt::QueuedConnection);
            }
            send((QString("R_ACK_RLS")+json["uuid"].toString()).toUtf8(),1,sender_index);
        }*/
    }
    if(!isHandled){
        emit communicationReadyRead(msg);
        ndb<<"外泄消息";
    }
    }
}
//}
//}


void TransmissionEngine::on_request_resend(){
    //检测map连续 这个检测连续算法已经测试，没有问题
    auto it = currentFileMap.constBegin();
    int pre = -1;
//    it++;
    QList<int> resendList;
    for(;it != currentFileMap.constEnd();it++){
        if(it.key() != pre+1){
            for(int i=pre+1;i<it.key();i++){
                resendList.append(i);
            }
        }
        pre=it.key();
    }
    if(currentFileMap.lastKey() != currentFileTotal){
        for(int i=currentFileMap.lastKey()+1;i<=currentFileTotal;i++){
            resendList.append(i);
        }
    }
    
    ninfo<<"var:list(丢包) ="<<resendList;
    
    
    //请求重传
    QJsonObject json;
    json.insert("request_resend",0);
//    MessageBoxW(0,QString(tr("正在请求重传%1个包")).arg(resendList.size()).toStdWString().c_str(),L"Resend",0);
    foreach(auto i , resendList){
        json["request_resend"] = i;
        send(QJsonDocument(json).toJson(),1,clients.indexOf(receive_sender));
        emit messageChanged(QString(tr("正在请求重传%1个包")).arg(resendList.size()));
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents,100);
    }
    
    timer_fileResend.stop();
    if(resendList.size() != 0){//如果重新传输了那么就不能执行后面的合并文件否则会合并一个错误的文件
//        timer_fileResend.start(3000); // 3秒后再次检查文件完整性
        QTimer::singleShot(3000,this,&TransmissionEngine::on_request_resend);
        return;
    }
    
    timer_clear_currentFileMap.stop();
    timer_clear_currentFileMap.start(10000);
    
    QEventLoop loop;        //定义一个新的事件循环
    QTimer::singleShot(500, &loop,&QEventLoop::quit);//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec(); 
    
    //合并文件
    QByteArray n;
    foreach(QByteArray i,currentFileMap){
        n.append(i);
    }
//    ui->textBrowser_debug1->setText(n);
    ninfo<<"Log:文件请求已完成，字节数："<<n.size();
    emit messageChanged(tr("正在释放文件"));
#ifdef Q_OS_WIN
    timeEndPeriod(1);
#endif
//    releaseFile(n);
    emit SPTP_readyRead(n);
    emit messageChanged(tr("文件释放成功"));
    QEventLoop loop2;        //定义一个新的事件循环
    QTimer::singleShot(3500, &loop2,&QEventLoop::quit);//创建单次定时器，槽函数为事件循环的退出函数
    loop2.exec(); 
    sendReliableMessage(clients.indexOf(receive_sender),"FILE_RELEASE_SUCCESSFULLY");
    
    //清除状态
    currentFileMap.clear();currentFileTotal = -1;/*;receive_lost_count=0;*/receive_last_pack_index=-1;receive_last_ack_total=-1;timer_fileResend.stop();/*receive_last_ack_index=-1*//*;foreach(auto i,schedule_list)i->setEnabled(true);*/receive_sender={"",0};
    
    //裂变传播
    if(!sendTask.empty()){
        emit messageChanged("开始传播文件");
        process_events_without_useript;
        for(auto t : sendTask){
            //查找
            int index = -1;
            for(int i=0;i<clients.size();i++){
                if(clients[i].operator QString const() == t){
                    index=i;
                    break;
                }
            }
//            QMetaObject::invokeMethod(this,"sendFileTo",Qt::QueuedConnection,Q_ARG(int,index));//QueuedConnection在事件循环运行并且顺序按照invoke的顺序运行。sendFileTo不能在除了事件循环以外的其他地方运行
            /*SPTP_sendTo(index,n);
            emit messageChanged("正在等待状态重置……(请耐心等候)");
            QEventLoop loop;
            QTimer::singleShot(21000,&loop,&QEventLoop::quit);
            connect(this,&TransmissionEngine::signal_resend_finished,&loop,&QEventLoop::quit);
            loop.exec();*/
//            ui->textEdit_debug1->append(QString("发送文件到%1").arg(index));
            auto task_ptr = std::make_shared<const file_sending_task>(clients[index], n);
            queue_fileSendingTask.append((task_ptr));
        }
        sendTask.clear();//清空以便下次
        emit signal_resend_finished();
    }
}

void TransmissionEngine::on_reliableMessage_received(QString msg, TransmissionEngine::QPrivateSignal){
    bool is_handled = false;
    
    if(msg == "FILE_RELEASE_SUCCESSFULLY" && !chunks.empty()){
        is_handled=true;
        emit signal_resend_finished();
        emit messageChanged(tr("文件发送可能成功"));
//        QSound::play("C:/Windows/Media/Alarm02.wav");
    }
    else if(msg.startsWith("SEND_TASK")){
        is_handled=true;
        QString task = msg.mid(9);
        sendTask = task.split(';');
        emit messageChanged(QString("接受传输任务分配成功 任务数:%1").arg(sendTask.size()));
        ninfo<<"传输任务："<<sendTask;
//            ui->textBrowser_debug1->append("传输任务：");
//            ui->textBrowser_debug1->append(sendTask.join(";"));
    }
    else if(msg=="PLEASE_REQ_RESEND"){
        is_handled = true;
        ndb<<"接收到PLEASE_REQ_RESEND";
        multiDelay(100);
        ndb<<"开始进行";
        QMetaObject::invokeMethod(this,&TransmissionEngine::on_request_resend,Qt::QueuedConnection);
    }
    else if(msg=="DING"){
        is_handled = true;
        QMessageBox::information(qApp->activeWindow(),"叮","有人叮了你一下");
    }
    
    if(!is_handled){
        emit reliableMessageReceived(msg);
    }
}


void TransmissionEngine::on_bh_received(QByteArray msg){
    const int headerSize = sizeof(header_common_p);
    if (msg.size() < headerSize) {
        nwarning<<"BH数据包长度不足，丢弃";
        return;
    }

    header_common_p hCommon;
    memcpy(&hCommon, msg.constData(), headerSize);

    hCommon.check_type      = qFromBigEndian(hCommon.check_type);
    hCommon.src_port        = qFromBigEndian(hCommon.src_port);
    hCommon.src_public_ip_1 = qFromBigEndian(hCommon.src_public_ip_1);
    hCommon.src_public_ip_2 = qFromBigEndian(hCommon.src_public_ip_2);
    if (hCommon.check_type != mt_bh) {
        nwarning<<"BH包类型校验失败";
        return;
    }
    QHostAddress srcIp;
    quint16 srcPort = hCommon.src_port;

    if (hCommon.ip_type == 4) {
        quint32 ipv4Host = qFromBigEndian(static_cast<quint32>(hCommon.src_public_ip_1));
        srcIp = QHostAddress(ipv4Host);
    }
    else if (hCommon.ip_type == 6) {
        Q_IPV6ADDR ipv6Addr;
        memcpy(&ipv6Addr[0], &hCommon.src_public_ip_1, 8);
        memcpy(&ipv6Addr[8], &hCommon.src_public_ip_2, 8);
        srcIp = QHostAddress(ipv6Addr);
    }
    else {
        nwarning << "不支持的IP类型：" << hCommon.ip_type;
        return;
    }
    
    QByteArray payload1 = msg.mid(headerSize);
    msg_common msgCommon;
    msgCommon.src={srcIp.toString(),srcPort};
    msgCommon.msg=payload1;
    
    if(payload1.startsWith("CT")||payload1.startsWith("TC")){
        if(payload1.size() < (int)sizeof(msg_ctrl_p)) return;
    
        msg_ctrl_p ctrl;
        memcpy(&ctrl, payload1.constData(), sizeof(ctrl));
    
        // 端序转换
        ctrl.check_type = qFromBigEndian(ctrl.check_type);
        if(ctrl.check_type != mt_ctrl) return;
    
        // 读取指令和参数
        QByteArray cmd = QByteArray(ctrl.ctrl);
        QString value = QByteArray(ctrl.value);
    
        // QVariant 转回原始类型
        QVariant var = value;;
        
        msg_ctrl msgCtrl;
        msgCtrl.src=msgCommon.src;
        msgCtrl.ctrl=cmd;
        msgCtrl.value=value;
        
        emit SPTP_ctrlMsgReceived(msgCtrl);
    }
    else{
        emit SPTP_commonMsgReceived(msgCommon);
    }
}


QVector<QVector<QPair<ipport, ipport>>> TransmissionEngine::planAutoSend(QList<device> dsts){
    QQueue<ipport> senders;
    QQueue<ipport> receivers;
    QVector<QVector<QPair<ipport,ipport>>> result;
    foreach(ipport i,dsts)receivers.append(i);
    senders.append(public_ip);
    
    //开始规划
    ninfo<<"开始规划发送表";
    while(!receivers.empty()){//规划到没有可用的接收者了
        QQueue<ipport> new_senders;
        QVector<QPair<ipport,ipport>> thisRound;
        ninfo<<"----------";
        foreach(ipport sender , senders){
            if(receivers.empty())break;
            thisRound.append(QPair<ipport,ipport>(sender,receivers.front()));
            ninfo<<"配对："<<sender<<" "<<receivers.front();
            new_senders.append(receivers.front());
            receivers.pop_front();
        }
        senders.append(new_senders);
        result.append(thisRound);
    }
    ninfo<<"结果："<<(result);
    return result;
}


#undef process_events_without_useript