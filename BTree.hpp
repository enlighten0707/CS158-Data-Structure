#include "utility.hpp"
#include <functional>
#include <cstddef>
#include "exception.hpp"
#include <cstdio>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <map>

namespace sjtu {
    static const char WRITE_PATH[256] = "C:\\Users\\86150\\Desktop\\records.txt";
    static const int UNIT = 4096;

    template <class Key, class Value, class Compare = std::less<Key> >
    class BTree {
    private:

        struct Index{
            Key key;
            long child;
        };
        struct Record{
            Key key;
            Value value;
        };

        static const size_t M = (4096 - 3 * sizeof(long) - sizeof(size_t)) / (sizeof(Index));
        static const size_t L = (4096 - 3 * sizeof(long) - sizeof(size_t)) / (sizeof(Record));

        struct TreeNode{
            long parent=0;
            long prev=0;
            long succ=0;
            size_t size=0;
            Index index[M];
        };

        struct LeafNode{
            long parent=0;
            long prev=0;
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
                fp = fopen(path, mode);
            ++fileLevel;
        }

        void closeFile() const{
            if (fileLevel == 1)
                fclose(fp);
            --fileLevel;
        }
        template <class T>
        size_t read(T *val, off_t offset){
#ifndef _NO_DEBUG
            openFile();
#endif
            fseek(fp, offset, SEEK_SET);
            char ch[UNIT];
            size_t s = fread(ch, UNIT, 1, fp);
            *val = *(reinterpret_cast<T *>(ch));
#ifndef _NO_DEBUG
            closeFile();
#endif
            return s;
        }

        template <class T>
        size_t write(T *val, off_t offset, const char *mode = "rb+"){
#ifndef _NO_DEBUG
            openFile(mode);
#endif
            fseek(fp, offset, SEEK_SET);
            char *ch;
            ch = reinterpret_cast<char *>(val);
            size_t s = fwrite(ch, UNIT, 1, fp);
#ifndef _NO_DEBUG
            closeFile();
#endif
            return s;
        }

        long alloc(){
            long s = core.slot;
            core.slot += UNIT;
            write(&core, core.pos);
            return s;
        }

        Index *begin(TreeNode &tn){
            return tn.index;
        }
        Record *begin(LeafNode &ln){
            return ln.record;
        }
        Index *end(TreeNode &tn){
            return tn.index + tn.size;
        }
        Record *end(LeafNode &ln){
            return ln.record + ln.size;
        }

        Index* binaryIndexSearch(TreeNode& tn,const Key& key){
            //找到的 Key 刚好大于目标Key
            if(tn.size==0) return NULL;
            int l=0,r=tn.size-1;
            while(l<r){
                int mid=(l+r)>>1;
                if(comp(tn.index[mid].key,key))
                    l=mid+1;
                else r=mid;
            }
            return tn.index+l;
        }

        long findKey(const Key& key,bool mode = false) {
            //找到Key所在叶子节点的位置
            //mode=false： 找到一个一定存在的key
            //mode=true:key 不一定存在，找到插入位置
            TreeNode tn;
            read(&tn,core.root);
            long parent=core.root,next;
            if (tn.size == 0)
                return tn.index[0].child;
            Index *id;
            for (int i = 1; i <= core.height - 2; ++i){
                //height包含叶结点的一层高度
                id = binaryIndexSearch(tn, key);
                next = id->child;
                read(&tn, id->child);
//                if (parent > 0 && tn.parent != parent) {
//                    tn.parent = parent;
//                    write(&tn, next);
//                }
                parent = next;
            }
            id = binaryIndexSearch(tn, key);
            next = id->child;
            LeafNode ln;
            read(&ln, id->child);
//            if (parent > 0 && ln.parent != parent) {
//                ln.parent = parent;
//                write(&ln, next);
//            }
            if (!comp(key, id->key) && !comp(id->key, key))
                return id->child;
            else if (mode)
                return id->child;
            else
                return 0;
        }

