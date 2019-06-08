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
    static const char WRITE_PATH[256] = "records.txt";
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
        //mutable int fileLevel = 0;

        struct CoreData{
            int root = 0, slot = 0, pos = UNIT;
            size_t _size = 0;
            int height = 0;
        };

        CoreData core;
        Compare comp;
        bool fp_open;


//        void openFile(const char *mode = "rb+") const{
//            if (fileLevel == 0)
//                fp = std::fopen(path, mode);
//            ++fileLevel;
//            //std::cout<<"open success\n";
//        }

//        void closeFile() const{
//            if (fileLevel == 1)
//                std::fclose(fp);
//            --fileLevel;
//            //std::cout<<"close success\n";
//        }
//
//        void write(void *place, int offset){
//            openFile();
//            //std::rewind(fp);
//            std::fseek(fp,offset,SEEK_SET);
//            std::fwrite(place,UNIT,1,fp);
//            std::fflush(fp);
//
//            closeFile();
//        }
//
//        void read(void* place, int offset) {
//            openFile();
//            std::fseek(fp, offset, SEEK_SET);
//            std::fread(place,UNIT,1, fp);
//            closeFile();
//        }

        bool file_already_exists;

        inline void openFile() {
            file_already_exists = 1;
            if (fp_open == 0) {
                fp = fopen(path, "rb+");
                if (fp == nullptr) {
                    file_already_exists = 0;
                    fp = std::fopen(path, "w");
                    std::fclose(fp);
                    fp = std::fopen(path, "rb+");
                } else read(&core, core.pos,1, sizeof(CoreData));
                fp_open = 1;
            }
        }

        inline void closeFile() {
            if (fp_open == 1) {
                std::fclose(fp);
                fp_open = 0;
            }
        }

        inline void read(void *place, int offset,int count,int Size) const {
            std::fseek(fp, offset, SEEK_SET);
            std::fread(place, Size, count, fp);
        }

        inline void write(void *place, int offset,int count,int Size) const {
            std::fseek(fp, offset, SEEK_SET);
            std::fwrite(place, Size, count, fp);
        }

