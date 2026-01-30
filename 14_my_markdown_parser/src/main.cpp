#include <iostream>
#include <fstream>
#include <sstream>
#include "Markdown.hpp"

using namespace std;

int main(int argc, char** argv)
{
    ifstream ifs("D:/desktop/MyMarkdownParser/test.md");
    if(!ifs.is_open()){
        cerr << "Failed to open file." << endl;
        return -1;
    }
    stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
    string content = ss.str();

    cout << content << endl;

    markdown::Lexer lexer(content);
    lexer.tokenize();
    auto tokens = lexer.getTokens();
    if (tokens.empty()) {
        std::cerr << "No tokens were produced by the lexer." << std::endl;
        return -1;
    }

    // Print tokens
    for (const auto& token : tokens) {
        cout << token->toString() << endl;
    }

    // markdown::Parser parser(tokens);
    // parser.parse();

    // markdown::HtmlCodeGenerator  htmlGenerator;
    // auto html = htmlGenerator.generate(parser.getAst());
    // if (html.empty()) {
    //     cerr << "HTML generation failed." << std::endl;
    //     return -1; 
    // }

    // ofstream ofs("D:/desktop/MyMarkdownParser/test.html");
    // if (!ofs.is_open()) {
    //     cerr << "Failed to open output file for writing." << std::endl;
    //     return -1;
    // }
    // ofs << html;
    // ofs.close();

    return 0;
}
