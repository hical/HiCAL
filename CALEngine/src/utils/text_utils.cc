#include "text_utils.h"
#include "porter.c"

using namespace std;
vector<string> text_utils::tokenize(const string &text, bool skip_numeric_terms = false){
    vector<string> words;
    int st = 0;
    while(st < (int)text.length()){
        int end = 0;
        bool skip = false;
        while(isalnum(text[st+end])){
            end++;
            if(skip_numeric_terms && isdigit(text[st+end]))
                skip = true;
        }
        if(end > 0 && !skip){
            words.push_back(text.substr(st, end));
        }
        st += end + 1;
    }
    return words;
}

vector<string> text_utils::get_stemmed_words(const string &str){
    char temp_str[str.length()+1];
    strcpy(temp_str, str.c_str());
    int st = 0, end = 0;
    string stemmed_text;

    while(temp_str[st]){
        if(!isalpha(temp_str[st])){
            stemmed_text.push_back(temp_str[st++]);
        }else{
            end = 0;
            while(isalpha(temp_str[st+end])){
                temp_str[st+end] = tolower(temp_str[st+end]);
                end++;
            }
            end--;
            int new_end = stem(temp_str, st, st + end);
            int final_st = st + end + 1;
            while(st <= new_end){
                stemmed_text.push_back(temp_str[st++]);
            }
            st = final_st;
        }
    }
    return tokenize(stemmed_text);
}

