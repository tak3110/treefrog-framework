#ifndef TATOMICQUEUE_H
#define TATOMICQUEUE_H

#include <QAtomicPointer>
#include <QList>
#include <QSemaphore>


template<class T>
class TAtomicQueue
{
public:
    TAtomicQueue() : queue(), counter(0) { }
    void enqueue(const T &t);
    QList<T> dequeue();
    bool wait(int timeout);

private:
    QAtomicPointer<QList<T> > queue;
    QSemaphore counter;
};


template <class T>
inline void TAtomicQueue<T>::enqueue(const T &t)
{
    QList<T> *newQue = 0;
    for (;;) {
        QList<T> *oldQue = queue.fetchAndStoreOrdered(0);

        if (!newQue) {
            newQue = (oldQue) ? new QList<T>(*oldQue) : new QList<T>();
            newQue->append(t);
        } else {
            if (oldQue) {
                *newQue << *oldQue;
            }
        }

        if (oldQue)
            delete oldQue;

        if (queue.testAndSetOrdered(0, newQue)) {
            counter.release(1);
            break;
        }
    }
}


template <class T>
inline QList<T> TAtomicQueue<T>::dequeue()
{
    QList<T> ret;
    QList<T> *ptr = queue.fetchAndStoreOrdered(0);

    if (ptr) {
        counter.acquire(ptr->count());
        ret = *ptr;
        delete ptr;
    }
    return ret;
}


// template <class T>
// inline QList<T> TAtomicQueue<T>::dequeue(int timeout)
// {
//     QList<T> ret;
//     int sem = counter.available();
//     if (sem > 0) {
//         if (counter.tryAcquire(sem, timeout)) {
//             QList<T> *ptr = queue.fetchAndStoreOrdered(0);
//             if (ptr) {
//                 ret = *ptr;
//                 delete ptr;
//                 int cnt = ret.count();
//                 if (cnt > sem) {
//                     counter.acquire(cnt - sem);
//                 } else if (sem > cnt) {
//                     counter.release(sem - cnt);
//                 }
//             } else {
//                 counter.release(sem);
//             }
//         }
//     }
//     return ret;
// }


template <class T>
inline bool TAtomicQueue<T>::wait(int timeout)
{
    if (counter.tryAcquire(1, timeout)) {
        counter.release(1);
        return true;
    }
    return false;
}

#endif // TATOMICQUEUE_H
