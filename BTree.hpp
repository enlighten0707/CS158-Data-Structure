#include "utility.hpp"
#include <functional>
#include <cstddef>
#include "exception.hpp"
#include <stdio.h>
#include <fstream>
#include <cstring>
#include <unistd.h>

namespace sjtu {
    static const char WRITE_PATH[256] = "C:\\Users\\86150\\Desktop\\records.txt";
    static const int UNIT = 4096;

    template <class Key, class Value, class Compare = std::less<Key> >
    class BTree {
    private:
        typedef std::pair<Key,long> Index;
        typedef std::pair<Key,Value> Record;

        static const size_t M = (4096 - 3 * sizeof(long) - sizeof(size_t)) / (sizeof(Index));
        static const size_t L = (4096 - 3 * sizeof(long) - sizeof(size_t)) / (sizeof(Record));

        struct TreeNode{
            long parent=0;
            long self=0;
            long succ=0;
            size_t size=0;
            Index index[M];
        };

        struct LeafNode{
            long parent=0;
            long self=0;
            long succ=0;
            size_t size=0;
            Record record[L];
        };

        char path[1024];
        mutable FILE *fp = nullptr;
        mutable int fileLevel = 0;

        struct CoreData{
            long root = 0, slot = 0, pos = UNIT;
            size_t _size = 0;
            int height = 0;
        };

        CoreData core;
        Compare comp;

        void openFile(const char *mode = "rb+") const{
            if (fileLevel == 0)
                fp = std::fopen(path, mode);
            ++fileLevel;
        }

        void closeFile() const{
            if (fileLevel == 1)
                std::fclose(fp);
            --fileLevel;
        }

        void write(void *place, size_t offset, size_t Size=UNIT){
            openFile();
            std::fseek(fp,offset,SEEK_SET);
            std::fwrite(place,Size,1,fp);
            closeFile();
        }

        void read(void* place, size_t offset ,size_t Size=UNIT) {
            openFile();
            std::fseek(fp, offset, SEEK_SET);
            std::fread(place,Size,1, fp);
            closeFile();
        }

        long alloc(){
            long s = core.slot;
            core.slot += UNIT;
            write(&core, core.pos);
            return s;
        }

    public:
        // Default Constructor and Copy Constructor
        BTree(const char *PATH = WRITE_PATH) {
            std::strcpy(path, PATH);
            if (access(path, 0) == -1){
                FILE *tmpfp = std::fopen(path, "wb+");
                fclose(tmpfp);
            }
            openFile();
            TreeNode rt;
            alloc();
            alloc();
            core.root = alloc();
            LeafNode ln;
            ln.parent = core.root;
            rt.index[0].second = alloc();
            write(&rt, core.root);
            write(&ln, rt.index[0].second);
            core.height = 2;
            write(&core, core.pos);
        }

        ~BTree() {
            openFile("wb+");
            closeFile();
            core.root = 0;
            core.slot = 0;
            core.pos = UNIT;
            core._size = 0;
            core.height = 0;
            openFile();
            TreeNode rt;
            alloc();
            alloc();
            core.root = alloc();
            LeafNode ln;
            ln.parent = core.root;
            rt.index[0].second = alloc();
            write(&rt, core.root);
            write(&ln, rt.index[0].second);
            core.height = 2;
            write(&core, core.pos);
        }


        // Insert: Insert certain Key-Value into the database
        // Return a pair, the first of the pair is the iterator point to the new
        // element, the second of the pair is Success if it is successfully inserted

        long Find(const Key& key){
            //找到插入key的叶结点的位置，如果插入的是最大或最小的key应当归并在中间
            TreeNode tn;
            read(&tn,core.root);
            long next;
            if(core._size==0)
                return tn.index[0].second;

            for(int i=0;i<core.height-2;++i){
                int j=0;
                while(j<tn.size&&comp(tn.index[j].key,key)) ++j;
                if(j==tn.size) --j;
                next=tn.index[j].child;
                read(&tn,next);
            }
            int j=0;
            while(j<tn.size&&comp(tn.index[j].key,key)) ++j;
            if(j==tn.size) --j;
            next=tn.index[j].child;
            return next;
        }

        void upDateIndex(long offset,const Key& oldKey ,const Key& newKey){
            //将某个index的值更新，与mode无关
            if(offset==0)
                return;
            TreeNode tn;
            read(&tn,offset);
            int i;
            for(i=0;i<tn.size;++i){
                if(!comp(tn.index[i].key,oldKey)&&!comp(oldKey,tn.index[i].key))break;
            }
            tn.index[i].first=newKey;
            write(&tn,offset);
            if(i==tn.size-1)
                upDateIndex(tn.parent,oldKey,newKey);
        }

