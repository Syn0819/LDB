#ifndef __SKIPLIST_H
#define __SKIPLIST_H

#include <iostream>
#include <mutex>
#include <cstdlib>
#include <math.h>
#include <cstring>
#include <fstream>
#include <istream>
#include <stdio.h>
#include <memory>
#define STORE_FILE "store/dumpFile"

namespace skiplist {
#define EXIT_SUCCESS 0
#define EXIT_FAILED 1

std::mutex mtx;
std::string delimiter = ":";

template<typename K, typename V>
class Node {
public:
    int node_level;
    // Linear array to hold pointers to next node of different level
    Node<K, V> **forward;

    Node() {}
    Node(K k, V v, int);
    ~Node();
    K get_key() const;
    V get_value() const;
    void set_value(V);

private:
    K key;
    V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
    std::cout << "Node constructor" << std::endl;
    this->key = k;
    this->value = v;
    this->node_level = level;

    this->forward = new Node<K, V>*[level+1];
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1));
};

template<typename K, typename V>
Node<K, V>::~Node() {
    delete []forward;
};

template<typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
};

template<typename K, typename V>
V Node<K, V>::get_value() const {
    return value;
};

template<typename K, typename V>
void Node<K, V>::set_value(V value) {
    this->value = value;
};

template<typename K, typename V>
class SkipList {
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    int insert_element(K, V);
    int size();
    Node<K, V>* create_node(K, V, int);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:
    int _max_level;             // The maximux number of level in this skiplist
    int _skip_list_level;       // Tht current level in this skiplist
    int _element_count;         // The number of element in this skiplist       
    std::shared_ptr<Node<K, V>> _header;    // The pointer to header node
    std::ofstream _file_writer; 
    std::ifstream _file_reader;
};

// constructor
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
    this->_max_level = max_level;
    this->_element_count = 0;
    this->_skip_list_level = 0;

    K k;
    V v;
    this->_header = std::shared_ptr<Node<K, V>>(new Node<K, V>(k, v, _max_level));
};

// destructor
template<typename K, typename V>
SkipList<K, V>::~SkipList() {
    // free resouces
    _header.reset();
    if(_file_writer.is_open()) {
        _file_writer.close();
    }
    if(_file_reader.is_open()) {
        _file_reader.close();
    }
};

template<typename K, typename V>
int SkipList<K, V>::get_random_level(){

    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    std::cout << "get random level: " << k << std::endl;
    return k;
};

template<typename K, typename V>
Node<K, V>* SkipList<K,V>::create_node(K key, V value, int level) {
    Node<K, V> *n = new Node<K,V>(key, value, level);
    return n;
}

template<typename K, typename V>
int SkipList<K,V>::insert_element(K key, V value) {
    // start insert element k,v
    std::cout << "start insert element" << std::endl;
    mtx.lock();
    //  
    Node<K, V> *currentNode = this->_header.get();
    // update is used to record the search way, and initialize it
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    // record node in the search way
    // from the current level of this skiplist to the level 0
    // if the current node have next node and the next code's key less than the given key
    //      the node go to the next node
    for(int i = _skip_list_level; i >= 0; i--) {
        while(currentNode->forward[i] != nullptr && currentNode->forward[i]->get_key() < key) {
            currentNode = currentNode->forward[i];
        }
        update[i] = currentNode;
    }
    std::cout << "record search way finish" << std::endl;
    // find the insert place
    currentNode = currentNode->forward[0];
    if(currentNode != nullptr && currentNode->get_key() == key) {
        // check the node is existed
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return EXIT_FAILED;
    }
    if(currentNode == nullptr || currentNode->get_key() != key) {
        std::cout << "start insert now" << std::endl;
        // check the node is not existed, and instert the node now
        // first, get the random level of this node
        // second, for each level from [0, random_level], set the forword array
        int random_level = get_random_level();

        // if the random level biger than the current skip list level, then need to update the update array
        if(random_level > _skip_list_level) {
            for(int i = _skip_list_level+1; i < random_level + 1; i++) {
                update[i] = _header.get();
            }
            _skip_list_level = random_level;
        }
        std::cout << "work for random level finish" << std::endl;
        // create the node
        Node<K, V>* newNode = create_node(key, value, random_level);
        std::cout << "create a new node" << std::endl;
        // insert the node
        for(int i = 0; i <= random_level; i++) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        std::cout << "Insert Node successfully, which key=" << key <<", value=" << value << std::endl;

        ++_element_count;
    }
    
    mtx.unlock();
    return EXIT_SUCCESS;
}

template<typename K, typename V>
bool SkipList<K, V>::search_element(K key) {
    // search the given key
    // search need mutex? In this storage, YES. Since read and write may concureent
    // if a thread read a KV pair while another thread write a new value to this pair
    // it can not be promise we read the latest value
    mtx.lock();
    Node<K, V> *currentNode = _header.get();

    for(int i = _skip_list_level; i >= 0; i--) {
        while(currentNode->forward[i] != nullptr && currentNode->forward[i]->get_key() < key) {
            currentNode = currentNode->forward[i];
        }
    }

    currentNode = currentNode->forward[0];

    if(currentNode == nullptr) {
        std::cout << "Error! the given greater than all KV pair in storage" << std::endl;
        mtx.unlock();
        return false;
    }

    if(currentNode && currentNode->get_key() == key) {
        std::cout << "Found key: " << key << ", the value is: " << currentNode->get_value() << std::endl;
    }

    mtx.unlock();
    return true;
}


template<typename K, typename V>
void SkipList<K, V>::delete_element(K key) {
    // delete the given key
    // search element before the given key
    std::cout << "start delete element" << std::endl;
    mtx.lock();

    Node<K, V> *currentNode = this->_header.get();
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

    for(int i = _skip_list_level; i >= 0; --i) {
        while(currentNode->forward[i] != nullptr && currentNode->forward[i]->get_key() < key) {
            currentNode = currentNode->forward[i];
        }
        update[i] = currentNode;
    }
    
    currentNode = currentNode->forward[0];
    if(currentNode != nullptr && currentNode->get_key() == key) {
        // find the key which need to delete
        for(int i = 0; i <= _skip_list_level; ++i) {
            if(update[i]->forward[i] != currentNode) {
                break;
            }
            update[i]->forward[i] = currentNode->forward[i];
        }

        while(_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            --_skip_list_level;            
        }
        
        std::cout << "Successfully deleted key: " << key << std::endl;
        --_element_count; 
    }
    mtx.unlock();
    return;
}

}

#endif
