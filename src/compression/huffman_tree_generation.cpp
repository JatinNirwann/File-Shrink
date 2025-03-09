
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>

class Node{
    public:
        char character;
        int frequency;
        Node* left;
        Node* right;

        Node(char ch ='\0', int freq =0):
            character(ch),
            frequency(freq),
            left(nullptr),
            right(nullptr)
        {}
};


class HuffmanEncoder{
    private:
    std::vector<Node*> nodes;

    void calculateFrequencies(const std::string& word){
        std::unordered_map<char,int>frequencies;

        
    }
}