//        size_t fread(void *ptr, size_t size_of_elements,
//                     size_t number_of_elements, FILE *a_file);
//
//        size_t fwrite(const void *ptr, size_t size_of_elements,
//                      size_t number_of_elements, FILE *a_file);


        int alloc(){
            int s = core.slot;
            core.slot += UNIT;
            write(&core, core.pos,1, sizeof(CoreData));
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

        void clear() {
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
            write(&rt, core.root,1, sizeof(TreeNode));
            write(&ln, rt.index[0].second,1, sizeof(LeafNode));
            core.height = 2;
            write(&core, core.pos, 1,sizeof(CoreData));
            closeFile();
        }
        iterator find(const Key& key) {}
        const_iterator find(const Key& key) const {}

        // Default Constructor and Copy Constructor
        BTree(const char *PATH = WRITE_PATH) {
            std::strcpy(path, PATH);
//            if (access(path, 0) == -1){
//                FILE *tmpfp = std::fopen(path, "wb+");
//                fclose(tmpfp);
//            }
//            openFile();
//            TreeNode rt;
//            alloc();
//            alloc();
//            core.root = alloc();
//            LeafNode ln;
//            ln.parent = core.root;
//            rt.index[0].second = alloc();
//            ln.self=rt.index[0].second;
//            rt.self=core.root;
//            write(&rt, core.root);
//            write(&ln, rt.index[0].second);
//            core.height = 2;
//            write(&core, core.pos);
            fp = nullptr;
            openFile();
            if (file_already_exists == 0) build_tree();
        }

        inline void build_tree() {
            TreeNode rt;
            alloc();
            alloc();
            core.root = alloc();
            LeafNode ln;
            ln.parent = core.root;
            rt.index[0].second = alloc();
            ln.self=rt.index[0].second;
            rt.self=core.root;
            write(&rt, core.root,1, sizeof(TreeNode));
            write(&ln, rt.index[0].second,1, sizeof(LeafNode));
            core.height = 2;
            write(&core, core.pos,1, sizeof(CoreData));

        }



        ~BTree() {
            //openFile("wb+");
            //closeFile();
//            core.root = 0;
//            core.slot = 0;
//            core.pos = UNIT;
//            core._size = 0;
//            core.height = 0;
//            //openFile();
//            TreeNode rt;
//            alloc();
//            alloc();
//            core.root = alloc();
//            LeafNode ln;
//            ln.parent = core.root;
//            rt.index[0].second = alloc();
//            write(&rt, core.root);
//            write(&ln, rt.index[0].second);
//            core.height = 2;
//            write(&core, core.pos);
            closeFile();
        }


        // Insert: Insert certain Key-Value into the database
        // Return a pair, the first of the pair is the iterator point to the new
        // element, the second of the pair is Success if it is successfully inserted

        int Find(const Key& key){
            //找到插入key的叶结点的位置，如果插入的是最大或最小的key应当归并在中间
            //std::cout<<"Find Key:\n";
            TreeNode tn;
            read(&tn,core.root,1, sizeof(TreeNode));
            int next,parent=core.root;
            if(core._size==0)
                return tn.index[0].second;

            for(int i=0;i<core.height-2;++i){
                int j=0;
                while(j<tn.size&&comp(tn.index[j].first,key)) ++j;
                if(j==tn.size) --j;
                next=tn.index[j].second;
                read(&tn,next,1, sizeof(TreeNode));
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
            read(&tn,offset,1, sizeof(TreeNode));
            int i;
            for(i=0;i<tn.size;++i){
                if(!comp(tn.index[i].first,oldKey)&&!comp(oldKey,tn.index[i].first))break;
            }
            tn.index[i].first=newKey;
            write(&tn,offset,1, sizeof(TreeNode));
            if(i==tn.size-1)
                upDateIndex(tn.parent,oldKey,newKey);
        }

        void insert(const Key& key, const Value& value){
            int curPos=Find(key);

            //std::cout<<curPos<<std::endl;

            LeafNode ln;
            read(&ln,curPos,1, sizeof(LeafNode));
            if(ln.size!=L){//此叶结点未满
                if(ln.size==0){
                    ln.record[0].first=key;
                    ln.record[0].second=value;
                    ++ln.size;
                    ++core._size;
                    write(&core,core.pos,1, sizeof(CoreData));
                    TreeNode tn;
                    read(&tn,core.root,1, sizeof(TreeNode));
                    tn.index[0].first=key;
                    tn.size++;
                    write(&tn,core.root,1, sizeof(TreeNode));
                    write(&ln,curPos,1, sizeof(LeafNode));
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
                    write(&core,core.pos,1, sizeof(CoreData));
                    if(i==ln.size-1)
                        upDateIndex(ln.parent,ln.record[i-1].first,key);
                    write(&ln,curPos,1, sizeof(LeafNode));
                }

            }
            else if(ln.succ!=0){//后面有一个兄弟
                LeafNode brother;
                read(&brother,ln.succ,1, sizeof(LeafNode));
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
                            for (i = ln.size -1 ; i > 0; --i) {
                                if (comp(ln.record[i - 1].first, key))break;
                                ln.record[i] = ln.record[i - 1];
                            }
                            ln.record[i].first = key;
                            ln.record[i].second = value;
                            upDateIndex(ln.parent, brother.record[0].first, ln.record[ln.size - 1].first);
                        }
                        ++core._size;
                        write(&core, core.pos,1, sizeof(CoreData));
                        write(&ln, curPos,1, sizeof(LeafNode));
                        write(&brother, ln.succ,1, sizeof(LeafNode));
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

                        write(&ln, ln.self,1, sizeof(LeafNode));
                        write(&newNode, newNode.self,1, sizeof(LeafNode));
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

                    write(&ln,ln.self,1, sizeof(LeafNode));
                    write(&newNode,newNode.self, 1,sizeof(LeafNode));
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

                write(&ln,ln.self,1, sizeof(LeafNode));
                write(&newNode,newNode.self,1, sizeof(LeafNode));
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
            read(&tn,offset,1, sizeof(TreeNode));
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

                if(i==tn.size)
                    upDateIndex(tn.parent,oldKey,key);
                ++tn.size;
                write(&tn,offset,1,sizeof(TreeNode));
            }
            else if(tn.succ!=0){//后面有一个邻居
                TreeNode sib;
                read(&sib,tn.succ,1, sizeof(TreeNode));
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
                            write(&sib, sib.self,1, sizeof(TreeNode));
                            upDateParent(sib.self, child, mode);
                        } else {
                            //从节点末尾移动一个孩子给邻居
                            for (int i = sib.size; i > 0; --i)
                                sib.index[i] = sib.index[i - 1];
                            sib.index[0] = tn.index[tn.size - 1];
                            upDateParent(sib.self, sib.index[0].second, mode);
                            ++sib.size;
                            --tn.size;
                            write(&tn, tn.self,1, sizeof(TreeNode));
                            write(&sib, sib.self,1, sizeof(TreeNode));
                            int i;
                            for (i = tn.size ; i > 0; --i) {
                                if (comp(tn.index[i - 1].first, key))break;
                                tn.index[i] = tn.index[i - 1];
                            }
                            //Key oldKey = tn.index[i].first;
                            tn.index[i].first = key;
                            tn.index[i].second = child;
                            upDateParent(tn.self,child,mode);
                            ++tn.size;
                            write(&tn, tn.self,1, sizeof(TreeNode));
                            upDateIndex(tn.parent, sib.index[0].first, tn.index[tn.size - 1].first);
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

                        write(&tn, tn.self,1, sizeof(TreeNode));
                        write(&newNode, newNode.self,1, sizeof(TreeNode));
                        upDateIndex(tn.parent, oldKey, tn.index[tn.size - 1].first);
                        insertIndex(tn.parent, oldKey, newNode.self, false);

                        if (comp(key, tn.index[tn.size - 1].first)){
                            insertIndex(tn.self, key, child, mode);
                            upDateParent(tn.self,child,mode);
                        }

                        else{
                            insertIndex(newNode.self, key, child, mode);
                            upDateParent(newNode.self,child,mode);
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
                        write(&tn,tn.self,1, sizeof(TreeNode));
                        write(&newNode,newNode.self,1, sizeof(TreeNode));
                        upDateIndex(tn.parent,oldKey,tn.index[tn.size-1].first);
                        insertIndex(tn.parent,oldKey,newNode.self,false);
                        if(comp(key,tn.index[tn.size-1].first)){
                            insertIndex(tn.self,key,child,mode);
                            upDateParent(tn.self,child,mode);
                        }

                        else{
                            insertIndex(newNode.self,key,child,mode);
                            upDateParent(newNode.self,child,mode);
                        }

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
                        write(&newRoot,rootPos,1, sizeof(TreeNode));
                        write(&tn,tn.self,1, sizeof(TreeNode));
                        write(&newNode,newNode.self,1, sizeof(TreeNode));
                        write(&core,core.pos,1, sizeof(CoreData));
                        if(comp(key,tn.index[tn.size-1].first)){
                            insertIndex(tn.self,key,child,mode);
                            upDateParent(tn.self,child,mode);
                        }

                        else{
                            insertIndex(newNode.self,key,child,mode);
                            upDateParent(newNode.self,child,mode);
                        }

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
                     write(&tn,tn.self,1, sizeof(TreeNode));
                     write(&newNode,newNode.self,1, sizeof(TreeNode));
                     upDateIndex(tn.parent,oldKey,tn.index[tn.size-1].first);
                     insertIndex(tn.parent,oldKey,newNode.self,false);
                     if(comp(key,tn.index[tn.size-1].first)){
                         insertIndex(tn.self,key,child,mode);
                         upDateParent(tn.self,child,mode);
                     }

                     else{
                         insertIndex(newNode.self,key,child,mode);
                         upDateParent(newNode.self,child,mode);
                     }

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
                    write(&newRoot,rootPos,1, sizeof(TreeNode));
                    write(&tn,tn.self,1, sizeof(TreeNode));
                    write(&newNode,newNode.self,1, sizeof(TreeNode));
                    write(&core,core.pos,1, sizeof(CoreData));
                    if(comp(key,tn.index[tn.size-1].first)){
                        insertIndex(tn.self,key,child,mode);
                        upDateParent(tn.self,child,mode);
                    }

                    else{
                        insertIndex(newNode.self,key,child,mode);
                        upDateParent(newNode.self,child,mode);
                    }

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
                read(&ln,offset,1, sizeof(LeafNode));
                ln.parent=newParent;
                write(&ln,offset,1, sizeof(LeafNode));
            }
            else{
                TreeNode tn;
                read(&tn,offset,1, sizeof(TreeNode));
                tn.parent=newParent;
                write(&tn,offset,1, sizeof(TreeNode));
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
            read(&ln,leafPos,1, sizeof(LeafNode));
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
            read(&tn,core.root,1, sizeof(TreeNode));

            std::cout<<"root size:"<<tn.size<<std::endl;
            std::cout<<core.root<<std::endl;
            std::cout<<core._size<<std::endl;
            std::cout<<core.slot<<std::endl;
            std::cout<<core.pos<<std::endl;
            std::cout<<"***********************"<<std::endl;
        }

        int debugFind(const Key& key){
            //用于调试
            std::cout<<"height:"<<core.height<<std::endl;
            TreeNode tn;
            read(&tn,core.root,1, sizeof(TreeNode));

            int next;

            for(int i=0;i<core.height-2;++i){
                std::cout<<"TreeNode size:"<<tn.size<<std::endl;
                for(int k=0;k<tn.size;++k)
                    std::cout<<tn.index[k].first<<' ';
                std::cout<<std::endl;
                int j=0;
                while(j<tn.size&&comp(tn.index[j].first,key)) ++j;
                if(j==tn.size) --j;
                next=tn.index[j].second;
                read(&tn,next,1, sizeof(TreeNode));
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
                read(&ln,tn.index[i].second,1, sizeof(LeafNode));
                std::cout<<"LeafNode"<<i<<" size: "<<ln.size<<std::endl;
                for(int k=0;k<ln.size;++k)
                    std::cout<<ln.record[k].first<<' ';
                //std::cout<<ln.record[0].first<<' '<<ln.record[ln.size-1].first;
                std::cout<<std::endl;
            }
            return next;
        }

    };
}  // namespace sjtu

