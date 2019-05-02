#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include <cstddef>

namespace sjtu {
    template<class T>
    class deque {
        friend class iterator;
    private:
        int sumSize;
        struct node
        {
            T* Data;
            node *pre,*next;
            node(T* d=NULL,node* p=NULL,node* n=NULL):Data(d),pre(p),next(n){}
            ~node(){if(Data)delete Data;}
        };
        struct mapIndex{
            int curSize;
            node* DataHead;
            node* DataTail;
            mapIndex *pre,*next;
            mapIndex(int cS,node* DH,node* DT,mapIndex* p,mapIndex* n)
            {curSize=cS;DataHead=DH;DataTail=DT;pre=p;next=n;}
            mapIndex():curSize(0),DataHead(NULL),DataTail(NULL),pre(NULL),next(NULL){}
        };
        mapIndex *mapHead,*mapTail;
    public:
        class const_iterator;
        class iterator {
            friend  class deque<T>;
        private:
            mapIndex* curMap;
            node* curNode;
        public:
            iterator(mapIndex* curM=NULL,node* curN=NULL):curMap(curM),curNode(curN){}
            bool isValid(const iterator &rhs){
                mapIndex* p=curMap;
                while(p){
                    if(p==rhs.curMap) return true;
                    p=p->next;
                }
                p=rhs.curMap;
                while(p){
                    if(p==curMap) return true;
                    p=p->next;
                }
                return false;
            }
            int Distance()const{
                node* a=curNode;
                mapIndex* p=curMap;
                int distance=0;
                while(a!=p->DataHead){
                    ++distance;
                    a=a->pre;
                }
                p=p->pre;
                while(p){
                    distance+=p->curSize;
                    p=p->pre;
                }
                return distance;
            }
            /**
           * return a new iterator which pointer n-next elements
           *   even if there are not enough elements, the behaviour is **undefined**.
           * as well as operator-
           */
            iterator operator+(const int &n) const {
                int i=0;
                mapIndex* tmpMap=curMap;
                node* tmpNode=curNode;
                iterator cur;
                if(n==0){
                    cur.curNode=curNode;
                    cur.curMap=curMap;
                    return cur;
                }
                if(n<0)  return *this-(-n);

                if(curMap->next->next==NULL&&curNode->next->next==NULL){
                    cur.curNode=tmpNode->next;
                    cur.curMap=tmpMap;
                    return cur;
                }
                while(i<n&&tmpNode->next!=curMap->DataTail){
                        ++i;
                        tmpNode=tmpNode->next;
                    }
                    if(i==n) {
                        cur.curMap = curMap;
                        cur.curNode = tmpNode;
                        return cur;
                    }

                tmpMap=curMap->next;
                while(i+(tmpMap->curSize)<n){
                    i+=tmpMap->curSize;
                    if(tmpMap->next==NULL) break;
                    tmpMap=tmpMap->next;
                }
                if(tmpMap->next==NULL){
                    tmpMap=tmpMap->pre;
                    cur.curMap=tmpMap;
                    cur.curNode=tmpMap->DataTail;
                    return cur;
                }
                tmpNode=tmpMap->DataHead;
                while(i<n){
                    ++i;
                    tmpNode=tmpNode->next;
                }
                cur.curMap=tmpMap;
                cur.curNode=tmpNode;
                return cur;
            }
            iterator operator-(const int &n) const {
                mapIndex* tmpMap=curMap;
                node* tmpNode=curNode;
                iterator cur;
                int i=0;
                if(n==0){
                    cur.curNode=curNode;
                    cur.curMap=curMap;
                    return cur;
                }
                if(n<0) return *this+(-n);
                while(i<n&&tmpNode->pre!=curMap->DataHead){
                    ++i;
                    tmpNode=tmpNode->pre;
                }
                if(i==n){
                    cur.curMap=curMap;
                    cur.curNode=tmpNode;
                    return cur;

                }
                tmpMap=curMap->pre;
                while(i+tmpMap->curSize<n){
                    i+=tmpMap->curSize;
                    tmpMap=tmpMap->pre;
                }
                tmpNode=tmpMap->DataTail;
                while(i<n){
                    ++i;
                    tmpNode=tmpNode->pre;
                }
                cur.curMap=tmpMap;
                cur.curNode=tmpNode;
                return cur;
            }
            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.