        void createTreeNode(long offset, TreeNode *tn, TreeNode *nx){
            nx->parent = tn->parent;
            nx->succ = tn->succ;
            nx->prev = offset;
            tn->succ = alloc();
            if (nx->succ != 0){
                TreeNode on;
                read(&on, nx->succ);
                on.prev = tn->succ;
                write(&on, nx->succ);
            }
        }
        void createLeafNode(long offset, LeafNode *ln, LeafNode *nx){
            nx->parent = ln->parent;
            nx->succ = ln->succ;
            nx->prev = offset;
            ln->succ = alloc();
            if (nx->succ != 0){
                //nx不是index的最后一个节点，更新后继节点的pre为新节点
                LeafNode on;
                read(&on, nx->succ);
                on.prev = ln->succ;
                write(&on, nx->succ);
            }//暂时不将新节点写入，而是把位置暂存在前一个结点的succ中
        }

        void updateChildIndex(long parent, const Key &oldKey, const Key &newKey){
            //改变当前操作节点的父亲，关于这个节点孩子的信息，即改变Key
            if (parent == 0)
                return;
            TreeNode tn;
            read(&tn, parent);
            if (tn.size == 0) {
                tn.index[0].key = newKey;
                ++tn.size;
                write(&tn, parent);
                updateChildIndex(tn.parent, oldKey, newKey);
            }
            else {
                Index *id = binaryIndexSearch(tn, oldKey);
                if (id == end(tn))//没找到，Key超出范围了。有必要？
                    return;
                id->key = newKey;
                write(&tn, parent);
                if (id == end(tn) - 1)//改变了该节点最后一个，继续向上更新
                    updateChildIndex(tn.parent, oldKey, newKey);
            }
        }

        void copyIndex(Index *first, Index *last, Index *des){
            while (first != last){
                *des = *first;
                ++first;
                ++des;
            }
        }

        void copyRecord(Record *first, Record *last, Record *des){
            while (first != last){
                *des = *first;
                ++first;
                ++des;
            }
        }
        void copyBackIndex(Index *first, Index *last, Index *des){
            while (last != first){
                *(des - 1) = *(last - 1);
                --last;
                --des;
            }
        }
        void copyBackRecord(Record *first, Record *last, Record *des){
            while (last != first){
                *(des - 1) = *(last - 1);
                --last;
                --des;
            }
        }


