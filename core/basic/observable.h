#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <QObject>

class ObservableHelper;
template <typename Func1>
void _connHelper(QObject *obj,Func1 func,ObservableHelper *h);

template<typename T>
class Observable
{
public:
    Observable();
    explicit Observable(T val);
    ~Observable();
    
    void set(const T val);
    const T get() const;
    operator const T() const;
    void operator=(const T val);
    
    template<typename Func1>
    inline void subscribe(QObject* obj,Func1 slot){
        _connHelper(obj,slot,helper);
    }
    template<typename Func1>
    inline void use(QObject* obj,Func1 slot){
        _connHelper(obj,slot,helper);
        slot();
    }
    
private:
    ObservableHelper *helper;
};




#include <QVariant>

class ObservableHelper : public QObject{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ get WRITE set NOTIFY updated)
public:
    ObservableHelper();
    
    void set(QVariant v){this->v=v;emit updated();}
    const QVariant get() const{return v;}
    
signals:
    void updated();
private:
    QVariant v;
};



template<typename T>
Observable<T>::Observable(){
    helper=new ObservableHelper;
    qRegisterMetaType<T>();
}

template<typename T>
Observable<T>::Observable(T val){
    helper=new ObservableHelper;
    qRegisterMetaType<T>();
    set(val);
}

template<typename T>
Observable<T>::~Observable()
{
    helper->deleteLater();
}

template<typename T>
void Observable<T>::set(const T val){
    if(val!=get())
        helper->set(QVariant::fromValue(val));
}

template<typename T>
const T Observable<T>::get() const{
    return helper->get().value<T>();
}

template<typename T>
Observable<T>::operator const T() const{
    return get();
}

template<typename T>
void Observable<T>::operator=(const T val){
    set(val);
}



template<typename Func1>
void _connHelper(QObject *obj, Func1 func, ObservableHelper *h){
    QObject::connect(h,&ObservableHelper::updated,obj,func);
}

#define OBS(type,name) Observable<type> o_##name

#endif // OBSERVABLE_H