            int operator-(const iterator &rhs) const {
                if(!twoitrValid(rhs)) throw invalid_iterator();
                return Distance()-rhs.Distance();
            }

            bool twoitrValid(const iterator &rhs)const{
                mapIndex* p=curMap;
                mapIndex* q=rhs.curMap;
                while(p){
                    if(p==q) return true;
                    p=p->next;
                }
                p=curMap;
                while(p){
                    if(p==q) return true;
                    p=p->pre;
                }
                return false;
            }
            iterator operator+=(const int &n) {
                iterator cur=(*this)+n;
                (*this)=cur;
                return *this;
            }
            iterator operator-=(const int &n) {
                iterator cur=*this-n;
                *this=cur;
                return *this;
            }

            iterator operator++(int) {
                if(curMap->next->next==NULL&&curNode->next==NULL)
                    throw invalid_iterator();
                iterator old=*this;
                iterator cur=*this+1;
                *this=cur;
                return old;
            }

            iterator& operator++() {
                if(curMap->next->next==NULL&&curNode->next==NULL)
                    throw invalid_iterator();
                iterator cur=(*this)+1;
                (*this)=cur;
                return *this;
            }

            iterator operator--(int) {
                if(curMap->pre->pre==NULL&&curNode->pre->pre==NULL)
                    throw invalid_iterator();
                iterator old=*this;
                iterator cur=*this-1;
                *this=cur;
                return old;
            }

            iterator& operator--() {
                if(curMap->pre->pre==NULL&&curNode->pre->pre==NULL)
                    throw invalid_iterator();
                iterator cur=*this-1;
                *this=cur;
                return *this;
            }

            T& operator*() const {
                if(curMap->pre==NULL||curMap->next==NULL)
                    throw invalid_iterator();
                if(curNode->pre==NULL||curNode->next==NULL)
                    throw invalid_iterator();
                return *(curNode->Data);
            }
            /**
             * TODO it->field
             */
            T* operator->() const noexcept {
                T* p=curNode->Data;
                return p;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            bool operator==(const iterator &rhs) const {
                if(curNode==rhs.curNode)
                    return true;
                return false;
            }
            bool operator==(const const_iterator &rhs) const {
                if(curNode==rhs.curNode)
                    return true;
                return false;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                if(curNode==rhs.curNode)
                    return false;
                return true;
            }
            bool operator!=(const const_iterator &rhs) const {
                if(curNode==rhs.curNode)
                    return false;
                return true;
            }
        };
        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            mapIndex* curMap;
            node* curNode;
        public:
            const_iterator(mapIndex* curM=NULL,node* curN=NULL):curMap(curM),curNode(curN){}

            const_iterator(const const_iterator &other) {
                curNode=other.curNode;
                curMap=other.curMap;
            }
            const_iterator(const iterator &other) {
                curNode=other.curNode;
                curMap=other.curMap;
            }
            T& operator*() const {
                if(curMap->pre==NULL||curMap->next==NULL)
                    throw invalid_iterator();
                if(curNode->pre==NULL||curNode->next==NULL)
                    throw invalid_iterator();
                return *(curNode->Data);
            }

            bool isValid(const iterator &rhs){
                mapIndex* p=curMap;
                while(p){
                    if(p==rhs.curMap) return true;
                    p=p->next;
                }
                p=rhs.curMap;
                while(p){
                    if(p==curMap) return true;
                    p=p->next;
                }
                return false;
            }
            int Distance()const{
                node* a=curNode;
                mapIndex* p=curMap;
                int distance=0;
                while(a!=p->DataHead){
                    ++distance;
                    a=a->pre;
                }
                p=p->pre;
                while(p){
                    distance+=p->curSize;
                    p=p->pre;
                }
                return distance;
            }