        void insertNewIndex(long offset, const Key &key, long child, bool mode = false){
            //mode=true:被更新的是某个叶子节点的父亲
            //mode=false:被更新的是非叶节点的父亲
            //offset:parent pos , child:newNode pos , key: accordingly
            //similar to insert
            TreeNode tn;
            read(&tn, offset);
            if (tn.size != M){//index is not full
                int i;
                for (i = tn.size; i > 0; --i){
                    if (!comp(key, tn.index[i - 1].key))
                        break;
                    tn.index[i] = tn.index[i - 1];
                }
                tn.index[i].key = key;
                tn.index[i].child = child;
//                if (i == tn.size){
//                    updateChildIndex(tn.parent, tn.index[i - 1].key, key);
//                }
                if (i == tn.size-1){
                    updateChildIndex(tn.parent, tn.index[i - 1].key, key);
                }
                ++tn.size;
                write(&tn, offset);
            }
            else if (tn.succ != 0){//后面有兄弟
                TreeNode sib;
                read(&sib, tn.succ);
                if (sib.size != M){//兄弟没满,给兄弟一个
                    copyBackIndex(begin(sib), end(sib), end(sib) + 1);
                    if (comp(tn.index[tn.size - 1].key, key)) {
                        //正好在父亲节点结尾加index，无需挪动前一个TreeNode
                        sib.index[0].key = key;
                        sib.index[0].child = child;
                        ++sib.size;
                        write(&sib, tn.succ);
                        if (mode)//为什么是更新最后一个节点而不是第一个节点？？
                            updateLeafParentPos(end(sib) - 1, end(sib), tn.succ);
                        else
                            updateNodeParentPos(end(sib) - 1, end(sib), tn.succ);
                        /*
                         * 或许应该是：
                         * if (mode)
                            updateLeafParentPos(begin(sib) ,begin(sib)+1, tn.succ);
                        else
                            updateNodeParentPos(begin(sib) ,begin(sib)+1, tn.succ);
                        */
                    }
                    else {//在父亲节点的中部加index,需要把末尾的那个index移动过去
                        //copyIndex(end(tn) - 1, end(tn), begin(sib));
                        sib.index[0]=tn.index[tn.size-1];
                        //index下面挂了孩子，孩子需要知道父亲改变了
                        if (!mode)
                            updateNodeParentPos(begin(sib), begin(sib) + 1, tn.succ);
                        else
                            updateLeafParentPos(begin(sib), begin(sib) + 1, tn.succ);
                        ++sib.size;
                        --tn.size;
                        write(&sib, tn.succ);

                        int i;
                        for (i = tn.size; i > 0; --i) {
                            if (!comp(key, tn.index[i - 1].key))
                                break;
                            tn.index[i] = tn.index[i - 1];
                        }
                        tn.index[i].key = key;
                        tn.index[i].child = child;
                        ++tn.size;
                        write(&tn, offset);
//                        if (i == tn.size - 1) {
//                            updateChildIndex(tn.parent, sib.index[0].key, key);
//                        }
//                        else {
                            updateChildIndex(tn.parent, sib.index[0].key, tn.index[tn.size - 1].key);
                        //}
                    }
                }
                else{//兄弟也满了，需要分裂节点，并继续向上更新
                    TreeNode newNode;
                    createTreeNode(sib.prev, &tn, &newNode);
                    int newPos = tn.succ;
                    read(&sib, newNode.succ);
                    Key old = tn.index[tn.size - 1].key;
                    copyIndex(end(tn) - M / 2, end(tn), begin(newNode));
                    tn.size -= M / 2;
                    newNode.size += M / 2;
                    write(&tn, offset);
                    write(&newNode, newPos);
                    write(&sib, newNode.succ);
                    if (!mode)
                        updateNodeParentPos(begin(newNode), end(newNode), newPos);
                    else
                        updateLeafParentPos(begin(newNode), end(newNode), newPos);
                    updateChildIndex(tn.parent, old, tn.index[tn.size - 1].key);
                    insertNewIndex(tn.parent, newNode.index[newNode.size - 1].key, sib.prev);//内部调用时，一定有mode=false
                    if (comp(key, tn.index[tn.size - 1].key))//前期调整工作完成，选择需要插入new index 的位置
                        insertNewIndex(newNode.prev, key, child);
                    else
                        insertNewIndex(tn.succ, key, child);
                }
            }
            else{//后面没兄弟
                TreeNode newNode;
                createTreeNode(offset, &tn, &newNode);
                int newPos = tn.succ;
                Key old = tn.index[tn.size - 1].key;
                copyIndex(end(tn) - M / 2, end(tn), begin(newNode));
                tn.size -= M / 2;
                newNode.size = M / 2;
                if (!mode)
                    updateNodeParentPos(begin(newNode), end(newNode), newPos);
                else
                    updateLeafParentPos(begin(newNode), end(newNode), newPos);
                if (tn.parent != 0){//非根节点
                    write(&tn, offset);
                    write(&newNode, newPos);
                    updateChildIndex(tn.parent, old, tn.index[tn.size - 1].key);
                    insertNewIndex(tn.parent, newNode.index[newNode.size - 1].key, tn.succ);
                    if (comp(key, tn.index[tn.size - 1].key))
                        insertNewIndex(newNode.prev, key, child);
                    else
                        insertNewIndex(tn.succ, key, child);
                }
                else{//操作的是根节点，根节点容量满了，分裂，树增高一层
                    TreeNode newRoot;
                    int rootPos = alloc();
                    newRoot.prev = newRoot.succ = newRoot.parent = 0;
                    newRoot.size = 2;
                    newRoot.index[0].key = tn.index[tn.size - 1].key;
                    newRoot.index[0].child = newNode.prev;
                    newRoot.index[1].key = newNode.index[newNode.size - 1].key;
                    newRoot.index[1].child = newPos;
                    write(&newRoot, rootPos);
                    core.root = rootPos;
                    ++core.height;
                    write(&core, core.pos);
                    tn.parent = newNode.parent = rootPos;
                    write(&tn, offset);
                    write(&newNode, newPos);
                    if (comp(key, tn.index[tn.size - 1].key))
                        insertNewIndex(newNode.prev, key, child);
                    else
                        insertNewIndex(tn.succ, key, child);
                }
            }
        }

        void updateNodeParentPos(Index *first, Index *last, long newParent){
            TreeNode tn;
            while (first != last){
                read(&tn, first->child);
                tn.parent = newParent;
                write(&tn, first->child);
                ++first;
            }
        }

