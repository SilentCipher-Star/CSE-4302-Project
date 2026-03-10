#ifndef SEARCH_TRIE_HPP
#define SEARCH_TRIE_HPP

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <memory>

struct TrieNode {
    std::map<char, std::shared_ptr<TrieNode>> children;
    bool isEndOfWord = false;
    std::string category; // e.g., "Student", "Course", "Notice"
};

class GlobalSearchTrie {
public:
    void insert(std::string word, std::string category) {
        auto current = root;
        for (char ch : word) {
            ch = std::tolower(ch);
            if (current->children.find(ch) == current->children.end()) {
                current->children[ch] = std::make_shared<TrieNode>();
            }
            current = current->children[ch];
        }
        current->isEndOfWord = true;
        current->category = category;
    }

    std::vector<std::pair<std::string, std::string>> search(std::string prefix) {
        std::vector<std::pair<std::string, std::string>> results;
        auto current = root;
        for (char ch : prefix) {
            ch = std::tolower(ch);
            if (current->children.find(ch) == current->children.end()) return results;
            current = current->children[ch];
        }
        findAllWords(current, prefix, results);
        return results;
    }

private:
    std::shared_ptr<TrieNode> root = std::make_shared<TrieNode>();

    void findAllWords(std::shared_ptr<TrieNode> node, std::string currentWord, 
                      std::vector<std::pair<std::string, std::string>>& results) {
        if (node->isEndOfWord) {
            results.push_back({currentWord, node->category});
        }
        for (auto const& [ch, child] : node->children) {
            findAllWords(child, currentWord + ch, results);
        }
    }
};

#endif