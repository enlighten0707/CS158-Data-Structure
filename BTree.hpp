#include "utility.hpp"
#include <functional>
#include <cstddef>
#include "exception.hpp"
#include <stdio.h>
#include <fstream>
#include <cstring>
#include <unistd.h>

//#define DEBUG

namespace sjtu {
    static const char WRITE_PATH[256] = "C:\\Users\\86150\\Desktop\\records.txt";
    //static const int UNIT = 4096;
    static const int UNIT = 4096;

    template <class Key, class Value, class Compare = std::less<Key> >
    class BTree {
    private:
        typedef std::pair<Key,int> Index;
        typedef std::pair<Key,Value> Record;

        static const size_t M = (UNIT - 3 * sizeof(int) - sizeof(size_t)) / (sizeof(Index));
        static const size_t L = (UNIT - 3 * sizeof(int) - sizeof(size_t)) / (sizeof(Record));

//        static const size_t M = 30;
//        static const size_t L = 50;

        struct TreeNode{
            int parent=0;
            int self=0;
            int succ=0;
            size_t size=0;
            Index index[M];
        };

        struct LeafNode{
            int parent=0;
            int self=0;
            int succ=0;
            size_t size=0;
            Record record[L];
        };

        char path[1024];
        mutable FILE *fp = nullptr;
        mutable int fileLevel = 0;

        struct CoreData{
            int root = 0, slot = 0, pos = UNIT;
            size_t _size = 0;
            int height = 0;
        };

        CoreData core;
        Compare comp;

        void openFile(const char *mode = "rb+") const{
            if (fileLevel == 0)
                fp = std::fopen(path, mode);
            ++fileLevel;
            //std::cout<<"open success\n";
        }

        void closeFile() const{
            if (fileLevel == 1)
                std::fclose(fp);
            --fileLevel;
            //std::cout<<"close success\n";
        }

        void write(void *place, int offset){
            //openFile();
            //std::rewind(fp);
            std::fseek(fp,offset,SEEK_SET);
            std::fwrite(place,UNIT,1,fp);
            std::fflush(fp);

            //closeFile();
        }

        void read(void* place, int offset) {
            //openFile();
            std::fseek(fp, offset, SEEK_SET);
            std::fread(place,UNIT,1, fp);
            //closeFile();
        }


        int alloc(){
            //std::cout<<"#alloc:";
            int s = core.slot;
            core.slot += UNIT;
            write(&core, core.pos);
            //std::cout<<s<<' '<<core.slot<<' '<<core.pos<<std::endl;
            return s;
        }

    public:
        class const_iterator;
        class iterator {
        private:
            // Your private members go here
        public:
            bool modify(const Value& value){

            }
            iterator() {
                // TODO Default Constructor
            }
            iterator(const iterator& other) {
                // TODO Copy Constructor
            }
            // Return a new iterator which points to the n-next elements
            iterator operator++(int) {
                // Todo iterator++
            }
            iterator& operator++() {
                // Todo ++iterator
            }
            iterator operator--(int) {
                // Todo iterator--
            }
            iterator& operator--() {
                // Todo --iterator
            }
            // Overloaded of operator '==' and '!='
            // Check whether the iterators are same
            bool operator==(const iterator& rhs) const {
                // Todo operator ==
            }
            bool operator==(const const_iterator& rhs) const {
                // Todo operator ==
            }
            bool operator!=(const iterator& rhs) const {
                // Todo operator !=
            }
            bool operator!=(const const_iterator& rhs) const {
                // Todo operator !=
            }
        };
        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            // Your private members go here
        public:
            const_iterator() {
                // TODO
            }
            const_iterator(const const_iterator& other) {
                // TODO
            }
            const_iterator(const iterator& other) {
                // TODO
            }
            // And other methods in iterator, please fill by yourself.
        };

        BTree(const BTree& other) {
            // Todo Copy
        }
        BTree& operator=(const BTree& other) {
            // Todo Assignment
        }
        OperationResult erase(const Key& key) {
            // TODO erase function
            return Fail;  // If you can't finish erase part, just remaining here.
        }
        // Return a iterator to the beginning
        iterator begin() {}
        const_iterator cbegin() const {}
        // Return a iterator to the end(the next element after the last)
        iterator end() {}
        const_iterator cend() const {}