            const_iterator operator+(const int &n) const {
                int i=0;
                mapIndex* tmpMap=curMap;
                node* tmpNode=curNode;
                const_iterator cur;
                if(n==0){
                    cur.curNode=curNode;
                    cur.curMap=curMap;
                    return cur;
                }
                if(n<0)  return *this-(-n);

                if(curMap->next->next==NULL&&curNode->next->next==NULL){
                    cur.curNode=tmpNode->next;
                    cur.curMap=tmpMap;
                    return cur;
                }
                while(i<n&&tmpNode->next!=curMap->DataTail){
                    ++i;
                    tmpNode=tmpNode->next;
                }
                if(i==n){
                    cur.curMap=curMap;
                    cur.curNode=tmpNode;
                    return cur;
                }

                tmpMap=curMap->next;
                while(i+(tmpMap->curSize)<n){
                    i+=tmpMap->curSize;
                    tmpMap=tmpMap->next;
                    if(tmpMap->next==NULL) break;
                }
                if(tmpMap->next==NULL){
                    tmpMap=tmpMap->pre;
                    cur.curMap=tmpMap;
                    cur.curNode=tmpMap->DataTail;
                    return cur;
                }
                tmpNode=tmpMap->DataHead;
                while(i<n){
                    ++i;
                    tmpNode=tmpNode->next;
                }
                cur.curMap=tmpMap;
                cur.curNode=tmpNode;
                return cur;
            }
            const_iterator operator-(const int &n) const {
                mapIndex* tmpMap=curMap;
                node* tmpNode=curNode;
                const_iterator cur;
                int i=0;
                if(n==0){
                    cur.curNode=curNode;
                    cur.curMap=curMap;
                    return cur;
                }
                if(n<0) return *this+(-n);
                while(i<n&&tmpNode->pre!=curMap->DataHead){
                    ++i;
                    tmpNode=tmpNode->pre;
                }
                if(i==n){
                    cur.curMap=curMap;
                    cur.curNode=tmpNode;
                    return cur;

                }
                tmpMap=curMap->pre;
                while(i+tmpMap->curSize<n){
                    i+=tmpMap->curSize;
                    tmpMap=tmpMap->pre;
                }
                tmpNode=tmpMap->DataTail;
                while(i<n){
                    ++i;
                    tmpNode=tmpNode->pre;
                }
                cur.curMap=tmpMap;
                cur.curNode=tmpNode;
                return cur;
            }

            int operator-(const const_iterator &rhs) const {
                return Distance()-rhs.Distance();
            }
            const_iterator operator+=(const int &n) {
                const_iterator cur=(*this)+n;
                (*this)=cur;
                return *this;
            }
            const_iterator operator-=(const int &n) {
                const_iterator cur=*this-n;
                *this=cur;
                return *this;
            }

            const_iterator operator++(int) {
                if(curMap->next->next==NULL&&curNode->next==NULL)
                    throw invalid_iterator();
                const_iterator old=*this;
                const_iterator cur=*this+1;
                *this=cur;
                return old;
            }

            const_iterator& operator++() {
                if(curMap->next->next==NULL&&curNode->next==NULL)
                    throw invalid_iterator();
                const_iterator cur=(*this)+1;
                (*this)=cur;
                return *this;
            }

            const_iterator operator--(int) {
                if(curMap->pre->pre==NULL&&curNode->pre->pre==NULL)
                    throw invalid_iterator();
                const_iterator old=*this;
                const_iterator cur=*this-1;
                *this=cur;
                return old;
            }

            const_iterator& operator--() {
                if(curMap->pre->pre==NULL&&curNode->pre->pre==NULL)
                    throw invalid_iterator();
                const_iterator cur=*this-1;
                *this=cur;
                return *this;
            }

