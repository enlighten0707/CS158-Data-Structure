//
// Created by 金乐盛 on 2018/5/14.
//

#ifndef B_TREE_BNODE_H
#define B_TREE_BNODE_H
#include <iostream>
#include <fstream>
#include "bppair.h"
#define PAIR_IN_LEAF_CONSUMER 25
#define KEY_IN_INTER_CONSUMER 248
#define LEAST_PAIR_IN_LEAF_CONSUMER (PAIR_IN_LEAF_CONSUMER / 2 + 0.999)
#define LEAST_KEY_IN_INTER_CONSUMER (KEY_IN_INTER_CONSUMER / 2 + 0.999)
using namespace tool;
namespace sjtu{

    template <class Key, class Value, class Compare = std::less<Key>>
    struct internal_node{
        long offset;//the offset of this
        long father;
        int numOfkey;//the number of keys
        int type;//if it's 0, it's a normal node which its children are nodes; else its children are leaf_nodes
        pair<Key, long> index[KEY_IN_INTER_CONSUMER];//there are indexes point to the next level.
    };

    template <class Key, class Value, class Compare = std::less<Key>>
    struct leaf_node{//which contains some pair(key and value)
        long offset;//the offset of this
        long next;//the next leaf_node
        long father;//its father_node
        int numOfpair;//how much pair in the node
        pair<Key, Value> children[PAIR_IN_LEAF_CONSUMER];//how much pair    !!!there may be an error

        //long pre;//the pre leaf_node
    };


    struct bpt_meta{
        //int numOftree;
        long firstleaf;//where is the first leaf
        long root;//where is the root
        long solt;//where is the fp*
        int size;
    };






}
#endif //B_TREE_BNODE_H