        void clear() {}
        iterator find(const Key& key) {}
        const_iterator find(const Key& key) const {}

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
            ln.self=rt.index[0].second;
            rt.self=core.root;
            write(&rt, core.root);
            write(&ln, rt.index[0].second);
//            TreeNode newNode;
//            read(&newNode,core.root);
//            std::cout<<"###"<<newNode.self<<std::endl;
            core.height = 2;
            write(&core, core.pos);
            //std::cout<<"M="<<M<<"  L="<<L<<std::endl;
        }

        ~BTree() {
            //openFile("wb+");
            //closeFile();
            core.root = 0;
            core.slot = 0;
            core.pos = UNIT;
            core._size = 0;
            core.height = 0;
            //openFile();
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
            closeFile();
        }


        // Insert: Insert certain Key-Value into the database
        // Return a pair, the first of the pair is the iterator point to the new
        // element, the second of the pair is Success if it is successfully inserted

        int Find(const Key& key){
            //找到插入key的叶结点的位置，如果插入的是最大或最小的key应当归并在中间
            //std::cout<<"Find Key:\n";
            TreeNode tn;
            read(&tn,core.root);

            int next,parent=core.root;
            if(core._size==0)
                return tn.index[0].second;

            for(int i=0;i<core.height-2;++i){
                int j=0;
                while(j<tn.size&&comp(tn.index[j].first,key)) ++j;
                if(j==tn.size) --j;
                next=tn.index[j].second;
                read(&tn,next);
            }
            int j=0;
            while(j<tn.size&&comp(tn.index[j].first,key)) ++j;
            if(j==tn.size) --j;
            next=tn.index[j].second;
            return next;
        }

        void upDateIndex(int offset,const Key& oldKey ,const Key& newKey){
            //将某个index的值更新，与mode无关
            if(offset==0)
                return;
            TreeNode tn;
            read(&tn,offset);
            int i;
            for(i=0;i<tn.size;++i){
                if(!comp(tn.index[i].first,oldKey)&&!comp(oldKey,tn.index[i].first))break;
            }
            tn.index[i].first=newKey;
            write(&tn,offset);
            if(i==tn.size-1)
                upDateIndex(tn.parent,oldKey,newKey);
        }

        void insert(const Key& key, const Value& value){
            int curPos=Find(key);

            //std::cout<<curPos<<std::endl;

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
                    ln.record[i].first=key;
                    ln.record[i].second=value;
                    ++ln.size;
                    ++core._size;
                    write(&core,core.pos);
                    if(i==ln.size-1)
                        upDateIndex(ln.parent,ln.record[i-1].first,key);
                    write(&ln,curPos);
                }

            }
            else if(ln.succ!=0){//后面有一个兄弟
                LeafNode brother;
                read(&brother,ln.succ);
                if(brother.parent==ln.parent) {
                    if (brother.size != L) {
                        //如果兄弟没有满，就将一个孩子给兄弟寄养
#ifdef DEBUG
                        std::cout << key << " give brother a child" << std::endl;
#endif

                        for (int i = brother.size; i > 0; --i)
                            brother.record[i] = brother.record[i - 1];
                        if (comp(ln.record[ln.size - 1].first, key)) {
                            //节点正好放在兄弟的末尾，大小夹在两个record之间
                            brother.record[0].first = key;
                            brother.record[0].second = value;
                            ++brother.size;
                        } else {
                            //需要移动，放到前一个结点内部
                            brother.record[0] = ln.record[ln.size - 1];
                            ++brother.size;
                            int i;
                            for (i = ln.size - 1; i > 0; --i) {
                                if (comp(ln.record[i - 1].first, key))break;
                                ln.record[i] = ln.record[i - 1];
                            }
                            ln.record[i].first = key;
                            ln.record[i].second = value;
                            upDateIndex(ln.parent, brother.record[0].first, ln.record[ln.size - 1].first);
                        }
                        ++core._size;
                        write(&core, core.pos);
                        write(&ln, curPos);
                        write(&brother, ln.succ);
#ifdef DEBUG
                        debugFind(key);
#endif
                    } else {
                        //兄弟也满了，这时需要新加入一个叶子节点，进行分裂，并向上检查更新
#ifdef DEBUG
                        std::cout << key << " split a node" << std::endl;
#endif
                        LeafNode newNode;
                        createLeafNode(ln, newNode);
                        int i = L / 2, j = 0;
                        for (; i < L; ++i, ++j) {
                            newNode.record[j] = ln.record[i];
                            ++newNode.size;
                            --ln.size;
                        }

                        write(&ln, ln.self);
                        write(&newNode, newNode.self);
                        upDateIndex(ln.parent, newNode.record[newNode.size - 1].first, ln.record[ln.size - 1].first);
                        insertIndex(ln.parent, newNode.record[newNode.size - 1].first, newNode.self, true);
                        insert(key, value);
#ifdef DEBUG
                        debugFind(key);
#endif
                    }
                }
                else{
                    //后面没有兄弟，不能往后读取，新插入索引，方法与上一种类似
#ifdef DEBUG
                    std::cout<<key<<" a new brother"<<std::endl;
#endif
                    LeafNode newNode;
                    createLeafNode(ln, newNode);
                    int i=L/2,j=0;
                    for(;i<L;++i,++j){
                        newNode.record[j]=ln.record[i];
                        ++newNode.size;
                        --ln.size;
                    }

                    write(&ln,ln.self);
                    write(&newNode,newNode.self);
                    upDateIndex(ln.parent,newNode.record[newNode.size-1].first,ln.record[ln.size-1].first);
                    insertIndex(ln.parent,newNode.record[newNode.size-1].first,newNode.self,true);
                    insert(key,value);
#ifdef DEBUG
                    debugFind(key);
#endif
                }

            }
            else{
                //后面没有兄弟，不能往后读取，新插入索引，方法与上一种类似
#ifdef DEBUG
                std::cout<<key<<" a new brother"<<std::endl;
#endif
                LeafNode newNode;
                createLeafNode(ln, newNode);
                int i=L/2,j=0;
                for(;i<L;++i,++j){
                    newNode.record[j]=ln.record[i];
                    ++newNode.size;
                    --ln.size;
                }

                write(&ln,ln.self);
                write(&newNode,newNode.self);
                upDateIndex(ln.parent,newNode.record[newNode.size-1].first,ln.record[ln.size-1].first);
                insertIndex(ln.parent,newNode.record[newNode.size-1].first,newNode.self,true);
                insert(key,value);
#ifdef DEBUG
                debugFind(key);
#endif
            }
        }

        void insertIndex(int offset,const Key& key,int child,bool mode){
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
                Key oldKey=tn.index[i-1].first;
                tn.index[i].first=key;
                tn.index[i].second=child;
                upDateParent(offset,child,mode);

                if(i==tn.size-1)
                    upDateIndex(tn.parent,oldKey,key);
                ++tn.size;
                write(&tn,offset);
            }
            else if(tn.succ!=0){//后面有一个邻居
                TreeNode sib;
                read(&sib,tn.succ);
                if(sib.parent==tn.parent) {
                    if (tn.size != M) {
#ifdef DEBUG
                        std::cout << key << " index**give sib a child" << std::endl;
#endif
                        //邻居没有满，可以寄养一个给邻居
                        if (comp(tn.index[tn.size - 1].first, key)) {
                            //新的孩子需要寄养在邻居那里
                            for (int i = sib.size; i > 0; --i)
                                sib.index[i] = sib.index[i - 1];
                            sib.index[0].first = key;
                            sib.index[0].second = child;
                            ++sib.size;
                            write(&sib, sib.self);
                            upDateParent(sib.self, child, mode);
                        } else {
                            //从节点末尾移动一个孩子给邻居
                            for (int i = sib.size; i > 0; --i)
                                sib.index[i] = sib.index[i - 1];
                            sib.index[0] = tn.index[tn.size - 1];
                            upDateParent(sib.self, sib.index[0].second, mode);
                            ++sib.size;
                            --tn.size;
                            write(&tn, tn.self);
                            write(&sib, sib.self);
                            int i;
                            for (i = tn.size - 1; i > 0; --i) {
                                if (comp(tn.index[i - 1].first, key))break;
                                tn.index[i] = tn.index[i - 1];
                            }
                            //Key oldKey = tn.index[i].first;
                            tn.index[i].first = key;
                            tn.index[i].second = child;
                            ++tn.size;
                            write(&tn, tn.self);
                            if (i == tn.size - 1)
                                upDateIndex(tn.parent, sib.index[0].first, key);
                            else upDateIndex(tn.parent, sib.index[0].first, tn.index[tn.size - 1].first);

                        }
#ifdef  DEBUG
                        debugFind(key);
#endif
                    } else {
                        //邻居满了，这时需要预处理，分裂当前节点
#ifdef  DEBUG
                        std::cout << key << " index**split new node" << std::endl;

#endif
                        TreeNode newNode;
                        createTreeNode(tn, newNode);
                        Key oldKey = tn.index[tn.size - 1].first;
                        int i = M / 2, j = 0;
                        for (; i < M; ++i, ++j) {
                            newNode.index[j] = tn.index[i];
                            ++newNode.size;
                            --tn.size;
                            upDateParent(newNode.self, newNode.index[j].second, mode);
                        }

                        write(&tn, tn.self);
                        write(&newNode, newNode.self);
                        upDateIndex(tn.parent, oldKey, tn.index[tn.size - 1].first);
                        insertIndex(tn.parent, oldKey, newNode.self, false);

                        if (comp(key, tn.index[tn.size - 1].first))
                            insertIndex(tn.self, key, child, mode);
                        else
                            insertIndex(newNode.self, key, child, mode);
#ifdef DEBUG
                        debugFind(key);
#endif
                    }
                }
                else{
                    //后面没有邻居,同样需要分裂
#ifdef  DEBUG
                    std::cout<<key<<" index**a new sib"<<std::endl;
#endif
                    TreeNode newNode;
                    createTreeNode(tn, newNode);
                    Key oldKey=tn.index[tn.size-1].first;
                    int i=M/2,j=0;
                    for(;i<M;++i,++j){
                        newNode.index[j]=tn.index[i];
                        ++newNode.size;
                        --tn.size;
                        upDateParent(newNode.self,newNode.index[j].second,mode);
                    }

                    if(tn.parent!=0){
                        //操作非根节点
                        write(&tn,tn.self);
                        write(&newNode,newNode.self);
                        upDateIndex(tn.parent,oldKey,tn.index[tn.size-1].first);
                        insertIndex(tn.parent,oldKey,newNode.self,false);
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
#ifdef DEBUG
                    debugFind(key);
#endif
                }
            }
            else{
                //后面没有邻居,同样需要分裂
#ifdef  DEBUG
                std::cout<<key<<" index**a new sib"<<std::endl;
#endif
                TreeNode newNode;
                createTreeNode(tn, newNode);
                Key oldKey=tn.index[tn.size-1].first;
                int i=M/2,j=0;
                for(;i<M;++i,++j){
                    newNode.index[j]=tn.index[i];
                    ++newNode.size;
                    --tn.size;
                    upDateParent(newNode.self,newNode.index[j].second,mode);
                }

                if(tn.parent!=0){
                    //操作非根节点
                     write(&tn,tn.self);
                     write(&newNode,newNode.self);
                     upDateIndex(tn.parent,oldKey,tn.index[tn.size-1].first);
                     insertIndex(tn.parent,oldKey,newNode.self,false);
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
#ifdef DEBUG
                debugFind(key);
#endif
            }

        }
        void upDateParent(int newParent,int offset,bool mode){
            //将孩子节点的父亲节点更新
            if(mode){
                LeafNode ln;
                read(&ln,offset);
                ln.parent=newParent;
                write(&ln,offset);
            }
            else{
                TreeNode tn;
                read(&tn,offset);
                tn.parent=newParent;
                write(&tn,offset);
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
            int leafPos=Find(key);
            LeafNode ln;
            read(&ln,leafPos);
            for(int i=0;i<ln.size;++i){
                if(!comp(key,ln.record[i].first)&&!comp(ln.record[i].first,key))
                    return ln.record[i].second;
            }
        }



        bool empty() const { return core._size==0;}

        size_t size() const { return core._size;}

        void print(){
            std::cout<<"height:"<<core.height<<std::endl;
            TreeNode tn;
            read(&tn,core.root);
            std::cout<<"root size:"<<tn.size<<std::endl;
            for(int i=0;i<tn.size;++i){
                std::cout<<tn.index[i].first<<' ';
                LeafNode ln;
                read(&ln,tn.index[i].second);
                std::cout<<ln.size<<'\t'<<ln.record[ln.size-1].first;
                std::cout<<std::endl;
            }
        }

        int debugFind(const Key& key){
            //用于调试
            std::cout<<"key:"<<key<<std::endl;
            std::cout<<"height:"<<core.height<<std::endl;
            TreeNode tn;
            read(&tn,core.root);

            int next;
            if(core._size==0){
                std::cout<<tn.index[0].second;
                return tn.index[0].second;
            }

            for(int i=0;i<core.height-2;++i){
                std::cout<<"TreeNode size:"<<tn.size<<std::endl;
                for(int k=0;k<tn.size;++k)
                    std::cout<<tn.index[k].first<<' ';
                std::cout<<std::endl;
                int j=0;
                while(j<tn.size&&comp(tn.index[j].first,key)) {
                    ++j;
                }

                if(j==tn.size) --j;
                std::cout<<"j="<<j<<std::endl;
                next=tn.index[j].second;
                read(&tn,next);
            }
            if(core.height==2){
                std::cout<<"TreeNode size:"<<tn.size<<std::endl;
                for(int k=0;k<tn.size;++k)
                    std::cout<<tn.index[k].first<<' ';
                std::cout<<std::endl;
            }

            int j=0;
            while(j<tn.size&&comp(tn.index[j].first,key)) {
                ++j;
            }
            if(j==tn.size) --j;
            std::cout<<"j="<<j<<std::endl;
            next=tn.index[j].second;

            for(int i=0;i<tn.size;++i){
                LeafNode ln;
                read(&ln,tn.index[i].second);
                std::cout<<"LeafNode"<<i<<" size: "<<ln.size<<std::endl;
                for(int k=0;k<ln.size;++k)
                    std::cout<<ln.record[k].first<<' ';
                std::cout<<std::endl;
            }
            return next;
        }

    };
}  // namespace sjtu