        void updateLeafParentPos(Index *first, Index *last, long newParent) {
            LeafNode ln;
            while (first != last) {
                read(&ln, first->child);
                ln.parent = newParent;
                write(&ln, first->child);
                ++first;
            }
        }

        Record *binarySearchRecord(LeafNode &ln, const Key &key){
            if (ln.size == 0)
                return end(ln);
            size_t l = 0, r = ln.size - 1, mid;
            while (l < r){
                mid = (l + r) / 2;
                if (comp(ln.record[mid].key, key))
                    l = mid + 1;
                else
                    r = mid;
            }
            return ln.record + l;
        }


    public:
        typedef pair<const Key, Value> value_type;

        class const_iterator;
        class iterator {
        private:
            long recordPos;
            Record* pos;
        public:
            bool modify(const Value& value){

            }
            iterator(long rP=0,Record* p=NULL):recordPos(rP),pos(p) {}

            iterator(const iterator& other) {
                // TODO Default Constructor
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
            value_type& operator*() const {
                // Todo operator*, return the <K,V> of iterator
            }
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
            value_type* operator->() const noexcept {
                /**
                 * for the support of it->first.
                 * See
                 * <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/>
                 * for help.
                 */
            }
        };
        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            long recordPos;
            Record* pos;
        public:
            const_iterator(long rP=0,Record* p=NULL):recordPos(rP),pos(p) {}

            const_iterator(const const_iterator& other) {
                // TODO
            }
            const_iterator(const iterator& other) {
                // TODO
            }
            // And other methods in iterator, please fill by yourself.
        };

        // Default Constructor and Copy Constructor
        BTree(bool CLEAR = false, const char *PATH = WRITE_PATH) {
            strcpy(path, PATH);
            if (CLEAR) {
                openFile("w+");
                closeFile();
                openFile();
                TreeNode rt;
                alloc();
                alloc();
                core.root = alloc();
                LeafNode ln;
                ln.parent = core.root;
                rt.index[0].child = alloc();
                write(&rt, core.root);
                write(&ln, rt.index[0].child);
                core.height = 2;
                write(&core, core.pos);
            }
            else{
                if (access(path, 0) == -1){
                    FILE *tmpfp = fopen(path, "wb+");
                    fclose(tmpfp);
                }
                openFile();
                char ch = fgetc(fp);
                if (ch == EOF){
                    TreeNode rt;
                    alloc();
                    alloc();
                    core.root = alloc();
                    LeafNode ln;
                    ln.parent = core.root;
                    rt.index[0].child = alloc();
                    write(&rt, core.root);
                    write(&ln, rt.index[0].child);
                    core.height = 2;
                    write(&core, core.pos);
                }
                else
                    read(&core, UNIT);
            }
        }
        BTree(const BTree& other) {
            // Todo Copy
        }
        BTree& operator=(const BTree& other) {
            // Todo Assignment
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
            rt.index[0].child = alloc();
            write(&rt, core.root);
            write(&ln, rt.index[0].child);
            core.height = 2;
            write(&core, core.pos);
        }