            T* operator->() const noexcept {
                T* p=curNode->Data;
                return p;
            }

            bool operator==(const iterator &rhs) const {
                if(curNode==rhs.curNode)
                    return true;
                return false;
            }
            bool operator==(const const_iterator &rhs) const {
                if(curNode==rhs.curNode)
                    return true;
                return false;
            }

            bool operator!=(const iterator &rhs) const {
                if(curNode==rhs.curNode)
                    return false;
                return true;
            }
            bool operator!=(const const_iterator &rhs) const {
                if(curNode==rhs.curNode)
                    return false;
                return true;
            }
        };

        deque() {
            mapHead=new mapIndex;
            mapTail=new mapIndex;
            mapIndex* p=new mapIndex(0,NULL,NULL,mapHead,mapTail);
            mapHead->next=p;
            mapTail->pre=p;
            p->DataHead=new node;
            p->DataTail=new node;
            p->DataHead->next=p->DataTail;
            p->DataTail->pre=p->DataHead;
            sumSize=0;
        }
        deque(const deque &other) {
            mapHead=new mapIndex;
            mapTail=new mapIndex;
            mapHead->next=mapTail;
            mapTail->pre=mapHead;

            sumSize=other.sumSize;
            mapIndex* tmpMap=other.mapHead->next;
            mapIndex *p=mapHead,*q;
            node *tmpNode,*r,*s;
            while(tmpMap!=other.mapTail){
                q=new mapIndex(tmpMap->curSize,0,0,p,mapTail);
                p->next=q;
                mapTail->pre=q;
                q->DataHead=new node;
                q->DataTail=new node;
                q->DataTail->pre=q->DataHead;
                q->DataHead->next=q->DataTail;
                tmpNode=tmpMap->DataHead->next;
                s=q->DataHead;
                while(tmpNode!=tmpMap->DataTail){
                    r=new node(NULL,s,q->DataTail);
                    r->Data=new T(*(tmpNode->Data));
                    s->next=r;
                    q->DataTail->pre=r;
                    tmpNode=tmpNode->next;
                    s=s->next;
                }
                tmpMap=tmpMap->next;
                p=p->next;
            }
        }

        ~deque() {
            mapIndex *p=mapHead->next,*q;
            while(p!=mapTail){
                node *a=p->DataHead->next,*b;
                while(a!=p->DataTail){
                    b=a->next;
                    delete a;
                    a=b;
                }
                delete p->DataHead;
                delete p->DataTail;
                p=p->next;
            }
            p=mapHead->next;
            while(p){
                q=p->next;
                delete p;
                p=q;
            }
            delete mapHead;
        }