        void insert(const Key& key, const Value& value){
            long curPos=Find(key);
            LeafNode ln;
            read(&ln,curPos);
            if(ln.size!=L){//此叶结点未满
                if(ln.size==0){
                    ln.record[0].first=key;
                    ln.record[0].second=value;
                    ++ln.size;
                    ++core._size;
                    write(&core,core.pos);
                    TreeNode tn;
                    read(&tn,core.root);
                    tn.index[0].first=key;
                    tn.size++;
                    write(&tn,core.root);
                    write(&ln,curPos);
                }
                else{
                    int i;
                    for(i=ln.size;i>0;--i){
                        if(comp(ln.record[i-1].first,key))break;
                        ln.record[i]=ln.record[i-1];
                    }
                    Key oldKey;
                    oldKey=ln.record[i].first;
                    ln.record[i].first=key;
                    ln.record[i].second=value;
                    ++ln.size;
                    ++core._size;
                    write(&core,core.pos);
                    if(i==ln.size-1)
                        upDateIndex(ln.parent,oldKey,ln.record[i].first);
                    write(&ln,curPos);
                }

            }
            else if(ln.succ!=0){//后面有一个兄弟
                LeafNode brother;
                read(&brother,ln.succ);
                if(brother.size!=L){//如果兄弟没有满，就将一个孩子给兄弟寄养
                    for(int i=brother.size;i>0;--i)
                        brother.record[i]=brother.record[i-1];
                    brother.record[0]=ln.record[ln.size-1];
                    ++brother.size;
                    int i;
                    for(i=ln.size-1;i>0;--i){
                        if(comp(ln.record[i-1].first,key))break;
                        ln.record[i]=ln.record[i-1];
                    }
                    Key oldKey=ln.record[i].first;
                    ln.record[i].first=key;
                    ln.record[i].second=value;
                    ++core._size;
                    write(&core,core.pos);
                    if(i==ln.size-1)
                        upDateIndex(ln.parent,oldKey,ln.record[i].first);
                    write(&ln,curPos);
                    write(&brother,ln.succ);
                }
                else{
                    //兄弟也满了，这时需要新加入一个叶子节点，进行分裂，并向上检查更新
                    LeafNode newNode;
                    createLeafNode(ln, newNode);
                    Key oldKey=ln.record[ln.size-1].first;
                    int i=L/2,j=0;
                    for(;i<L;++i,++j)
                        newNode.record[j]=ln.record[i];
                    newNode.size+=L/2;
                    ln.size-=L/2;
                    upDateIndex(ln.parent,oldKey,ln.record[ln.size-1].first);
                    insertIndex(ln.parent,oldKey,newNode.self,true);
                    write(&ln,ln.self);
                    write(&newNode,newNode.self);
                    insert(key,value);
                }
            }
            else{
                //后面没有兄弟，不能往后读取，新插入索引，方法与上一种类似
                LeafNode newNode;
                createLeafNode(ln, newNode);
                Key oldKey=ln.record[ln.size-1].first;
                int i=L/2,j=0;
                for(;i<L;++i,++j)
                    newNode.record[j]=ln.record[i];
                newNode.size+=L/2;
                ln.size-=L/2;
                upDateIndex(ln.parent,oldKey,ln.record[ln.size-1].first);
                insertIndex(ln.parent,newNode.record[newNode.size-1].first,newNode.self,true);
                write(&ln,ln.self);
                write(&newNode,newNode.self);
                insert(key,value);
            }
        }