        // Insert: Insert certain Key-Value into the database
        // Return a pair, the first of the pair is the iterator point to the new
        // element, the second of the pair is Success if it is successfully inserted
        pair<iterator, OperationResult> insert(const Key& key, const Value& value) {
            long childPos = findKey(key, true);//当前要操作的叶结点在文件中的储存位置
            LeafNode ln;
            read(&ln, childPos);
            if (ln.size != L){//当前叶结点未满
                int i;
                for (i = ln.size; i > 0; --i){
                    if (!comp(key, ln.record[i - 1].key))
                        break;
                    ln.record[i] = ln.record[i - 1];
                }
                ln.record[i].key = key;
                ln.record[i].value = value;
                //一般不会插入到最后一个节点，除非Key是目前来说最大的
//                if (i == ln.size){
//                    updateChildIndex(ln.parent, ln.record[i - 1].key, key);
//                }
                if (i == ln.size-1){
                    updateChildIndex(ln.parent, ln.record[i - 1].key, key);
                }
                ++ln.size;
                write(&ln, childPos);
                ++core._size;
                write(&core, core.pos);
            }
            else if (ln.succ != 0){//叶结点满了，后面有一个兄弟
                LeafNode sib;
                read(&sib, ln.succ);
                if (sib.size != L){//兄弟没有满，给兄弟一个
                    copyBackRecord(begin(sib), end(sib), end(sib) + 1);
                    //copyRecord(end(ln) - 1, end(ln), begin(sib));
                    sib.record[0]=ln.record[ln.size-1];
                    ++sib.size;
                    --ln.size;
                    write(&sib, ln.succ);

                    int i ;
                    for (i = ln.size; i > 0; --i) {
                        if (!comp(key, ln.record[i - 1].key))
                            break;
                        ln.record[i] = ln.record[i - 1];
                    }
                    ln.record[i].key = key;
                    ln.record[i].value = value;
                    ++ln.size;
                    write(&ln, childPos);
                    updateChildIndex(ln.parent, sib.record[0].key, ln.record[ln.size - 1].key);
                    /*if (i == ln.size - 1) {
                        updateChildIndex(ln.parent, sib.record[0].key, key);
                    }
                    else {
                        updateChildIndex(ln.parent, sib.record[0].key, ln.record[ln.size - 1].key);
                    }*/
                    ++core._size;
                    write(&core, core.pos);
                }
                else{//自己满了，兄弟也满了，需要分裂节点，同时更新父亲的索引
                    LeafNode newNode;
                    createLeafNode(sib.prev, &ln, &newNode);
                    int newPos = ln.succ;//记录新节点将要写入的位置
                    read(&sib, newNode.succ);
                    Key old = ln.record[ln.size - 1].key;//第一个叶结点的索引一定会变化
                    copyRecord(end(ln) - L / 2, end(ln), begin(newNode));
                    ln.size -= L / 2;
                    newNode.size += L / 2;
                    write(&ln, childPos);
                    write(&newNode, newPos);
                    write(&sib, newNode.succ);
                    updateChildIndex(ln.parent, old, ln.record[ln.size - 1].key);
                    insertNewIndex(ln.parent, newNode.record[newNode.size - 1].key, sib.prev, true);
                    insert(key, value);
                }
            }
            else{//后面没有兄弟
                LeafNode newNode;
                createLeafNode(childPos, &ln, &newNode);
                int newPos = ln.succ;
                Key old = ln.record[ln.size - 1].key;
                copyRecord(end(ln) - L / 2, end(ln), begin(newNode));
                ln.size -= L / 2;
                newNode.size = L / 2;
                write(&ln, childPos);
                write(&newNode, newPos);
                updateChildIndex(ln.parent, old, ln.record[ln.size - 1].key);
                insertNewIndex(ln.parent, newNode.record[newNode.size - 1].key, ln.succ, true);
                insert(key, value);
            }
//            iterator itr;
//            //itr.modify(key);
//            pair<iterator, OperationResult> ans(itr,Success);
//            return ans;

        }
        // Erase: Erase the Key-Value
        // Return Success if it is successfully erased
        // Return Fail if the key doesn't exist in the database
        OperationResult erase(const Key& key) {
            return Fail;
        }
        Value at(const Key& key){
            long pos = findKey(key, true);
            LeafNode ln;
            read(&ln, pos);
            Record *rc = binarySearchRecord(ln, key);
            return rc->value;
        }
        // Return a iterator to the beginning
        iterator begin() {}
        const_iterator cbegin() const {}
        // Return a iterator to the end(the next element after the last)
        iterator end() {}
        const_iterator cend() const {}
        // Check whether this BTree is empty
        bool empty() const { return core._size==0;}
        // Return the number of <K,V> pairs
        size_t size() const { return core._size;}
        // Clear the BTree
        void clear() {}
        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key& key) const {}
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is
         * returned.
         */
        iterator find(const Key& key) {
            iterator itr;
            long pos = findKey(key, true);
            itr.recordPos=pos;
            LeafNode ln;
            read(&ln, pos);
            Record *rc = binarySearchRecord(ln, key);
            itr.pos=rc;
            return itr;
        }
        const_iterator find(const Key& key) const {
            const_iterator itr;
            long pos = findKey(key, true);
            itr.recordPos=pos;
            LeafNode ln;
            read(&ln, pos);
            Record *rc = binarySearchRecord(ln, key);
            itr.pos=rc;
            return itr;
        }
    };
}  // namespace sjtu