        deque &operator=(const deque &other) {
            if(this==&other) return *this;
            clear();
            mapIndex* a=mapHead->next;
            delete a->DataTail;
            delete a->DataHead;
            delete a;
            mapHead->next=mapTail;
            mapTail->pre=mapHead;

            sumSize=other.sumSize;
            mapIndex* tmpMap=other.mapHead->next;
            mapIndex *p=mapHead,*q;
            node *tmpNode,*r,*s;
            while(tmpMap!=other.mapTail){
                q=new mapIndex(tmpMap->curSize,0,0,p,mapTail);
                p->next=q;
                mapTail->pre=q;
                q->DataHead=new node;
                q->DataTail=new node;
                q->DataTail->pre=q->DataHead;
                q->DataHead->next=q->DataTail;
                tmpNode=tmpMap->DataHead->next;
                s=q->DataHead;
                while(tmpNode!=tmpMap->DataTail){
                    r=new node(NULL,s,q->DataTail);
                    r->Data=new T(*(tmpNode->Data));
                    s->next=r;
                    q->DataTail->pre=r;
                    tmpNode=tmpNode->next;
                    s=s->next;
                }
                tmpMap=tmpMap->next;
                p=p->next;
            }
            return *this;
        }
        /**
         * access specified element with bounds checking
         * throw index_out_of_bound if out of bound.
         */
        T & at(const size_t &pos) {
            if(pos>=sumSize||pos<0) throw index_out_of_bound();
            if(empty()) throw container_is_empty();
            if(pos==0)
                return *(mapHead->next->DataHead->next->Data);
            iterator cur(mapHead->next,mapHead->next->DataHead);
            cur+=pos+1;
            return *cur;
        }
        const T & at(const size_t &pos) const {
            if(pos>=sumSize||pos<0) throw index_out_of_bound();
            if(empty()) throw container_is_empty();
            iterator cur(mapHead->next,mapHead->next->DataHead);
            cur+=pos+1;
            return *cur;
        }
        T & operator[](const size_t &pos) {
            if(pos>=sumSize||pos<0) throw index_out_of_bound();
            if(empty()) throw container_is_empty();
            if(pos==0)
                return *(mapHead->next->DataHead->next->Data);
            iterator cur(mapHead->next,mapHead->next->DataHead);
            cur+=pos+1;
            return *(cur.curNode->Data);
        }
        const T & operator[](const size_t &pos) const {
            if(pos>=sumSize||pos<0) throw index_out_of_bound();
            if(empty()) throw container_is_empty();
            if(pos==0)
                return front();
            iterator cur(mapHead->next,mapHead->next->DataHead);
            cur+=pos+1;
            return *(cur.curNode->Data);
        }
        /**
         * access the first element
         * throw container_is_empty when the container is empty.
         */
        const T & front() const {
            if(empty()) throw container_is_empty();
            mapIndex* tmpMap=mapHead->next;
            node* tmpNode=tmpMap->DataHead->next;
            return *(tmpNode->Data);
        }
        /**
         * access the last element
         * throw container_is_empty when the container is empty.
         */
        const T & back() const {
            if(empty()) throw  container_is_empty();
            mapIndex* tmpMap=mapTail->pre;
            node* tmpNode=tmpMap->DataTail->pre;
            return *(tmpNode->Data);
        }
        /**
         * returns an iterator to the beginning.
         */
        iterator begin() {
            mapIndex* tmpMap=mapHead->next;
            node* tmpNode=tmpMap->DataHead->next;
            iterator cur(tmpMap,tmpNode);
            return cur;
        }
        const_iterator cbegin() const {
            mapIndex* tmpMap=mapHead->next;
            node* tmpNode=tmpMap->DataHead->next;
            const_iterator cur(tmpMap,tmpNode);
            return cur;
        }
        /**
         * returns an iterator to the end.
         */
        iterator end() {
            mapIndex* tmpMap=mapTail->pre;
            node* tmpNode=tmpMap->DataTail;
            iterator cur(tmpMap,tmpNode);
            return cur;
        }
        const_iterator cend() const {
            mapIndex* tmpMap=mapTail->pre;
            node* tmpNode=tmpMap->DataTail;
            const_iterator cur(tmpMap,tmpNode);
            return cur;
        }
        /**
         * checks whether the container is empty.
         */
        bool empty() const {
            return sumSize==0;
        }
        /**
         * returns the number of elements
         */
        size_t size() const {
            return sumSize;
        }
        /**
         * clears the contents
         */
        void clear() {
            if(empty())return;
            sumSize=0;
            mapIndex *p=mapHead->next,*q;
            while(p!=mapTail){
                node *a=p->DataHead->next,*b;
                p->DataHead->next=NULL;
                while(a){
                    b=a->next;
                    delete a;
                    a=b;
                }
                delete p->DataHead;
                p=p->next;
            }

            p=mapHead->next;
            mapHead->next=NULL;
            while(p){
                q=p->next;
                delete p;
                p=q;
            }

            mapTail=new mapIndex;
            mapIndex* a=new mapIndex(0,NULL,NULL,mapHead,mapTail);
            mapHead->next=a;
            mapTail->pre=a;
            a->DataHead=new node;
            a->DataTail=new node;
            a->DataHead->next=a->DataTail;
            a->DataTail->pre=a->DataHead;
        }
        /**
         * inserts elements at the specified locat on in the container.
         * inserts value before pos
         * returns an iterator pointing to the inserted value
         *     throw if the iterator is invalid or it point to a wrong place.
         */
        iterator insert(iterator pos, const T &value) {
            if(!itrValid(pos))  throw invalid_iterator();
            node* p=pos.curNode->pre;
            T* tmp=new T(value);
            node *q=new node(tmp,p,pos.curNode);
            p->next=q;
            pos.curNode->pre=q;
            ++sumSize;
            ++((pos.curMap)->curSize);
            check(pos.curMap);
            iterator cur;
            cur.curNode=q;
            p=(pos.curMap)->DataHead->next;
            while(p!=(pos.curMap)->DataTail){
                if(p==q){
                    cur.curMap=pos.curMap;

                    return cur;
                }
                p=p->next;
            }
            cur.curMap=pos.curMap->next;
            return cur;
        }
        /**
         * removes specified element at pos.
         * removes the element at pos.
         * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
         * throw if the container is empty, the iterator is invalid or it points to a wrong place.
         */
        iterator erase(iterator pos) {
            if(empty()) throw container_is_empty();
            node *p=pos.curNode->pre;
            node *q=pos.curNode->next;
            p->next=q;
            q->pre=p;
            delete pos.curNode;
            iterator cur;

            --sumSize;
            --(pos.curMap)->curSize;

            cur.curMap=pos.curMap;
            cur.curNode=q;
            Erasecheck(cur.curMap,cur.curNode);

            return cur;
        }
        /**
         * adds an element to the end
         */
        void push_back(const T &value) {
            if(empty()){
                mapIndex* p=mapHead->next;
                if(p!=mapTail){
                    p->curSize=1;
                    T* tmp=new T(value);
                    node* q=new node(tmp,p->DataHead,p->DataTail);
                    p->DataHead->next=q;
                    p->DataTail->pre=q;
                    ++sumSize;
                }
                else{
                    p=new mapIndex(1,NULL,NULL,mapHead,mapTail);
                    mapHead->next=p;
                    mapTail->pre=p;
                    p->DataHead=new node;
                    p->DataTail=new node;
                    T* tmp=new T(value);
                    node* q=new node(tmp,p->DataHead,p->DataTail);
                    p->DataHead->next=q;
                    p->DataTail->pre=q;
                    ++sumSize;
                }
                return;
            }
            mapIndex *tmpMap=mapTail->pre;
            node* tmpNode=tmpMap->DataTail->pre;
            node* p=tmpNode->next;
            T* tmp=new T(value);
            node* q=new node(tmp,tmpNode,p);
            tmpNode->next=q;
            p->pre=q;
            ++sumSize;
            ++tmpMap->curSize;
            check(tmpMap);
        }
        /**
         * removes the last element
         *     throw when the container is empty.
         */
        void pop_back() {
            if(empty()) throw container_is_empty();
            mapIndex *tmpMap=mapTail->pre;
            node* tmpNode=tmpMap->DataTail->pre;
            node* p=tmpNode->pre;
            node* q=tmpNode->next;
            p->next=q;
            q->pre=p;
            delete tmpNode;
            --sumSize;
            --tmpMap->curSize;
            //std::cout<<"step succeed"<<std::endl;
            check(tmpMap);
        }
        /**
         * inserts an element to the beginning.
         */
        void push_front(const T &value) {
            if(empty()){
                mapIndex* p=mapHead->next;
                if(p!=mapTail){
                    p->curSize=1;
                    T* tmp=new T(value);
                    node* q=new node(tmp,p->DataHead,p->DataTail);
                    p->DataHead->next=q;
                    p->DataTail->pre=q;
                    ++sumSize;
                }
                else{
                    p=new mapIndex(1,NULL,NULL,mapHead,mapTail);
                    mapHead->next=p;
                    mapTail->pre=p;
                    p->DataHead=new node;
                    p->DataTail=new node;
                    T* tmp=new T(value);
                    node* q=new node(tmp,p->DataHead,p->DataTail);
                    p->DataHead->next=q;
                    p->DataTail->pre=q;
                    ++sumSize;

                }
                return;
            }
            mapIndex *tmpMap=mapHead->next;
            node* p=tmpMap->DataHead;
            node* q=p->next;
            T* tmp=new T(value);
            node* tmpNode=new node(tmp,p,q);
            p->next=tmpNode;
            q->pre=tmpNode;
            ++sumSize;
            ++tmpMap->curSize;
            check(tmpMap);
        }
        /**
         * removes the first element.
         *     throw when the container is empty.
         */
        void pop_front() {
            if(empty()) throw container_is_empty();
            mapIndex *tmpMap=mapHead->next;
            node* p=tmpMap->DataHead;
            node* tmpNode=p->next;
            node* q=tmpNode->next;
            p->next=q;
            q->pre=p;
            delete tmpNode;
            --sumSize;
            --tmpMap->curSize;
            check(tmpMap);
        }