        void insertIndex(long offset,const Key& key,long child,bool mode){
            //加入一个新的index(pair)，包含了新的Key和child信息
            //mode=false 孩子是内部节点 mode=true 孩子是叶结点
            //如果更新到根，根节点又满了，有可能长高一层
            if(offset==0)
                return;
            TreeNode tn;
            read(&tn,offset);
            if(tn.size!=M){
                //索引节点未满
                int i;
                for(i=tn.size;i>0;--i){
                    if(comp(tn.index[i-1].first,key))break;
                    tn.index[i]=tn.index[i-1];
                }
                Key oldKey;
                if(i==tn.size) oldKey=tn.index[tn.size-1].first;
                else oldKey=tn.index[i].first;
                tn.index[i].first=key;
                tn.index[i].second=child;
                upDateParent(offset,child,mode);
                ++tn.size;
                if(i==tn.size-1)
                    upDateIndex(tn.parent,oldKey,tn.index[i].first);
                write(&tn,offset);
            }
            else if(tn.succ!=0){//后面有一个邻居
                TreeNode sib;
                read(&sib,tn.succ);
                if(tn.size!=M){
                    //邻居没有满，可以寄养一个给邻居
                    if(comp(tn.index[tn.size-1].first,key)){
                        //新的孩子需要寄养在邻居那里
                        for (int i = sib.size; i > 0; --i)
                            sib.index[i] = sib.index[i - 1];
                        sib.index[0].first=key;
                        sib.index[0].second=child;
                        ++sib.size;
                        write(&sib,sib.self);
                        upDateParent(sib.self,child,mode);
                    }
                    else {
                        for (int i = sib.size; i > 0; --i)
                            sib.index[i] = sib.index[i - 1];
                        sib.index[0] = tn.index[tn.size - 1];
                        upDateParent(sib.self, sib.index[0].second, mode);
                        ++sib.size;
                        int i;
                        for (i = tn.size - 1; i > 0; --i) {
                            if (comp(tn.index[i - 1].first, key))break;
                            tn.index[i] = tn.index[i - 1];
                        }
                        Key oldKey = tn.index[i].first;
                        tn.index[i].first = key;
                        tn.index[i].second = child;
                        if (i == tn.size - 1)
                            upDateIndex(tn.parent, oldKey, tn.index[i].first);
                        write(&tn, tn.self);
                        write(&sib, sib.self);
                    }
                }
                else{
                    //邻居满了，这时需要预处理，分裂当前节点
                    TreeNode newNode;
                    createTreeNode(&tn, &newNode);
                    Key oldKey=tn.index[tn.size-1].first;
                    int i=M/2,j=0;
                    for(;i<M;++i,++j){
                        newNode.index[j]=tn.index[i];
                        upDateParent(newNode.self,newNode.index[j].second,mode);
                    }
                    newNode.size+=M/2;
                    tn.size-=M/2;
                    upDateIndex(tn.parent,oldKey,tn.index[tn.size-1].first);
                    insertIndex(tn.parent,oldKey,newNode.self,false);
                    write(&tn,tn.self);
                    write(&newNode,newNode.self);
                    if(comp(key,tn.index[tn.size-1].first))
                        insertIndex(tn.self,key,child,mode);
                    else
                        insertIndex(newNode.self,key,child,mode);
                }
            }
            else{
                //后面没有邻居,同样需要分裂
                TreeNode newNode;
                createTreeNode(&tn, &newNode);
                Key oldKey=tn.index[tn.size-1].first;
                int i=M/2,j=0;
                for(;i<M;++i,++j){
                    newNode.index[j]=tn.index[i];
                    upDateParent(newNode.self,newNode.index[j].second,mode);
                }
                newNode.size+=M/2;
                tn.size-=M/2;
                upDateIndex(tn.parent,oldKey,tn.index[tn.size-1].first);
                insertIndex(tn.parent,oldKey,newNode.self,false);

                if(tn.parent!=0){
                    //操作非根节点
                     write(&tn,tn.self);
                     write(&newNode,newNode.self);
                     if(comp(key,tn.index[tn.size-1].first))
                         insertIndex(tn.self,key,child,mode);
                     else
                         insertIndex(newNode.self,key,child,mode);
                }
                else{
                    //是根节点，目前根节点被一分为二，需要找到新的根节点
                    TreeNode newRoot;
                    int rootPos=alloc();
                    newRoot.parent=newRoot.succ=0;
                    newRoot.size=2;
                    newRoot.index[0].first=tn.index[tn.size-1].first;
                    newRoot.index[0].second=tn.self;
                    tn.parent=rootPos;
                    newRoot.index[1].first=newNode.index[newNode.size-1].first;
                    newRoot.index[1].second=newNode.self;
                    newNode.parent=rootPos;
                    core.height++;
                    core.root=rootPos;
                    write(&newRoot,rootPos);
                    write(&tn,tn.self);
                    write(&newNode,newNode.self);
                    if(comp(key,tn.index[tn.size-1].first))
                        insertIndex(tn.self,key,child,mode);
                    else
                        insertIndex(newNode.self,key,child,mode);
                }
            }

        }
        void upDateParent(long newParent,long offset,bool mode){
            //将孩子节点的父亲节点更新
            if(mode){
                LeafNode ln;
                read(&ln,offset);
                ln.parent=newParent;
                write((&ln,offset));
            }
            else{
                TreeNode tn;
                read(&tn,offset);
                tn.parent=newParent;
                write((&tn,offset));
            }
        }

        void createLeafNode(LeafNode &ln, LeafNode &nx){
            //创建一个新的的节点，节点offset被自己记录
            nx.parent = ln.parent;
            nx.succ = ln.succ;
            ln.succ = alloc();
            nx.self=ln.succ;
        }

        void createTreeNode(TreeNode &tn, TreeNode &nx){
            nx.parent = tn.parent;
            nx.succ = tn.succ;
            tn.succ = alloc();
            nx.self=tn.succ;
        }

        Value at(const Key& key){
            long pos = Find(key);
            std::cout<<pos<<std::endl;
            LeafNode ln;
            read(&ln, pos);
            Record *rc = binarySearchRecord(ln, key);
            return rc->value;
        }

        bool empty() const { return core._size==0;}

        size_t size() const { return core._size;}
    };
}  // namespace sjtu

