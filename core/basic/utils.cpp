#include "utils.h"
#include <general.h>
#include <../../libary/Qt-AES/qaesencryption.h>
#include <QCryptographicHash>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <QRandomGenerator>
#include <QApplication>
#include <QBuffer>


QByteArray Utils::encode(const QByteArray &msg, const QString &pwd){
#ifndef DEBUG_NO_ENCRYPTION
    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray key = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
//    QByteArray iv = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Md5);
    QByteArray iv;iv.resize(16);
    QRandomGenerator::system()->generate(iv.begin(),iv.end());
    QByteArray encode = iv+encription.encode(msg,key,iv)/*.toBase64()*/;
    return /*qCompress(*/encode/*,9)*/;
#else   
    return msg;
#endif
}


QByteArray Utils::decode(const QByteArray &msg,const QString &pwd){
#ifndef DEBUG_NO_ENCRYPTION
    if(msg.size()<16)return QByteArray();
    QAESEncryption encription(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray key = QCryptographicHash::hash(pwd.toUtf8(),QCryptographicHash::Sha256);
    QByteArray iv = msg.mid(0,16);
    const auto msg2=msg.mid(16);
    QByteArray decoded = encription.removePadding(encription.decode(/*QByteArray::fromBase64(msg)*//*qUncompress(*/msg2/*)*/,key,iv));
    return decoded;
#else
    return msg;
#endif
}


QByteArray Utils::encodeRaw(const QByteArray &msg, const QByteArray &pwd){
#ifndef DEBUG_NO_ENCRYPTION
    QAESEncryption aes(QAESEncryption::AES_256,QAESEncryption::CBC);
    QByteArray iv;iv.resize(16);
    QRandomGenerator::system()->generate(iv.begin(),iv.end());
    return iv+aes.encode(msg,pwd,iv);
#else
    return msg;
#endif
}


QByteArray Utils::decodeRaw(const QByteArray &msg, const QByteArray &pwd){
#ifndef DEBUG_NO_ENCRYPTION
    const QByteArray &iv=msg.mid(0,16);
    const QByteArray &msgBody=msg.mid(16);
    QAESEncryption aes(QAESEncryption::AES_256,QAESEncryption::CBC);
    return aes.removePadding(aes.decode(msgBody,pwd,iv));
#else
    return msg;
#endif
}


QByteArray Utils::mergeFile(QDir folder, QSet<QString> incremental_sync_set, bool c){
    QByteArray f;
    auto fil = traverseFolder(folder);
    bool sync_all = incremental_sync_set.empty();
    foreach(auto fim , fil){
        auto fi=fim.first;
        if(fi.isFile()){
            ninfo<<"处理文件"<<fi.absoluteFilePath();
//            ndb<<"文件标识："<<QDir("files").relativeFilePath(fi.absoluteFilePath());
            if(sync_all || incremental_sync_set.contains(QDir("files/").relativeFilePath(fi.filePath()))){
                f += "FILE\n";
    //            QString filepath = fi.canonicalFilePath();
    //            QString dirpath = QDir("files/").canonicalPath();
    //            f += filepath.mid(dirpath.size()+1) + "\n";
                QString absPath = fi.canonicalFilePath();
                QString relativePath = QDir("files/../").relativeFilePath(fim.second.absolutePath());
                f += relativePath.toUtf8() + "\n";
                QFile file(absPath);
                file.open(QIODevice::ReadOnly);
    //            f += qCompress(file.readAll(),9).toBase64() + "\n";
                QByteArray data = qCompress(file.readAll(),9);
                f += QString::number(data.size()).toUtf8() + "\n";
                if(c)f += data;else f += "[FILE_CONTENTS_HERE]\n";
            }
            else{
                ninfo<<"忽略用户不同步的文件"<<fi.absoluteFilePath();
            }
        }
        else{//无需目录递归DFS
            ninfo<<"处理目录"<<fi.absoluteFilePath();
            f += "DIR\n";
            QString absPath = fi.canonicalFilePath();
            QString relativePath = QDir("files/../").relativeFilePath(fi.absoluteFilePath());
            f += relativePath.toUtf8() + "\n";
//            f += mergeFile(absPath);
//            f += mergeFile(QDir(relativePath),c);
        }
    }
    return f;
}


void Utils::releaseFile( QByteArray msg){
    while(!msg.isEmpty()){//循环读取
//        QString operation = stm.readLine();
        QString operation = readLine(msg);
        
        //解析
        if(operation == "FILE"){//解析文件
            QString filename = /*stm.readLine()*/readLine(msg);//获取文件名
            bool enableAttackCheck = true;
            if(QFile(filename+".private.stlink").exists()){
                QFile f(filename+".private.stlink");f.open(QFile::ReadOnly);
                filename=QDir(f.readAll()).absolutePath();
                enableAttackCheck=false;
            }
            QFile file(/*stack.top().filePath(filename)*/filename);
            file.open(QIODevice::WriteOnly);
            if(!file.isOpen()){
                ninfo<<"Error:File isn't open.Details:"<<file.errorString();
            }
            int size = /*stm.readLine()*/readLine(msg).toInt();
            QByteArray value = qUncompress(msg.left(size));
            msg = msg.mid(size);
//            QByteArray value = qUncompress(QByteArray::fromBase64(stm.readLine().toUtf8()));//文件内容
            
            //攻击检测
            if((enableAttackCheck && !filename.startsWith("files/")) || filename.contains("..")){//路径便利攻击
                bool f = filename.contains(".dll")||filename.contains(".exe")||filename.contains(".nprivate")||filename.contains("config.json")||filename.contains(".ini")||filename.contains(".sys");
                bool isSystemPath = filename.contains("/Windows/") || 
                                     filename.contains("/System/") ||
                                     filename.contains("/etc/") ||
                                     filename.contains("/bin/");
                
            }
            else file.write(value);
            file.close();
            ninfo<<"释放文件"<<filename<<"大小"<<value.size();
//            stm.readLine();
        }
        
        if(operation == "DIR"){//处理目录
            /*QString dirName = stm.readLine();
            
            QDir currentDir = stack.top();
            if(!QDir(currentDir.filePath(dirName)).exists()){
                currentDir.mkpath(dirName);
            }
            
//            stack.push(QDir(QDir(stack.top()).filePath(dirName)));*/
            
//            QDir dir = stm.readLine();
            QDir dir = QString(readLine(msg));
            dir.mkpath(".");
            ninfo<<"创建目录"<<dir;
        }
        
//        if(operation == "END_DIR"){//处理目录
//            if(!stack.empty()){
//                stack.pop();
//            }
//        }
    }
}


void Utils::multiDelay(float ms,std::function<void()> dosth){
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
        
        if(dosth){
            dosth();
        }
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


QByteArray Utils::readLine(QByteArray &ba){
    QByteArray ret;
    int newLinePos = ba.indexOf("\n");
    if(newLinePos == -1){
        ret = ba;
        ba.clear();
        return ret;
    }
    else{
        ret = ba.left(newLinePos+1);
        if(ret.endsWith("\r\n"))ret.chop(2);
        if(ret.endsWith("\n"))ret.chop(1);
        ba = ba.mid(newLinePos+1);
    }
    return ret;
}


QMap<QString, QByteArray> Utils::generateFileHashMap(QDir baseDir){
    QMap<QString,QByteArray> result;
    auto infoList = traverseFolder(baseDir);
    foreach(auto ifp,infoList){
        auto info = ifp.first;
        if(info.isFile()){
            QFile f(info.absoluteFilePath());
            if(f.open(QFile::ReadOnly)){
                QCryptographicHash hash(QCryptographicHash::Md5);
                hash.addData(&f); 
                f.close();
                result.insert(info.absoluteFilePath(),hash.result());
            }
            else{
                ncritical<<"文件打开失败！File:"<<f.fileName()<<" Info:"<<f.errorString();
            }
        }
        if(info.isDir()){
            //什么也不做
        }
    }
    return result;
}


QList<QPair<QFileInfo, QDir> > Utils::traverseFolder(QDir folder){
    QList<QPair<QFileInfo,QDir>> ret;
    QFileInfoList info = folder.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries,QDir::Name|QDir::DirsLast);
//    ninfo<<"进入目录"<<folder.absolutePath()<<"长度"<<info.size();
    foreach(QFileInfo fi ,info){
        if(fi.isFile()){
//            ninfo<<"处理文件"<<fi.absoluteFilePath();
            if(fi.fileName().endsWith(".private.stlink")){
                //处理符号链接
                QFile f(fi.absoluteFilePath());
                f.open(QFile::ReadOnly);
                QString dir = f.readAll().trimmed();
                f.close();
                if(QFile(dir).exists()){
                    QFileInfo info(dir);
                    QDir logicalDir = QDir(fi.absolutePath()).absoluteFilePath(QFileInfo(dir).fileName());
                    ret.append(qMakePair(info,logicalDir));
                }
            }
            else{
                ret.append(qMakePair(fi,QDir(fi.absoluteFilePath())));
            }
        }
        else{//目录递归DFS
//            ninfo<<"处理目录"<<fi.absoluteFilePath();
            ret.append(qMakePair(fi,QDir(fi.absoluteFilePath())));
            ret<<(traverseFolder(QDir(fi.absoluteFilePath())));
        }
    }
    return ret;
}

QByteArray Utils::getMachineUniqueId(){
    winComp({
        return "STID-WIN?"+QCryptographicHash::hash(QSysInfo::machineUniqueId()+"&&STID_WIN&&",QCryptographicHash::Sha256);
    })
    androidComp({
        return "安卓系统暂时不支持";
                })
}


void Utils::restart(){
    QMetaObject::invokeMethod(qApp,[]{qApp->exit(EXIT_CODE_RESTART);});
}


QByteArray Utils::serSchedule(QList<Schedule*> schedule_list){
    QBuffer b;b.open(QBuffer::ReadWrite);
    QDataStream d(&b);
    foreach(auto i,schedule_list){
        d<<(*i);
    }
    b.seek(0);
    return b.readAll();
}


//QList<Schedule*> Utils::unserSchedule(QByteArray dat){
//    QBuffer b;b.open(QBuffer::ReadWrite);
//    b.write(dat);
//    QDataStream d(&b);
//    foreach(auto i,schedule_list)i->deleteLater();
//    schedule_list.clear();
//    while(!d.atEnd()){
//        Schedule *s = new Schedule(); 
//        d>>(*s);
//        schedule_list.append(s);
//        connect(s,&Schedule::triggered,this,[this]{auto c=clients;c.removeAll(public_ip);sendFile(c);});
//    }
//}