        void check(mapIndex* &curMap){
            if(curMap->curSize==0){
                if(curMap->next->next==NULL) {
                    if(sumSize==0)
                        return;
                    else{
                        delete curMap->DataTail;
                        delete curMap->DataHead;
                        mapIndex *p=curMap->next,*q=curMap->pre;
                        p->pre=q;
                        q->next=p;
                        delete curMap;
                        curMap=q;
                        return ;
                    }
                }
                mapIndex *p=curMap->pre,*q=curMap->next;
                delete curMap->DataTail;
                delete curMap->DataHead;
                p->next=q;
                q->pre=p;
                delete curMap;
                curMap=q;
                return;
            }
            if(curMap->curSize<600) return;
            else {
                int newSize=(curMap->curSize)>>1;
                //split
                node *a=curMap->DataHead,*b,*c;
                for(int i=0;i<newSize;++i)
                    a=a->next;
                b=a->next;
                c=curMap->DataTail->pre;
                mapIndex *p=curMap->next,*q;
                q=new mapIndex(curMap->curSize-newSize,NULL,NULL,curMap,p);
                curMap->next=q;
                p->pre=q;
                q->DataHead=new node;
                q->DataHead->next=b;
                b->pre=q->DataHead;
                q->DataTail=new node;
                q->DataTail->pre=c;
                c->next=q->DataTail;
                a->next=curMap->DataTail;
                curMap->DataTail->pre=a;
                curMap->curSize=newSize;
            }
        }
        void Erasecheck(mapIndex* &curMap,node* &curNode){
            if(curMap->curSize==0){
                if(curMap->next->next==NULL) {
                    if(sumSize==0){
                        curNode=curMap->DataTail;
                        return;
                    }
                    else{
                        delete curMap->DataTail;
                        delete curMap->DataHead;
                        mapIndex *p=curMap->next,*q=curMap->pre;
                        p->pre=q;
                        q->next=p;
                        delete curMap;
                        curMap=q;
                        curNode=q->DataTail;
                        return ;
                    }
                }
                mapIndex *p=curMap->pre,*q=curMap->next;
                delete curMap->DataTail;
                delete curMap->DataHead;
                p->next=q;
                q->pre=p;
                delete curMap;
                curMap=q;
                curNode=q->DataHead->next;
                return;
            }
            if(curNode->next==NULL){
                if(curMap->next->next==NULL) return;
                curMap=curMap->next;
                curNode=curMap->DataHead->next;
                return ;
            }
        }

        void print(){
            mapIndex* p=mapHead->next;
            int i;
            while(p!=mapTail){
                std::cout<<p->curSize<<' ';
                p=p->next;
            }
        }

        bool itrValid(iterator itr){
            mapIndex* p=mapHead;
            while(p){
                if(p==itr.curMap) return true;
                p=p->next;
            }
            return false;
        }
    };

}

#endif
