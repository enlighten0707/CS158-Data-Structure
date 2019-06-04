//
// Created by 金乐盛 on 2018/5/16.
//

#ifndef BPLUSTREE_BPPAIR_H
#define BPLUSTREE_BPPAIR_H
#include <iostream>
#include <fstream>
#define LENGTH_CHAR 30
namespace tool{
    template <class Key, class Value, class Compare = std::less<Key>>
    struct pair{
        Key first;
        //Key secondKey;
        Value second;
        pair(){first = Key(); second = Value();}
        pair(Key k, Value v):first(k), second(v){}
        pair(const pair &p):first(p.first), second(p.second){}
        pair& operator=(const pair &other){
            first=other.first;
            second=other.second;
        }
    };


    class mychar{
        friend std::ostream &operator<< (std::ostream &out, mychar &mychar1);
        friend std::istream &operator>> (std::istream &in, mychar &mychar1);
        friend mychar operator+(mychar &c1, mychar &c2);
        friend bool operator==(const mychar &mc, const char *filename);
    private:
        void count(){
            length = 0;
            while(true){
                if(ch[length] != '\0'){
                    length++;
                }
                else break;
            }
        }

    public:
        char ch[LENGTH_CHAR] = {0};
        int length = 0;
        explicit mychar(const char *ch = ""){
            clear();
            this->add(ch);
        }

        bool contain(const char*ch = ""){
            for(int i = 0; i < length; i++){
                if(ch[0] == this->ch[i])
                    return true;
            }
            return false;
        }
        void init(const char *ch = ""){
            clear();
            add(ch);
        }


        void clear(){
            for(int i = 0; i < LENGTH_CHAR; i++){
                this->ch[i] = '\0';
            }
            length = 0;
        }

        void add(const char c[]){
            int l = 0;
            while(true){
                if(c[l] != '\0'){
                    l++;
                }
                else break;
            }
            for(int i = 0; i < l; i++){
                ch[length + i] = c[i];
            }
            length += l;
        }

        void add(const mychar &c){
            for(int i = 0; i < c.length; i++){
                ch[length + i] = c.ch[i];
            }
            length += c.length;
        }




    };

    std::ostream &operator<< (std::ostream &out, mychar &mychar1){
        out << mychar1.ch;
        return out;
    }
    std::istream &operator>> (std::istream &in, mychar &mychar1){
        mychar1.clear();
        in >> mychar1.ch;
        mychar1.count();
        return in;
    }

    mychar operator+(mychar &c1, mychar &c2){
        mychar c;
        c.length = c1.length + c2.length;
        for(int i = 0; i < c2.length; i++){
            c.ch[i] = c2.ch[i];
        }
        for(int i = 0; i < c1.length; i++){
            c.ch[c2.length + i] = c1.ch[i];
        }
        return c;
    }

    bool operator==(const mychar &mc, const char *filename){
        for(int i = 0; i < mc.length; i++){
            if(mc.ch[i] != filename[i]) return false;
        }
        if(filename[mc.length] != '\0') return false;
        return true;
    }

    bool operator==(const mychar &mc, const mychar &mc2){
        if(mc.length != mc2.length) return false;
        for(int i = 0; i < mc.length; i++){
            if(mc.ch[i] != mc2.ch[i]) return false;
        }
        return true;
    }

    bool operator>(const mychar &mc, const mychar &mc2){
        if(mc.length > mc2.length) return true;
        if(mc.length < mc2.length) return false;
        for(int i = 0; i < mc.length; i++){
            if(mc.ch[i] > mc2.ch[i]) return true;
            if(mc.ch[i] < mc2.ch[i]) return false;
        }
        return false;

    }
    bool operator<(const mychar &mc1, const mychar &mc2){
        return mc2 > mc1;
    }
}
#endif //BPLUSTREE_BPPAIR_H
